#pragma once
#include "../RSLib/SortAndSearch.inl"
#include "../RSLib/SortAndSearch.h"
#include "../RAPT/SortAndSearch.h"
#include <cmath>

/** A class for generating ratios of numbers that can be used - for example - as frequency ratios
for the various oscillators in an oscillator array (for "supersaw"/"superwave" stuff), 
delay-lengths in a feedback delay network, and so on. */

#include "../TypeDefinitions.h"
#include "BasicFunctions.h"
#include "ArrayTools.h"
#include "../Range.h"

namespace RAPT {

template <class T> class rsRatioGenerator {

public:
  /** Used to select the formula/algorithm by which a ratio is computed.
  Determines what kind of ratios will be produced in our dispatching methods.

  Warning: all the rangeSplit... algorithms are O(N^2) and dynamically allocate
  temporary memory, so you may want to avoid them in a realtime context - i want
  to at least get rid of dynamic memory allocation later and hopefully find a
  faster algorithm - but i'm not sure, if the latter is possible (or if it is
  possible, if can figure it out) */
  enum class RatioKind // maybe rename to RatioFormula or Algorithm
  {
    metallic,
    primePower,
    primePowerDiff,
    rangeSplitSkewed,
    rangeSplitOdd,
    rangeSplitEven,
    linToExp // morph between linear and exponential
  };

  //-----------------------------------------------------------------------------------------------
  /** \name Setup */

  /** If you want to use the formulas based on prime numbers, you must pass a
  pointer to vector of prime numbers and ensure that the vector pointed is still
  alive when you call a function that generates a frcation from prime numbers.
  The size of the table must be ...large enough...tbc.. */
  void setPrimeTable(std::vector<RAPT::rsUint32> *newTable) {
    primeTable = newTable;
  }

  /** Sets the kind of ratios that should be produced. */
  void setRatioKind(RatioKind newKind) { kind = newKind; }
  // maybe rename to setAlgorithm

  /** Sets the first parameter for the formula - the meaning of the parameter
  varies depending on which formula is selected. */
  void setParameter1(T newParam) { p1 = newParam; }
  //...

  //-----------------------------------------------------------------------------------------------
  /** \name Processing */

  /** Fills the given array with the ratios of the selected kind with selected
   * parameters. */

  template <class T> void fillRatioTable(T *r, int N) {
    if (primeTable == nullptr)
      return;
    bool sorted = false;
    typedef RatioKind RK;
    switch (kind) {
    case RK::metallic: {
      for (int i = 0; i < N; i++)
        r[i] = metallic(T(i), p1);
      sorted = true;
    } break;
    case RK::primePower: {
      for (int i = 0; i < N; i++) {
        r[i] = pow(T(primeTable->at(i)), p1);
      }
      sorted = true;
    } break;
    case RK::primePowerDiff: {
      for (int i = 0; i < N; i++) {
        r[i] =
            pow(T(primeTable->at(i + 1)), p1) - pow(T(primeTable->at(i)), p1);
      }
    } break;
    case RK::rangeSplitSkewed: {
      rangeSplits(r, N, p1, 0);
      sorted = true;
    } break;
    case RK::rangeSplitOdd: {
      rangeSplits(r, N, p1, 1);
      sorted = true;
    } break;
    case RK::rangeSplitEven: {
      rangeSplits(r, N, p1, 2);
      sorted = true;
    } break;
    case RK::linToExp: {
      for (int i = 0; i < N; i++) {
        T linVal = T(1) + T(i) / T(N - 1);
        T expVal = exp(linVal) / exp(2); // optimize!
        r[i] = (T(1) - p1) * linVal + p1 * expVal;
        sorted = true;
      }
      // rangeSplits(r, N, p1, 2); sorted = true;
    } break;
    }
    if (!sorted) {
        RSLib::rsHeapSort(r, N);
    }
      

    rsAssert(
        rsIsFiniteNumbers(r, N)); // catch numeric singularities in the algos
                                  // rsAssert(rsIsSortedStrictlyAscending(r,
                                  // N)); // catch algorithmic flaws

    // have boolean options like invert (map to 1..2 and take reciprocal),
    // reverse (map to 0...1 and take 1-..), extract frac part
  }

  /** Returns the so called metallic ratio of given index n. For n = 0, it's
  just 1, for n = 1 it is called the golden ratio, n = 2: silver, n = 3: bronze
  and beyond that, they don't have names.
  https://en.wikipedia.org/wiki/Metallic_mean
  The index is actually supposed to be a nonegative integer, but for extended
  flexibility, the function allows you to pass a real number as well. The golden
  ratio has many interesting mathematical properties, like being the "most
  irrational" number possible in the sense that it's hardest to approximate by a
  continued fraction expansion, see here:
  https://www.youtube.com/watch?v=CaasbfdJdJg  */
  static inline T metallic(T n, T p = 0.5) {
    // return T(0.5) * (n + sqrt(n*n+T(4)));
    return T(0.5) * (n + pow(n * n + T(4), p));
  }

  /** p1-th power of n-th prime number where p1 is our first continuous
  parameter set by setParameter1 */
  inline T primePower(int n) {
    rsAssert(primeTable != nullptr);
    rsAssert(n < (int)primeTable->size());
    return pow(T(primeTable->at(n)), p1);
  }

  /** Difference of the p1-th powers of n+1-th and n-th prime number. */
  inline T primePowerDiff(int n) { return primePower(n + 1) - primePower(n); }

  template <class T>
  void rangeSplits(T *splitPoints, int numSplitPoints, T ratio,
                   int strategy = 1) {
    // prototype implementation - may not be optimal in terms of efficiency, but
    // is algorithmically easier to understand

    int N = numSplitPoints - 1; // number of segments
    T r = ratio;
    typedef rsRange<T> Range;
    std::vector<Range> s(
        N); // array/set of segments/intervals/ranges (initially empty)
    s[0] = Range(0.0, 1.0); // seed set-of-ranges is { [0,1) }
    int n = 1;              // current number of ranges in the set
    while (n < N) {
      int k = rsArrayTools::maxIndex(
          &s[0], n); // index of largest range in the current set

      // split s[k] into two ranges:
      Range rl, ru;
      if (strategy == 0)
        splitRange(s[k], rl, ru, r); // always use r
      else if (strategy == 1) {      // alternating, ...
        if (rsIsOdd(n)) {
          splitRange(s[k], rl, ru, r);
        } // ...odd n uses r
        else {
          splitRange(s[k], rl, ru, 1 - r);
        }
      }                         // ...even n uses 1-r
      else if (strategy == 2) { // alternating, ...
        if (rsIsEven(n)) {
          splitRange(s[k], rl, ru, r);
        } // ...even n uses r
        else {
          splitRange(s[k], rl, ru, 1 - r);
        }
      } // ...odd n uses 1-r
      // is it somehow possible to continuously morph between the odd and even
      // version of this strategy? ...maybe just "crossfade" the result
      // split-point arrays? that would add another potentially interesting
      // dimension for tweaking ...maybe even vector-crossfade between
      // skew-right/skew-left/alternate-odd/alternate-even?

      // the lower part of s[k] replaces sk, the upper part gets appended to the
      // array:
      s[k] = rl;
      s[n] = ru;
      n++;
    }
    // the cost of this algorithm is O(N^2) because in each iteration, we search
    // for the maximum which is itself an O(N) operation - can we avoid this
    // search by somehow keeping the array of ranges sorted (by size), thereby
    // turning it into an O(N) algorithm?

    // sort the array ranges by their start-point:
    RAPT::rsHeapSort(&s[0], N, &rangeStartLess); // maybe make sorting optional
    // this is an O(N*log(N)) operation - so if we can turn the above into an
    // O(N) operation, the overall comlexity of range splitting would still be
    // O(N*log(N)) - certainly much better than O(N^2) - but to achieve O(N), we
    // would have to avoid the final sorting too - maybe by always keeping a
    // version sorted by size and another version sorted by start around?

    // copy range start-points into an array split-points:
    for (n = 0; n < N; n++)
      splitPoints[n] = s[n].getMin();
    splitPoints[n] = 1.0; // ...and add the end of the last range
    // hmm..the last is always 1 and the first is always 0 - an these actually
    // cannot be properly called split-point - maybe return only the inner, true
    // split-points - then N = numSplitPoins+1
  }

  // under construction - actually, the ratio need not to be passed - use p1 in
  // the function and the split-strategy should use an enum-class

  // what about plastic ratios? oh - there's only one such ratio - but maybe
  // powers of that can be used? what about powers of some general base?
  // https://en.wikipedia.org/wiki/Plastic_number

protected:
  RatioKind kind = RatioKind::metallic;
  T p1 = T(0.5); // p2, p3, ...

  std::vector<RAPT::rsUint32> *primeTable = nullptr;
  // table of prime numbers - we use a pointer to share it among instances
};

using RatioGenerator = rsRatioGenerator<double>;
} // namespace RAPT
