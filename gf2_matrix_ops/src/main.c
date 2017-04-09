#include "gf2_matrix.h"

#include <stdio.h>

int main(int argc, char**argv) {
    printf("Creating Empty Matrix of size: 5:\n\n");
    GF2_MATRIX m = create(2,5);
    GF2_print(m);

    printf("\nGet element (1,5): %u\n", getValue(1,5,m));

    printf("\nSet element (1,5) to 1\n");
    setValue(1,5,1,m);
    GF2_print(m);

    printf("\nGet element (1,5): %u\n", getValue(1,5,m));

    printf("\nGet element (1,4): %u\n", getValue(1,4,m));

    printf("\nCreating Identity Matrix of size: 10\n\n");
    GF2_MATRIX n = createIdentityMatrix(10);
    GF2_print(n);

    destroy(m);
    destroy(n);
}
