# Problem Summary

Assume you have a 32-bit processor with support for widening multiplication (64-bit result). Write a big-integer multiplication function, dmult(x, y), where x and y are represented by arrays that represent a very large integer. The result of the function should be an array that contains the product of the two inputs. Describe any design, performance, and optimization considerations.

## Multiplication of Large Integers

### Research:

https://people.eecs.berkeley.edu/~vazirani/algorithms/chap2.pdf

https://en.wikipedia.org/wiki/Multiplication_algorithm

### Synopsis:

There are several ways to approach this problem:
  * hardware-accelerated multiplication using the ALU (Arithmetic Logic Unit) on the chip. 
      * easiest, but works only for primitive values - in this case 32-bit intgers with 64-bit products
  * naive method (i.e. long multiplication as taught school)
      * typically performed on the binary level and implemented with a combination of shifts and add operations
      * space optimizations possible in regards to not keeping every single partial product in memory
      * implemented with algorithmic complexity of BIG_O(n^2) with 2 for loops.
      * this approach is more taxing on the computationally due to the number of operations required, but relatively stable in terms of memory used.
  * divide & conquer algorithms (i.e. Karatsuba, Toom-Cook)
      * improves upon naive method by reducing the number of adds and shifts required to compute the result to BIG_O(n^1.59). With the absence of Gauss' trick, the recursion tree would have BIG_O(n^2), so the elimination of the additional multiplication that Gauss' trick employs is why divide and conquer improves upon the traditional approach. The work at each level increases geometrically by a factor of 3/2.
      * implemented through a recursive approach by splitting the arrays into multiple sub-problems
      * this approach decreases the computational load by reducing the number of operations, but to due the recursive nature of the algorithm it uses more memory in order to store intermidiate results to computed sub-problems.
      * optimizations are possible by not requiring the n == 1 terminating condition due to the fact that ALUs can perform 32-bit integer multiplications.
      * practical for numbers that are several thousands decimal digits
      * most big num libraries employ Karatsuba's algorithm
  * Fast Fourier Transforms (i.e. Schönhage–Strassen algorithm, Fürer's algorithm)
      * fastest known approach for multiplication of large integers
      * BIG_O(n\*log(n)\*log(log(n)) and n\*log(n)\*2^BIG_O(lg\*n))
      * practical for numbers with 10,000 to 40,000 decimal digits
  
## Solution
The ideal solution is entirely dependent on the number of digits *n* that a multiplicand has. If *n* is sufficiently small, Karatsuba is slower than the naive method due to the overhead of recursions. If *n* is in the order of several thousand digits, Karatsuba / Toom-Cook offer better performance than the naive method. And last by not least, there comes a point around when *n* > 10,000 digits where FFT-based algorithms become asymptotically faster than the recursion-based ones. The optimial solution determines the appropriate algorithm to use based on the size of *n*.

In practice today, bignum libraries employ the Karatsuba algorithm and may contain optimizations based on size of *n*. They can also reduce the number of recursions required based on whether the native architecture supports 16-bit or 32-bit integer multiplication, so the terminating condition of *n* == 1 can be changed to *n* == 2 bytes or 4 bytes respectively. This allows us to make use of the hardware acclerated multipliers for the last step.
