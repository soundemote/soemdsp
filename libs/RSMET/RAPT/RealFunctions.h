#pragma once

#include <string_view>
#include <complex>

#include "ArrayTools.h"

#include "soemdsp/sehelper.hpp"
#include "../RSLib/MathBasics.inl"
#include "IntegerFunctions.h"
#include "soemdsp/semath.hpp"

template <typename t> int rsRoundToInt(t v) { roundToInt(v); }

namespace RAPT {

    template <typename t>
    void rsSinCos(t x, t *sinResult, t *cosResult) {
      *sinResult = sin(x);
      *cosResult = cos(x);
    }


/** This file contains functions for real arguments that are either themselves
elementary functions (such as sin, exp, log, pow, tanh, etc. - although, we
don't duplicate these C-standard functions here) or can be expressed as
combinations thereof (involving composition and arithmetic operations), like
gauss, log2, cheby, etc...  */

// \todo templatize them, wrap into class rsRealFunctions (do the same for
// integer functions) \todo maybe have special polynomials like cheby in an
// extra file - but maybe not \todo move some of them into the cpp file -
// un-inline

/** A table of values of the factorial of some integer number n, that is: n!,
where n should be between 0...31 (inclusive) - the values are tabulated and any
value outside the range 0...31 will be an access violation. */
static const int rsNumFactorials = 32;
// static long double rsFactorials[rsNumFactorials] =
static double rsFactorials[rsNumFactorials] = {
    1.0,
    1.0,
    2.0,
    6.0,
    24.0,
    120.0,
    720.0,
    5040.0,
    40320.0,
    362880.0,
    3628800.0,
    39916800.0,
    479001600.0,
    6227020800.0,
    87178291200.0,
    1307674368000.0,
    20922789888000.0,
    355687428096000.0,
    6402373705728000.0,
    121645100408832000.0,
    2432902008176640000.0,
    51090942171709440000.0,
    1124000727777607680000.0,
    25852016738884976640000.0,
    620448401733239439360000.0,
    15511210043330985984000000.0,
    403291461126605635584000000.0,
    10888869450418352160768000000.0,
    304888344611713860501504000000.0,
    8841761993739701954543616000000.0,
    265252859812191058636308480000000.0,
    8222838654177922817725562880000000.0};
// hmm..."long double"? ...i guess that's just the same as double in a 64-bit
// build but 80 bit in a 32-bit build - maybe we should make it consistent and
// just use double? maybe check the numerical errors in both cases the long
// double variant doesn't compile anymore when we instantiate rsPolynomial with
// std::complex<double>, so it has been changed to double
// ToDo:
// -check, if the large values still make sense or if we get overflow problems
// -maybe move to some Constants.h file

/** A table of reciprocal values of the factorial of some integer number n, that
is: 1/n!, where n should be between 0...31 (inclusive) - the values are
tabulated and any value outside the range 0...31 will be an access violation.
These numbers are useful for making Taylor series approximations. */
static const int rsNumInverseFactorials = 32;
// static long double rsInverseFactorials[rsNumInverseFactorials] =
static double rsInverseFactorials[rsNumInverseFactorials] = {
    1.0,
    1.0,
    1.0 / 2.0,
    1.0 / 6.0,
    1.0 / 24.0,
    1.0 / 120.0,
    1.0 / 720.0,
    1.0 / 5040.0,
    1.0 / 40320.0,
    1.0 / 362880.0,
    1.0 / 3628800.0,
    1.0 / 39916800.0,
    1.0 / 479001600.0,
    1.0 / 6227020800.0,
    1.0 / 87178291200.0,
    1.0 / 1307674368000.0,
    1.0 / 20922789888000.0,
    1.0 / 355687428096000.0,
    1.0 / 6402373705728000.0,
    1.0 / 121645100408832000.0,
    1.0 / 2432902008176640000.0,
    1.0 / 51090942171709440000.0,
    1.0 / 1124000727777607680000.0,
    1.0 / 25852016738884976640000.0,
    1.0 / 620448401733239439360000.0,
    1.0 / 15511210043330985984000000.0,
    1.0 / 403291461126605635584000000.0,
    1.0 / 10888869450418352160768000000.0,
    1.0 / 304888344611713860501504000000.0,
    1.0 / 8841761993739701954543616000000.0,
    1.0 / 265252859812191058636308480000000.0,
    1.0 / 8222838654177922817725562880000000.0};

/** Inverse hyperbolic cosine. The argument must be >= 1. For the general case,
use the complex version acosh_c defined in ComplexFunctions.h */
inline double rsAcosh(double x);

/** Symmetrized inverse hyperbolic cosine. */
inline double rsAcoshs(double x);

/** Inverse hyperbolic sine. */
inline double rsAsinh(double x);

/** Returns -1.0 if x is below low, 0.0 if x is between low and high and 1.0 if
x is above high. */
inline double rsBelowOrAbove(double x, double low, double high);

/** Binomial coefficient for real n. */
inline double rsBinomialCoefficientReal(double n, int k);

// Bessel function J_n(x) */
// inline double besselj(double x, double order);

/** Returns the value of the n-th chebychev polynomial. */
inline double rsCheby(double x, double n);

/** Returns the value of the n-th chebychev polynomial. */
inline double rsCheby(double x, int n);

/** Returns the next greater integer number. */
// inline double ceil(double x);

/** Center clips the x at threshold t. */
inline double rsCenterClip(double x, double t);

/** Clips x into the range min...max. */
// template <class T>
// inline T rsClip(T x, T min, T max);

/** Approximates cos^2(0.5*pi*x) over the domain -1.0...+1.0 via
y = a*(1-x^2)^2 + (1-a)*(1-x^2)^3
with a minimax-optimized value for a. */
inline double rsCosSquaredApprox(double x);

/** Hyperbolic cosine. */
template <class T> T rsCosh(T x);

/** Hyperbolic cotangent, equals: 1 / hyperbolic tangent. */
template <class T> T rsCoth(T x);

/** Hyperbolic cosecant, equals: 1 / hyperbolic sine. */
template <class T> T rsCsch(T x);

/** Converts an angle in degrees into radiant. */
template <class T> inline T rsDegreeToRadiant(T degrees);

/** Distance of x to a multiple of period p */
template <class T> T rsDistanceToMultipleOf(T x, T p);

/** Evaluates the complete elliptic integral of the first kind with elliptic
modulus k. K will be assigned to the quarter period K(k) and Kprime will be
assigned to the quarter period K'(k) = K(k'), k' = sqrt(1-k^2). M is the number
of Landen iterations which determines the precision of the calculation. */


/** Computes a sin/cos based equal power crossfade according to the value of x.
The gain for the first signal will be stored in gain1, the gain for the second
signal in gain2. If the range of x is not 0.0...1.0, use optional the xMin and
xMax arguments to specify the range. For example, if x is a panning value
between -100...+100, a constant power pan can be achieved via
equalPowerGainFactors(x, gainL, gainR, -100.0, 100.0), or if x is a dry/wet
control between 0.0...100.0: equalPowerGainFactors(x, gainDry, gainWet, 0.0,
100.0). \todo: (maybe) move to RSLib
*/
inline void rsEqualPowerGainFactors(double x, double *gain1, double *gain2,
                                       double xMin = 0.0, double xMax = 1.0);

/** Returns the Euclidean distance between points at coordinates (x1,y1),
 * (x2,y2). */
inline double rsEuclideanDistance(double x1, double y1, double x2,
                                     double y2);

/** Evaluates the quartic polynomial y = a4*x^4 + a3*x^3 + a2*x^2 + a1*x + a0 at
 * x. */
inline double rsEvaluateQuartic(double x, double a0, double a1, double a2,
                                   double a3, double a4);

/** Calculates the exponential function with base 10. */
inline double rsExp10(double x);

/** Calculates the exponential function with base 2. */
inline double rsExp2(double x);

/** foldover at the specified value */
inline double rsFoldOver(double x, double min, double max);

/** Returns the fractional part of x. */
inline double rsFrac(double x);

// gamma function
// double rsGamma(double x);

/** Gaussian function (aka normal distribution) with mean mu and standard
 * deviation sigma. */
inline double rsGauss(double x, double mu, double sigma);

/** Performs M iterations of the Landen transformation of an elliptic modulus k
and returns the results in the array v which must be of length M. */
template <class T> void rsLanden(T k, int M, T *v) {
  // performs M iterations of the Landen transformation of an elliptic modulus k
  // and returns the results in the array v which must be of length M+1
  int n;
  if (k == 0.0 || k == 1.0) {
    v[0] = k;
    for (n = 1; n < M; n++)
      v[n] = 0.0;
    return;
  } else {
    for (n = 0; n < M; n++) {
      k = (k / (T(1) + sqrt(T(1) - k * k)));
      k *= k;
      v[n] = k;
    }
  }
}

template <class T> void rsEllipticIntegral(T k, T *K, T *Kprime, int M = 7) {
  T kmin = T(1e-6);
  T kmax = sqrt(1 - kmin * kmin);
  T kp, L;
  T *v = new T[M];
  T *vp = new T[M];
  int n;

  if (k == 1.0)
    *K = RS_INF(T);
  else if (k > kmax) {
    kp = sqrt(1 - k * k);
    L = -log(kp / 4);
    *K = L + (L - 1) * kp * kp / 4;
  } else {
    rsLanden(k, M, v);
    for (n = 0; n < M; n++)
      v[n] += 1.0;
    //*K = rsArrayTools::product(v, M) * T(0.5 * rsPI);
  }

  if (k == 0.0)
    *Kprime = RS_INF(T);
  else if (k < kmin) {
    L = -log(k / 4);
    *Kprime = L + (L - 1) * k * k / 4;
  } else {
    kp = sqrt(1 - k * k);
    rsLanden(kp, M, vp);
    for (n = 0; n < M; n++)
      vp[n] += 1.0;
    //*Kprime = rsArrayTools::product(vp, M) * T(0.5 * rsPI);
  }

  delete[] v;
  delete[] vp;
}

/** Just returns the input value. The purpose of this function is to be used,
when a function pointer is required, but it should behave neutrally. */
// template<class T>
// T rsIdentity(T x);

/** Calculates the logistic function with slope parameter b. */
inline double rsLogistic(double x, double b);

/** Calculates the logarithm to base 2. */
inline double rsLog2(double x);

/** Calculates logarithm to an arbitrary base b. */
inline double rsLogB(double x, double b);

/** Minkowski distribution - a generalization of the gaussian distribution,
where the exponent k inside the exponential is an addtional parameter. For k=2,
it reduces to the Gaussian distribution. k<2 makes the distribution more spikey
and the tails longer, values k>2 make the distribution more rectangular. */
inline double rsMinkowski(double x, double mu, double sigma, double k);

/** Normalized sinc function: nSinc(x) = sin(pi*x)/(pi*x). It has zero crossings
at the integers (except at 0 where the value is 1) and the value of the integral
from minus to plus infinity equals 1. */
template <class T> T rsNormalizedSinc(T x);

/** Calculates the power of the absolute-value of some number and re-applies its
original sign afterwards, if base==0, the result will be 0 also. */
inline double rsPowBipolar(double base, double exponent);

/** Power function for intergers. Multiplies the base exponent-1 time with
itself and returns the result. */
// inline int powInt(int base, int exponent);

/** Generates a 2*pi periodic pulse wave with a duty-cycle (high level fraction)
of d. The DC amount of -2*d is compensated such that the resulting waveform is
DC-free. */
inline double rsPulseWave(double x, double d);

/** Quantizes x to the interval i. */
inline double rsQuant(double x, double i);

/** Quantizes x (assumed to be in -1.0 - 1.0) to a grid which represents a
resolution of nBits bits. */
inline double rsQuantToBits(double x, double nBits);

/** Returns the absolute value of x. */
// inline double rsAbs(double x);

/** Converts an angle in radiant into degrees. */
inline double rsRadiantToDegree(double radiant);

/** Returns the nearest integer (as double, without typecast). */
inline double rsRound(double x);

/** Calculates the sine of x assuming that x is in the range 0...pi/2 - this is
not checked for. */
inline double rsSin(double x, int numTerms = 16);

/** Generates a 2*pi periodic saw wave. */
inline double rsSawWave(double x);
// move to AudioFunctions

/** Hyperbolic secant, equals: 1 / hyperbolic cosine. */
template <class T> T rsSech(T x);

/** Returns the sign of x as double. */
// inline double rsSign(double x);
//  remove - replaced by templated version in MathBasics.h/inl

/** Calculates sine and cosine of x - this is more efficient than calling sin(x)
and cos(x) seperately. */
// inline void rsSinCos(double x, double* sinResult, double* cosResult);
// inline void rsSinCos(float x, float* sinResult, float* cosResult);

/** Unnormalized sinc function: sinc(x) = sin(x)/x. It has zero crossings at
multiples of pi (except at 0*pi where the value is 1) and the value of the
integral from minus to plus infinity equals pi. */
template <class T> T rsSinc(T x);

/** Hyperbolic sine. */
template <class T> T rsSinh(T x);

/** Hyperbolic sine and cosine at the same time. */
template <class T> void rsSinhCosh(T x, T *sinhResult, T *coshResult);

/** Calculates a parabolic approximation of the sine and cosine of x. */
inline void rsSinCosApprox(double x, double *sinResult, double *cosResult);

/** Computes (an approximation of) the sine-integral of x, that is: y = Si(x) */
template <class T> T rsSineIntegral(T x);

/** Applies a softclipping with a tanh shaped transtion region between the
linear part and the clipping value. */
inline double rsSoftClip(double x, double lowClamp = -1.0,
                            double highClamp = 1.0, double lowKnee = 0.1,
                            double highKnee = 0.1);

/** Generates a 2*pi periodic square wave. */
inline double rsSqrWave(double x);

/** Same syntax as fmod but brings the result into the symmetric interval
 * -y/2...y/2 */
template <class T> inline T rsSrem(T x, T y);

/** The unit step function - returns 1 for x>=0, 0 else. */
inline double rsStep(double x);

/** Hyperbolic tangent. */
template <class T> T rsTanh(T x);

/** Rational approximation of the hyperbolic tangent. */
inline double rsTanhApprox(double x);

/** Generates a 2*pi periodic triangle wave. */
inline double rsTriWave(double x);

//-----------------------------------------------------------------------------------------------
// implementation:

inline double rsSqrt(double x) { return sqrt(x); }

inline double rsAcosh(double x) {
  if (x >= 1)
    return log(x + rsSqrt(x * x - 1));
  else {
    RS_DEBUG_BREAK; // argument must be >= 1, use the complex version otherwise
    return 0.0;
  }
}

inline double rsAcoshs(double x) {
  if (fabs(x) >= 1)
    return log(fabs(x + rsSqrt(x * x - 1)));
  else {
    RS_DEBUG_BREAK; // fabs(x) must be >= 1, use the complex version otherwise
    return 0.0;
  }
}

inline double rsAsinh(double x) { return log(x + rsSqrt(x * x + 1)); }

inline double rsBelowOrAbove(double x, double low, double high) {
  if (x < low)
    return -1.0;
  else if (x > high)
    return 1.0;
  else
    return 0.0;
}

// inline double besselj(double x, double order) { return jv(order, x); }

inline double rsBinomialCoefficientReal(double n, int k) {
  if (k == 0)
    return 1.0;
  else {
    double num = n;
    for (int i = 1; i < k; i++)
      num *= (n - (double)i);
    return num / (double)RAPT::rsFactorial(k);
  }
}

inline double rsCheby(double x, double n) {
  if (fabs(x) <= 1.0)
    return cos(n * acos(x));
  else if (x > 0.0)
    return cosh(n * rsAcosh(x));
  else
    return pow(-1.0, rsRoundToInt(n)) * cosh(n * rsAcosh(-x));
}

inline double rsCheby(double x, int n) {
  double t0 = 1.0;
  double t1 = x;
  double tn = 1.0;
  for (int i = 0; i < n; i++) {
    tn = 2 * x * t1 - t0;
    t0 = t1;
    t1 = tn;
  }
  return t0;
}

/*
inline double rsCeil(double x)
{
  //double frac = x - floor(x);
  return x + (1-frac(x));
}
*/

/*
template <class T>
inline T rsClip(T x, T min, T max)
{
  if( x > max )
    return max;
  else if ( x < min )
    return min;
  else return x;
}
*/

template <class T> T rsCosh(T x) {
  T c = T(0.5) * exp(x);
  return c + (T(0.25) / c);
}

inline double rsCosSquaredApprox(double x) {
  // approximant: y = a*(1-x^2)^2 + (1-a)*(1-x^2)^3;

  double a = 0.564765625; // ...minimax-optimized
  double b = 1.0 - a;
  double y1 = 1.0 - x * x; // y1 = (1-x^2)
  double y2 = y1 * y1;     // y2 = (1-x^2)^2
  y1 = y1 * y2;            // y1 = (1-x^2)^3
  return a * y2 + b * y1;
}

template <class T> T rsCoth(T x) {
  if (x > 350.0) // avoids internal overflow
    return 1.0;
  T c = exp(2 * x);
  return (c + 1) / (c - 1);
}

template <class T> T rsCsch(T x) {
  if (fabs(x) > 709.0) // avoids internal overflow and denormal results
    return 0.0;
  T c = exp(x);
  return (2 * c) / (c * c - 1);
}

template <class T> inline T rsDegreeToRadiant(T degrees) {
  return T(rsPI / 180.0) * degrees;
}

// distance of x to a multiple of period p
template <class T> T rsDistanceToMultipleOf(T x, T p) {
  rsAssert(p > 0);
  x = rsAbs(x);
  x = fmod(x, p);
  return rsMin(x, p - x);
}

inline void rsEqualPowerGainFactors(double x, double *gain1, double *gain2,
                                       double xMin, double xMax) {
  double tmp = RSLib::rsLinToLin(x, xMin, xMax, 0.0, rsPI / 2.0);
  rsSinCos(tmp, gain2, gain1);
}

inline double rsEuclideanDistance(double x1, double y1, double x2,
                                     double y2) {
  return rsSqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

inline double rsEvaluateQuartic(double x, double a0, double a1, double a2,
                                   double a3, double a4) {
  double x2 = x * x;
  return x * (a3 * x2 + a1) + x2 * (a4 * x2 + a2) + a0;
}

inline double rsExp10(double x) { return std::exp(LN10 * x); }

inline double rsExp2(double x) { return std::exp(LN2 * x); }

inline double rsFoldOver(double x, double min, double max) {
  if (x > max)
    return max - (x - max);
  else if (x < min)
    return min - (x - min);
  else
    return x;
}

inline double rsFrac(double x) { return x - floor(x); }

inline double rsGauss(double x, double mu, double sigma) {
  return (1.0 / (rsSqrt(2 * rsPI) * sigma)) *
         std::exp(-((x - mu) * (x - mu)) / (2 * sigma * sigma));
}

inline double rsLogistic(double x, double b) {
  return 1.0 / (1 + std::exp(-b * x));
}

/*
inline double rsLog10(double x)
{
  return ONE_OVER_LN10 * log(x);
}
*/

inline double rsLog2(double x) { return LN2_INV * log(x); }

inline double rsLogB(double x, double b) { return log(x) / log(b); }

inline double rsMinkowski(double x, double mu, double sigma, double k) {
  return (1 / (rsSqrt(2 * rsPI) * sigma)) *
         std::exp(-(pow(fabs(x - mu), k)) / (2 * pow(sigma, k)));
}

inline double rsPowBipolar(double base, double exponent) {
  if (base > 0.0)
    return pow(base, exponent);
  else if (base < 0.0)
    return RSLib::rsSign(base) * pow(fabs(base), exponent);
  else
    return 0.0;
}

inline double rsPulseWave(double x, double d) {
  double tmp = fmod(x, 2 * rsPI);
  if (tmp < rsPI * d || tmp >= 2 * rsPI - rsPI * d)
    return 1.0 + 2 * d;
  else
    return -1.0 + 2 * d;
}

/*
inline int rsPowInt(int base, int exponent)
{
  int result = base;
  for(int p=1; p<exponent; p++)
    result *= base;
  return result;
}
*/

//inline double rsQuant(double x, double i) {
//  if (i <= RS_MIN(double)) {
//    RS_DEBUG_BREAK;
//    return x;
//  }
//
//  double tmp = fmod(x, i);
//
//  if (x >= 0) {
//    if (tmp < 0.5 * i)
//      return x - tmp;
//    else
//      return x - tmp + i;
//  } else {
//    if (tmp < -0.5 * i)
//      return x - tmp - i;
//    else
//      return x - tmp;
//  }
//}

inline double rsQuantToBits(double x, double nBits) {
  double interval = 2 / pow(2.0, nBits);
  return rsQuant(x, interval);
}

/*
inline double rsAbs(double x)  // actually slower than fabs - grrrr
{
  unsigned long long *pInt = reinterpret_cast<unsigned long long *> (&x);
  *pInt &= 0x7FFFFFFFFFFFFFFFULL;
  return *reinterpret_cast<double *> (pInt);
}
*/

inline double rsRadiantToDegree(double radiant) {
  return (180.0 / rsPI) * radiant;
}

inline double rsRound(double x) {
  if (x - floor(x) >= 0.5)
    return ceil(x);
  else
    return floor(x);
}

/*
inline int rosic::rsRoundToInt(double const x)
{
  int n;
#if defined(__unix__) || defined(__GNUC__)
    // 32-bit Linux, Gnu/AT&T syntax:
    __asm ("fldl %1 \n fistpl %0 " : "=m"(n) : "m"(x) : "memory" );
#else
    // 32-bit Windows, Intel/MASM syntax:
    __asm fld qword ptr x;
    __asm fistp dword ptr n;
#endif
    return n;
  }
  */

/*
inline int rosic::roundToIntSSE2(double const x)
{
  return _mm_cvtsd_si32(_mm_load_sd(&x));
}
*/

template <class T> T rsSech(T x) {
  if (fabs(x) > 709.0) // avoids internal overflow and denormal results
    // if( fabs(x) > 354.0 )    // ...GCC on linux needs lower threshold - but
    // we should handle that in the test
    return 0.0;
  T c = exp(x);
  return (2 * c) / (c * c + 1);
}

/*
inline void rsSinCos(double x, double* sinResult, double* cosResult)
{
#  if defined RS_COMPILER_MS_X86
  double s, c;     // do we need these intermediate variables?
  __asm fld x
  __asm fsincos
  __asm fstp c
  __asm fstp s
  *sinResult = s;
  *cosResult = c;
#  else
  *sinResult = sin(x);
  *cosResult = cos(x);
#  endif
   // here'S some info, how to (not) optimize this in X64 builds:
   // http://www.gamedev.net/topic/598105-how-to-implement-sincos-on-vc-64bit/


    //// old:
    //#ifndef RS_COMPILER_MICROSOFT
    //  *sinResult = sin(x);
    //  *cosResult = cos(x);
    //#else
    //  double s, c;     // do we need these intermediate variables?
    //  __asm fld x
    //  __asm fsincos
    //  __asm fstp c
    //  __asm fstp s
    //  *sinResult = s;
    //  *cosResult = c;
    //#endif
}
*/

template <class T> T rsSinh(T x) {
  T c = T(0.5) * exp(x);
  return c - (T(0.25) / c);
}

template <class T> void rsSinhCosh(T x, T *s, T *c) {
  T ep = T(0.5) * exp(x);
  T em = T(0.25) / ep;
  *s = ep - em;
  *c = ep + em;
}

inline void rsSinCosApprox(double x, double *sinResult, double *cosResult) {
  static const double c = 0.70710678118654752440;

  // restrict input x to the range 0.0...2*PI:
  while (x > 2.0 * rsPI)
    x -= 2 * rsPI;
  while (x < 0.0)
    x += 2 * rsPI;

  if (x < rsPI / 2) {
    double tmp1 = x;
    double tmp2 = (2 / rsPI) * tmp1 - 0.5;
    double tmp3 = (2 - 4 * c) * tmp2 * tmp2 + c;
    *sinResult = tmp3 + tmp2;
    *cosResult = tmp3 - tmp2;
  } else if (x < rsPI) {
    double tmp1 = (x - rsPI / 2);
    double tmp2 = 0.5 - (2 / rsPI) * tmp1;
    double tmp3 = (2 - 4 * c) * tmp2 * tmp2 + c;
    *sinResult = tmp2 + tmp3;
    *cosResult = tmp2 - tmp3;
  } else if (x < 1.5 * rsPI) {
    double tmp1 = (x - rsPI);
    double tmp2 = (2 / rsPI) * tmp1 - 0.5;
    double tmp3 = (4 * c - 2) * tmp2 * tmp2 - c;
    *sinResult = tmp3 - tmp2;
    *cosResult = tmp3 + tmp2;
  } else {
    double tmp1 = (x - 1.5 * rsPI);
    double tmp2 = (2 / rsPI) * tmp1 - 0.5;
    double tmp3 = (2 - 4 * c) * tmp2 * tmp2 + c;
    *sinResult = tmp2 - tmp3;
    *cosResult = tmp2 + tmp3;
  }
}

inline double rsSin(double x, int numTerms) {
  /*
  double accu = 1.0;
  double n    = 1.0;
  double nSq  = 1.0;
  double xSq  = x*x;
  double piSq = rsPI*rsPI;
  for(int i=1; i<=numTerms; i++)
  {
    accu *= (1.0 - xSq / (piSq*nSq));
    n    += 1.0;
    nSq   = n*n;
  }
  return x*accu;
  */

  long double accu = 0.0;
  long double num = x;
  long double xSq = x * x;
  for (int k = 0; k < numTerms; k += 2) {
    accu += rsInverseFactorials[2 * k + 1] * num;
    num *= xSq;
    accu -= rsInverseFactorials[2 * k + 3] * num;
    num *= xSq;
  }
  return (double)accu;

  /*
  double xSq   = x*x;
  double num1  = x;      // first term is x^1
  double accu1 = 0.0;    // accumulates the positive terms
  double num2  = x*x*x;  // first term is x^3
  double accu2 = 0.0;    // accumulates the negative terms
  double x4    = num2*x; // x^4
  for(int k=0; k<numTerms; k+=2)
  {
    accu1 += inverseFactorials[2*k+1] * num1;
    num1  *= xSq;
    accu2 -= inverseFactorials[2*k+3] * num2;
    num2  *= xSq;
  }
  return accu1 - accu2;
  */
}

inline double rsSawWave(double x) {
  double tmp = fmod(x, 2 * rsPI);
  if (tmp < rsPI)
    return tmp / rsPI;
  else
    return (tmp / rsPI) - 2.0;
}

// inline double rsSign(double x)
//{
//   if( x < 0.0 )
//     return -1.0;
//   else if( x > 0.0 )
//     return 1.0;
//   else
//     return 0.0;
// }
//  remove - replaced by templated version in MathBasics.h/inl (which is also
//  faster)

inline double rsSqrWave(double x) {
  double tmp = fmod(x, 2 * rsPI);
  if (tmp < rsPI)
    return 1.0;
  else
    return -1.0;
}
//
//inline double rsSoftClip(double x, double lowClamp, double highClamp,
//                            double lowKnee, double highKnee) {
//  double mid = 0.5 * (highClamp + lowClamp);
//  double high = highClamp - mid;
//  double low = lowClamp - mid;
//  double highThresh = mid + (1.0 - highKnee) * high;
//  double lowThresh = mid + (1.0 - lowKnee) * low;
//
//  // we want to deal with the absolute values of the thresholds and clamp
//  // values:
//  lowThresh = mid - lowThresh;
//  lowClamp = mid - lowClamp;
//
//  double highAlpha = highClamp - highThresh;
//  if (highAlpha == 0.0)
//    highAlpha += RS_MIN(double);
//  double highBeta = 1.0 / highAlpha;
//
//  double lowAlpha = lowClamp - lowThresh;
//  if (lowAlpha == 0.0)
//    lowAlpha += RS_MIN(double);
//  double lowBeta = 1.0 / lowAlpha;
//
//  double out;
//  if (x >= mid) // positive half-wave
//  {
//    if (x <= highThresh)
//      out = x;
//    else
//      out = highThresh + highAlpha * tanh(highBeta * (x - highThresh));
//  } else // negative half-wave
//  {
//    x = mid - x;
//    if (x <= lowThresh)
//      out = x;
//    else
//      out = lowThresh + lowAlpha * tanh(lowBeta * (x - lowThresh));
//    out = mid - out;
//  }
//
//  return out;
//}

template <class T> inline T rsSrem(T x, T y) {
  T z = fmod(x, y);
  if (fabs(z) > 0.5 * y)
    z = z - y * RSLib::rsSign(z);
  return z;
}

inline double rsStep(double x) {
  if (x >= 0.0)
    return 1.0;
  else
    return 0.0;
}

template <class T> T rsTanh(T x) {
  if (x > 350.0) // avoid internal overflow - what about -350?
    return 1.0;
  T c = exp(2 * x);
  return (c - 1) / (c + 1);
}

inline double rsTanhApprox(double x) {
  double a = fabs(2 * x);
  double b = 24 + a * (12 + a * (6 + a));
  return 2 * (x * b) / (a * b + 48);
}

inline double rsTriWave(double x) {
  double tmp = fmod(x, 2 * rsPI);
  if (tmp < 0.5 * rsPI)
    return tmp / (0.5 * rsPI);
  else if (tmp < 1.5 * rsPI)
    return 1.0 - ((tmp - 0.5 * rsPI) / (0.5 * rsPI));
  else
    return -1.0 + ((tmp - 1.5 * rsPI) / (0.5 * rsPI));
}


// template<class T>
// T rsIdentity(T x)
//{
//   return x;
// }

// ToDo: get rid of new/delete - use static sized stack allocated memory or, if
// possible, try to convert everything to on-the-fly computations or let the
// caller pass a workspace. Also, we don't need different arrays for v and vp.
// We can just reuse v. Maybe we should warp the block from rsLanden(..) to *K =
// into a samll lambda function, also rsLanden itself could be an internal
// lambda

// see also:
// https://github.com/jgaeddert/liquid-dsp/blob/master/src/filter/src/ellip.c

// Computes auxiliary functions f(x) and g(x) needed in the evaluation of Si(x)
// and Ci(x). This approximation follows "Handbook of mathematical functions" by
// Abramowitz/Stegun, page 232.
template <class T>
void rsSinCosIntegralAux(T x, T *f, T *g) // usable fo x >= 1
{
  T x2 = x * x;   // x^2
  T x4 = x2 * x2; // x^4
  T x6 = x4 * x2; // x^6
  T x8 = x4 * x4; // x^8

  // Eq. 5.2.38:
  static const T fa1 = 38.027264, fb1 = 40.021433, fa2 = 265.187033,
                 fb2 = 322.624911, fa3 = 335.677320, fb3 = 570.236280,
                 fa4 = 38.102495, fb4 = 157.105423;
  *f = (x8 + fa1 * x6 + fa2 * x4 + fa3 * x2 + fa4) /
       (x * (x8 + fb1 * x6 + fb2 * x4 + fb3 * x2 + fb4));

  // Eq. 5.2.39:
  static const T ga1 = 42.242855, gb1 = 48.196927, ga2 = 302.757865,
                 gb2 = 482.485984, ga3 = 352.018498, gb3 = 1114.978885,
                 ga4 = 21.821899, gb4 = 449.690326;
  *g = (x8 + ga1 * x6 + ga2 * x4 + ga3 * x2 + ga4) /
       (x2 * (x8 + gb1 * x6 + gb2 * x4 + gb3 * x2 + gb4));
}
template <class T>
T rsSineIntegralViaAuxFunctions(T x) // usable for |x| > 1
{
  T xAbs = fabs(x);
  T s, c, f, g;
  rsSinCos(xAbs, &s, &c);
  rsSinCosIntegralAux(xAbs, &f, &g);
  return rsSign(x) * (rsPI / 2 - f * c - g * s); // Eq. 5.2.8
}
template <class T>
T rsSineIntegralViaPowerSeries(T x) // recommended for |x| <= 3
{
  T xAbs = fabs(x); // |x|
  T sum = 0.0;      // accumulator for result
  T mx2 = -x * x;   // -x^2
  int k = 1;        // k := 2*n+1
  T xk = x;         // accumulator for x^k = x^(2*n+1)
  for (int n = 0; n < 15; n++) {
    T old = sum;
    sum += rsInverseFactorials[k] * xk / k;
    if (sum == old)
      break; // converged - maybe measure, if this really leads to efficiency
             // improvements on the average -> throw arguments between 0 and 3
             // at the function, see if it performs better with or without the
             // conditional early exit also, if we know that |x| < 3, we might
             // get away with fewer than 15 terms, in experiments, it never
             // exceeded 13, when |x| < 3 ...but maybe we should shift the
             // crossover-point for the algorithm higher than 3
    xk *= mx2;
    k += 2;
  }
  return sum;
}
template <class T>
T rsSineIntegralViaContinuedFractions(T x) // recommended for |x| > 3
{
  static const int maxIterations = 100;
  T xAbs = fabs(x);
  T a;
  std::complex<T> b(1.0, xAbs);
  // std::complex<T> c = 1.0 / RS_TINY;    // gives compiler warning
  std::complex<T> c = 1.0 / 1.175494e-38; // == float minimum, fixes warning
  std::complex<T> d = 1.0 / b;
  std::complex<T> h = d;
  std::complex<T> cd;
  for (int i = 1; i <= maxIterations; i++) {
    a = -i * i;
    b += 2.0;
    d = 1.0 / (a * d + b);
    c = b + a / c;
    cd = c * d; // factor
    h *= cd;    // product
    if (fabs(cd.real() - 1.0) + fabs(cd.imag()) < RS_EPS(T))
      break;
  }
  T si, co;
  rsSinCos(xAbs, &si, &co);
  h *= std::complex<T>(co, -si);
  return rsSign(x) * (0.5 * rsPI + h.imag());
}
template <class T> T rsSineIntegral(T x) {
  T xAbs = fabs(x);
  if (xAbs <= 3.0)
    return rsSineIntegralViaPowerSeries(x);
  else {
    return rsSineIntegralViaContinuedFractions(
        x); // exact but slow (really? -> measure)
    // return rsSineIntegralViaAuxFunctions(x);       // approximation
  }
}
// compare with this implementation:
// http://www.mymathlib.com/functions/sin_cos_integrals.html

template <class T> T rsSinc(T x) {
  if (rsAbs(x) < RS_EPS(T))
    return 1.0;
  return sin(x) / x;
}

template <class T> T rsNormalizedSinc(T x) { return rsSinc(rsPI * x); }

/*

Ideas:

-rsSoftAbs(x) = x * tanh(a*x) where a controls the softness, see:
   https://www.desmos.com/calculator/7at9bzeszu
 maybe also try some other sigmoid functions instead of tanh. But be aware that
this will make small signals even smaller which may be undesirable in amplitude
envelope estimation. But in this context, the softening would be pointless
anyway because the abs is typically lowpassed afterwards. Maybe it's useful for
a soft rectifier-like distortion. ...maybe make a FuncShaper preset
"SoftRectifier". With a = 1, near zero, it will look like x^2 and far from zero,
it will look like |x|



*/
}
