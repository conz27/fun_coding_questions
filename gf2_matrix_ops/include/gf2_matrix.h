#include <stddef.h>
#include <stdint.h>

typedef struct _matrix {
    size_t rows;         // Rows in the Matrix
    size_t cols;         // Columns in the Matrix
    uint8_t *m;         // Pointer to Matrix Array.
} Matrix;

typedef uint8_t GF2_ELEM;
typedef Matrix* GF2_MATRIX;

/**
 * Creates a 'row by col' matrix of any size.
 * @param[in] rows the number of rows in the matrix.
 * @param[in] cols the number of columns in the matrix.
 * @return a GF2_MATRIX pointer.
 */
GF2_MATRIX create(const size_t rows, const size_t cols);

/**
 * Creates a 'dim by dim' identity matrix.
 * @param dim[in] the dimension of the matrix.
 * @return a GF2_MATRIX pointer.
 */
GF2_MATRIX createIdentityMatrix(const size_t dim);

/**
 * Destroys a created matrix.
 * @param[in] a GF2_MATRIX pointer.
 */
void destroy(GF2_MATRIX m);

/**
 * Retrieves a value from the GF2_MATRIX.
 * @param[in] row the row in matrix {row >=1,row<=dim}
 * @param[in] col the column in the matrix {col >=1,col <=dim}
 * @param[in] m the matrix from which to retrieve element.
 * @return the value at (rol,col), it will be a 0 or 1 based on GF2 property.
 */
GF2_ELEM getValue(const size_t row, const size_t col, const GF2_MATRIX m);

/**
 * Sets a value in the GF2_MATRIX.
 * @param[in] row the row in matrix {row >=1,row<=dim}
 * @param[in] col the column in the matrix {col >=1,col <=dim}
 * @param[in] e the element which to insert into the matrix.
 * @param[in] m the matrix from which to retrieve element.
 */
void setValue(const size_t row, const size_t col, const GF2_ELEM e, GF2_MATRIX m);

/**
 * Print a GF2_MATRIX to the screen.
 * @param[in] the matrix which to print.
 */
void GF2_print(GF2_MATRIX m);
