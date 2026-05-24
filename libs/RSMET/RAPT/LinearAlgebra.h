#pragma once

#include "MatrixTools.h"
#include "../rsmethelper.hpp"
#include "../RSLib/MathBasics.inl"

/** Collection of functions for linear algebra such as solving systems of linear equations, matrix 
inversion, etc.

ToDo: 
-get rid of rs-prefix in the function names (they are now wrapped into a class which already 
 has the prefix)
-rename to rsLinearAlgebraOld and eventually deprecate it in favor of rsLinearAlgebraNew which 
 should then be named rsLinearAlgebra
*/

namespace RAPT {

class rsLinearAlgebra {

public:
  /** Solves the 2x2 system A*x = y where A is a 2x2 matrix and x, y are
  2-dimensional vectors. The result is returned in x, the other parameters will
  not be modified. */
  template <class T>
  static void rsSolveLinearSystem2x2(const T A[2][2], T x[2], const T y[2]);
  // rename to solve2x2

  /** Solves the 3x3 system A*x = y where A is a 3x3 matrix and x, y are
  3-dimensional vectors. The result is returned in x, the other parameters will
  not be modified. */
  template <class T>
  static void rsSolveLinearSystem3x3(const T A[3][3], T x[3], const T y[3]);
  // todo: check, if this is the right form of const-ness
  // rename to solve3x3

  /** Computes the first eigenvalue of the matrix [[a,b],[c,d]] where "first"
  means the one with the minus-sign in the term under the square-root in the
  formula. For real eigenvalues, this is the smaller of the two. If a matrix
  with real coefficients has complex eigenvalues ... it currently doesn't work
  (encounters a sqrt of a negative number...todo: maybe return the real part of
  the result) */
  template <class T> static T eigenvalue2x2_1(T a, T b, T c, T d);

  /** Computes the second eigenvalue of the matrix [[a,b],[c,d]]. */
  template <class T> static T eigenvalue2x2_2(T a, T b, T c, T d);

  /** Computes the first eigenvector of the matrix [[a,b],[c,d]] and stores the
  result in vx, vy. It may optionally normalize the the result to unit length.
*/
  template <class T>
  static void eigenvector2x2_1(T a, T b, T c, T d, T *vx, T *vy,
                               bool normalize = true);

  /** Computes the second eigenvector of the matrix [[a,b],[c,d]] */
  template <class T>
  static void eigenvector2x2_2(T a, T b, T c, T d, T *vx, T *vy,
                               bool normalize = true);

  /** Solves a*x + b*y = p subject to x^2 + y^2 = min. */
  template <class T> static void solveMinNorm(T a, T b, T p, T *x, T *y);

  /** Solves the linear system of equations:
  \f[ a_{00}x_0      + a_{01}x_1      + \ldots + a_{0 (N-1)}    x_{N-1} &= b_0
  \f] \f[ a_{10}x_0      + a_{11}x_1      + \ldots + a_{1 (N-1)}    x_{N-1} &=
  b_1      \f] \f[ \vdots \f] \f[ a_{(N-1) 0}x_0 + a_{(N-1) 1}x_1 + \ldots +
  a_{(N-1)(N-1)} x_{N-1} &= b_{N-1}  \f] which is respresented by the matrix
  equation: \f[ \mathbf{A x} = \mathbf{b} \f] that is, it computes the solution
  vector x which satisfies the system of equations (if any). Note that the
  indexing scheme for the matrix and array entries above conforms with the
  C-array-indexing (starting at index 0) rather than the math-textbook-indexing
  (starting at index 1). The matrix A is represented as a pointer to pointer to
  double - the first dereferencing is therefore a pointer to double and must
  point to the beginning of a row. The vectors x and b are simple pointers
  pointing to an array of doubles which must be of length N as well. When
  accessing A as two-dimensional array, this means that the the first index in A
  must indicate the row, the second indicates the column. The vector b
  represents the right hand side of the equation and x will contain the solution
  vetor on return. The function uses Gaussian elimination with partial pivoting
  and subsequent backsubstitution. The boolean return value informs whether a
  solution could be computed (the algorithm will fail when the matrix is
  singular) - when it returns false, it means that either there is no solution
  at all or that there is not a unique solution.  */
  template <class T>
  // static bool rsSolveLinearSystem(const T **A, T *x, const T *b, int N); //
  // compiler error
  static bool rsSolveLinearSystem(T **A, T *x, const T *b,
                                  int N)
  {
    int i, j;

    // allocate memory for temporary copies of the coefficient matrix A and
    // target vector b:
    T *tmpB = new T[N];
    T *tmpA = new T[N * N];
    T **tmpAP = new T *[N];

    // assign the pointer array for the matrix (tmpAP) to to the beginnings of
    // the rows:
    for (i = 0; i < N; i++)
      tmpAP[i] = &(tmpA[i * N]);

    // copy the data into the temporary arrays:
    for (i = 0; i < N; i++) {
      tmpB[i] = b[i];
      for (j = 0; j < N; j++)
        tmpAP[i][j] = A[i][j];
    }

    // solve the linear system in place with the temporary arrays:
    bool success = rsSolveLinearSystemInPlace(tmpAP, x, tmpB, N);

    // free allocated memory:
    delete[] tmpB;
    delete[] tmpA;
    delete[] tmpAP;

    return success;
  }
  // static bool rsSolveLinearSystem(T **A, T *x, T *b, int N);
  //  maybe have possibly different types for the matrix elements and vector
  //  elements - some equations (for curves in Salomon's Computer Graphics...
  //  for example) are formulated in terms of matrices-of-numbers and
  //  vectors-of-points

  /** Solves the linear system just as solveLinearSystem() does - but doesn't
  allocate temporary heap memory and instead destroys the coefficient matrix A
  and the target vector b during the process because the computation is done in
  place. In fact, the function solveLinearSystem() just makes temporary copies
  of the matrix A and target vector b and then calls this function with these
  copies to do the actual computation. If you don't need the matrix or vector
  anymore after solving the system, you can use this function directly to get
  rid of the copying overhead. The algorithm is Gaussian elimination with
  partial pivoting (...i think -> verify this). */
  template <class T>
  static bool rsSolveLinearSystemInPlace(T **A, T *x, T *b, int N);
  // deprecate! use rsMatrix instead ...but keep around for the comments

  // todo: add functions to solve NxM systems with N != M (find minimum-norm
  // solution for underdetermined systems and least-squares approximation for
  // overdetermined systems... maybe we should have two different functions
  // solveUnderDeterminedSystem, solveOverDeterminedSystem...or something. the
  // overdetermined case can then be used inside polynomial curve-fitting and
  // multiple linear regression routines (i think polynomial fits use a
  // Vandermonde matrix (created from data vectors) and then works the same a
  // multiple linear regression -> look it up...) (maybe) make a dispatcher
  // function  rsSolveLinearSystem(T **A, T *x, T *b, int N, int M); that
  // dispatches the 3 cases to the 3 functions

  /** Inverts the matrix A via Gauss-Jordan elimination with partial pivoting.
   */
  template <class T> static bool rsInvertMatrix(T **A, int N);
  // deprecate!

  /** Solves the tridiagonal system of equations defined by a NxN matrix having
  the 3 nonzero diagonals 'lowerDiagonal', 'mainDiagonal' and 'upperDiagonal'
  where the 'mainDiagonal' array should have N elements and 'lowerDiagonal' and
  'upperDiagonal' represent the the diagonals below and above the main diagonal
  respectively and should have N-1 elements. The 'rightHandSide' argument
  represents the right hand side of the equation (vector with N elements) and
  'solution' ís where the solution vector will be stored (N elements). */
  template <class T>
  static bool rsSolveTridiagonalSystem(T *lowerDiagonal, T *mainDiagonal,
                                       T *upperDiagonal, T *rightHandSide,
                                       T *solution, int N);
  // allocates memory

  /** Solves a pentadiagonal linear system of equations with given diagonals and
  right-hand side using a simple algorithm without pivot-search. lowerDiag1 is
  the one directly below the main diagonal, lowerDiag2 the one below lowerDiag1
  - and similarly for upperDiag1/upperDiag2. In the process of the computations,
  the right hand side vector is destroyed. the same is true for mainDiag and the
  two inner sub/superdiagonals lowerDiag1, upperDiag1. Note also that you can't
  use the same array for lowerDiag1 and upperDiag1, even if your matrix is
  symmetric.
  ..What about lowerDiag2/upperDiag2? are these preserved and may these point to
  the same vector? It's probably safest to assume that everything may get messed
  up and all arrays should be distinct. */
  template <class T>
  static bool rsSolvePentaDiagonalSystem(T *lowerDiag2, T *lowerDiag1,
                                         T *mainDiag, T *upperDiag1,
                                         T *upperDiag2, T *righHandSide,
                                         T *solution, int N);

  /** Given NxN matrices A and B whose columns are assumed to constitue two
  bases for R^N and the coordinates va of a vector v with respect to basis A,
  this function returns the coordinates of the same vector v with respect to
  basis B and return them in vb. The return value informs, if the solution of
  the linear system succeeded (it may not, if B is not a basis - but what if A
  is not a basis? will it work anyway? quite possible, i think - because it just
  restricts the vector v to lie in some subspace of R^N which is no problem). */
  template <class T>
  static bool rsChangeOfBasisColumnWise(T **A, T **B, T *va, T *vb, int N);

  /** Similar to rsChangeOfBasisColumnWise but here, the basis vectors are given
  by the rows of the matrices A and B. */
  template <class T>
  static bool rsChangeOfBasisRowWise(T **A, T **B, T *va, T *vb, int N);

  /** Given NxN matrices A and B whose columns are assumed to constitue two
  bases for R^N, this function computes the change-of-basis matrix C which
  converts coordinates with respect to basis A into coordinates with respect to
  basis B such that for any vector v with coordinates va in A and coordinates vb
  in B, we have vb = C * va. */
  template <class T>
  static bool rsChangeOfBasisMatrixColumnWise(T **A, T **B, T **C, int N);

  /** Similar to rsChangeOfBasisMatrixColumnWise but here, the basis vectors are
  given by the rows of the matrices A and B. */
  template <class T>
  static bool rsChangeOfBasisMatrixRowWise(T **A, T **B, T **C, int N);
};


template <class T>
void rsLinearAlgebra::rsSolveLinearSystem2x2(const T A[2][2], T x[2],
                                             const T y[2]) {
  // T det = (A[0][0]*A[1][1] - A[0][1]*A[1][0]); // determinant, for debugging
  T s = T(1) / (A[0][0] * A[1][1] - A[0][1] * A[1][0]);
  x[0] = s * (A[1][1] * y[0] - A[0][1] * y[1]);
  x[1] = s * (A[0][0] * y[1] - A[1][0] * y[0]);
  // add: 0, sub: 4, mul: 10, div: 1
}

template <class T>
void rsLinearAlgebra::rsSolveLinearSystem3x3(const T A[3][3], T x[3],
                                             const T y[3]) {
  T k1 = A[1][1] * A[2][0] - A[1][0] * A[2][1];
  T k2 = A[1][2] * A[2][1] - A[1][1] * A[2][2];
  T k3 = A[2][2] * y[1] - A[1][2] * y[2];
  T k4 = A[1][0] * y[2] - A[2][0] * y[1];
  T c =
      T(1) / (A[0][0] * k2 + A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
              A[0][2] * k1);
  x[0] = +c * (A[0][1] * k3 + A[0][2] * (A[1][1] * y[2] - A[2][1] * y[1]) +
               k2 * y[0]);
  x[1] = -c * (A[0][0] * k3 + A[0][2] * k4 +
               (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * y[0]);
  x[2] = +c * (A[0][0] * (A[2][1] * y[1] - A[1][1] * y[2]) + A[0][1] * k4 +
               k1 * y[0]);

  // add: 8, sub: 8, mul: 31, div: 1
  // maybe optimize further - there are still some products above that are
  // computed twice: A[1][0]*A[2][2], A[1][2]*A[2][0], A[1][1]*y[2]
  // ...but then do performance tests

  /*
  // un-optimized maxima output - measure performance against optimized version
  above: T s = T(1) /
  (A[0][0]*(A[1][2]*A[2][1]-A[1][1]*A[2][2])+A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])+A[0][2]*(A[1][1]*A[2][0]-A[1][0]*A[2][1]));
  x[0] =
  s*(A[0][1]*(A[2][2]*y[1]-A[1][2]*y[2])+A[0][2]*(A[1][1]*y[2]-A[2][1]*y[1])+(A[1][2]*A[2][1]-A[1][1]*A[2][2])*y[0]);
  x[1]
  =-s*(A[0][0]*(A[2][2]*y[1]-A[1][2]*y[2])+A[0][2]*(A[1][0]*y[2]-A[2][0]*y[1])+(A[1][2]*A[2][0]-A[1][0]*A[2][2])*y[0]);
  x[2] =
  s*(A[0][0]*(A[2][1]*y[1]-A[1][1]*y[2])+A[0][1]*(A[1][0]*y[2]-A[2][0]*y[1])+(A[1][1]*A[2][0]-A[1][0]*A[2][1])*y[0]);
  // add: 3*2+2=8, sub: 3*3+3=12  mul: 3*10+9=39, div: 1
  */
}

template <class T> inline void normalizeLength(T *vx, T *vy) {
  // return;  // preliminary
  T rx = rsAbs(*vx);
  rx *= rx;
  T ry = rsAbs(*vy);
  ry *= ry;
  T s = T(1) / sqrt(rx + ry);
  *vx *= s;
  *vy *= s;
  // this is written such it can work for T being a real or complex number class
  // ...but maybe it can be optimized even for the complex case?
}
// maybe de-inline and make a static class member

// move to where rsAbs is:
// template<class T> inline T rsReal(T x) { return x; }
// template<class T> inline T rsReal(std::complex<T> x) { return x.real(); }
// inline double rsReal(std::complex<double> x) { return x.real(); }

// template<class T> inline bool rsLess(   const T& x, const T& y) { return x <
// y; } template<class T> inline bool rsGreater(const T& x, const T& y) { return
// x > y; }

// move to Basics.h:
template <class T>
inline bool rsLess(const std::complex<T> &x,
                   const std::complex<T> &y) // maybe have a tolerance
{
  if (x.real() < y.real())
    return true;
  if (x.imag() < y.imag())
    return true;
  return false;
}

template <class T>
inline bool rsGreater(const std::complex<T> &x,
                      const std::complex<T> &y) // maybe have a tolerance
{
  if (x.real() > y.real())
    return true;
  if (x.imag() > y.imag())
    return true;
  return false;
}

template <class T> inline T eigenDiscriminant2x2(T a, T b, T c, T d) {
  return a * a + T(4) * b * c - T(2) * a * d + d * d;
}
// this expression occurs in all the square-roots in the functions below - it
// discriminates between real and complex eigenvalues, when the coeffs are real
// - dunno, if it's useful to have this function - if so, maybe add it to
// rsLinearAlgebra

template <class T> T rsLinearAlgebra::eigenvalue2x2_1(T a, T b, T c, T d) {
  return T(0.5) * (a + d - sqrt(a * a + T(4) * b * c - T(2) * a * d + d * d));
}

template <class T> T rsLinearAlgebra::eigenvalue2x2_2(T a, T b, T c, T d) {
  return T(0.5) * (a + d + sqrt(a * a + T(4) * b * c - T(2) * a * d + d * d));
}

template <class T>
void rsLinearAlgebra::eigenvector2x2_1(T a, T b, T c, T d, T *vx, T *vy,
                                       bool normalize) {
  if (b != T(0)) {
    *vx = T(1);
    *vy = T(-0.5) *
          (a - d + sqrt(a * a + T(4) * b * c - T(2) * a * d + d * d)) / b;
    if (normalize)
      normalizeLength(vx, vy);
  } else {
    if (rsLess(a, d)) { // .maybe we need a tolerance, i.e. if tol < d-a
      *vx = T(1);
      *vy = c / (a - d);
      if (normalize)
        normalizeLength(vx, vy);
    } else {
      *vx = T(0);
      *vy = T(1);
    }
  }
}
// ...needs tests with complex numbers - what if the function is called with
// real inputs but the matrix has complex eigenvalues - we will get a negative
// number in the sqrt - maybe we should use something like: d =
// eigenDiscriminant2x2(a,b,c,d); *vy = T(-0.5) * (a - d + sqrt(max(d,0)) ) / b;
// ..in this case, it would return the real part of the complex eigenvalue which
// would be consistent with rsPolynomial::rootsQuadraticReal

template <class T>
void rsLinearAlgebra::eigenvector2x2_2(T a, T b, T c, T d, T *vx, T *vy,
                                       bool normalize) {
  if (b != T(0)) {
    *vx = T(1);
    *vy = T(-0.5) *
          (a - d - sqrt(a * a + T(4) * b * c - T(2) * a * d + d * d)) / b;
    if (normalize)
      normalizeLength(vx, vy);
  } else {
    if (rsGreater(a, d)) { // maybe tolerance is needed here too
      *vx = T(1);
      *vy = c / (a - d);
      if (normalize)
        normalizeLength(vx, vy);
    } else {
      *vx = T(0);
      *vy = T(1);
    }
  }
}

// ToDo:
// -get rid of the code duplication by refactoring
// -maybe write a function that computes both eigenvalues and eigenvectors at
// once - a lot of
//  intermediate results are used in all formules -> optimization
// -make it work also for real matrices with complex eigenvalues (but make sure
// to not mess it up
//  for complex matrices)

// the same sqrt appears in all 4 formulas - what's its significance? maybe its
// worth to factor out and give it a name? maybe eigenSqrt2x2 ...or has it to do
// with the determinant? i think, it's a sort of discriminant that discriminates
// the cases of real and complex eigenvalues (when the coeffs are real) see also
// here: https://en.wikipedia.org/wiki/Eigenvalue_algorithm#2%C3%972_matrices
// maybe compute the gap - maybe this is the square root above?

// the general formula can be found with the following sage code:
// var("a b c d")
// A = matrix([[a, b], [c, d]])
// A.eigenvectors_right()
// [(1/2*a + 1/2*d - 1/2*sqrt(a^2 + 4*b*c - 2*a*d + d^2), [(1, -1/2*(a - d +
// sqrt(a^2 + 4*b*c - 2*a*d + d^2))/b)],  1),
//  (1/2*a + 1/2*d + 1/2*sqrt(a^2 + 4*b*c - 2*a*d + d^2), [(1, -1/2*(a - d -
//  sqrt(a^2 + 4*b*c - 2*a*d + d^2))/b)],  1) ]
// special cases are obtained by setting b=0 and maybe additionally d=a, these
// are the right eigenvectors - maybe have similar functions for the left
// eigenvectors?

template <class T>
void rsLinearAlgebra::solveMinNorm(T a, T b, T p, T *x, T *y) {
  rsAssert(a * a + b * b > T(0),
           "At least one coeff must be nonzero or we get a division by zero");
  T s = p / (a * a + b * b);
  *x = s * a;
  *y = s * b;
}
// Formulas can be derived by minimizing x^2 + y^2 subject to the constraint a*x
// + b*y - p = 0 using a Lagrange multiplier l for the constraint. With Sage, it
// looks like this: var("a b p l x y") L = x^2 + y^2 + l*(a*x + b*y - p)  #
// Lagrange function L = L(x,y,l) L_x = diff(L, x)                   #
// derivative of L with respect to x L_y = diff(L, y)                   #
// derivative of L with respect to y L_l = diff(L, l)                   #
// derivative of L with respect to l solve([L_x==0,L_y==0,L_l==0],[x,y,l])
//
// result: [[x == a*p/(a^2 + b^2), y == b*p/(a^2 + b^2), l == -2*p/(a^2 + b^2)]]
// see also:
// https://ask.sagemath.org/question/38079/can-sage-do-symbolic-optimization/

//template <class T>
//bool rsLinearAlgebra::rsSolveLinearSystemInPlace(T **A, T *x, T *b, int N) {
//  bool matrixIsSingular = false;
//  int i, j, k, p;
//  // double biggest; // actually, it should be T, but then the pivot search
//  // doesn't work for complex
//  T biggest;
//  T multiplier; // matrices because rsAbs returns a real number for complex
//                // inputs and two
//  T tmpSum; // complex numbers can't be compared for size anyway -> figure out a
//            // solution
//
//  T tol = T(1.e-12); // ad hoc, seems reasonable for T == double
//  // ToDo: use something based on std::numeric_limits<T> and/or let the user
//  // pick a threshold. also, It should probably be a relative value. If T is
//  // something like rsFraction, we may want to use an entirely different
//  // criterion, like using simpler fractions as pivots to decrease the
//  // likelihood of integer overflow. Numerators and denominators of fractions
//  // tend to blow up quickly (exponentially?) in a sequence of arithmetic
//  // operations. Maybe we should implement a templated rsIsBetterPivot and
//  // rsIsValidPivot with different explicit specializations for rsFraction and
//  // float/double/complex etc. Oh - and what if T is itself a matrix type as in
//  // block matrices? In this case, we need an invertible matrix as pivot.
//
//  // outermost loop over the rows to be scaled and subtracted from the rows
//  // below them:
//  for (i = 0; i < N; i++) {
//
//    // search for largest pivot in the i-th column from the i-th row downward:
//    p = i;
//    biggest = T(0);
//    for (j = i; j < N; j++) {
//      if (rsGreaterAbs(A[j][i], biggest))
//      // if(rsAbs(A[j][i]) > biggest)  // rsAbs because abs uses the integer
//      // version on linux and
//      { // fabs is only for floats (can't take modular integers, for
//        biggest = rsAbs(A[j][i]); // example)
//        p = j;
//      }
//    }
//    if (rsIsCloseTo(biggest, T(0), tol)) {
//      matrixIsSingular = true;
//      rsError("Matrix singular (or numerically close to)");
//      // shouldn't we return here?
//    }
//
//    // swap current row with pivot row (a pointer switch in the first index of A
//    // and an exchange of two values in b):
//    if (p != i) {
//      rsSwap(A[i], A[p]);
//      rsSwap(b[i], b[p]);
//      p = i;
//    }
//
//    // subtract a scaled version of the pivot-row p (==i) from all rows below it
//    // (j=i+1...N-1), the multiplier being the i-th column element of the
//    // current row divided by the i-th column element of the pivot-row - but we
//    // do the subtraction only for those columns which were not already zeroed
//    // out by a previous iteration, that is: only from the i-th column rightward
//    // (k = i...N-1):
//    for (j = i + 1; j < N; j++) {
//      multiplier = A[j][i] / A[p][i];
//      b[j] -= multiplier * b[p];
//      for (k = i; k < N; k++)
//        A[j][k] -= multiplier * A[p][k];
//    }
//  }
//
//  // matrix A is now in upper triangular form - solve for x[0...N-1] by
//  // backsubstitution (maybe factor out):
//  x[N - 1] = b[N - 1] / A[N - 1][N - 1];
//  for (i = N - 2; i >= 0; i--) {
//    // multiply the already obtained x-values by their coefficients from the
//    // current row and accumulate them:
//    tmpSum = T(0);
//    for (j = i + 1; j < N; j++)
//      tmpSum += A[i][j] * x[j];
//
//    // this accumulated sum is to be subtracted from the target value, and the
//    // result of that subtraction must be divided diagonal-element which
//    // corresponds to the index of the new x-value which is to be computed:
//    x[i] = (b[i] - tmpSum) / A[i][i];
//  }
//
//  // Done: The vector x now contains the solution to the system A*x=b (unless
//  // the matrix was singular in which case it contains meaningless numbers or
//  // not-a-numbers). A and b contain garbage now.
//
//  return !matrixIsSingular;
//}

template <class T> bool rsLinearAlgebra::rsInvertMatrix(T **A, int N) {
  bool matrixIsSingular = false;

  int i, j, k, p;
  double biggest;
  T multiplier;

  T *tmpA = new T[N * N];
  T **tmpAP = new T *[N];

  // assign the pointer array for the matrix (tmpAP) to to the beginnings of the
  // rows:
  for (i = 0; i < N; i++)
    tmpAP[i] = &(tmpA[i * N]);

  // copy the data from A into the temporary matrix and initialize the matrix A
  // with the unit matrix:
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      tmpAP[i][j] = A[i][j];
      A[i][j] = 0.0;
    }
    A[i][i] = 1.0;
  }

  // outermost loop over the rows to be scaled and subtracted from the rows
  // below them:
  for (i = 0; i < N; i++) {

    // search for largest pivot in the i-th column from the i-th row downward:
    p = i;
    biggest = 0.0;
    for (j = i; j < N; j++) {
      if (fabs(tmpAP[j][i]) > biggest) {
        biggest = fabs(tmpAP[j][i]);
        p = j;
      }
    }
    if (RSLib::rsIsCloseTo(biggest, 0.0, 1.e-12)) {
      matrixIsSingular = true;
      rsError("Matrix close to singular.");
    }

    // swap the current row with pivot row (a pointer switch in the first index
    // of A and ...?):
    if (p != i) {
      rsSwap(tmpAP[i], tmpAP[p]);
      rsSwap(A[i], A[p]);
      p = i;
    }

    // divide the pivot-row row by the pivot element to get a 1 on the diagonal:
    multiplier = 1.0 / tmpAP[i][i];
    for (k = 0; k < N; k++) {
      tmpAP[p][k] *= multiplier;
      A[p][k] *= multiplier;
    }

    // subtract a properly scaled version of the pivot-row from all other rows
    // to get zeros in this column (this is different from Gaussian eleimination
    // where it is subtracted only from the rows below):
    for (j = 0; j < N; j++) {
      multiplier = tmpAP[j][i];
      if (j != i) {
        for (k = 0; k < N; k++) {
          tmpAP[j][k] -= multiplier * tmpAP[p][k];
          A[j][k] -= multiplier * A[p][k];
        }
      }
    }
  }

  // free temporarily allocated memory:
  delete[] tmpA;
  delete[] tmpAP;

  return !matrixIsSingular;
}

template <class T>
bool rsLinearAlgebra::rsSolveTridiagonalSystem(T *lower, T *main, T *upper,
                                               T *rhs, T *solution, int N) {
  if (main[0] == T(0)) {
    rsError("Division by zero.");
    return false;
  }

  T divisor = main[0];
  T *tmp = new T[N];
  solution[0] = rhs[0] / divisor;
  for (int n = 1; n < N; n++) {
    tmp[n] = upper[n - 1] / divisor;
    divisor = main[n] - lower[n - 1] * tmp[n];
    if (divisor == T(0)) {
      rsError("Division by zero.");
      delete[] tmp;
      return false;
    }
    solution[n] = (rhs[n] - lower[n - 1] * solution[n - 1]) / divisor;
  }
  for (int n = N - 2; n >= 0; n--)
    solution[n] -= tmp[n + 1] * solution[n + 1];

  delete[] tmp;
  return true;

  // ToDo: get rid of the memory allocation, either by using a workspace pointer
  // or (better) by somehow avoiding using a temp-array altogether. Maybe it's
  // acceptable to destroy the rhs and/or the matrix, if this is needed to make
  // it work. Often, it's only constructed to solve a single system. See the
  // implementation for pentadiagonal systems below. It doesn't use any temp
  // arrays
}

template <class T>
bool rsSolvePentaDiagonalSystem(T *M, T *L, T *D, T *U, T *V, T *B, T *x,
                                int N) {
  // Gaussian elimination without pivot-search - we just always use D[i] as
  // pivot element:
  int i;
  T k;
  for (i = 0; i < N - 2; i++) {
    if (D[i] == T(0))
      return false; // encountered a zero pivot
    k = L[i] / D[i];
    D[i + 1] -= k * U[i];
    B[i + 1] -= k * B[i];
    U[i + 1] -= k * V[i];
    k = M[i] / D[i];
    L[i + 1] -= k * U[i];
    D[i + 2] -= k * V[i];
    B[i + 2] -= k * B[i];
  }
  if (D[i] == T(0))
    return false;
  k = L[i] / D[i]; // a final partial step outside the loop
  D[i + 1] -= k * U[i];
  B[i + 1] -= k * B[i];

  // Gaussian elimination is done - now do the backsubstitution to find the
  // solution vector:
  x[N - 1] = B[N - 1] / D[N - 1];
  x[N - 2] = (B[N - 2] - U[N - 2] * x[N - 1]) / D[N - 2];
  for (i = N - 3; i >= 0; i--)
    x[i] = (B[i] - U[i] * x[i + 1] - V[i] * x[i + 2]) / D[i];
  return true;
}

template <class T>
bool rsLinearAlgebra::rsChangeOfBasisColumnWise(T **A, T **B, T *va, T *vb,
                                                int N) {
  // coordinates of v in canonical basis:
  T *ve = new T[N];
  rsMatrixTools::matrixVectorMultiply(A, va, ve, N, N);

  // coordinates of v in basis B: A * va = ve = B * vb
  bool result = rsSolveLinearSystem(B, vb, ve, N);

  delete[] ve;
  return result;
}

template <class T>
bool rsLinearAlgebra::rsChangeOfBasisRowWise(T **A, T **B, T *va, T *vb,
                                             int N) {
  T *ve = new T[N];
  rsMatrixTools::transposedMatrixVectorMultiply(A, va, ve, N, N);
  //rsArrayTools::transposeSquareArray(B, N);
  bool result = rsSolveLinearSystem(B, vb, ve, N);
  //rsArrayTools::transposeSquareArray(B, N);
  delete[] ve;
  return result;
}

template <class T>
bool rsLinearAlgebra::rsChangeOfBasisMatrixColumnWise(T **A, T **B, T **C,
                                                      int N) {
  T **Bi;
  rsMatrixTools::allocateMatrix(Bi, N, N);
  rsMatrixTools::copyMatrix(B, Bi, N, N);
  bool result = rsInvertMatrix(Bi, N);              // Bi = B^-1
  rsMatrixTools::matrixMultiply(Bi, A, C, N, N, N); // C  = B^-1 * A
  rsMatrixTools::deallocateMatrix(Bi, N, N);
  return result;
}

template <class T>
bool rsLinearAlgebra::rsChangeOfBasisMatrixRowWise(T **A, T **B, T **C, int N) {
  T **Bi;
  rsMatrixTools::allocateMatrix(Bi, N, N);
  //rsArrayTools::transposeSquareArray(B, Bi, N);
  bool result = rsInvertMatrix(Bi, N); // Bi = B^-T
  rsMatrixTools::matrixMultiplySecondTransposed(Bi, A, C, N, N,
                                                N); // C  = B^-T * A^T
  rsMatrixTools::deallocateMatrix(Bi, N, N);
  return result;

  // alternative algorithm:
  // rsTransposeSquareArray(A, N);
  // rsTransposeSquareArray(B, N);
  // bool result = rsChangeOfBasisMatrixColumnWise(A, B, C, N);
  // rsTransposeSquareArray(A, N);
  // rsTransposeSquareArray(B, N);
  // return result;
}

} // namespace RAPT
