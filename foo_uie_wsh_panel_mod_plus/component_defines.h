#pragma once

/* 
 * IMPORTANT NOTE:
 * For modders: You should change these Defines/Guids below, in order to 
 *   co-exist with the original release of WSH Panel Mod.
 */

#define WSPM_NAME "WSH Panel Mod Plus"
#define WSPM_WINDOW_CLASS_NAME "uie_wsh_panel_mod_plus_class"
#define WSPM_DLL_NAME "foo_uie_wsh_panel_mod_plus.dll"

// {D573DC73-8933-4E53-92EB-0062B1E62DAB}
extern const GUID g_ui_pref_window_guid;
FOOGUIDDECL const GUID g_ui_pref_window_guid = 
{ 0xd573dc73, 0x8933, 0x4e53, { 0x92, 0xeb, 0x0, 0x62, 0xb1, 0xe6, 0x2d, 0xab } };


// {3C2C4BDC-D387-44E9-A2A8-EAAEADCA14CE}
extern const GUID g_wsh_panel_window_extension_guid;
FOOGUIDDECL const GUID g_wsh_panel_window_extension_guid =
{ 0x3c2c4bdc, 0xd387, 0x44e9, { 0xa2, 0xa8, 0xea, 0xae, 0xad, 0xca, 0x14, 0xce } };

// {4F901CF9-9D97-42C1-B7EE-1DEEE92A3AEF}
extern const GUID g_wsh_panel_window_dui_guid;
FOOGUIDDECL const GUID g_wsh_panel_window_dui_guid = 
{ 0x4f901cf9, 0x9d97, 0x42c1, { 0xb7, 0xee, 0x1d, 0xee, 0xe9, 0x2a, 0x3a, 0xef } };

// {DD1C27A4-1ECE-4B28-BFD7-644C254A1D80}
extern const GUID g_guid_prop_sets;
FOOGUIDDECL const GUID g_guid_prop_sets = 
{ 0xdd1c27a4, 0x1ece, 0x4b28, { 0xbf, 0xd7, 0x64, 0x4c, 0x25, 0x4a, 0x1d, 0x80 } };

// {C0A8E593-132E-4B4A-AF2E-AD5721C8CEDC}
extern const GUID g_guid_cfg_safe_mode;
FOOGUIDDECL const GUID g_guid_cfg_safe_mode = 
{ 0xc0a8e593, 0x132e, 0x4b4a, { 0xaf, 0x2e, 0xad, 0x57, 0x21, 0xc8, 0xce, 0xdc } };

// {B590089C-E1E0-41E1-8344-1889C708BE87}
extern const GUID g_guid_cfg_cui_warning_reported;
FOOGUIDDECL const GUID g_guid_cfg_cui_warning_reported = 
{ 0xb590089c, 0xe1e0, 0x41e1, { 0x83, 0x44, 0x18, 0x89, 0xc7, 0x8, 0xbe, 0x87 } };

// {208AB7F8-4F89-4577-BA4D-F1E7990D83FD}
extern const GUID g_guid_mainmenu_help;
FOOGUIDDECL const GUID g_guid_mainmenu_help = 
{ 0x208ab7f8, 0x4f89, 0x4577, { 0xba, 0x4d, 0xf1, 0xe7, 0x99, 0xd, 0x83, 0xfd } };
