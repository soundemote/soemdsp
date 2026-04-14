#pragma once

#include <algorithm>
#include <vector>

class rsSmoothingFilter {

  public:
    /** Constructor. */
    rsSmoothingFilter() {
        y1.resize(1);
        coeffs.resize(1);
        reset();
    }

    /** Sets the time constant (in seconds) which is the time it takes to reach around
    1 - 1/e = 63% of the target value when starting from zero. You must also pass the samplerate
    at which the smoother should operate here. */
    void setTimeConstantAndSampleRate(double timeConstant, double sampleRate) {
        decay = sampleRate * timeConstant;
        updateCoeffs();
    }
    /** Sets the time constant (in seconds) which is the time it takes to reach around
  1 - 1/e = 63% of the target value when starting from zero. You must also pass the samplerate
  at which the smoother should operate here. */
    void setTimeInSamples(double samples) {
        decay = samples;
        updateCoeffs();
    }

    /** Sets up the number of samples that it takes for the unit-step response to reach a value
    of 1/2. */
    void setNumSamplesToReachHalf(double numSamples);
    // does not work

    /** Sets the order of the filter, i.e. the number of first order lowpass stages. */
    void setOrder(int newOrder) {
        order = std::max(1, newOrder);

        y1.resize(order);
        coeffs.resize(order);

        reset();
        // todo: if newOrder > oldOrder, init only the vector values in y1 above oldOrder-1 to 0
        // not all of them

        updateCoeffs();
    }

    /** If some shape other than SIGMOID is chosen, this parameter sets the amount of the shape. If
    zero, it reduces to the sigmoid shape (i.e. all stages have same time constant). If 1, the time
    constants of the stages follow an 1/n rule. The general rule is 1/n^p where p is the parameter
    set here. */
    void setShapeParameter(int newParam) {
        shapeParam = newParam;
        updateCoeffs();
    }

    /** Returns a smoothed output sample. */
    double getSample(double in) {
        // return y1[0] = in + coeff*(y1[0]-in); // this would be the 1st order leaky integrator

        y1[0] = in + coeffs[0] * (y1[0] - in);
        for (size_t i = 1; i < order; i++)
            y1[i] = y1[i - 1] + coeffs[i] * (y1[i] - y1[i - 1]);
        return y1[order - 1];
    }

    /** Resets the internal filter state to 0. */
    void reset() { setStates(0); }

    /** Sets the internal filter state to the given value. */
    void setStates(double value) {
        for (int i = 0; i < order; i++)
            y1[i] = value;
    }

  protected:
    /** Updates our filter coefficients according to the setting of decay and order. */
    void updateCoeffs() {
        double tmp;
        if (shapeParam != 0) {
            for (int i = 0; i < order; i++) {
                tmp = decay / static_cast<double>(pow(i + 1, shapeParam)); // tau[n] = tau[0] / n^p // p == shapeParam
                // if(i > 0)
                //   tmp *= (shapeParam+1);
                coeffs[i] = exp(-order / tmp);
            }
            // maybe try, if it responds different to modulations of the time-constants are in reverse
            // order (from short to long instead of long to short)
        } else {
            // all filter stages use the same time-constant
            tmp = exp(-order / decay); // amounts to divide the time-constant by the order
            for (int i = 0; i < order; i++)
                coeffs[i] = tmp;

        }
    }

    std::vector<double> y1;     // y[n-1] of the lowpass stages
    std::vector<double> coeffs; // lowpass filter coefficients
    double decay = double(0.1); // normalized decay == timeConstant * sampleRate ...rename to numSamplesToReachHalf
    int order    = 1;           // number of lowpass stages, now redundant with y1.size()

    int shapeParam = 0; // rename to asymmetry

    // maybe we should instead of "decay" maintain "sampleRate" and "timeConstant" variables and
    // provide functions to set them separately. That's more convenient for the user. It increases
    // our data a bit, but the convenience may be worth it.
};
