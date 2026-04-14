#include "JerobeamWirdoSpiral.h"

using namespace soemdsp::oscillator;
using std::clamp;

void JerobeamWirdoSpiral::getSampleFrame(double* outL, double* outR) {
    phas = phasor(freq);
    if (cut < 1000.0) {
        phas = toInt(phas * cut) / cut; // turns ramp into stair -> sample&hold
    }

    double cross_rot  = (phas > tri) * cross_spread * kTAU - cross_spread * pi;
    double cross_phas = triangle(phas, tri);
    if (sync_cut < 1.0) {
        cross_phas = toInt(cross_phas * clamp(abs(dens) * sync_cut, 1.0, 1000.0)) / clamp(abs(dens) * sync_cut, 1.0, 1000.0);
    }
    double crossbow = cross_phas * length - clamp(length - 1.0, 0.0, 1.0);

    double cross_x = crossbow * cos(cross_rot);
    double cross_y = crossbow * sin(cross_rot);

    double spirot   = crossbow * dens;
    double spirot_x = cross_x * cos(spirot) + cross_y * sin(spirot);
    double spirot_y = cross_y * cos(spirot) - cross_x * sin(spirot);
    double splash   = sin((triangle(phas * splash_dens + phasor(splash_speed), 1.0)) * kTAU * scrap);

    if (scrap < .25) {
        splash /= sin(scrap * kTAU);
    }

    if (scrap < .5) {
        splash = splash * 2.0 - 1.0;
    } else if (scrap < .75) { // not working yet
        splash = splash * (2.0 + sin(scrap * kTAU)) - (sin(scrap * kTAU) + 1.0) * (1.0 + sin(scrap * kTAU));
    }

    if (splash_cut < 10.0) {
        splash = int(splash * splash_cut) / splash_cut;
    }

    *outL = spirot_x;
    *outR = spirot_y * cos(z_rot * pi / 2.0) + splash * splash_depth; //*sin(z_rot*pi/2);

    run_gen();
}
