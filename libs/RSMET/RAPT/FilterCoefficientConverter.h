#pragma once

#include <complex>
#include "ArrayTools.h"

/** This class contains static functions to convert between the coefficients for various filter 
representations and realization structures. */

namespace RAPT {

template <class T> class rsFilterCoefficientConverter {

  typedef std::complex<T> Complex; // preliminary

public:
  /** Converts direct form FIR coefficients to FIR lattice filter reflection
   * coefficients. */
  static void directFormToLatticeFir(T *directFormCoeffs, int order,
                                     T *reflectionCoeffs) {
    int N = order; // number of direct-form and reflection coefficients
    T *a = new T[N + 1];
    T *aOld = new T[N + 1];
    T *k = reflectionCoeffs;
    T scaler;
    int i, m;

    // the recursion assumes the leading FIR coefficient to be unity, so we
    // normalize the FIR-vector by its leading coefficient and remember it as
    // overall gain-factor:
    T gain = directFormCoeffs[0];

    // copy the sign inverted, gain normalized direct-form coefficients into an
    // internal array (Eq 6.50):
    scaler = T(1) / gain;
    a[0] = directFormCoeffs[0] * scaler;
    aOld[0] = a[0];
    for (i = 1; i <= N; i++) {
      a[i] = -directFormCoeffs[i] * scaler;
      aOld[i] = a[i];
    }

    // do the recursion (Eq. 6.56c):
    for (i = N; i >= 1; i--) {
      k[i] = a[i];

      scaler = T(1) / (T(1) - k[i] * k[i]);
      for (m = 1; m <= (i - 1); m++)
        a[m] = (aOld[m] + k[i] * aOld[i - m]) * scaler;

      // update the aOld-array:
      for (m = 0; m <= N; m++)
        aOld[m] = a[m];
    }

    // store the gain factor in the first element of the reflection coefficient
    // vector:
    k[0] = gain;

    // invert the sign of the reflection coeffs to follow the MatLab convention
    // (but leave the additional gain factor in k[0] as is):
    for (i = 1; i <= N; i++)
      k[i] = -k[i];

    delete[] a;
    delete[] aOld;
  }
  // allocates heap memory

  /** Converts FIR lattice filter reflection coefficients to direct form FIR
   * coefficients. */
  static void latticeToDirectFormFir(T *reflectionCoeffs, int order,
                                     T *directFormCoeffs) {
    int N = order; // number of direct-form and reflection coefficients
    T *a = directFormCoeffs;
    T *aOld = new T[N + 1];
    T *k = reflectionCoeffs;
    int i, m;

    // copy the direct-form coefficients into an internal array:
    for (i = 0; i <= N; i++)
      aOld[i] = a[i];

    // do the recursion (Eq. 6.54a 6.54b, tweaked to accomodate for MatLabs
    // sign-convention):
    for (i = 1; i <= N; i++) {
      a[i] = -k[i];

      for (m = 1; m <= (i - 1); m++)
        a[m] = aOld[m] + k[i] * aOld[i - m];

      // update the aOld-array:
      for (m = 0; m <= N; m++)
        aOld[m] = a[m];
    }

    // in the first element of the reflection coefficient vector is the overall
    // gain-factor - we need to scale the FIR vector with this factor and also
    // use it as leading coefficient, we also need to invert the sign:
    for (i = 1; i <= N; i++)
      a[i] = -a[i] * k[0];
    a[0] = k[0];

    delete[] aOld;
  }
  // allocates heap memory

  /** Converts complex poles and zeros into coefficients for a biquad cascade in
  which each stage implements the difference equation: \f[ y[n] = b_0 x[n] + b_1
  x[n-1] + b_2 x[n-2] - a_1 y[n-1] - a_2 y[n-2] \f] The arrays of poles and
  zeros are understood to contain only one pole (zero) for each complex
  conjugate pair of poles and zeros of the actual filter. If there is a first
  order stage present (a real pole/zero) than this real pole/zero should be the
  last entry in the array and the flag lastStageIsFirstOrder should be set to
  true. ...to be deprecated */
  static void polesAndZerosToBiquadCascade(Complex *poles, int numPoles,
                                           Complex *zeros, int numZeros, T *b0,
                                           T *b1, T *b2, T *a1, T *a2,
                                           bool lastStageIsFirstOrder) {
    int order = 2 * rsMax(numPoles, numZeros);
    if (lastStageIsFirstOrder)
      order -= 1;

    int numBiquads;
    if (rsIsEven(order))
      numBiquads = order / 2;
    else
      numBiquads = (order + 1) / 2;

    int b;
    for (b = 0; b < numBiquads; b++) {
      b0[b] = T(1);
      b1[b] = T(-2) * zeros[b].real();
      b2[b] =
          zeros[b].real() * zeros[b].real() + zeros[b].imag() * zeros[b].imag();
      a1[b] = T(-2) * poles[b].real();
      a2[b] =
          poles[b].real() * poles[b].real() + poles[b].imag() * poles[b].imag();
    }

    // overwrite the coefficients of the last stage, when it must be a first
    // order stage:
    if (lastStageIsFirstOrder) {
      b1[numBiquads - 1] = -zeros[numBiquads - 1].real();
      b2[numBiquads - 1] = 0.0;
      a1[numBiquads - 1] = -poles[numBiquads - 1].real();
      a2[numBiquads - 1] = 0.0;
    }
  }

  static void polesAndZerosToBiquadCascade(Complex *poles, Complex *zeros,
                                           int order, T *b0, T *b1, T *b2,
                                           T *a1, T *a2) {
    int numBiquads;
    if (rsIsEven(order))
      numBiquads = order / 2;
    else
      numBiquads = (order + 1) / 2;
    // use (order+1)/2 regardless - if order is even, truncation will take place
    // and the result is the same

    int b;
    // for(b=0; b<numBiquads; b++)
    for (b = 0; b < order / 2; b++) {
      b0[b] = 1.0;
      b1[b] = -(zeros[2 * b] + zeros[2 * b + 1]).real();
      b2[b] = (zeros[2 * b] * zeros[2 * b + 1]).real();
      a1[b] = -(poles[2 * b] + poles[2 * b + 1]).real();
      a2[b] = (poles[2 * b] * poles[2 * b + 1]).real();

      /*
      b1[b] = -2.0 * zeros[2*b].re;
      b2[b] = zeros[2*b].re * zeros[2*b].re + zeros[2*b].im * zeros[2*b].im;
      a1[b] = -2.0 * poles[2*b].re;
      a2[b] = poles[2*b].re * poles[2*b].re + poles[2*b].im * poles[2*b].im;
      */
    }

    // overwrite the coefficients of the last stage, when it must be a first
    // order stage:
    if (rsIsOdd(order)) {
      b0[b] = 1.0;
      b1[numBiquads - 1] = -zeros[order - 1].real();
      b2[numBiquads - 1] = 0.0;
      a1[numBiquads - 1] = -poles[order - 1].real();
      a2[numBiquads - 1] = 0.0;
    }
  }
  // maybe define an alias pz2sos

  /**  !!! NOT YET FULLY IMPLEMETED !!!
  Converts the coefficients of a cascade biquad of biquad filters each of which
  having the form: \f[ y[n] = b_0 x[n] + b_1 x[n-1] + b_2 x[n-2] - a_1 y[n-1] -
  a_2 y[n-2] \f] into coefficients for a direct form filter of the form: \f[
  y[n] = sum_{k=0}^K b_k x[n-k] - sum_{m=1}^M a_m y[n-m] \f] The b0, b1, b2, a1,
  a2 arrays should contain the coefficients for the individual biquad stages.
  Then, when B is the number of biquad stages, the b array will have to be of
  size K=2*B+1 (b_0,..., b_K) and the a array will have to be of size M=2*B
  (a_1,...,a_M) */
  static void biquadCascadeToDirectForm(int numBiquads, T *b0, T *b1, T *b2,
                                        T *a1, T *a2, T *b, T *a) {
    // aquire and initialize memory for intermediate results and accumulation:
    int N = 2 * numBiquads + 1; // number of direct form coefficients
    long double *tmp = new long double[N];
    long double *aAccu = new long double[N];
    long double *bAccu = new long double[N];
    long double *aQuad = new long double[3];
    long double *bQuad = new long double[3];
    // i used long double to avoid roundoff error accumulation - but on 64 bit
    // systems, "long double" is the same as "double", so this doesn't help
    // anything anymore - so get rid of these temporary accumulation buffers and
    // avoid memory allocation

    int i;
    for (i = 0; i < N; i++) {
      aAccu[i] = 0.0;
      bAccu[i] = 0.0;
    }
    aAccu[0] = 1.0;
    bAccu[0] = 1.0;

    // calculate the direct form coefficients by recursively applying
    // convolution to the result of the previous convolution with the new
    // quadratic factor:
    for (i = 0; i < numBiquads; i++) {
      // aquire the current quadratic factor:
      aQuad[0] = 1.0;
      aQuad[1] = a1[i];
      aQuad[2] = a2[i];
      bQuad[0] = b0[i];
      bQuad[1] = b1[i];
      bQuad[2] = b2[i];

      // convolve the current quadratic factor with the result of the previous
      // convolution:
      rsArrayTools::copy(aAccu, tmp, N);
      rsArrayTools::convolve(tmp, N - 2, aQuad, 3, aAccu);
      rsArrayTools::copy(bAccu, tmp, N);
      rsArrayTools::convolve(tmp, N - 2, bQuad, 3, bAccu);
      // this can be optimized .... xLength does not need always be N-2 (can be
      // shorter in early iterations)
    }

    // copy (and typecast) the accumulated coefficents into the output arrays:
    for (i = 0; i < N; i++) {
      a[i] = (T)aAccu[i];
      b[i] = (T)bAccu[i];
    }

    // release temporarily aquired memory:
    delete[] tmp;
    delete[] aAccu;
    delete[] bAccu;
    delete[] aQuad;
    delete[] bQuad;
  }
  // allocates heap memory

  /** Calculates the magnitude-response of a digital biquad filter with
  coefficients b0, b1, b2, a0, a1, a1 at the normalized radian frequency
  'omega'.  */
  static T getBiquadMagnitudeAt(T b0, T b1, T b2, T a1, T a2, T omega) {
    T cosOmega = cos(omega);
    T cos2Omega = cos(T(2) * omega);
    T num = b0 * b0 + b1 * b1 + b2 * b2 +
            T(2) * cosOmega * (b0 * b1 + b1 * b2) + T(2) * cos2Omega * b0 * b2;
    T den = T(1) + a1 * a1 + a2 * a2 + T(2) * cosOmega * (a1 + a1 * a2) +
            T(2) * cos2Omega * a2;
    return sqrt(num / den);
  }
  // \todo remove - function is redundant  with the function in FilterAnalyzer

  /** Normalizes the biquad stages described by the given coefficients in such a
  way that each stage has unit magnitude at the normalized radian frequency
  'omega'. If a gainFactor is passed, it will normalize to this gain factor.  */
  static void normalizeBiquadStages(T *b0, T *b1, T *b2, T *a1, T *a2, T omega,
                                    int numStages, T gainFactor = 1.0) {
    T m; // magnitude of current stage
    T scaler;
    for (int s = 0; s < numStages; s++) {
      // divide all the feedforward coefficients by the current magnitude at
      // omega:
      m = getBiquadMagnitudeAt(b0[s], b1[s], b2[s], a1[s], a2[s], omega);
      scaler = gainFactor / m;
      b0[s] *= scaler;
      b1[s] *= scaler;
      b2[s] *= scaler;
    }
  }

  // todo: biquadToPartialFractions - converts a biquad to it partial fraction
  // expansion

  // static void biquadToStateVariable(T* b0, T* b1, T* b2, T* a1, T* a2, ...);

  // static void biquadToPhasor(T* b0, T* b1, T* b2, T* a1, T* a2, T* rc, T* rs,
  // T* wx, T* wy, T* wi);
  //  update equation:
  //  |x| = r * |c -s| * |x| + |in|
  //  |y|       |s  c|   |y|   |0 |
  //  output equation:
  //  out = wx*x + wy*y + wi*in
  //  where r is a decay factor <= 1; c,s are rotation matrix coeffs (sin/cos of
  //  angle w); wx,wy,wi are weights for the state-variables x,y and input -> 5
  //  independent coeffs (r can be absorbed into the rotation matrix, turning it
  //  into a spiraling matrix) the filter's state is a 2D vector - could be
  //  called state-vector filter
};
} // namespace RAPT
