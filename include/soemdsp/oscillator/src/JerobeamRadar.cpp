#include "../JerobeamRadar.h"

using namespace soemdsp::oscillator;
using std::abs;
using std::clamp;

void JerobeamRadar::update_x_y() {
    // x_, y_: x and y projected on a circular shape
    x_ = sin(x * (constant::kPIz4 + (1 - abs(y)) * constant::kPIz4));
    y_ = y * cos(x * constant::kPIz4);
    // r, ph: polar coordinates (radius, angle)
    r = (sign(y_) + (y_ == 0)) * sqrt(x_ * x_ + y_ * y_);

    if (y_ != 0)
        ph = atan(x_ / y_);
    else
        ph = constant::kPIz2 * sign(x_);
}

void JerobeamRadar::render(double in_phas, double& out1, double& out2, double& out3) {
    double phas = triphase = triangle(in_phas, tri1);
    if (phase_inv)
        phas = 1 - triangle(in_phas, tri1);

    // phas = (pow(phas, 20)*pow1/10+phas*(1-pow1/10))*length; // different fade method
    if ((pow1_up && in_phas < tri1) || (pow1_down && in_phas >= tri1))
        phas = pow(phas, pow1);

    phas = phas * (dens + frontring / (tunnel_inv + (!tunnel_inv) * length)) / dens; // frontring

    double sphas = phas;
    if (in_phas > tri1 && spiral_return)
        sphas = 2 - phas;

    double sin_phas = clamp(pow(triangle(sphas * length * dens, tri2), pow2), -1.e+100, +1.e+100); // clampping needed to avoid inf
    double f002_sin, f002_cos;
    rot = rotator.getSample();
    math::sincos((sin_phas - (tunnel_inv * frontring) - rot / lap - (!tunnel_inv * length * dens)) * constant::kTAU * lap, /*&*/f002_sin, /*&*/f002_cos);
    double lilsin = f002_cos * ration;
    double lilcos = f002_sin * ration;

    phas *= length;
    phas = (1 - pow2_bend) * (floor(phas * dens) / dens + sin_phas / dens) + pow2_bend * phas;

    if (ringcut) {
        phas = (floor(phas * dens + tunnel_inv * (1 - frontring)) + rot - tunnel_inv * (1 - frontring)) / dens; // ringcut
    }

    if (!tunnel_inv) {
        phas = 1 - phas - (1 - length) + frontring / dens;
    }

    phas = clamp(phas - frontring / dens, 0.0, 1.0);
    // phas = clamp(phas*(dens+frontring)/(dens+1)-frontring/dens, 0, 1); // frontring

    double ph_sin_neg, ph_cos_neg;
    math::sincos(-ph, /*&*/ph_sin_neg, /*&*/ph_cos_neg);
    double lilsin1 = lilsin * ph_sin_neg + lilcos * ph_cos_neg;
    double lilcos1 = lilcos * ph_sin_neg - lilsin * ph_cos_neg;

    double f003_sin, f003_cos;
    math::sincos(phas * abs(r) * constant::kTAU, /*&*/f003_sin, /*&*/f003_cos);
    double bigsin = +f003_cos;
    double bigcos = -f003_sin;

    double lil_x = lilsin1 * bigsin;
    double lil_y = lilcos1;
    double lil_z = lilsin1 * bigcos * sign(r);

    //---------------- the real deal -----------------

    double big_x{};
    double big_y{};
    double big_z = -constant::kTAU * phas;
    if (r != 0) {
        big_z = bigcos / abs(r);
        big_x = (bigsin - 1) / r;
    }

    double wave_x1 = big_x + lil_x;
    double wave_y1 = big_y + lil_y;
    double wave_z2 = big_z + lil_z;

    double ph_sin, ph_cos;
    math::sincos(ph, /*&*/ph_sin, /*&*/ph_cos);
    double wave_x  = wave_x1 * ph_sin + wave_y1 * ph_cos;
    double wave_y2 = wave_y1 * ph_sin - wave_x1 * ph_cos;

    double syz = 2 * (size + .33) * (abs(x) * (1 - y) + .5);
    wave_x     = size * wave_x + (1 - size) * (wave_x + x * (1 - ratio) + x * ratio) * syz;
    wave_y2    = size * wave_y2 + (1 - size) * (wave_y2 - y) * syz;
    wave_z2    = size * wave_z2 + (1 - size) * wave_z2 * syz;

    double siz_sin, siz_cos;
    math::sincos((1 - size) * constant::kPIz2, /*&*/siz_sin, /*&*/siz_cos);
    double wave_y = wave_y2 * siz_cos + wave_z2 * siz_sin;
    double wave_z = wave_z2 * siz_cos - wave_y2 * siz_sin;

    out1 = wave_x;
    out2 = wave_y;
    out3 = wave_z;
}

void JerobeamRadar::getSampleFrame(double& outL, double& outR) {
    double wave_x, wave_y, wave_z;
    render(phasor.getUnipolarValue(), /*&*/wave_x, /*&*/wave_y, /*&*/wave_z);

    double depth = (1 - z_depth) * (1 - abs(wave_z) * constant::k1zTAU) + z_depth * (pow(z_depth * 9 + 1, wave_z));
    double f001  = (depth * (1 - z_end) + z_end) / ((1 - size) + size * ration);
    outL         = wave_x * f001;
    outR         = wave_y * f001 + y_fix_for_zoom;

    phasor.increment();
}
