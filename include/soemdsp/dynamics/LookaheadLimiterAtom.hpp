#pragma once

// External-memory lookahead limiter coeffs + process atom (mem + base).
// exp()/db→gain live in *Changed / syncControlParams — not in process().

#include <cmath>
#include <cstdint>

namespace soemdsp::dynamics::atom {

// Control slots (doubles):
//   base+0 = ceilingDb
//   base+1 = attackMs
//   base+2 = releaseMs
//   base+3 = sampleRate
//   base+4 = dipGain
// Audio delay buffers stay caller-owned (not in this control block).
struct LookaheadLimiter
{
    static constexpr int kMaxSamples = 16384;

    inline double& ceilingDb_() noexcept { return mem[base + 0]; }
    inline double& attackMs_() noexcept { return mem[base + 1]; }
    inline double& releaseMs_() noexcept { return mem[base + 2]; }
    inline double& sampleRate_() noexcept { return mem[base + 3]; }
    inline double& dipGain_() noexcept { return mem[base + 4]; }

    static double dbToGain(double db) noexcept
    {
        if (!(db * 0.0 == 0.0))
        {
            return 1.0;
        }
        return std::exp(db * 0.11512925464970229);
    }

    void ceilingChanged() noexcept
    {
        ceiling_ = dbToGain(ceilingDb_());
        if (ceiling_ < 1.0e-6)
        {
            ceiling_ = 1.0e-6;
        }
        makeup_ = 1.0 / ceiling_;
    }

    void attackChanged() noexcept
    {
        const double rate = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        const double attMs = attackMs_() > 0.0 ? attackMs_() : 0.0;
        attCoeff_ = attMs <= 0.0 ? 1.0 : 1.0 - std::exp(-1.0 / (attMs * 0.001 * rate));
    }

    void releaseChanged() noexcept
    {
        const double rate = sampleRate_() > 1.0 ? sampleRate_() : 44100.0;
        double relMs = releaseMs_();
        if (!(relMs * 0.0 == 0.0) || relMs < 0.0)
        {
            relMs = 100.0;
        }
        relCoeff_ = relMs <= 0.0 ? 1.0 : 1.0 - std::exp(-1.0 / (relMs * 0.001 * rate));
    }

    void sampleRateChanged() noexcept
    {
        attackChanged();
        releaseChanged();
    }

    void syncControlParams() noexcept
    {
        const double ceilingDb = ceilingDb_();
        const double attackMs = attackMs_();
        const double releaseMs = releaseMs_();
        const double sampleRate = sampleRate_();
        const double dipGain = dipGain_();

        if (ceilingDb != lastCeilingDb_)
        {
            lastCeilingDb_ = ceilingDb;
            ceilingChanged();
        }
        if (sampleRate != lastSampleRate_)
        {
            lastSampleRate_ = sampleRate;
            sampleRateChanged();
            lastAttackMs_ = attackMs;
            lastReleaseMs_ = releaseMs;
        }
        else
        {
            if (attackMs != lastAttackMs_)
            {
                lastAttackMs_ = attackMs;
                attackChanged();
            }
            if (releaseMs != lastReleaseMs_)
            {
                lastReleaseMs_ = releaseMs;
                releaseChanged();
            }
        }
        if (dipGain != lastDipGain_)
        {
            lastDipGain_ = dipGain;
        }
    }

    // lookaheadSamples: already-resolved delay in samples (host may derive from ms).
    // gainCompensation / lookaheadEnabled: 0 or 1 style flags from host.
    double process(
      double left,
      double right,
      int lookaheadSamples,
      bool lookaheadEnabled,
      bool gainCompensation) noexcept
    {
        const double lIn = left;
        const double rIn = right;
        int la = lookaheadEnabled ? lookaheadSamples : 0;
        if (la < 0)
        {
            la = 0;
        }
        if (la > kMaxSamples - 1)
        {
            la = kMaxSamples - 1;
        }

        const double peak = std::fabs(lIn) > std::fabs(rIn) ? std::fabs(lIn) : std::fabs(rIn);
        if (peak > env_)
        {
            env_ += attCoeff_ * (peak - env_);
        }
        else
        {
            env_ += relCoeff_ * (peak - env_);
        }
        if (env_ < 1.0e-25)
        {
            env_ = 0.0;
        }

        double targetGain = 1.0;
        if (env_ > ceiling_)
        {
            targetGain = ceiling_ / env_;
            const double dip = lastDipGain_;
            if (dip != 1.0 && targetGain < 1.0)
            {
                targetGain = std::exp(dip * std::log(targetGain));
            }
        }
        if (targetGain < gain_)
        {
            gain_ += attCoeff_ * (targetGain - gain_);
        }
        else
        {
            gain_ += relCoeff_ * (targetGain - gain_);
        }
        if (!(gain_ * 0.0 == 0.0) || gain_ < 0.0)
        {
            gain_ = 0.0;
        }
        if (gain_ > 1.0)
        {
            gain_ = 1.0;
        }

        delayL_[pos_] = static_cast<float>(lIn);
        delayR_[pos_] = static_cast<float>(rIn);
        int readPos = pos_ - la;
        readPos %= kMaxSamples;
        if (readPos < 0)
        {
            readPos += kMaxSamples;
        }
        double dL = static_cast<double>(delayL_[readPos]);
        double dR = static_cast<double>(delayR_[readPos]);
        pos_ = (pos_ + 1) % kMaxSamples;

        dL *= gain_;
        dR *= gain_;
        if (dL > ceiling_)
        {
            dL = ceiling_;
        }
        else if (dL < -ceiling_)
        {
            dL = -ceiling_;
        }
        if (dR > ceiling_)
        {
            dR = ceiling_;
        }
        else if (dR < -ceiling_)
        {
            dR = -ceiling_;
        }
        if (gainCompensation)
        {
            dL *= makeup_;
            dR *= makeup_;
        }
        if (!(dL * 0.0 == 0.0))
        {
            dL = 0.0;
        }
        if (!(dR * 0.0 == 0.0))
        {
            dR = 0.0;
        }

        lastLeft_ = dL;
        lastRight_ = dR;
        lastGain_ = gain_;
        return 0.5 * (dL + dR);
    }

    void reset() noexcept
    {
        for (int i = 0; i < kMaxSamples; ++i)
        {
            delayL_[i] = 0.0f;
            delayR_[i] = 0.0f;
        }
        pos_ = 0;
        gain_ = 1.0;
        env_ = 0.0;
        lastLeft_ = 0.0;
        lastRight_ = 0.0;
        lastGain_ = 1.0;
    }

    double* mem = nullptr;
    std::uint32_t base{};

    double lastCeilingDb_{};
    double lastAttackMs_{};
    double lastReleaseMs_{};
    double lastSampleRate_{};
    double lastDipGain_{ 1.0 };

    double ceiling_{ 1.0 };
    double makeup_{ 1.0 };
    double attCoeff_{ 1.0 };
    double relCoeff_{ 1.0 };

    float delayL_[kMaxSamples]{};
    float delayR_[kMaxSamples]{};
    int pos_{};
    double gain_{ 1.0 };
    double env_{};
    double lastLeft_{};
    double lastRight_{};
    double lastGain_{ 1.0 };
};

} // namespace soemdsp::dynamics::atom
