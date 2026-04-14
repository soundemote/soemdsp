#pragma once

namespace FCFBinaryData
{
    extern const char*   btn_left_trigger_always_64x44_2x_png;
    const int            btn_left_trigger_always_64x44_2x_pngSize = 4812;

    extern const char*   knob_send_svg;
    const int            knob_send_svgSize = 1460;

    extern const char*   btn_right_trigger_always_64x22_2x_png;
    const int            btn_right_trigger_always_64x22_2x_pngSize = 5020;

    extern const char*   btn_right_trigger_legato_64x22_2x_png;
    const int            btn_right_trigger_legato_64x22_2x_pngSize = 3941;

    extern const char*   backround_2x_png;
    const int            backround_2x_pngSize = 688988;

    extern const char*   btn_left_trigger_legato_64x22_2x_png;
    const int            btn_left_trigger_legato_64x22_2x_pngSize = 4351;

    extern const char*   btn_mono_disabled_2x_png;
    const int            btn_mono_disabled_2x_pngSize = 1028;

    extern const char*   btn_mono_enabled_1_2x_png;
    const int            btn_mono_enabled_1_2x_pngSize = 1089;

    extern const char*   btn_stereo_disabled_2x_png;
    const int            btn_stereo_disabled_2x_pngSize = 1492;

    extern const char*   btn_stereo_enabled_2x_png;
    const int            btn_stereo_enabled_2x_pngSize = 1637;

    extern const char*   button_donate_png;
    const int            button_donate_pngSize = 14928;

    extern const char*   drag_window_svg;
    const int            drag_window_svgSize = 403;

    extern const char*   ef_adsr_section_2x_png;
    const int            ef_adsr_section_2x_pngSize = 101620;

    extern const char*   factory_icon_svg;
    const int            factory_icon_svgSize = 603;

    extern const char*   fcf_modes_buttons_png;
    const int            fcf_modes_buttons_pngSize = 10178;

    extern const char*   knob_big_ringpointer_svg;
    const int            knob_big_ringpointer_svgSize = 755;

    extern const char*   knob_big_png;
    const int            knob_big_pngSize = 15468;

    extern const char*   knob_curve_svg;
    const int            knob_curve_svgSize = 1163;

    extern const char*   knob_small_png;
    const int            knob_small_pngSize = 3153;

    extern const char*   knob_small_ringpointer_svg;
    const int            knob_small_ringpointer_svgSize = 784;

    extern const char*   load_icon_svg;
    const int            load_icon_svgSize = 617;

    extern const char*   save_icon_svg;
    const int            save_icon_svgSize = 652;

    extern const char*   user_icon_svg;
    const int            user_icon_svgSize = 594;

    extern const char*   valuebox_bottom_png;
    const int            valuebox_bottom_pngSize = 1460;

    extern const char*   logo_svg;
    const int            logo_svgSize = 1131;

    extern const char*   ShareTechMonoRegular_ttf;
    const int            ShareTechMonoRegular_ttfSize = 43272;

    extern const char*   RobotoRegular_ttf;
    const int            RobotoRegular_ttfSize = 171676;

    extern const char*   RobotoBold_ttf;
    const int            RobotoBold_ttfSize = 170760;

    extern const char*   TekoLight_ttf;
    const int            TekoLight_ttfSize = 301528;

    extern const char*   TekoMedium_ttf;
    const int            TekoMedium_ttfSize = 310028;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 30;

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
