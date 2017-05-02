#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#define MAX_SUBJECT_BYTES 1024

/**
 * Retrieves the SUBJECT or ISSUER fields from an X509 certificate as string. The maximum allowable
 * size for the subject string is currently constrained at 1024 characters.
 *
 * @param[in] subj_or_issuer The subject or issuer as an X509_NAME structure pointer.
 * @param[out] buffer the buffer where the string will be stored.
 * @param[out] bufferLen the size of the buffer that was written to.
 * @return 0 if successful, -1 if an error occurred.
 */
int x509_subject_as_string(X509_NAME* subj_or_issuer, char *buffer, int* bufferLen) {
    BIO *bio = BIO_new(BIO_s_mem());
    X509_NAME_print(bio, subj_or_issuer,0);
    BUF_MEM *buf;
    BIO_get_mem_ptr(bio, &buf);
    if(buf->length > MAX_SUBJECT_BYTES) {
        printf("ERROR: insufficient buffer!\n");
        BIO_free(bio);
        return -1;
    }
    memcpy(buffer, buf->data, buf->length);
    buffer[buf->length] = '\0';
    *bufferLen = buf->length;
    BIO_free(bio);
    return 0;
}

/**
 * This program takes in one parameter - the X509 certificate file.
 *
 * It then proceeds to determine:
 *
 *  1) If the certificate is a Self-Signed.
 *
 *  2) The fingerprint of the certificate - determined by the hashing algorithm used in the
 *     certificate itself.
 *
 * The program will terminate with code zero if it completes successfully and -1 if errors prevent
 * it from running correctly.
 */
int main(int argc, char**argv) {
    if (argc < 2) {
        printf("ERROR: must pass in path to certificate file.\n");
        exit(-1);
    } else if (argc > 2) {
        printf("ERROR: too many arguments! Only require path to certificate file.\n");
        exit(-1);
    }

    const char *file = NULL, *sigAlgorithm = NULL, *digestName = NULL;
    BIO *certbio = NULL;
    BIO *outbio = NULL;
    X509 *cert = NULL;
    EVP_PKEY *evp_pubkey;
    const EVP_MD *digest = NULL;
    unsigned char fingerprint[EVP_MAX_MD_SIZE];
    unsigned char subject[MAX_SUBJECT_BYTES];
    unsigned char issuer[MAX_SUBJECT_BYTES];
    int ret, fingerprintSize, subjectSize, issuerSize;
    int sigNID;

    // =============================================================================================
    // Initialize OpenSSL
    // =============================================================================================

    // Load Error Strings
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    // Add all ciphers & digests to the table.
    OpenSSL_add_all_algorithms();

    certbio = BIO_new(BIO_s_file());
    outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

    // =============================================================================================
    // Validate certificate file path
    // =============================================================================================
    file = argv[1];
    if(access(file, F_OK) == -1){
        BIO_printf(outbio, "ERROR: certificate file: %s was not found!\n", file);
        goto cleanup;
    }
    BIO_printf(outbio, "Found certificate file: %s\n", file);
    // =============================================================================================
    // Load Certificate
    // =============================================================================================
    if( !BIO_read_filename(certbio, file)){
        BIO_printf(outbio, "ERROR: failed to load file into certbio\n");
        goto cleanup;
    }

    if ( !(cert = PEM_read_bio_X509(certbio, NULL, 0, NULL)) ) {
        BIO_printf(outbio, "ERROR: failed to load certificate into memory\n");
        goto cleanup;
    }
    // =============================================================================================
    // Determine if Certificate is self-signed
    // =============================================================================================
    if( x509_subject_as_string(X509_get_issuer_name(cert), issuer, &issuerSize) ){
        BIO_printf(outbio, "ERROR: failed to extract issuer from certificate\n");
        goto cleanup;
    }
    if( x509_subject_as_string(X509_get_subject_name(cert), subject, &subjectSize) ){
        BIO_printf(outbio, "ERROR: failed to extract subject from certificate\n");
        goto cleanup;
    }

    // Self-Signed certificates have "Issuer" and "Subject" fields that are identical. However,
    // subordinate certificates do not. The TLD (Top Level Domain) may be the same, but if other
    // parameters like the CN, ST, C, O, OU, emailAddress are different then the certificates are
    // not equivalent and hence not self-signed.
    BIO_printf(outbio, "Self-Signed: ");
    if( issuerSize == subjectSize && strcmp(issuer, subject) == 0){
        evp_pubkey = X509_get_pubkey(cert);
        if(X509_verify(cert, evp_pubkey)) {
            BIO_printf(outbio, "Yes\n");
        } else {
            BIO_printf(outbio, "No - invalid public key!\n");
        }
    } else {
        BIO_printf(outbio, "No - subject / issuer mistmatch!\n");
    }
    //printf("Issuer: %s\n", issuer);
    //printf("Subject: %s\n", subject);

    // =============================================================================================
    // Print Certificate's Fingerprint
    // =============================================================================================
    sigNID = OBJ_obj2nid(cert->sig_alg->algorithm);
    if(sigNID == NID_undef){
        BIO_printf(outbio, "unable to find signature algorithm name");
        goto cleanup;
    }
    sigAlgorithm = OBJ_nid2ln(sigNID);
    if(memcmp(sigAlgorithm, "sha256", 6) == 0) {
        digestName = "sha256";
    } else if(memcmp(sigAlgorithm, "sha1", 4) == 0) {
        digestName = "sha1";
    } else if(memcmp(sigAlgorithm, "md5", 3) == 0) {
        digestName = "md5";
    } else {
        BIO_printf(outbio, "unsupported sigAlg: %s\n", sigAlgorithm);
        goto cleanup;
    }

    digest = EVP_get_digestbyname(digestName);
    if(!X509_digest(cert, digest, fingerprint, &fingerprintSize)) {
        BIO_printf(outbio, "failed to compute fingerprint!\n");
        goto cleanup;
    }
    BIO_printf(outbio, "Fingerprint (%s): ", digestName);
    for(int i = 0; i < fingerprintSize; i++) {
        BIO_printf(outbio, "%02X%c", fingerprint[i], (i+1 == fingerprintSize) ? '\n' : ':');
    }

    // =============================================================================================
    // Clean-up OpenSSL
    // =============================================================================================
cleanup:
    if(cert) X509_free(cert);
    if(certbio) BIO_free_all(certbio);
    if(outbio) BIO_free_all(outbio);
    if(evp_pubkey) EVP_PKEY_free(evp_pubkey);
    ERR_free_strings();
    EVP_cleanup();

    return 0;
}
