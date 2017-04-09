#include "gf2_matrix.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

GF2_MATRIX create(const size_t rows, const size_t cols) {
    int row, col;
    GF2_MATRIX m = (GF2_MATRIX) malloc(sizeof(Matrix) + rows*cols - 1);
    if(m) {
        m->rows = rows;
        m->cols = cols;
        m->m = (uint8_t*)( m + 2*sizeof(size_t));
        for(col = 0; col < m->cols; col++) {
            for(row = 0; row < m->rows; row++) {
                *(m->m + col*cols + row) = 0;
            }
        }
    }
    return m;
}

GF2_MATRIX createIdentityMatrix(const size_t dim){
    int i;
    GF2_MATRIX m = create(dim, dim);
    for(i = 0; i < dim; i++){
        *(m->m + i*dim + i) = 1;
    }
    return m;
}
void destroy(GF2_MATRIX m) {
    if(m) {
        free(m);
        m = NULL;
    }
}

GF2_ELEM getValue(const size_t row, const size_t col, const GF2_MATRIX m) {
    assert(m != NULL);
    assert(row >= 1 && row <= m->rows);
    assert(col >= 1 && col <= m->cols);
    return *(m->m + (col-1)*m->cols + (row-1));
}

void setValue(const size_t row, const size_t col, const GF2_ELEM e, GF2_MATRIX m) {
    assert(m != NULL);
    assert(row >= 1 && row <= m->rows);
    assert(col >= 1 && col <= m->cols);
    
    // To enforce the GF2 property, any value set in the matrix must be a 0 or 1; hence (e & 0x1).
    *(m->m + (col-1)*m->cols + (row-1)) |= (e & 0x1); 
}

void GF2_print(GF2_MATRIX m) {
    int row, col;
    for(row = 0; row < m->rows; row++) {
        for(col = 0; col < m->cols; col++) {
            printf("%u ", *(m->m + col*m->cols + row));
        }
        printf("\n");
    }
}
