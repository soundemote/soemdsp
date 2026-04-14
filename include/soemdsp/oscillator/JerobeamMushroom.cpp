#include "JerobeamMushroom.h"

using namespace soemdsp::oscillator;
using std::clamp;

void JerobeamMushroom::getSampleFrame(double* outL, double* outR) {
    phas           = phasor.getSample();
    double caprot  = capstemRotator.getSample();
    double stemrot = clusterRotator.getSample();

    double phas_x_nom_x_2 = phas * toDouble(nom) * 2; /*optimization*/
    ph                    = triangle(phas_x_nom_x_2, sharp) * grow;
    double stair          = floor(phas_x_nom_x_2) / toDouble(nom);
    double phuk           = fmod(ph * wobble + stair, 1.0);

    double formula_sin, formula_cos;
    sinCos((ph - caprot) * dens * kTAU, &formula_sin, &formula_cos); /*optimization*/

    double shroom_x = formula_sin * width;
    double shroom_y = -formula_cos * width;
    // double shroom_z = formula_cos*width;

    double shroom_head_x = shroom_x * sin(ph * kTAU) * .5;
    double shroom_head_y = shroom_y * 0.1 * sin(ph * kTAU) * clamp(dens, 0.0, 10.0) / 10.0;
    // double shroom_head_z = shroom_z * cos(ph*kTAU)*.5;

    double shroom_stem_x = shroom_x * -.4 * stem;
    double shroom_stem_y = shroom_y * -.1 * stem;
    // double shroom_stem_z = shroom_z * -.4*stem;

    if (ph > head) {
        shroom_x = shroom_head_x;
        shroom_y = shroom_head_y;
        // shroom_z = shroom_head_z;
    } else if (ph > (1.0 - trans) * head) {
        double _1_m_trans_x_head = (1.0 - trans) * head; /*optimization*/
        double formula2          = (ph - _1_m_trans_x_head) / (head - _1_m_trans_x_head);
        shroom_x                 = shroom_head_x * formula2 + shroom_stem_x * (1.0 - formula2);
        shroom_y                 = shroom_head_y * formula2 + shroom_stem_y * (1.0 - formula2);
        // shroom_z = shroom_head_z*formula2 + shroom_stem_z*(1-formula2);
    } else {
        shroom_x = shroom_stem_x;
        shroom_y = shroom_stem_y;
        // shroom_z = shroom_stem_z;
    }

    shroom_x += ph * cos((phuk + stemrot - 0.25) * kTAU) * 0.5 * spread;
    shroom_y += ph * 2.0 - 1.0;
    // shroom_z += ph * sin((phuk+stemrot-0.25)*kTAU) * 0.5 * spread;

    double dual = (round(phas >= .5) * 2.0 - 1.0) * apart;
    shroom_x += shroom_x + dual;

    if (nom > 0) {
        shroom_x = -shroom_x;
    }

    // rotator.apply(&shroom_x, &shroom_y, &shroom_z);

    *outL = shroom_x;
    *outR = shroom_y;
}
