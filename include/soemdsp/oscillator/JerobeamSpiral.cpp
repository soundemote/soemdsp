#include "JerobeamSpiral.h"

#include "soemdsp/semath.hpp"

using std::clamp;

namespace soemdsp::oscillator {

void JerobeamSpiral::rotate(double in_x, double in_y, double in_z, double rot_x, double rot_y, double* out_x, double* out_y, double* out_z) {
    double cos_rotx, cos_roty, sin_rotx, sin_roty;
    sinCos(rot_x, &sin_rotx, &cos_rotx); /*optimization*/
    sinCos(rot_y, &sin_roty, &cos_roty); /*optimization*/

    double help_11 = in_x * cos_rotx - in_y * sin_rotx;
    double help_12 = in_x * sin_rotx + in_y * cos_rotx;

    double help_21 = help_11 * cos_roty - in_z * sin_roty;
    double help_22 = help_11 * sin_roty + in_z * cos_roty;

    *out_x = help_12;
    *out_y = help_21;
    *out_z = help_22;
}

void JerobeamSpiral::render(double in_x, double in_y, double in_z, double zdepth, double* out_l, double* out_r) {
    double formula = zdepth * 1.25 * (in_z / 2 + 0.5); /*optimization*/
    double L       = in_x - formula * in_x;
    double R       = in_y - formula * in_y;

    double m = 1 + zdepth;

    *out_l = L * m;
    *out_r = R * m;
}

void JerobeamSpiral::spiral(double lophas, double phasor, double dense, double div, double morph, double* x_out, double* y_out, double* z_out) {
    double clamp_morph_0c1 = clamp(morph, 0.0, 1.0); /*optimization*/
    double clamp_morph_0c2 = clamp(morph, 0.0, 2.0); /*optimization*/

    double formula_001 = kPIz2 * (lophas - .5) * clamp_morph_0c2 + kPIz4; /*optimization*/
    double losin, locos;
    sinCos(formula_001, &losin, &locos); /*optimization*/

    double lo_x        = 0.0;
    double formula_002 = pow(clamp_morph_0c1, 2.0); /*optimization*/
    double one_z_div   = 1.0 / div;                 /*optimization*/
    double lo_y        = formula_002 * (1 - one_z_div * losin);
    double lo_z        = formula_002 * (1 - one_z_div * locos);

    double formula_003 = kPI / (2.0 + 6.0 * (1.0 - clamp_morph_0c1)) * (lophas - .5) * clamp_morph_0c2 + kPIz4; /*optimization*/
    sinCos(formula_003, &losin, &locos);                                                                        /*optimization*/

    double kTAU_x_phasor = kTAU * phasor; /*optimization*/
    double sp0sin, sp0cos;
    sinCos(kTAU_x_phasor, &sp0sin, &sp0cos); /*optimization*/

    double spiral0_x = sp0sin;
    double spiral0_y = sp0cos * losin;
    double spiral0_z = sp0cos * locos;

    double sp1sin, sp1cos;
    sinCos(dense * kTAU_x_phasor - kPIz2, &sp1sin, &sp1cos); /*optimization*/
    sp1sin *= -1.0;

    double sp1sin_x_sp01in = sp1sin * sp0sin;
    double spiral1_x       = div * sp1sin_x_sp01in;
    double spiral1_y       = div * ((sp1sin * sp0cos) * losin + sp1cos * locos);
    double spiral1_z       = div * (sp1cos * -losin + (sp1sin * sp0cos) * locos);

    double sp2sin, sp2cos;
    sinCos(dense * dense * kTAU * phasor, &sp2cos, &sp2sin); /*optimization*/
    sp2cos *= -1.0;

    double div_x_div = div * div; /*optimization*/
    double spiral2_x = div_x_div * (sp2cos * sp0cos + sp2sin * sp1sin_x_sp01in);
    double spiral2_y = div_x_div * ((sp2cos * -sp0sin + sp2sin * sp1sin * sp0cos) * losin + (sp2sin * sp1cos) * locos);
    double spiral2_z = div_x_div * ((sp2sin * sp1cos) * -losin + (sp2cos * -sp0sin + sp2sin * sp1sin * sp0cos) * locos);

    double wave_x = (1.0 * lo_x + 1.0 * spiral0_x + 1.0 * spiral1_x + 1.0 * spiral2_x);
    double wave_y = (1.0 * lo_y + 1.0 * spiral0_y + 1.0 * spiral1_y + 1.0 * spiral2_y);
    double wave_z = (1.0 * lo_z + 1.0 * spiral0_z + 1.0 * spiral1_z + 1.0 * spiral2_z);

    double x = exp((morph)*log(div));
    wave_x *= x;
    wave_y *= x;
    wave_z *= x;

    double y{};
    double formula_004 = exp(morph * log(dense)) / 4; /*optimization*/
    if (formula_004 < 1)
        y = pow(1 - formula_004, 2);

    x = x * sin(kPIz4) * y;
    wave_x -= x;
    wave_y += x;

    rotate(wave_x, wave_y, wave_z, 0.0, 0.0, x_out, y_out, z_out);
}

void JerobeamSpiral::getSampleFrame(double* outL, double* outR) {
    double z_darkness = pow(pow(z_amount, 2) * 5.0 + 1.0, z_history);

    phasor.setFrequency(frequency * z_darkness);

    fphas_ends        = waveshape::unipolar::trisaw(phasor.getSample(), sharp);
    double fphas_mids = crvmult * (asin((asin(fphas_ends * 2.0 - 1.0) / kPI + .5) * 2.0 - 1.0) / kPI + .5);
    double lophas     = bright_dist * fphas_mids + (1.0 - bright_dist) * fphas_ends;

    double morph  = phasor_morph.getSampleBipolar() + 0.5;
    double morph2 = morph + 1.0;
    if (morph2 > 1.5) {
        morph2 -= 2.0;
    }

    double fmod_lophas_p5c1p0 = fmod((lophas - .5), 1.0); /*optimization*/

    double phas  = fmod(fmod_lophas_p5c1p0 * exp(morph * log_dense) / 4.0 + 0.375, 1.0);
    double phas2 = fmod(fmod_lophas_p5c1p0 * exp(morph2 * log_dense) / 4.0 + 0.375, 1.0);

    phas += phasor_position.getSample();

    double wave_x1, wave_y1, wave_z1;
    spiral(lophas, phas, dense, div, morph, &wave_x1, &wave_y1, &wave_z1);
    double wave_x2, wave_y2, wave_z2;
    spiral(lophas, phas2, dense, div, morph2, &wave_x2, &wave_y2, &wave_z2);

    double aswitch;
    aswitch = sin(kPI * morph) / 2.0 + .5;

    double wave_x = wave_x1 * aswitch + wave_x2 * (1.0 - aswitch);
    double wave_y = wave_y1 * aswitch + wave_y2 * (1.0 - aswitch);
    double wave_z = wave_z1 * aswitch + wave_z2 * (1.0 - aswitch);

    double vol_correct = 1.0 / (1 + div + div * div);
    double zdepth_z_2  = zdepth / 2.0; /*optimization*/
    vol_correct        = (vol_correct + zdepth_z_2 - vol_correct * zdepth_z_2);
    wave_x *= vol_correct;
    wave_y *= vol_correct;
    wave_z *= vol_correct;

    wave_y += .25;
    wave_z += .36;

    rotate(wave_x, wave_y, wave_z, -kTAU * phasor_rotX.getSample(), kTAU * phasor_rotY.getSample() - kPIz2, &wave_x, &wave_y, &wave_z);
    render(wave_x, wave_y, wave_z, zdepth, outL, outR);

    // TODO: renable and test bpfilter here
    // z_history = bpfilter.getSample(wave_z); // *(1.0 + zdepth);
    z_history = wave_z; // *(1.0 + zdepth);
}
} // namespace soemdsp::oscillator
