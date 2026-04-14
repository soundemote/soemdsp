#pragma once

namespace MouseKeyboardBinary
{
    extern const char*   key_image_a_pressed_png;
    const int            key_image_a_pressed_pngSize = 407;

    extern const char*   key_image_cf_png;
    const int            key_image_cf_pngSize = 375;

    extern const char*   key_image_cf_pressed_png;
    const int            key_image_cf_pressed_pngSize = 383;

    extern const char*   key_image_d_png;
    const int            key_image_d_pngSize = 398;

    extern const char*   key_image_d_pressed_png;
    const int            key_image_d_pressed_pngSize = 408;

    extern const char*   key_image_eb_png;
    const int            key_image_eb_pngSize = 363;

    extern const char*   key_image_eb_pressed_png;
    const int            key_image_eb_pressed_pngSize = 382;

    extern const char*   key_image_g_png;
    const int            key_image_g_pngSize = 397;

    extern const char*   key_image_g_pressed_png;
    const int            key_image_g_pressed_pngSize = 406;

    extern const char*   key_image_last_png;
    const int            key_image_last_pngSize = 317;

    extern const char*   key_image_last_pressed_png;
    const int            key_image_last_pressed_pngSize = 338;

    extern const char*   key_image_sharp_png;
    const int            key_image_sharp_pngSize = 168;

    extern const char*   key_image_sharp_pressed_png;
    const int            key_image_sharp_pressed_pngSize = 198;

    extern const char*   key_image_a_png;
    const int            key_image_a_pngSize = 398;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 14;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
