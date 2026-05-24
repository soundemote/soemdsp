#pragma once

// todo: change the #defines into constants and wrap them into the RAPT namespace - we don't want
// to pollute everything with #defines

// mathematical constants (maybe prepend RS_ - but turn them into constexpr):

// Provide a PI macro (do NOT define kPI here - semath.hpp defines constexpr kPI)
#ifndef PI
  #define PI 3.141592653589793238462643383279502884
#endif

#define TWO_PI 6.283185307179586476925286766559                     // maybe use TAU
#define PI_INV 0.31830988618379067153776752674503                   // 1/PI
#define EULER 2.7182818284590452353602874713527
#define EULER_CONSTANT 0.5772156649015328606065120900824024310421
#define SQRT2 1.4142135623730950488016887242097
#define SQRT2_INV 0.70710678118654752440084436210485                // 1 / sqrt(2)
#define LN10 2.3025850929940456840179914546844
#define LN10_INV 0.43429448190325182765112891891661                 // 1 / log(10)
#define LN2 0.69314718055994530941723212145818                      // log(2)
#define LN2_INV 1.4426950408889634073599246810019                   // 1 / log(2)
#define GOLDEN_RATIO 1.6180339887498948482045868343656381           // (1+sqrt(5))/2
#define SEMITONE_FACTOR 1.0594630943592952645618252949463           // 12th root of 2
#define PI_F ((float)PI)
#define NANO 0.000000001  // used by romos - get rid of usage there - define a constexpr there

// music/audio related constants:
