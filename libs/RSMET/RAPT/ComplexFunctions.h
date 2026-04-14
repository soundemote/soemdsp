#pragma once

#include "RealFunctions.h"
#include <complex>

namespace RAPT {

/** A collection of functions for complex arguments.

\todo: a few of the elliptic functions are still missing - implement them
\todo: remove the functions that are available in the standard library
(epx,log,pow,sin,etc.)

*/

/** Inverse complex Jacobian elliptic function cd with elliptic modulus k. */
template <class T> std::complex<T> rsAcdC(std::complex<T> w, T k);

/** Complex inverse cosine (arccosine) function. */
template <class T> std::complex<T> rsAcosC(std::complex<T> z);

/** Complex inverse hyperbolic cosine function. */
template <class T> std::complex<T> rsAcoshC(std::complex<T> z);

/** Complex inverse sine (arcsine) function. */
template <class T> std::complex<T> rsAsinC(std::complex<T> z);

/** Complex inverse hyperbolic sine function. */
template <class T> std::complex<T> rsAsinhC(std::complex<T> z);

/** Inverse complex Jacobian elliptic function sn with elliptic modulus k. */
template <class T> std::complex<T> rsAsnC(std::complex<T> w, T k);

/** Complex inverse tangent (arctangent) function. */
template <class T> std::complex<T> rsAtanC(std::complex<T> z);

/** Complex inverse hyperbolic tangent function. */
template <class T> std::complex<T> rsAtanhC(std::complex<T> z);

/** Complex Jacobian elliptic function cd with elliptic modulus k. */
template <class T> std::complex<T> rsCdC(std::complex<T> u, T k);

/** Complex cosine function. */
template <class T> std::complex<T> rsCosC(std::complex<T> z);

/** Complex hyperbolic cosine function. */
template <class T> std::complex<T> rsCoshC(std::complex<T> z);

/** Calculates the complex exponential of a complex number. */
// template<class T>
// std::complex<T> rsExpC(std::complex<T> z);

/** Calculates the natural (base e) logarithm of a complex number - as the
complex logarithm is a multifunction, it returns the principal value. */
// template<class T>
// std::complex<T> rsLogC(std::complex<T> z);
//  not needed anymore (is part of the standard library)

/** Raises a complex number to a complex power. */
template <class T>
std::complex<T> rsPowC(std::complex<T> basis, std::complex<T> exponent);

/** Complex sine function. */
template <class T> std::complex<T> rsSinC(std::complex<T> z);

/** Complex hyperbolic sine function. */
template <class T> std::complex<T> rsSinhC(std::complex<T> z);

/** std::complex<T> Jacobian elliptic function sn with elliptic modulus k. */
template <class T> std::complex<T> rsSnC(std::complex<T> u, T k);

/** Calculates the (primitive) square root of a complex number. The second
square root is obtained by using the negative value. */
// template<class T>
// std::complex<T> rsSqrtC(std::complex<T> z);

/** Complex tangent function. */
template <class T> std::complex<T> rsTanC(std::complex<T> z);

/** Complex hyperbolic tangent function. */
template <class T> std::complex<T> rsTanhC(std::complex<T> z);

/** Returns true if real or imaginary part (or both) are plus or minus infinity,
false otherwise. */
template <class T> inline bool rsIsInfinite(std::complex<T> z) {
  if (z.real() == RS_INF(T) || z.real() == -RS_INF(T) ||
      z.imag() == RS_INF(T) || z.imag() == -RS_INF(T))
    return true;
  else
    return false;
}

/** Returns the number of finite values in the complex array "a" of length "N".
 */
template <class T> int rsGetNumFiniteValues(std::complex<T> *a, int N);

/** Given an array "z" of "N" complex numbers, this function copies those values
from "z" to "zF" that are finite. The return value returns the number of copied
values, i.e. the effective (used) length of the returned array "zF". The
remaining values in "zF" are left as is. */
template <class T>
int rsCopyFiniteValues(const std::complex<T> *z, std::complex<T> *zF, int N);

/** Returns the product of finite values in the complex array "a" of length "N".
 */
template <class T>
std::complex<T> rsProductOfFiniteFactors(std::complex<T> *a, int N);

/** Given an array "z" of "N" complex numbers, this function copies those values
from "z" to "zL" that are in the left half-plane (including the imaginary axis).
The return value returns the number of copied values, i.e. the effective (used)
length of the returned array "zL". The remaining values in "zL" are left as is.
*/
template <class T>
int rsOnlyLeftHalfPlane(std::complex<T> *z, std::complex<T> *zL, int N);

/** Similar to onlyLeftHalfPlane - copies upper halfplane values (including real
 * axis). */
template <class T>
int rsOnlyUpperHalfPlane(std::complex<T> *z, std::complex<T> *zU, int N);

/** Zeros the imaginary parts in the passed complex array "z" when the absolute
value of the imaginary part is below the given threshold. */
template <class T>
void rsZeroNegligibleImaginaryParts(std::complex<T> *z, int length,
                                    T threshold);

/** Applies complex conjugation to all values in the buffer. */
template <class T> void rsConjugate(std::complex<T> *z, int length);

/** Compares two complex numbers for a less-than condition by first comparing
real parts and if they are equal, comparing imaginary parts. */
template <class T>
bool rsComplexLessByReIm(const std::complex<T> &left,
                         const std::complex<T> &right);

/** Like rsComplexLessByReIm but comparing imaginary parts first */
template <class T>
bool rsComplexLessByImRe(const std::complex<T> &left,
                         const std::complex<T> &right);

/** Sorts an array of complex numbers according to the less-than criterion
defined by the function complexLessByReIm. */
template <class T>
void rsSortComplexArrayByReIm(std::complex<T> *z, int length);

/** Returns true, when for all values in the passed z-array:
|z[i].im| <= |relativeTolerance*z[i].re|. That means that the absolute value of
the imaginary part must be a factor "relativeTolerance" smaller than the real
part for a number to be considered purely real. */
template <class T>
bool rsAreAllValuesReal(std::complex<T> *z, int length,
                        T relativeTolerance = T(0));

/** Returns true, if z[i] and z[i+1] are complex conjugates for all i from
 * 0..length-2. */
template <class T>
bool rsAreNeighborsConjugates(std::complex<T> *z, int length,
                              T absoluteTolerance = T(0));

/* Computes the 2 roots (zeros) of the quadratic equation x^2 + p*x + q = 0. */
template <class T>
inline void solveQuadraticEquation(std::complex<T> p, std::complex<T> q,
                                   std::complex<T> &root1,
                                   std::complex<T> &root2) {
  std::complex<T> tmp = sqrt(0.25 * p * p - q);
  root1 = -0.5 * p + tmp;
  root2 = -0.5 * p - tmp;
}

template <class T> std::complex<T> rsAcdC(std::complex<T> w, T k) {
  if (k == 1.0) {
    RS_DEBUG_BREAK; // k may not be equal to 1
    return std::complex<T>(0.0, 0.0);
  }

  int M = 7; // fixed number of Landen iterations
  T v[7];    // array to store the vector of descending elliptic moduli
  rsLanden(k, M, v);

  int n;
  T v1;
  for (n = 0; n < M; n++) {
    if (n == 0)
      v1 = k;
    else
      v1 = v[n - 1];
    // w = w / (T(1) + rsSqrtC(T(1) - w*w * v1*v1)) * 2.0/(1.0+v[n]);
    w = w / (T(1) + sqrt(T(1) - w * w * v1 * v1)) * T(2) / (T(1) + v[n]);
  }

  std::complex<T> u;
  if (w == T(1))
    u = 0.0;
  else
    u = T(2.0 / kPI) * rsAcosC(w);

  T K, Kprime;
  RAPT::rsEllipticIntegral(k, &K, &Kprime);
  T R = Kprime / K;

  u.real(rsSrem(u.real(), T(4)));
  u.imag(rsSrem(u.imag(), T(2 * R)));

  return u;
}

template <class T> std::complex<T> rsAcosC(std::complex<T> z) {
  std::complex<T> tmp = z + std::complex<T>(T(0), T(1)) * sqrt(T(1) - z * z);
  tmp = log(tmp);
  return tmp * std::complex<T>(0.0, -1.0);
}

template <class T> std::complex<T> rsAcoshC(std::complex<T> z) {
  std::complex<T> tmp = z + rsSqrtC(z - 1.0) * rsSqrtC(z + 1.0);
  return log(tmp);
}

template <class T> std::complex<T> rsAsinC(std::complex<T> z) {
  std::complex<T> tmp = z * std::complex<T>(0.0, 1.0) + rsSqrtC(1.0 - z * z);
  tmp = log(tmp);
  return tmp * std::complex<T>(0.0, -1.0);
}

template <class T> std::complex<T> rsAsinhC(std::complex<T> z) {
  std::complex<T> tmp = z + rsSqrtC(z * z + 1.0);
  return log(tmp);
}

template <class T> std::complex<T> rsAsnC(std::complex<T> w, T k) {
  return T(1) - rsAcdC(w, k);
}

template <class T> std::complex<T> rsAtanC(std::complex<T> z) {
  std::complex<T> tmp =
      (std::complex<T>(0.0, 1.0) + z) / (std::complex<T>(0.0, 1.0) - z);
  tmp = log(tmp);
  return tmp * std::complex<T>(0.0, -0.50);
}

template <class T> std::complex<T> rsAtanhC(std::complex<T> z) {
  std::complex<T> tmp =
      (std::complex<T>(1.0, 0.0) + z) / (std::complex<T>(1.0, 0.0) - z);
  tmp = rsLogC(tmp);
  return 0.5 * tmp;
}

template <class T> std::complex<T> rsCdC(std::complex<T> u, T k) {
  int M = 7; // fixed number of Landen iterations
  T v[7];    // array to store the vector of descending elliptic moduli
  rsLanden(k, M, v);

  // initialization:
  std::complex<T> w = rsCosC(u * T(kPI / 2.0));

  // ascending Landen/Gauss transformation:
  for (int n = M - 1; n >= 0; n--)
    w = (T(1) + v[n]) * w / (T(1) + v[n] * w * w);

  return w;
}

template <class T> std::complex<T> rsCosC(std::complex<T> z) {
  std::complex<T> tmp = exp(std::complex<T>(0.0, 1.0) * z); // tmp = exp(i*z);
  tmp += exp(std::complex<T>(0.0, -1.0) * z); // tmp = exp(i*z) + exp(-i*z)
  return T(0.5) * tmp;
}

template <class T> std::complex<T> rsCoshC(std::complex<T> z) {
  return T(0.5) * (expC(z) + expC(-z));
}

/*
template<class T>
std::complex<T> rsExpC(std::complex<T> z)
{
  std::complex<T> result;
  T tmp = exp(z.real());
  rsSinCos(z.imag(), &result.imag(), &result.real());
  result.real() *= tmp;
  result.imag() *= tmp;
  return result;
}

template<class T>
std::complex<T> rsLogC(std::complex<T> z)
{
  std::complex<T> tmp;
  tmp.real() = log(z.getRadius());
  tmp.imag() = z.getAngle();
  return tmp;
}
*/

template <class T>
std::complex<T> rsPowC(std::complex<T> basis, std::complex<T> exponent) {
  if (basis != std::complex<T>(0.0, 0.0))
    return exp(exponent * rsLogC((std::complex<T>)basis));
  else // basis == 0
  {
    if (exponent.real() == 0.0 && exponent.imag() == 0.0)
      return std::complex<T>(1.0, 0.0);
    else if (exponent.real() < 0.0 && exponent.imag() == 0.0)
      return std::complex<T>(RS_INF(T), 0.0);
    else if (exponent.real() > 0.0 && exponent.imag() == 0.0)
      return std::complex<T>(0.0, 0.0);
    else
      return exp(exponent * rsLogC((std::complex<T>)basis));
  }
}

template <class T> std::complex<T> rsSinC(std::complex<T> z) {
  std::complex<T> tmp = exp(std::complex<T>(0.0, 1.0) * z); // tmp = exp(i*z);
  tmp -= exp(std::complex<T>(0.0, -1.0) * z); // tmp = exp(i*z) - exp(-i*z)
  return tmp * std::complex<T>(0.0, -0.5);
}

template <class T> std::complex<T> rsSinhC(std::complex<T> z) {
  return T(0.5) * (expC(z) - expC(-z));
}

template <class T> std::complex<T> rsSnC(std::complex<T> u, T k) {
  int M = 7; // fixed number of Landen iterations
  T v[7];    // array to store the vector of descending elliptic moduli
  rsLanden(k, M, v);

  // initialization:
  std::complex<T> w = rsSinC(u * T(kPI / 2.0));

  // ascending Landen/Gauss transformation:
  for (int n = M - 1; n >= 0; n--)
    w = (T(1) + v[n]) * w / (T(1) + v[n] * w * w);

  return w;
}

/*
template<class T>
std::complex<T> RSLib::rsSqrtC(std::complex<T> z)
{
double r = rsSqrt(z.getRadius());
double p = 0.5*(z.getAngle());
double s, c;
sinCos(p, &s, &c);
return std::complex<T>(r*c, r*s);            // re = r*cos(p), im = r*sin(p)
}
// drag the implementation for Complex.h over here
*/

template <class T> std::complex<T> rsTanC(std::complex<T> z) {
  return sinC(z) / cosC(z);
}

template <class T> std::complex<T> rsTanhC(std::complex<T> z) {
  return sinhC(z) / coshC(z);
}

template <class T> int rsGetNumFiniteValues(std::complex<T> *a, int N) {
  int result = 0;
  for (int n = 0; n < N; n++) {
    if (!rsIsInfinite(a[n]))
      result++;
  }
  return result;
}

template <class T>
int rsCopyFiniteValues(const std::complex<T> *z, std::complex<T> *zF, int N) {
  int m = 0;
  for (int n = 0; n < N; n++) {
    if (!rsIsInfinite(z[n])) {
      zF[m] = z[n];
      m++;
    }
  }
  return m;
}

template <class T>
std::complex<T> rsProductOfFiniteFactors(std::complex<T> *a, int N) {
  std::complex<T> result = std::complex<T>(1.0, 0.0);
  for (int n = 0; n < N; n++) {
    if (!rsIsInfinite(a[n]))
      result *= a[n];
  }
  return result;
}

template <class T>
int rsOnlyLeftHalfPlane(std::complex<T> *z, std::complex<T> *zL, int N) {
  int m = 0;
  for (int n = 0; n < N; n++) {
    if (z[n].real() <= 0.0) {
      zL[m] = z[n];
      m++;
    }
  }
  return m;
}

template <class T>
int rsOnlyUpperHalfPlane(std::complex<T> *z, std::complex<T> *zU, int N) {
  int m = 0;
  for (int n = 0; n < N; n++) {
    if (z[n].imag() >= 0.0) {
      zU[m] = z[n];
      m++;
    }
  }
  return m;
}

template <class T>
void rsZeroNegligibleImaginaryParts(std::complex<T> *z, int length,
                                    T threshold) {
  for (int n = 0; n < length; n++) {
    if (fabs(z[n].imag()) < threshold)
      z[n].imag(0);
  }
}

template <class T> void rsConjugate(std::complex<T> *z, int length) {
  for (int n = 0; n < length; n++)
    z[n].imag(-z[n].imag());
}

template <class T>
bool rsComplexLessByReIm(const std::complex<T> &left,
                         const std::complex<T> &right) {
  if (left.real() < right.real())
    return true;
  else if (right.real() < left.real())
    return false;
  else {
    // real parts are equal - compare by imaginary parts:
    if (left.imag() < right.imag())
      return true;
    else if (right.imag() < left.imag())
      return false;
    else
      return false; // both complex numbers are equal
  }
}

template <class T>
bool rsComplexLessByImRe(const std::complex<T> &left,
                         const std::complex<T> &right) {
  if (left.imag() < right.imag())
    return true;
  else if (right.imag() < left.imag())
    return false;
  else {
    // imaginary parts are equal - compare by real parts:
    if (left.real() < right.real())
      return true;
    else if (right.real() < left.real())
      return false;
    else
      return false; // both complex numbers are equal
  }
}

template <class T>
void rsSortComplexArrayByReIm(std::complex<T> *z, int length) {
  rsHeapSort(z, length, rsComplexLessByReIm<T>);
}

template <class T>
bool rsAreAllValuesReal(std::complex<T> *z, int length, T relativeTolerance) {
  for (int i = 0; i < length; i++) {
    if (rsAbs(z[i].imag()) > rsAbs(relativeTolerance * z[i].real()))
      return false;
  }
  return true;
}

template <class T>
bool rsAreNeighborsConjugates(std::complex<T> *z, int length, T tol) {
  for (int i = 0; i < length - 1; i += 2) {
    if (z[i].real() == RS_INF(T) && z[i + 1].real() == RS_INF(T))
      continue;
    if (abs(z[i].real() - z[i + 1].real()) > tol)
      return false;
    if (abs(z[i].imag() + z[i + 1].imag()) > tol)
      return false;
  }
  return true;
}
} // namespace RAPT
