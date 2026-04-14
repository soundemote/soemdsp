#pragma once

#include "OMSBinaryData.h"

struct Resources : public ResourcesBase
{
	struct font
	{
		Font TekoRegular = loadFont(OMSBinaryData::TekoRegular_ttf, OMSBinaryData::TekoRegular_ttfSize);
		Font TekoMedium = loadFont(OMSBinaryData::TekoMedium_ttf, OMSBinaryData::TekoMedium_ttfSize);
		Font TekoSemiBold = loadFont(OMSBinaryData::TekoSemiBold_ttf, OMSBinaryData::TekoSemiBold_ttfSize);

		Font RobotoThin = loadFont(OMSBinaryData::RobotoThin_ttf, OMSBinaryData::RobotoThin_ttfSize);
		Font RobotoLight = loadFont(OMSBinaryData::RobotoLight_ttf, OMSBinaryData::RobotoLight_ttfSize);
		Font RobotoRegular = loadFont(OMSBinaryData::RobotoRegular_ttf, OMSBinaryData::RobotoRegular_ttfSize);
		Font RobotoMedium = loadFont(OMSBinaryData::RobotoMedium_ttf, OMSBinaryData::RobotoMedium_ttfSize);
		Font RobotoBold = loadFont(OMSBinaryData::RobotoBold_ttf, OMSBinaryData::RobotoBold_ttfSize);
		Font RobotoBlack = loadFont(OMSBinaryData::RobotoBlack_ttf, OMSBinaryData::RobotoBlack_ttfSize);
	} fonts;

	struct image
	{
		OneStateImage Background{ OMSBinaryData::radar_bg_synth_png, OMSBinaryData::radar_bg_synth_pngSize };

		OneStateImage breakpoint_bg{ OMSBinaryData::radar_bg_breakpoint_png, OMSBinaryData::radar_bg_breakpoint_pngSize };
		OneStateImage fx_bg{ OMSBinaryData::radar_bg_fx_png, OMSBinaryData::radar_bg_fx_pngSize };
		OneStateImage modulation_bg{ OMSBinaryData::radar_bg_modulation_png, OMSBinaryData::radar_bg_modulation_pngSize };

		OneStateImage preset_cancel{ OMSBinaryData::preset_cancel_btn_png, OMSBinaryData::preset_cancel_btn_pngSize };
		OneStateImage preset_x{ OMSBinaryData::preset_close_btn_png, OMSBinaryData::preset_close_btn_pngSize };
		OneStateImage preset_delete{ OMSBinaryData::preset_delete_btn_png, OMSBinaryData::preset_delete_btn_pngSize };
		OneStateImage preset_rename{ OMSBinaryData::preset_rename_btn_png, OMSBinaryData::preset_rename_btn_pngSize };
		OneStateImage preset_save{ OMSBinaryData::preset_save_btn_png, OMSBinaryData::preset_save_btn_pngSize };

		OneStateImage radar_knob_bg{ OMSBinaryData::radar_knob_bg_png, OMSBinaryData::radar_knob_bg_pngSize };
		OneStateImage radar_knob_bg_shadow{ OMSBinaryData::radar_knob_bg_shadow_png, OMSBinaryData::radar_knob_bg_shadow_pngSize };
		OneStateImage radar_knob_sm_base{ OMSBinaryData::radar_knob_sm_base_png, OMSBinaryData::radar_knob_sm_base_pngSize };
		OneStateImage radar_knob_sm_pointer{ OMSBinaryData::radar_knob_sm_pointer_png, OMSBinaryData::radar_knob_sm_pointer_pngSize };
		OneStateImage radar_knob_sm_shadow{ OMSBinaryData::radar_knob_sm_shadow_png, OMSBinaryData::radar_knob_sm_shadow_pngSize };

		OneStateImage radar_menu_lg{ OMSBinaryData::radar_menu_lg_png, OMSBinaryData::radar_menu_lg_pngSize };
		OneStateImage radar_menu_sm{ OMSBinaryData::radar_menu_sm_png, OMSBinaryData::radar_menu_sm_pngSize };
		OneStateImage radar_number_box{ OMSBinaryData::radar_number_box_png, OMSBinaryData::radar_number_box_pngSize };

		TwoStateImage radar_tab{ OMSBinaryData::radar_tab_breakpoint_inactive_png, OMSBinaryData::radar_tab_breakpoint_inactive_pngSize, OMSBinaryData::radar_tab_mod_active_png, OMSBinaryData::radar_tab_mod_active_pngSize };
		TwoStateImage radar_tab_corner{ OMSBinaryData::radar_tab_corner_sm_inactive_png, OMSBinaryData::radar_tab_corner_sm_inactive_pngSize, OMSBinaryData::radar_tab_corner_active_png, OMSBinaryData::radar_tab_corner_active_pngSize };
		OneStateImage radar_tab_fx_inactive{ OMSBinaryData::radar_tab_fx_inactive_png, OMSBinaryData::radar_tab_fx_inactive_pngSize };
		TwoStateImage radar_tab_mid{ OMSBinaryData::radar_tab_mid_inactive_png, OMSBinaryData::radar_tab_mid_inactive_pngSize, OMSBinaryData::radar_tab_mid_active_png, OMSBinaryData::radar_tab_mid_active_pngSize };

		TwoStateImage led_button_big{ OMSBinaryData::led_btn_off_bg_png, OMSBinaryData::led_btn_off_bg_pngSize, OMSBinaryData::led_btn_on_bg_png, OMSBinaryData::led_btn_on_bg_pngSize };
		TwoStateImage led_button_small{ OMSBinaryData::led_btn_off_sm_png, OMSBinaryData::led_btn_off_sm_pngSize, OMSBinaryData::led_btn_on_sm_png, OMSBinaryData::led_btn_on_sm_pngSize };
		TwoStateImage rocker_switch{ OMSBinaryData::switch_off_png, OMSBinaryData::switch_off_pngSize, OMSBinaryData::switch_on_png, OMSBinaryData::switch_on_pngSize };

		OneStateImage slider_handle{ OMSBinaryData::slider_handle_png, OMSBinaryData::slider_handle_pngSize };
		OneStateImage dropdown_color{ OMSBinaryData::dropdown_color_png, OMSBinaryData::dropdown_color_pngSize };
		OneStateImage icon_brightnes{ OMSBinaryData::icon_brightnes_png, OMSBinaryData::icon_brightnes_pngSize };
		OneStateImage icon_dots{ OMSBinaryData::icon_dots_png, OMSBinaryData::icon_dots_pngSize };
		OneStateImage icon_load{ OMSBinaryData::icon_load_png, OMSBinaryData::icon_load_pngSize };
		OneStateImage icon_overglow{ OMSBinaryData::icon_overglow_png, OMSBinaryData::icon_overglow_pngSize };
		OneStateImage icon_presetb{ OMSBinaryData::icon_presetb_png, OMSBinaryData::icon_presetb_pngSize };
		OneStateImage icon_save{ OMSBinaryData::icon_save_png, OMSBinaryData::icon_save_pngSize };
		OneStateImage icon_zdepth{ OMSBinaryData::icon_zdepth_png, OMSBinaryData::icon_zdepth_pngSize };

		OneStateImage leftInputBox{ OMSBinaryData::inputbox_left_png, OMSBinaryData::inputbox_left_pngSize };
		OneStateImage midInputBox{ OMSBinaryData::inputbox_middle_png, OMSBinaryData::inputbox_middle_pngSize };
		OneStateImage rightInputBox{ OMSBinaryData::inputbox_right_png, OMSBinaryData::inputbox_right_pngSize };

		Filmstrip wheel_fs{ OMSBinaryData::wheel_fs_png, OMSBinaryData::wheel_fs_pngSize, 25, 52 };
		Filmstrip knob1_filmstrip{ OMSBinaryData::knob1_filmstrip_png, OMSBinaryData::knob1_filmstrip_pngSize, 40, 40 };
		Filmstrip knob2_filmstrip{ OMSBinaryData::knob2_filmstrip_png, OMSBinaryData::knob2_filmstrip_pngSize, 40, 40 };
		Filmstrip knob3_filmstrip{ OMSBinaryData::knob3_filmstrip_png, OMSBinaryData::knob3_filmstrip_pngSize, 24, 24 };
		Filmstrip knob4_filmstrip{ OMSBinaryData::knob4_filmstrip_png, OMSBinaryData::knob4_filmstrip_pngSize, 20, 20 };
		Filmstrip knob5_filmstrip{ OMSBinaryData::knob5_filmstrip_png, OMSBinaryData::knob5_filmstrip_pngSize, 20, 20 };
		Filmstrip display_slider_filmstrip{ OMSBinaryData::display_slider_filmstrip_png, OMSBinaryData::display_slider_filmstrip_pngSize, 23, 15 };

		OneStateImage presetWindow_box_f{ OMSBinaryData::presetwindow_box_f_png, OMSBinaryData::presetwindow_box_f_pngSize };
		OneStateImage presetWindow_box_u{ OMSBinaryData::presetwindow_box_u_png, OMSBinaryData::presetwindow_box_u_pngSize };
		TwoStateImage presetWindow_f{ OMSBinaryData::presetwindow_factory_disabled_png, OMSBinaryData::presetwindow_factory_disabled_pngSize, OMSBinaryData::presetwindow_factory_enabled_png, OMSBinaryData::presetwindow_factory_enabled_pngSize };
		TwoStateImage presetWindow_u{ OMSBinaryData::presetwindow_user_disabled_png, OMSBinaryData::presetwindow_user_disabled_pngSize, OMSBinaryData::presetwindow_user_enabled_png, OMSBinaryData::presetwindow_user_enabled_pngSize };
	} images;
};
