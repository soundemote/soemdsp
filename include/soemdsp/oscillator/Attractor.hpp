#pragma once

#include "soemdsp/Wire.hpp"
#include "soemdsp/sehelper.hpp"
#include "soemdsp/SampleRate.hpp"

#include <array>

namespace soemdsp::oscillator {

struct Attractor {
    Attractor() {
        reset();
    }

    /* Because each attractor has a different set of constants, we use an array to set values arbitrarily */
    void setConstant(size_t index, double value) {
        C[index] = value;
    }
    // io
    virtual void run()   = 0;
    virtual void reset() = 0;
    double x{}, y{}, z{}, w{};

    // parameters
    void frequencyChanged() { // frequency
        h = frequency / SampleRate::freq_;
    }
    double frequency{ 20.0 };
    std::array<double, 10> C; // array of constants

    // coefficients
    double h{ frequency / SampleRate::freq_ }; // time step variable
    double dx{}, dy{}, dz{}, dw{};
};

struct Lorentz : public Attractor {
    Lorentz() {
        C[sigma] = 10.0;
        C[rho]   = 28.0;
        C[beta]  = 8.0 / 3.0;
        reset();
    }
    // io
    enum {
        sigma,
        rho,
        beta
    };

    void run() {
        dx = C[sigma] * (y - x);
        dy = x * (C[rho] - z) - y;
        dz = x * y - C[beta] * z;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = 0.5;
        y = 0.0;
        z = 0.0;
    }
};

class Aizawa
  : public Attractor // http://www.3d-meier.de/tut19/Seite3.html
{
    enum {
        a,
        b,
        c,
        d,
        e,
        f,
    };

    Aizawa() {
        C[a] = 0.95;
        C[b] = 0.7;
        C[c] = 0.6;
        C[d] = 3.5;
        C[e] = 0.25;
        C[f] = 0.1;
        reset();
    }

    void run() {
        dx = (z - C[b]) * x - C[d] * y;
        dy = C[d] * x + (z - C[b]) * y;
        dz = C[c] + C[a] * z - (pow(z, 3) / 3.0) - (x * x + y * y) * (1 + C[e] * z) + C[f] * z * pow(x, 3);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = 0;
        z = 0;
    }
};

class Bouali
  : public Attractor // http://www.3d-meier.de/tut19/Seite5.html
{
    enum {
        a,
        s,
    };

    Bouali() {
        C[a] = 0.3;
        C[s] = 1.0;
        reset();
    }

    void run() {
        dx = x * (4 - y) + C[a] * z;
        dy = -y * (1 - x * x);
        dz = -x * (1.5 - C[s] * z) - 0.05 * z;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = 1;
        y = 0.1;
        z = 0.1;
    }
};

class ChenLee
  : public Attractor // http://www.3d-meier.de/tut19/Seite8.html
{
    enum {
        a,
        b,
        c,
    };

    ChenLee() {
        C[a] = 5;
        C[b] = -10;
        C[c] = -0.38;
        reset();
    };

    void run() {
        dx = C[a] * x - y * z;
        dy = C[b] * y + x * z;
        dz = C[c] * z + x * y / 3.0;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = 1;
        y = 0;
        z = 4.5;
    }
};

class DequanLi
  : public Attractor // http://www.3d-meier.de/tut19/Seite9.html
{
    enum {
        a,
        c,
        d,
        e,
        k,
        f,
    };

    DequanLi() {
        C[a] = 40;
        C[c] = 1.833;
        C[d] = 0.16;
        C[e] = 0.65;
        C[k] = 55;
        C[f] = 20;
        reset();
    }

    void run() {
        dx = C[a] * (y - x) + C[d] * x * z;
        dy = C[k] * x + C[f] * y - x * z;
        dz = C[c] * z + x * y - C[e] * pow(x, 2);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .349;
        y = 0;
        z = -.16;
    }
};

class DenTSUCS2
  : public Attractor // http://www.3d-meier.de/tut19/Seite43.html
{
    enum {
        a,
        b,
        c,
        d,
        e,
        f,
    };

    DenTSUCS2() {
        C[a] = 40;
        C[b] = 55;
        C[c] = 1.833;
        C[d] = 0.16;
        C[e] = 0.65;
        C[f] = 20;
        reset();
    };

    void run() {
        dx = C[a] * (y - x) + C[d] * x * z;
        dy = C[b] * x - x * z + C[f] * y;
        dz = C[c] * z + x * y - C[e] * pow(x, 2);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = 1;
        z = -.1;
    }
};

class DenGenesioTesi
  : public Attractor // http://www.3d-meier.de/tut19/Seite11.html
{
    enum {
        a,
        b,
        c,
    };

    DenGenesioTesi() {
        C[a] = .44;
        C[b] = 1.1;
        C[c] = 1.0;
        reset();
    }

    void run() {
        dx = y;
        dy = z;
        dz = -C[c] * x - C[b] * y - C[a] * z + pow(x, 2);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class Hadley
  : public Attractor // http://www.3d-meier.de/tut19/Seite12.html
{
    enum {
        a,
        b,
        f,
        g,
    };

    Hadley() {
        C[a] = .2;
        C[b] = 4;
        C[f] = 8;
        C[g] = 1;
        reset();
    }

    void run() {
        dx = -pow(y, 2) - pow(z, 2) - C[a] * x + C[a] * C[f];
        dy = x * y - C[b] * x * z - y + C[g];
        dz = C[b] * x * y + x * z - z;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = 0;
        z = 0;
    }
};

class Halvorsen
  : public Attractor // http://www.3d-meier.de/tut19/Seite13.html
{
    enum {
        a,
    };

    Halvorsen() {
        C[a] = 1.4;
        reset();
    }

    void run() {
        dx = -C[a] * x - 4 * y - 4 * z - pow(y, 2);
        dy = -C[a] * y - 4 * z - 4 * x - pow(z, 2);
        dz = -C[a] * z - 4 * x - 4 * y - pow(x, 2);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = 1;
        y = 0;
        z = 0;
    }
};

class HyperchaoticQi
  : public Attractor // http://www.3d-meier.de/tut19/Seite90.html
{
    enum {
        a,
        b,
        c,
        d,
        e,
        f,
    };

    HyperchaoticQi() {
        C[a] = 50;
        C[b] = 24;
        C[c] = 13;
        C[d] = 8;
        C[e] = 33;
        C[f] = 30;

        reset();
    }

    void run() {
        dx = C[a] * (y - x) + y * z;
        dy = C[b] * (x + y) - x * z;
        dz = -C[c] * z - C[e] * w + x * y;
        dw = -C[d] * w + C[f] * z + x * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
        w += h * dw;
    }

    void reset() {
        x = 10;
        y = 15;
        z = 20;
        w = 22;
    }
};

class Dadra
  : public Attractor // http://www.3d-meier.de/tut19/Seite77.html
{
    enum {
        p,
        q,
        r,
        s,
        e,
    };

    Dadra() {
        C[p] = 3;
        C[q] = 2.7;
        C[r] = 1.7;
        C[s] = 2;
        C[e] = 9;
        reset();
    }

    void run() {
        dx = y - C[p] * x + C[q] * y * z;
        dy = C[r] * y - x * z + z;
        dz = C[s] * x * y - C[e] * z;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class DenSprottLinzJ
  : public Attractor // http://www.3d-meier.de/tut19/Seite29.html
{
    enum {
        a,
    };

    DenSprottLinzJ() {
        C[a] = 2;
        reset();
    }

    void run() {
        dx = C[a] * z;
        dy = -C[a] * y + z;
        dz = -x + y + y * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class LiuChen
  : public Attractor // http://www.3d-meier.de/tut19/Seite46.html
{
    enum {
        a,
        b,
        c,
        d,
        e,
        f,
        g,
    };

    LiuChen() {
        C[a] = 2.4;
        C[b] = -3.78;
        C[c] = 14;
        C[d] = -11;
        C[e] = 4;
        C[f] = 5.58;
        C[g] = -1;
        reset();
    };

    void run() {
        dx = C[a] * y + C[b] * x + C[c] * y * z;
        dy = C[d] * y - z + C[e] * x * z;
        dz = C[f] * z + C[g] * x * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = 1;
        y = 3;
        z = 5;
    }
};

class MultiSprottC
  : public Attractor // http://www.3d-meier.de/tut19/Seite192.html
{
    enum {
        a,
        b,
    };

    MultiSprottC() {
        C[a] = 2.5;
        C[b] = 1.5;
        reset();
    }

    void run() {
        dx = C[a] * (y - x);
        dy = x * z;
        dz = C[b] - y * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class SprottLinz
  : public Attractor // http://www.3d-meier.de/tut19/Seite20.html
{
    SprottLinz() {
        reset();
    }

    void run() {
        dx = y;
        dy = -x + y * z;
        dz = 1 - y * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class Thomas
  : public Attractor // http://www.3d-meier.de/tut19/Seite20.html
{
    enum {
        b,
    };

    Thomas() {
        C[b] = .19;
        reset();
    }

    void run() {
        dx = -C[b] * x + sin(y);
        dy = -C[b] * y + sin(z);
        dz = -C[b] * z + sin(x);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = 0;
        z = 0;
    }
};

class LorenzMod1
  : public Attractor // http://www.3d-meier.de/tut19/Seite79.html
{
    enum {
        a,
        b,
        c,
        d,
    };

    LorenzMod1() {
        C[a] = 0.1;
        C[b] = 4;
        C[c] = 14;
        C[d] = 0.08;
        reset();
    }

    void run() {
        dx = -C[a] * x + y * y - z * z + C[a] * C[c];
        dy = x * (y - C[b] * z) + C[d];
        dz = z + x * (C[b] * y + z);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class LorXYZ15
  : public Attractor // https://arxiv.org/ftp/arxiv/papers/1409/1409.7842.pdf
{
    enum {
        a,
        b,
        c,
    };

    LorXYZ15() {
        C[a] = 2;
        C[b] = 0.82942;
        C[c] = 7.178;
        reset();
    }

    void run() {
        double sinY = sin(y);
        dx          = z + sin(pow(y, C[a])) * (z + sinY);
        dy          = z + sin(x * y) * (sinY - C[b]);
        dz          = x * (sinY - C[c]);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .1;
        y = .1;
        z = .1;
    }
};

class WangSun
  : public Attractor // http://www.3d-meier.de/tut19/Seite99.html
{
    enum {
        a,
        b,
        c,
        d,
        e,
        f,
    };

    WangSun() {
        C[a] = .2;
        C[b] = -.01;
        C[c] = 1;
        C[d] = -0.4;
        C[e] = -1;
        C[f] = -1;
        reset();
    }

    void run() {
        dx = x * C[a] + C[c] * y * z;
        dy = C[b] * x + C[d] * y - x * z;
        dz = C[e] * z + C[f] * x * y;

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = .3;
        y = .1;
        z = 1;
    }
};

class HindenmarshRose
  : public Attractor // https://en.wikipedia.org/wiki/Hindmarsh%E2%80%93Rose_model
{
    enum {
        a,
        b,
        c,
        d,
        r,
        xR,
        s,
        I,
    };

    HindenmarshRose() {
        C[a]  = 1;
        C[b]  = 3;
        C[c]  = 1;
        C[d]  = 5;
        C[r]  = .005;
        C[s]  = 4.0;
        C[xR] = -1.6;
        C[I]  = 3.25;
        reset();
    }

    void run() {
        dx = y + (C[b] * x * x) - (C[a] * x * x * x) - z + C[I];
        dy = C[c] - (5 * x * x) - y;
        dz = C[r] * (C[s] * (x - C[xR]) - z);

        x += h * dx;
        y += h * dy;
        z += h * dz;
    }

    void reset() {
        x = -1.6; // membrane potential
        y = 4.0;
        z = 2.75;
    }
};
} // namespace soemdsp::oscillator
