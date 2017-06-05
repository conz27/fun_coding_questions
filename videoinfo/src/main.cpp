#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <cstdlib>
#include <cstdio>
#include <cmath>

using namespace std;

#define VERSION "0.1.0"

#define MAX_I(bits) ((1 << (bits)) - 1)
#define MAX_I_8_BITS MAX_I(8)

namespace {
const size_t ERROR_IN_COMMAND_LINE = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}
namespace po = boost::program_options;

typedef struct Context {
    std::string ref;             // reference video
    std::string test;            // test video
    uint height, width;          // video height x width
    // Sub-sampling parameters:
    uint8_t J, a, b;             // J = horizontal sampling reference
                                 // a = # of chrominance samples (Cr,Cb) in first row of J pixels
                                 // b = # of changes of chrominance samples (Cr,Cb) between first &
                                 //     second row of J pixels.
} Context;

typedef struct {
    uint8_t y;
    uint8_t u;
    uint8_t v;
} YCbCr;

#ifdef RGB888
/**
 * YCbCr to RGB888
 */
inline void yuv2rgb(uint8_t Y, uint8_t Cb, uint8_t Cr, uint8_t *r, uint8_t *g, uint8_t *b) {

    Cr = Cr - 128;
    Cb = Cb - 128;

    *r = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
    *g = Y - ((Cb >> 2) + (Cb >> 4) + (Cb >> 5)) - ((Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5));
    *b = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
}
#endif

inline double psnr(double mse){
    // ------------------------------------------------------------------------------------
    //    MAX_I = 2^B-1 where B = max possible pixel value; for 8-bit sampling MAX_I = 255
    //
    //    PSNR = 20*log10(MAX_I) - 10*log10(mse) where MAX_I = 255
    // ------------------------------------------------------------------------------------
    double psnr = 48.130803609 - 10*log10(mse);
    return (isinf(psnr)) ? 0 : psnr; // Return 0 if PSNR is infinite (i.e. 0dB)
}

void readYUV444(const Context& c) {
    auto frameSize = c.width * c.height;
    auto bytesPerFrame = 3 * frameSize; // Y*frameSize + U*frameSize + V*frameSize

    ifstream ref = ifstream(c.ref, ios::ate | ios::binary);
    if(!ref) {
        cerr << "ERROR: failed to open file: " << c.ref;
        exit(1);
    }

    ifstream tst = ifstream(c.test, ios::ate | ios::binary);
    if(!tst) {
        cerr << "ERROR: failed to open file: " << c.test;
        exit(1);
    }

    auto rSizeBytes = ref.tellg();
    auto tSizeBytes = tst.tellg();

    ref.seekg(0, ios::beg);
    tst.seekg(0, ios::beg);

    auto rFrames = rSizeBytes / bytesPerFrame;
    auto tFrames = tSizeBytes / bytesPerFrame;

#ifdef DEBUG
    cout << "bytesPerFrame = " << bytesPerFrame << endl;
    cout << "rSize = " << rSizeBytes << ", tSize = " << tSizeBytes << endl;
    cout << "rFrames = " << rFrames << ", tFrames = " << tFrames << endl;
#endif

    auto totalFrames = std::min(rFrames, tFrames);

    uint32_t sum_fs = 0;
    double seqScore = 0;

    auto start = chrono::system_clock::now();

    for (int f = 0; f < totalFrames; f++) {
        uint32_t frameOffset = f * bytesPerFrame; // f * w * h = byte offset for current frame
        double frameScore = 0;
#ifdef RGB888
        uint32_t mse_r = 0, mse_b = 0, mse_g = 0;
        double psnr_r = 0, psnr_b = 0, psnr_g = 0;
#else
        uint32_t mse_y = 0, mse_u = 0, mse_v = 0;
        double psnr_y = 0, psnr_u = 0, psnr_v = 0;
#endif

        for (int y = 0; y < c.height; ++y) {
            char rYBuf[c.width], rUBuf[c.width], rVBuf[c.width];
            char tYBuf[c.width], tUBuf[c.width], tVBuf[c.width];

            uint32_t offsetY = 0 * frameSize + y * c.width;
            uint32_t offsetU = 1 * frameSize + y * c.width;
            uint32_t offsetV = 2 * frameSize + y * c.width;

            //--------------------------------------------------------------------------------------
            // OPTIMIZATION:
            //
            // To improve cache locality, load rows of bytes from each of Y, U, V components. The
            // key is to access memory elements that are contiguous, so the fetch cost is only paid
            // when fetching the 1st of N contiguous elements.
            //--------------------------------------------------------------------------------------

            // Reference File
            ref.seekg(frameOffset + offsetY); // 1 row of Y
            ref.read(rYBuf, sizeof(rYBuf));
            ref.seekg(frameOffset + offsetU); // 1 row of U
            ref.read(rUBuf, sizeof(rUBuf));
            ref.seekg(frameOffset + offsetV); // 1 row of V
            ref.read(rVBuf, sizeof(rVBuf));

            // Test File
            tst.seekg(frameOffset + offsetY); // 1 row of Y
            tst.read(tYBuf, sizeof(tYBuf));
            tst.seekg(frameOffset + offsetU); // 1 row of U
            tst.read(tUBuf, sizeof(tUBuf));
            tst.seekg(frameOffset + offsetV); // 1 row of V
            tst.read(tVBuf, sizeof(tVBuf));

            for (int x = 0; x < c.width; ++x) {
                // 4:4:4
                YCbCr r, t;

                r.y = *(rYBuf + x);
                r.u = *(rUBuf + x);
                r.v = *(rVBuf + x);

                t.y = *(tYBuf + x);
                t.u = *(tUBuf + x);
                t.v = *(tVBuf + x);
#ifdef RGB888
                uint8_t rr, rg, rb;
                uint8_t tr, tg, tb;

                yuv2rgb(r.y, r.u, r.v, &rr, &rg, &rb);
                yuv2rgb(t.y, t.u, t.v, &tr, &tg, &tb);

                mse_r += (rr - tr) * (rr - tr);
                mse_b += (rg - tg) * (rg - tg);
                mse_g += (rb - tb) * (rb - tb);

#else
                mse_y += (r.y - t.y) * (r.y - t.y);
                mse_u += (r.u - t.u) * (r.u - t.u);
                mse_v += (r.v - t.v) * (r.y - t.v);
#endif
            }
        }
#ifdef RGB888
        psnr_r = psnr(mse_r / frameSize);
        psnr_g = psnr(mse_g / frameSize);
        psnr_b = psnr(mse_b / frameSize);

        frameScore = (psnr_r + psnr_g + psnr_b) / 3;
#else
        psnr_y = psnr(mse_y / frameSize);
        psnr_u = psnr(mse_u / frameSize);
        psnr_v = psnr(mse_v / frameSize);

        frameScore = (psnr_y + psnr_u + psnr_v) / 3;
#endif

#ifdef DEBUG
        cout << "Frame #" << f << ":" << endl;
        cout << "   Score: " << frameScore << "dB" << endl;
        cout << "   Byte Range: [" << frameOffset << "," << (frameOffset + bytesPerFrame - 1) << "]"
                << endl;
#endif
        sum_fs += frameScore;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;

    cout << "Sequence Score: " << psnr(sum_fs / totalFrames) << "dB" << endl;
    cout << "FPS: " << (totalFrames/diff.count()) << "/sec" << endl;

    ref.close();
    tst.close();
}

void readYUV422(const Context& c) {
    auto frameSize = c.width * c.height;
    auto bytesPerFrame = 2 * frameSize; // Y*frameSize + 0.5*U*frameSize + 0.5*V*frameSize

    ifstream ref = ifstream(c.ref, ios::ate | ios::binary);
    if(!ref) {
        cerr << "ERROR: failed to open file: " << c.ref;
        exit(1);
    }

    ifstream tst = ifstream(c.test, ios::ate | ios::binary);
    if(!tst) {
        cerr << "ERROR: failed to open file: " << c.test;
        exit(1);
    }

    auto rSizeBytes = ref.tellg();
    auto tSizeBytes = tst.tellg();

    ref.seekg(0, ios::beg);
    tst.seekg(0, ios::beg);

    auto rFrames = rSizeBytes / bytesPerFrame;
    auto tFrames = tSizeBytes / bytesPerFrame;

#ifdef DEBUG
    cout << "bytesPerFrame = " << bytesPerFrame << endl;
    cout << "rSize = " << rSizeBytes << ", tSize = " << tSizeBytes << endl;
    cout << "rFrames = " << rFrames << ", tFrames = " << tFrames << endl;
#endif

    auto totalFrames = std::min(rFrames, tFrames);

    uint32_t sum_fs = 0;
    double seqScore = 0;

    auto start = chrono::system_clock::now();

    for (int f = 0; f < totalFrames; f++) {
        uint32_t frameOffset = f * bytesPerFrame; // f * bytesPerFrame = byte offset for current frame
        double frameScore = 0;
#ifdef RGB888
        uint32_t mse_r = 0, mse_b = 0, mse_g = 0;
        double psnr_r = 0, psnr_b = 0, psnr_g = 0;
#else
        uint32_t mse_y = 0, mse_u = 0, mse_v = 0;
        double psnr_y = 0, psnr_u = 0, psnr_v = 0;
#endif

        for (int y = 0; y < c.height; ++y) {
            char rYBuf[c.width], rUBuf[c.width], rVBuf[c.width];
            char tYBuf[c.width], tUBuf[c.width], tVBuf[c.width];

            uint32_t offsetY = 0 * frameSize + y * c.width;
            uint32_t offsetU = 1 * frameSize + (y / 2) * c.width;
            uint32_t offsetV = 1.5 * frameSize + (y / 2) * c.width;

#ifdef FINE
            cout << "OffsetY = " << (frameOffset + offsetY) << endl;
            cout << "OffsetU = " << (frameOffset + offsetU) << endl;
            cout << "OffsetV = " << (frameOffset + offsetV) << endl;
#endif
            //--------------------------------------------------------------------------------------
            // OPTIMIZATION:
            //
            // To improve cache locality, load rows of bytes from each of Y, U, V components. The
            // key is to access memory elements that are contiguous, so the fetch cost is only paid
            // when fetching the 1st of N contiguous elements.
            //--------------------------------------------------------------------------------------

            // Reference File
            ref.seekg(frameOffset + offsetY); // 1 row of Y
            ref.read(rYBuf, sizeof(rYBuf));
            if( y % 2 == 0) {
                // Each row of U & V bytes contains enough information for 2 rows of Y bytes; hence
                // load new data for U and V on every even numbered Y row.
                ref.seekg(frameOffset + offsetU); // 1 row of U
                ref.read(rUBuf, sizeof(rUBuf));
                ref.seekg(frameOffset + offsetV); // 1 row of V
                ref.read(rVBuf, sizeof(rVBuf));
            }

            // Test File
            tst.seekg(frameOffset + offsetY); // 1 row of Y
            tst.read(tYBuf, sizeof(tYBuf));
            if( y % 2 == 0) {
                tst.seekg(frameOffset + offsetU); // 1 row of U
                tst.read(tUBuf, sizeof(tUBuf));
                tst.seekg(frameOffset + offsetV); // 1 row of V
                tst.read(tVBuf, sizeof(tVBuf));
            }

            for (int x = 0; x < c.width; ++x) {
                // 4:2:2
                YCbCr r, t;

                r.y = *(rYBuf + x);
                t.y = *(tYBuf + x);
                if( x % 2 == 0) {
                    // For every even column number, read from U and V buffer.
                    r.u = *(rUBuf + x);
                    r.v = *(rVBuf + x);
                    t.u = *(tUBuf + x);
                    t.v = *(tVBuf + x);

                } else {
                    // For every odd column, read the value of U and V buffer values from the even
                    // column before.
                    r.u = *(rUBuf + x - 1);
                    r.v = *(rVBuf + x - 1);
                    t.u = *(tUBuf + x - 1);
                    t.v = *(tVBuf + x - 1);
                }
#ifdef RGB888
                uint8_t rr, rg, rb;
                uint8_t tr, tg, tb;

                yuv2rgb(r.y, r.u, r.v, &rr, &rg, &rb);
                yuv2rgb(t.y, t.u, t.v, &tr, &tg, &tb);

                mse_r += (rr - tr) * (rr - tr);
                mse_b += (rg - tg) * (rg - tg);
                mse_g += (rb - tb) * (rb - tb);

#else
                mse_y += (r.y - t.y) * (r.y - t.y);
                mse_u += (r.u - t.u) * (r.u - t.u);
                mse_v += (r.v - t.v) * (r.y - t.v);
#endif
            }
        }
#ifdef RGB888
        psnr_r = psnr(mse_r / frameSize);
        psnr_g = psnr(mse_g / frameSize);
        psnr_b = psnr(mse_b / frameSize);

        frameScore = (psnr_r + psnr_g + psnr_b) / 3;
#else
        psnr_y = psnr(mse_y / frameSize);
        psnr_u = psnr(mse_u / frameSize);
        psnr_v = psnr(mse_v / frameSize);

        frameScore = (psnr_y + psnr_u + psnr_v) / 3;
#endif

#ifdef DEBUG
        cout << "Frame #" << f << ":" << endl;
        cout << "   Score: " << frameScore << "dB" << endl;
        cout << "   Byte Range: [" << frameOffset << "," << (frameOffset + bytesPerFrame - 1) << "]"
                << endl;
#endif
        sum_fs += frameScore;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;

    cout << "Sequence Score: " << psnr(sum_fs / totalFrames) << "dB" << endl;
    cout << "FPS: " << (totalFrames/diff.count()) << "/sec" << endl;

    ref.close();
    tst.close();
}


void readYUV420(const Context& c) {
    auto frameSize = c.width * c.height;
    auto bytesPerFrame = 1.5 * frameSize; // Y*frameSize + 0.5*U*frameSize + 0.0*V*frameSize

    ifstream ref = ifstream(c.ref, ios::ate | ios::binary);
    if(!ref) {
        cerr << "ERROR: failed to open file: " << c.ref;
        exit(1);
    }

    ifstream tst = ifstream(c.test, ios::ate | ios::binary);
    if(!tst) {
        cerr << "ERROR: failed to open file: " << c.test;
        exit(1);
    }

    auto rSizeBytes = ref.tellg();
    auto tSizeBytes = tst.tellg();

    ref.seekg(0, ios::beg);
    tst.seekg(0, ios::beg);

    auto rFrames = rSizeBytes / bytesPerFrame;
    auto tFrames = tSizeBytes / bytesPerFrame;

#ifdef DEBUG
    cout << "bytesPerFrame = " << bytesPerFrame << endl;
    cout << "rSize = " << rSizeBytes << ", tSize = " << tSizeBytes << endl;
    cout << "rFrames = " << rFrames << ", tFrames = " << tFrames << endl;
#endif

    auto totalFrames = std::min(rFrames, tFrames);

    uint32_t sum_fs = 0;
    double seqScore = 0;

    auto start = chrono::system_clock::now();

    for (int f = 0; f < totalFrames; f++) {
        uint32_t frameOffset = f * bytesPerFrame; // f * bytesPerFrame = byte offset for current frame
        double frameScore = 0;
#ifdef RGB888
        uint32_t mse_r = 0, mse_b = 0, mse_g = 0;
        double psnr_r = 0, psnr_b = 0, psnr_g = 0;
#else
        uint32_t mse_y = 0, mse_u = 0, mse_v = 0;
        double psnr_y = 0, psnr_u = 0, psnr_v = 0;
#endif

        for (int y = 0; y < c.height; y+=2) {
            char rY1Buf[c.width], rY2Buf[c.width], rUBuf[c.width], rVBuf[c.width];
            char tY1Buf[c.width], tY2Buf[c.width], tUBuf[c.width], tVBuf[c.width];

            uint32_t offsetY1 = 0 * frameSize + y * c.width;
            uint32_t offsetY2 = 0 * frameSize + (y + 1) * c.width;
            uint32_t offsetU = 1 * frameSize + (y / 2) * c.width;
            uint32_t offsetV = 1.25 * frameSize + (y / 2) * c.width;


#ifdef FINE
            cout << "OffsetY1 = " << (frameOffset + offsetY) << endl;
            cout << "OffsetY2 = " << (frameOffset + offsetV) << endl;
            cout << "OffsetU = " << (frameOffset + offsetU) << endl;
            cout << "OffsetV = " << (frameOffset + offsetV) << endl;

#endif
            //--------------------------------------------------------------------------------------
            // OPTIMIZATION:
            //
            // For better cache locality when processing, load 2 rows of Y and 1 row of U. Process
            // x and y elements in 2x2 squares since each square of 4 Y values shares 1 U and V
            // value.
            //--------------------------------------------------------------------------------------

            // Reference File
            ref.seekg(frameOffset + offsetY1); // 1st row of Y
            ref.read(rY1Buf, sizeof(rY1Buf));
            ref.seekg(frameOffset + offsetY2); // 2nd row of Y
            ref.read(rY2Buf, sizeof(rY2Buf));
            ref.seekg(frameOffset + offsetU); // 1 row of U
            ref.read(rUBuf, sizeof(rUBuf));
            ref.seekg(frameOffset + offsetV);  // 1 row of V
            ref.read(rVBuf, sizeof(rVBuf));


            // Test File
            tst.seekg(frameOffset + offsetY1); // 1st row of Y
            tst.read(tY1Buf, sizeof(tY1Buf));
            tst.seekg(frameOffset + offsetY2); // 2nd row of V
            tst.read(tY2Buf, sizeof(tY2Buf));
            tst.seekg(frameOffset + offsetU); // 1 row of U
            tst.read(tUBuf, sizeof(tUBuf));
            tst.seekg(frameOffset + offsetV);  // 1 row of V
            tst.read(tVBuf, sizeof(tVBuf));


            for (int x = 0; x < c.width; x+=2) {
                // 4:2:0

                //
                //     x=0   |   x=2   | ... | x = c.width
                // ---------------------------
                // | r1 | r2 | r1 | r2 | ... | Y1 Buffer
                // ---U1/V1-----U2/V2---------
                // | r3 | r4 | r3 | r4 | ... | Y2 Buffer
                // ---------------------------
                //
                // where [r1.u to r4.u] = U_1, [r1.v to r4.v] = V_1
                //
                YCbCr r1, r2, r3, r4, t1, t2, t3, t4;


                // Reference File
                r1.y = *(rY1Buf + x);
                r2.y = *(rY1Buf + x + 1);
                r3.y = *(rY2Buf + x);
                r4.y = *(rY2Buf + x + 1);
                r1.u = *(rUBuf + x / 2);
                r2.u = r1.u;
                r3.u = r1.u;
                r4.u = r1.u;
                r1.v = *(rVBuf + x / 2);
                r2.v = r1.v;
                r3.v = r1.v;
                r4.v = r1.v;

                // Test File
                t1.y = *(tY1Buf + x);
                t2.y = *(tY1Buf + x + 1);
                t3.y = *(tY2Buf + x);
                t4.y = *(tY2Buf + x + 1);
                t1.u = *(tUBuf + x / 2);
                t2.u = t1.u;
                t3.u = t1.u;
                t4.u = t1.u;
                t1.v = *(tVBuf + x / 2);
                t2.v = t1.v;
                t3.v = t1.v;
                t4.v = t1.v;

#ifdef RGB888
		// TODO:[NOT IMPLEMENTED] 4:2:0 for RGB888 color space
		uint8_t rr, rg, rb;
                uint8_t tr, tg, tb;

                yuv2rgb(r.y, r.u, r.v, &rr, &rg, &rb);
                yuv2rgb(t.y, t.u, t.v, &tr, &tg, &tb);

                mse_r += (rr - tr) * (rr - tr);
                mse_b += (rg - tg) * (rg - tg);
                mse_g += (rb - tb) * (rb - tb);

#else
                mse_y += (r1.y - t1.y) * (r1.y - t1.y);
                mse_u += (r1.u - t1.u) * (r1.u - t1.u);
                mse_v += (r1.v - t1.v) * (r1.y - t1.v);

                mse_y += (r2.y - t2.y) * (r2.y - t2.y);
                mse_u += (r2.u - t2.u) * (r2.u - t2.u);
                mse_v += (r2.v - t2.v) * (r2.y - t2.v);

                mse_y += (r3.y - t3.y) * (r3.y - t3.y);
                mse_u += (r3.u - t3.u) * (r3.u - t3.u);
                mse_v += (r3.v - t3.v) * (r3.y - t3.v);

                mse_y += (r4.y - t4.y) * (r4.y - t4.y);
                mse_u += (r4.u - t4.u) * (r4.u - t4.u);
                mse_v += (r4.v - t4.v) * (r4.y - t4.v);
#endif
            }
        }
#ifdef RGB888
        psnr_r = psnr(mse_r / frameSize);
        psnr_g = psnr(mse_g / frameSize);
        psnr_b = psnr(mse_b / frameSize);

        frameScore = (psnr_r + psnr_g + psnr_b) / 3;
#else
        psnr_y = psnr(mse_y / frameSize);
        psnr_u = psnr(mse_u / frameSize);
        psnr_v = psnr(mse_v / frameSize);

        frameScore = (psnr_y + psnr_u + psnr_v) / 3;
#endif

#ifdef DEBUG
        cout << "Frame #" << f << ":" << endl;
        cout << "   Score: " << frameScore << "dB" << endl;
        cout << "   Byte Range: [" << frameOffset << "," << (frameOffset + bytesPerFrame - 1) << "]"
                << endl;
#endif
        sum_fs += frameScore;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;

    cout << "Sequence Score: " << psnr(sum_fs / totalFrames) << "dB" << endl;
    cout << "FPS: " << (totalFrames/diff.count()) << "/sec" << endl;

    ref.close();
    tst.close();
}

inline void printUsage(const string appName, const po::options_description desc) {

    // HACK: Remove positional arguments from options list.
    typedef vector<boost::shared_ptr<po::option_description> > options_vector;
    options_vector& ov = (options_vector&) desc.options();
    ov.pop_back();
    ov.pop_back();

    cout << endl;
    cout << "USAGE: " << appName << " -s SAMPLING -w WIDTH -h HEIGHT ref-file test-file" << endl;
    cout << endl << "    Computes PSNR between a reference and test video streams." << endl;
    cout << desc << endl;
    cout << endl << "Positional Arguments: " << endl;
    cout << "  ref-file              Reference video file" << endl;
    cout << "  test-file             Test video file" << endl;
    cout << endl << "v" << VERSION << endl;
}

int main(int argc, char** argv) {

    std::string appName = boost::filesystem::basename(argv[0]);
    Context context;
    string subSampling;
    try {
        /** Define and parse the program options
         */

        po::options_description desc("Options");
        desc.add_options()
                ("help", "Print help messages")
                ("sampling,s",  po::value<string>(&subSampling)->required(), "One of '4:4:4', '4:2:2' , or '4:2:0'")
                ("height,h",    po::value<uint>(&context.height)->required(),"Height of video file")
                ("width,w",     po::value<uint>(&context.width)->required(), "Width of video file")
                ("ref-file",    po::value<string>(&context.ref)->required(), "Reference video file")
                ("test-file",   po::value<string>(&context.test)->required(),"Test video file");

        po::positional_options_description pos;
        pos.add("ref-file", 1);
        pos.add("test-file",1);

        po::variables_map vm;
        try {
            po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);

            if (vm.count("help")) {
                printUsage(appName, desc);
                return SUCCESS;
            }

            po::notify(vm); // throws on error, so do after help in case
                            // there are any problems
        } catch (po::error& e) {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            printUsage(appName, desc);
            return ERROR_IN_COMMAND_LINE;
        }

        // application code here //

    } catch (std::exception& e) {
        std::cerr << "Unhandled Exception reached the top of main: " << e.what()
                << ", application will now exit" << std::endl;
        return ERROR_UNHANDLED_EXCEPTION;

    }

    if (!regex_match(subSampling, regex("[0-9]:[0-9]:[0-9]"))) {
        cerr << "ERROR: chroma subsampling parameter invalid format!" << endl;
        exit(-1);
    }

    istringstream iss(subSampling);
    string component;
    for (int i = 0; getline(iss, component, ':') && i < 3; i++) {
        if (i == 0)
            context.J = (uint8_t) stoi(component);
        if (i == 1)
            context.a = (uint8_t) stoi(component);
        if (i == 2)
            context.b = (uint8_t) stoi(component);
    }

    // Compute Frame Size
    int frameSize = 0;
    if (context.J == 4) {
        if (context.a == 4 && context.b == 4) {
            readYUV444(context);
        } else if (context.a == 2 && context.b == 2) {
            // 4:2:2
            readYUV422(context);
        } else if (context.a == 2 && context.b == 0) {
            // 4:2:0
            readYUV420(context);
        } else {
            cerr << "ERROR: unsupported sub-sampling mode! Only: 4:4:4, 4:2:2, 4:2:0." << endl;
            exit(-1);
        }
    }

    return SUCCESS;
}
