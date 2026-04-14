#pragma once
//// rosic-indcludes:
//#include "rosic_ElementaryFunctionsReal.h"
#include "../GlobalDefinitions.h"
#include "../RAPT/BasicFunctions.h"
#include <complex>

// try to retire this code in favor of std::complex<double>

namespace rosic {

template <typename t> void rsSinCos(t x, t *sinResult, t *cosResult) {
  *sinResult = sin(x);
  *cosResult = cos(x);
}


/**

This is a class for complex numbers. It defines the basic arithmetic operations
between complex numbers as well as the special cases when one of the operands is
real (double).

ATTENTION: do not define any further member variables, nor switch the ordering
of re and im because the FourierTransformer classes rely on the fact that a
complex number consists of two doubles re, im and nothing else (the algorithms
actually run on buffers of doubles).

*/

class Complex {

public:
  //-------------------------------------------------------------------------------------------------------------------------------------
  // public member variables:

  /** Real part */
  double re;

  /** Imaginary part */
  double im;

  //-------------------------------------------------------------------------------------------------------------------------------------
  // construction/destruction:

  /** Constructor. Initializes real and imaginary part to zero. */
  Complex() { re = im = 0.0; }

  Complex(double reInit) {
    re = reInit;
    im = 0.0;
  }

  Complex(double reInit, double imInit) {
    re = reInit;
    im = imInit;
  }

  ~Complex() {}

  //-------------------------------------------------------------------------------------------------------------------------------------
  // overloaded operators:

  /** Compares two complex numbers of equality. */
  bool operator==(const Complex &z) const {
    if (re == z.re && im == z.im)
      return true;
    else
      return false;
  }

  /** Compares two complex numbers of inequality. */
  bool operator!=(const Complex &z) const {
    if (re != z.re || im != z.im)
      return true;
    else
      return false;
  }

  /** Defines the negative of a complex number. */
  Complex operator-() { return Complex(-re, -im); }

  /** Adds another complex number to this complex and returns the result. */
  Complex &operator+=(const Complex &z) {
    re += z.re;
    im += z.im;
    return *this;
  }

  /** Adds a real number to this complex and returns the result. */
  Complex &operator+=(const double &r) {
    re += r;
    return *this;
  }

  /** Subtracts another complex number from this complex and returns the result.
   */
  Complex &operator-=(const Complex &z) {
    re -= z.re;
    im -= z.im;
    return *this;
  }

  /** Subtracts a real number from this complex and returns the result. */
  Complex &operator-=(const double &r) {
    re -= r;
    return *this;
  }

  /** Multiplies this complex number by another complex number and returns the
   * result. */
  Complex &operator*=(const Complex &z) {
    double reNew = re * z.re - im * z.im;
    double imNew = re * z.im + im * z.re;
    re = reNew;
    im = imNew;
    return *this;
  }

  /** Multiplies this complex number by a real number and returns the result. */
  Complex &operator*=(const double &r) {
    re *= r;
    im *= r;
    return *this;
  }

  /** Divides this complex number by another complex number and returns the
   * result. */
  Complex &operator/=(const Complex &z) {
    double scale = 1.0 / (z.re * z.re + z.im * z.im);
    double reNew = scale * (re * z.re + im * z.im);
    double imNew = scale * (im * z.re - re * z.im);
    re = reNew;
    im = imNew;
    return *this;
  }

  /** Divides this complex number by a real number and returns the result. */
  Complex &operator/=(const double &r) {
    double scale = 1.0 / r;
    re *= scale;
    im *= scale;
    return *this;
  }


void setRadiusAndAngle(double newRadius, double newAngle) {
    rsSinCos(newAngle, &im, &re);
    re *= newRadius; // re = newRadius * cos(newAngle);
    im *= newRadius; // im = newRadius * sin(newAngle);
  }


  //-------------------------------------------------------------------------------------------------------------------------------------
  // getters:

double getRadius() {
    return sqrt(re * re + im * im);
    // return _hypot(re, im);
  }

  double getAngle() {
    if ((re == 0.0) && (im == 0.0))
      return 0.0;
    else
      return atan2(im, re);
  }

  void setRadius(double newRadius) {
    double phi = getAngle();
    rsSinCos(phi, &im, &re);
    re *= newRadius; // re = newRadius * cos(phi);
    im *= newRadius; // im = newRadius * sin(phi);
  }

  void setAngle(double newAngle) {
    double r = getRadius();
    rsSinCos(newAngle, &im, &re);
    re *= r; // re = r * cos(newAngle);
    im *= r; // im = r * sin(newAngle);
  }

  Complex getConjugate() { return Complex(re, -im); }

  Complex getReciprocal() {
    double scaler = 1.0 / (re * re + im * im);
    return Complex(scaler * re, -scaler * im);
  }

  bool isReal() { return (im == 0.0); }

  bool isImaginary() { return (re == 0.0); }

  bool isInfinite() {
    if (re == INF || re == NEG_INF || im == INF || im == NEG_INF)
      return true;
    else
      return false;
  }

  bool isIndefinite() {
    if (re == INDEF || re == -INDEF || im == INDEF || im == -INDEF)
      return true;
    else
      return false;
  }
}; // end of class Complex

// some binary operators are defined outside the class such that the left hand
// operand does not necesarrily need to be of class Complex:

/** Adds two complex numbers. */
inline Complex operator+(const Complex &z, const Complex &w) {
  return Complex(z.re + w.re, z.im + w.im);
}

/** Adds a complex and a real number. */
inline Complex operator+(const Complex &z, const double &r) {
  return Complex(z.re + r, z.im);
}

/** Adds a real and a complex number. */
inline Complex operator+(const double &r, const Complex &z) {
  return Complex(z.re + r, z.im);
}

/** Subtracts two complex numbers. */
inline Complex operator-(const Complex &z, const Complex &w) {
  return Complex(z.re - w.re, z.im - w.im);
}

/** Subtracts a real number from a complex number. */
inline Complex operator-(const Complex &z, const double &r) {
  return Complex(z.re - r, z.im);
}

/** Subtracts a complex number from a real number. */
inline Complex operator-(const double &r, const Complex &z) {
  return Complex(r - z.re, -z.im);
}

/** Multiplies two complex numbers. */
inline Complex operator*(const Complex &z, const Complex &w) {
  return Complex(z.re * w.re - z.im * w.im, z.re * w.im + z.im * w.re);
}

/** Multiplies a complex number and a real number. */
inline Complex operator*(const Complex &z, const double &r) {
  return Complex(z.re * r, z.im * r);
}

/** Multiplies a real number and a complex number. */
inline Complex operator*(const double &r, const Complex &z) {
  return Complex(z.re * r, z.im * r);
}

/** Divides two complex numbers. */
inline Complex operator/(const Complex &z, const Complex &w) {
  double scale = 1.0 / (w.re * w.re + w.im * w.im);
  return Complex(scale * (z.re * w.re + z.im * w.im),  // real part
                 scale * (z.im * w.re - z.re * w.im)); // imaginary part
}

/** Divides a complex number by a real number. */
inline Complex operator/(const Complex &z, const double &r) {
  double scale = 1.0 / r;
  return Complex(scale * z.re, scale * z.im);
}

/** Divides a real number by a complex number. */
inline Complex operator/(const double &r, const Complex &z) {
  double scale = r / (z.re * z.re + z.im * z.im);
  return Complex(scale * z.re, -scale * z.im);
}

// hacky stuff to convert/cast pointers between std::complex<double> and
// rosic::Complex (eventually, we want to not use rosic::Complex anymore, but
// for the transition, we may need to convert back and forth) - this works
// because ("array oriented access" section):
// https://en.cppreference.com/w/cpp/numeric/complex
inline std::complex<double> *rsCastPointer(Complex *p) {
  return reinterpret_cast<std::complex<double> *>(p);
}

inline Complex *rsCastPointer(std::complex<double> *p) {
  return reinterpret_cast<Complex *>(p);
}

template <typename ComplexType, typename RealType>
ComplexType rsSqrtC(ComplexType z) {
  ComplexType c;
  RealType x, y, w, r;
  if ((z.re == 0.0) && (z.im == 0.0)) {
    c.re = 0.0;
    c.im = 0.0;
    return c; // return rsComplex<RealType>(0, 0);
  } else {
    x = fabs(z.re); // use rsAbs
    y = fabs(z.im);
    if (x >= y) {
      r = y / x;
      w = RAPT::rsSqrt(x) *
          RAPT::rsSqrt(0.5 * (1.0 + RAPT::rsSqrt(1.0 + r * r)));
    } else {
      r = x / y;
      w = RAPT::rsSqrt(y) * RAPT::rsSqrt(0.5 * (r + RAPT::rsSqrt(1.0 + r * r)));
    }
    if (z.re >= 0.0) {
      c.re = w;
      c.im = z.im / (2.0 * w);
    } else {
      c.im = (z.im >= 0) ? w : -w; // if(z.im >= 0) c.im = w; else c.im = -w;
      c.re = z.im / (2.0 * c.im);
    }
    return c;
  }
}

} // namespace rosic
