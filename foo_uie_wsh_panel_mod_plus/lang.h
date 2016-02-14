#pragma once

enum t_ui_lang
{
	t_lang_en_us = 0,
	t_lang_zh_cn,
	t_lang_unknown = -1,
};

t_ui_lang get_ui_language();

void set_ui_language(t_ui_lang p_lang);

const wchar_t* load_lang(UINT id, std::wstring& p_out);

const char* load_lang(UINT id, pfc::string_base& p_out);