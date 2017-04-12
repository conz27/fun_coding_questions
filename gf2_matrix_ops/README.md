# Problem Summary:
Design a set of functions that allows for the manipulation of a Galois Field (GF2) matrix of any size. Your code should be able to create, destroy, get and set bits, and it should also be able to create an identity matrix. Describe any design, performance, and optimization decisions.

# Solution
A Galois Field of order 2 (i.e. p = 2 = GF(2)), means that field consists of 2 elements (0 or 1) and can be constructed by any integer, n mod p.

## Design & Assumptions
For simplicity and opitimization, any non-zero integer ```e``` inserted via ```setValue()``` is assumed to be '1' hence the following code on insertion:
```
( e & 0x1) where e is a primitive integer.
```
The matrix that is *(M x N)* where *M* is the number of rows and *N* is the number of columns is stored as a column-major 1D array, meaning that the array is laid out as follows:

1\*N\*(1..M) + 2\*N\*(1..M) + ... + k\*N\*(1..M) where 1 <= k <= N. Element kN denotes the beginning of the next column as k iterates from 1 to N.

Therefore, accessing a particular element in the matrix is achieved by performing simple pointer arithmetic:

``` (COL - 1)*N + (ROW - 1) where ROW and COL are inputs (1...M) and (1...N) respectively.```

## Optimization / Performance

**Cache Locality:**

Programs like MATLAB, for example, store their matrix columns in monotonically increasing memory locations; therefore, by processing data column-wise we are able to achieve maximum cache locality and efficiency. Complex algorithms and matrix manipulations are often modelled on programs like MATLAB, so portability can be made easier by respecting the column-major storage and hence requiring only minimal changes to the algorithm when translating to C or C++. The cache efficiency is achieved because when pages are loaded into the L1 , L2, and/or L3 caches, they load the memory around the element that is being accessed as well. For example, accessing the first element in the matrix (1,1), may load every single element from (1,1) to (1,20) if M = 20. Therefore, fetching time for (1,1) is slow, but subsequent accesses to (1,2) to (1,20) will be much quicker because the page has already been loaded.

Using software like ```cachegrind``` can help optimize your code for maximum cache efficiency.

NOTE: Deciding whether to store your array row or column major is entirely depedent on how you manipulate your matrix. You want to advantage of read and write speeds and optimize accordingly.

**Loop Unrolling:** (not implemented)

Loop unrolling can increase an algorithm's execution speed by reducing or eliminating instructions that control the loop:
* "end of loop" tests on each iteration
* pointer arithmetic (increment pointer or index)
* branch penalties
* reading data from memory

Re-writing a loop as a repeated sequence of indepedent statements will remove some of this computational overhead. These increases in speed usually come at the price of increased program code size and less clarity when reading the code.

**Reduction of Strength:** (not implemented)

Is a technqiue that replaces slow math operations with faster ones. The benefits are largely dependent on the target CPU and/or surrounding code. Some examples include:
* replace integer division or multiple by powers to 2 with logical shifts to the left or right
* replace integer multiplication by a constant with a combination of adds, shifts or subtracts.



