#pragma once

#include "soemdsp/sehelper.hpp"
#include "RealFunctions.h"

#include <complex>
namespace RAPT {

/** This class implements a cascade of biquad-filter stages where each of the
biquads implements the difference equation:

y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]

Each stage has its own set of coefficients which has to be set from outside this
class - this class does not do the filter-design. The coefficients can be
calculated by one of the "Designer" classes such as for example the
BiquadDesigner class. */

template <class TSig, class TCoef> // types for signal and coefficients
class rsBiquadCascade              // rename to BiquadChain
{
  typedef const TSig &CRSig;   // const reference to a signal value
  typedef const TCoef &CRCoef; // const reference to a coefficient value

public:
  //-----------------------------------------------------------------------------------------------
  /** \name Construction/Destruction */

  /** Constructor. You may pass the maximum number of stages that this cascade
  will be able to realize here (default is 12). */
  rsBiquadCascade(int newMaxNumStages = 12);

  /** Destructor. */
  ~rsBiquadCascade();

  //-----------------------------------------------------------------------------------------------
  /** \name Setup */

  /** Sets a new sample-rate. */
  // void setSampleRate(double newSampleRate);

  /** Sets the number of biquad stages. */
  void setNumStages(int newNumStages);

  /** Sets the order of the filter. The number of stages will we either half
  this value (if the order is even) or (order+1)/2 if the order is odd. */
  void setOrder(int newOrder);

  /** Sets up the global gain factor (by multiplying the feedforward
  coefficients of the first stage by the factor). */
  void setGlobalGainFactor(CRCoef newGainFactor);

  /** Copies the settings (numStages and the coefficients) from another instance
   * of this class. */
  void copySettingsFrom(rsBiquadCascade *other);

  /** Puts the given other biquad cascade in chain with this one. If this filter
  has currently M stages and the other has N stages, this will increase the
  number of stages of this filter to M+N. We will just copy the biquad
  coefficients of the other filter into our coefficients with a proper offset
  into our coefficient arrays, i.e. starting at the M-th one. The caller should
  ensure that M+N is <= our maxNumStages variable. */
  void chainWith(const rsBiquadCascade<TSig, TCoef> &other);

  /** Turns this biquad-cascade into an allpass filter that has the same poles
  as the original filter. The zeros are moevd to positions that are reflections
  of the poles in the unit circle. */
  void turnIntoAllpass();

  /** Multiplies all feedforward ('b') coefficients by some factor. */
  // void multiplyFeedforwardCoeffsBy(double factor);

  /** Allows the user to set the filter coefficients for the individual
  biquad-stages. The difference-equation of each of the biquad stages is: \f[
  y[n] = b_0 x[n] + b_1 x[n-1] + b_2 x[n-2] - a_1 y[n-1] - a_2 y[n-2] \f] */
  inline void setCoeffs(TCoef *newB0, TCoef *newB1, TCoef *newB2, TCoef *newA1,
                        TCoef *newA2);
  //, double newGain = 1.0);

  //-----------------------------------------------------------------------------------------------
  /** \name Inquiry */

  /** Returns the current number of stages. */
  int getNumStages() const { return numStages; }

  /** Returns the memory-address of the b0 array. */
  TCoef *getAddressB0() const { return b0; }

  /** Returns the memory-address of the b1 array. */
  TCoef *getAddressB1() const { return b1; }

  /** Returns the memory-address of the b2 array. */
  TCoef *getAddressB2() const { return b2; }

  /** Returns the memory-address of the a1 array. */
  TCoef *getAddressA1() const { return a1; }

  /** Returns the memory-address of the a2 array. */
  TCoef *getAddressA2() const { return a2; }

  /** Returns the global gain factor. */
  // TCoef getGlobalGainFactor() const { return gain; }

  /** Returns the complex transfer function at the given value of z. */
  std::complex<TCoef> getTransferFunctionAt(std::complex<TCoef> z);

  /** Writes the complex frequency-response of a biquad-cascade at the
  normalized radian frequencies given in 'w' into the array 'H'. */
  void getFrequencyResponse(
      TCoef *w, std::complex<TCoef> *H, int numBins,
      int accumulationMode = rsFilterAnalyzer<TCoef>::NO_ACCUMULATION) const;

  /** Writes the magnitdue-response of a biquad-cascade at the normalized radian
  frequencies given in 'w' into the array 'magnitudes'. Both arrays are assumed
  to be "numBins" long. "inDecibels" indicates, if the frequency response should
  be returned in decibels. If "accumulate" is true, the magnitude response of
  this biquad-cascade will be multiplied with (or added to, when "inDecibels" is
  true) to the magnitudes which are already there in the "magnitudes"-array.
  This is useful for calculating the magnitude response of several
  biquad-cascades in series. */
  void getMagnitudeResponse(TCoef *w, TCoef *magnitudes, int numBins,
                            bool inDecibels = false,
                            bool accumulate = false) const;

  /** Writes the magnitdue-response of a biquad-cascade at the physical
  frequencies given in 'frequencies' into the array 'magnitudes'. */
  void getMagnitudeResponse(TCoef *frequencies, CRCoef sampleRate,
                            TCoef *magnitudes, int numBins,
                            bool inDecibels = false,
                            bool accumulate = false) const;

  /** Returns an estimate of the time (in samples) it takes for the impulse
  response to ring down to "threshold". The estimate is based on the pole which
  has the highest Q (i.e. is closest to the unit circle). */
  TCoef getRingingTimeEstimate(CRCoef threshold) const;
  // todo: write a function getRingingFrequency - the normalized radian ringing
  // frequency is the angle of the pole that is closest to the unit circle

  //-----------------------------------------------------------------------------------------------
  /** \name Audio Processing */

  /** Calculates a single filtered output-sample via a cascade of biquads in
   * Direct-Form 1 */
  inline TSig getSampleDirect1(CRSig in);

  /** Calculates a single filtered output-sample via a cascade of biquads in
   * Direct-Form 2 */
  inline TSig getSampleDirect2(CRSig in);

  /** Calculates a single filtered output-sample via a cascade of biquads. */
  inline TSig getSample(CRSig in) { return getSampleDirect1(in); }

  //-----------------------------------------------------------------------------------------------
  /** \name Misc */

  /** Initializes the biquad coefficients to neutral values. */
  void initBiquadCoeffs();

  /** Sets the buffers for the previous input and output doubles of all biquad
   * stages to zero. */
  void reset();

protected:
  TCoef *a1, *a2, *b0, *b1, *b2; // filter coefficients
  TSig *x1, *x2, *y1, *y2;       // buffering
  int numStages;                 // current number of biquad-stages
  int maxNumStages;              // maximum number of biquad-stages
};

//-------------------------------------------------------------------------------------------------
// inlined functions:

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::setCoeffs(TCoef *newB0, TCoef *newB1,
                                                    TCoef *newB2, TCoef *newA1,
                                                    TCoef *newA2) {
  for (int i = 0; i < numStages; i++) {
    b0[i] = newB0[i];
    b1[i] = newB1[i];
    b2[i] = newB2[i];
    a1[i] = newA1[i];
    a2[i] = newA2[i];
  }
}

template <class TSig, class TCoef>
TSig rsBiquadCascade<TSig, TCoef>::getSampleDirect1(CRSig in) {
  TSig tmp{}, tmp2{};
  int i; // for the loop through the stages

  SE_BADVAL(in);

  tmp = in;

  // calculate current output-sample (y[n]) of all the BiQuad-stages (the output
  // of one stage is the input for the next stage):
  for (i = 0; i < numStages; i++) {
    tmp2 = tmp; // for x1[i]

    SE_BADVAL(tmp);

    // calculate current output-sample (y[n]) of BiQuad-stage i:
    tmp = b0[i] * tmp + (b1[i] * x1[i] + b2[i] * x2[i]) -
          (a1[i] * y1[i] + a2[i] * y2[i]);

    SE_BADVAL(tmp);

    // set x[n-1], x[n-2], y[n-1] and y[n-2] for the next iteration:
    x2[i] = x1[i];
    x1[i] = tmp2;
    y2[i] = y1[i];
    y1[i] = tmp;

    SE_BADVAL(x2[i]);
    SE_BADVAL(x1[i]);
    SE_BADVAL(y2[i]);
    SE_BADVAL(y1[i]);
  }

  return tmp;
}

template <class TSig, class TCoef>
TSig rsBiquadCascade<TSig, TCoef>::getSampleDirect2(CRSig in) {
  TSig x, y, g;
  y = x = in; // y = ... to make it work also with 0 stages as bypass

  // calculate current output-sample (y[n]) of all the BiQuad-stages (the output
  // of one stage is the input for the next stage):
  for (int i = 0; i < numStages; i++) {
    // calculate current output-sample (y[n]) of BiQuad-stage i:
    // g = x - a1[i]*y1[i] - a2[i]*y2[i];
    g = x - (a1[i] * y1[i] + a2[i] * y2[i]);
    // g = x + a1[i]*y1[i] + a2[i]*y2[i]; // this is wrong (a has wrong sign) -
    // for performance test
    y = b0[i] * g + b1[i] * y1[i] + b2[i] * y2[i];

    // set g[n-1], g[n-2] for the next iteration:
    y2[i] = y1[i];
    y1[i] = g;

    x = y; // output of one stage is input to the next
  }

  return y;
}
#include "BiquadCascade.h"
using namespace RAPT;

template <class TSig, class TCoef>
rsBiquadCascade<TSig, TCoef>::rsBiquadCascade(int newMaxNumStages) {
  if (newMaxNumStages >= 1)
    maxNumStages = newMaxNumStages;
  else
    maxNumStages = 12;

  // allcocate memory for coefficients and buffers:
  a1 = new TCoef[maxNumStages];
  a2 = new TCoef[maxNumStages];
  b0 = new TCoef[maxNumStages];
  b1 = new TCoef[maxNumStages];
  b2 = new TCoef[maxNumStages];
  x1 = new TSig[maxNumStages];
  x2 = new TSig[maxNumStages];
  y1 = new TSig[maxNumStages];
  y2 = new TSig[maxNumStages];

  // maybe do:
  // a1 = new TCoef[9*maxNumStages];
  // a2 = &a1[1*maxNumStages];
  // b0 = &a1[2*maxNumStages];
  // ...
  // to optimize class instantiation (less calls to new, it also ensures
  // contiguous memory) in the destructor, we then need to only delete a1
  // ..oh - no we need 2 calls to new bcs TCoef and TSig may be different - but
  // 2 is still better than 9

  initBiquadCoeffs();
  reset();
  // numStages = maxNumStages;  // shouldn't his be set to zero?
  numStages = 0;
}

template <class TSig, class TCoef>
rsBiquadCascade<TSig, TCoef>::~rsBiquadCascade() {
  delete[] a1;
  delete[] a2;
  delete[] b0;
  delete[] b1;
  delete[] b2;
  delete[] x1;
  delete[] x2;
  delete[] y1;
  delete[] y2;
}

// parameter settings:

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::setNumStages(int newNumStages) {
  if ((newNumStages >= 0) && (newNumStages <= maxNumStages))
    numStages = newNumStages;
  else
    RS_DEBUG_BREAK;
  reset();
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::setOrder(int newOrder) {
  if (rsIsEven(newOrder))
    setNumStages(newOrder / 2);
  else
    setNumStages((newOrder + 1) / 2);
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::setGlobalGainFactor(CRCoef newGain) {
  b0[0] *= newGain;
  b1[0] *= newGain;
  b2[0] *= newGain;
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::copySettingsFrom(rsBiquadCascade *other) {
  setNumStages(other->getNumStages());

  TCoef *pB0 = other->getAddressB0();
  TCoef *pB1 = other->getAddressB1();
  TCoef *pB2 = other->getAddressB2();
  TCoef *pA1 = other->getAddressA1();
  TCoef *pA2 = other->getAddressA2();
  for (int s = 0; s < numStages; s++) {
    b0[s] = pB0[s];
    b1[s] = pB1[s];
    b2[s] = pB2[s];
    a1[s] = pA1[s];
    a2[s] = pA2[s];
  }
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::chainWith(
    const rsBiquadCascade<TSig, TCoef> &bq) {
  int M = getNumStages();
  int N = bq.getNumStages();
  if (M + N > maxNumStages) {
    rsError("Too many filter stages");
    return;
  }
  for (int i = 0; i < N; i++) {
    b0[M + i] = bq.b0[i];
    b1[M + i] = bq.b1[i];
    b2[M + i] = bq.b2[i];
    a1[M + i] = bq.a1[i];
    a2[M + i] = bq.a2[i];
  }
  numStages = M + N;
  // ToDo: maybe return a bool: 1 if successful, 0 otherwise (in the error
  // branch)
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::turnIntoAllpass() {
  for (int i = 0; i < numStages; i++) {
    if (a2[i] == 0.0) // biquad stage is actually 1st order
    {
      b0[i] = a1[i];
      b1[i] = TCoef(1);
      b2[i] = 0;
    } else {

      b0[i] = a2[i];
      b1[i] = a1[i];
      b2[i] = TCoef(1);
    }
  }

  // normalize gain of each stage to unity:
  for (int i = 0; i < numStages; i++) {
    TCoef num = b0[i] * b0[i] + b1[i] * b1[i] + b2[i] * b2[i] +
                TCoef(2) * (b0[i] * b1[i] + b1[i] * b2[i]) +
                TCoef(2) * b0[i] * b2[i];
    TCoef den = TCoef(1) + a1[i] * a1[i] + a2[i] * a2[i] +
                TCoef(2) * (a1[i] + a1[i] * a2[i]) + TCoef(2) * a2[i];
    TCoef g = sqrt(den / num);
    b0[i] *= g;
    b1[i] *= g;
    b2[i] *= g;
  }
}

// inquiry:

template <class TSig, class TCoef>
std::complex<TCoef>
rsBiquadCascade<TSig, TCoef>::getTransferFunctionAt(std::complex<TCoef> z) {
  std::complex<TCoef> H(TCoef(1), TCoef(0));
  for (int i = 0; i < numStages; i++)
    H *= rsFilterAnalyzer<TCoef>::getBiquadTransferFunctionAt(
        b0[i], b1[i], b2[i], a1[i], a2[i], z);
  return H;
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::getFrequencyResponse(
    TCoef *w, std::complex<TCoef> *H, int numBins, int accumulationMode) const {
  rsFilterAnalyzer<TCoef>::getBiquadCascadeFrequencyResponse(
      b0, b1, b2, a1, a2, numStages, w, H, numBins, accumulationMode);
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::getMagnitudeResponse(TCoef *w,
                                                        TCoef *magnitudes,
                                                        int numBins,
                                                        bool inDecibels,
                                                        bool accumulate) const {
  rsFilterAnalyzer<TCoef>::getBiquadCascadeMagnitudeResponse(
      b0, b1, b2, a1, a2, numStages, w, magnitudes, numBins, inDecibels,
      accumulate);
}

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::getMagnitudeResponse(
    TCoef *frequencies, CRCoef sampleRate, TCoef *magnitudes, int numBins,
    bool inDecibels, bool accumulate) const {
  rsFilterAnalyzer<TCoef>::getBiquadCascadeMagnitudeResponse(
      b0, b1, b2, a1, a2, numStages, frequencies, sampleRate, magnitudes,
      numBins, inDecibels, accumulate);
}

template <class T> T biquadRingingTime(T a1, T a2, T threshold) {
  std::complex<T> p =
      -T(a1 / 2.0) + sqrt(std::complex<T>(a1 * T(a1 / 4.0) - a2)); // 1st pole
  return (T)rsLogB(threshold, abs(p));
}
// move to FilterAnalyzer

template <class TSig, class TCoef>
TCoef rsBiquadCascade<TSig, TCoef>::getRingingTimeEstimate(
    CRCoef threshold) const {
  TCoef rt = 0.0;
  for (int i = 0; i < numStages; i++)
    rt = rsMax(rt, biquadRingingTime(a1[i], a2[i], threshold));
  return rt;
  // Or maybe we should add them? the maximum seems more suitable for a parallel
  // connection, but this here is a serial connection. -> more experiments
  // needed
}

//-------------------------------------------------------------------------------------------------
// others:

template <class TSig, class TCoef>
void rsBiquadCascade<TSig, TCoef>::initBiquadCoeffs() {
  for (int i = 0; i < maxNumStages; i++) {
    b0[i] = 1.0;
    b1[i] = 0.0;
    b2[i] = 0.0;
    a1[i] = 0.0;
    a2[i] = 0.0;
  }
}

template <class TSig, class TCoef> void rsBiquadCascade<TSig, TCoef>::reset() {
  for (int i = 0; i < maxNumStages; i++) {
    x2[i] = 0.0;
    x1[i] = 0.0;
    y2[i] = 0.0;
    y1[i] = 0.0;
  }
}
} // namespace RAPT
