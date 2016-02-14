#include "stdafx.h"
#include "lang.h"
#include "helpers.h"

t_ui_lang get_ui_language()
{
	static t_ui_lang lang = t_lang_unknown;
	if (lang == t_lang_unknown)
	{
		titleformat_object::ptr obj;
		static_api_ptr_t<titleformat_compiler>()->compile_safe(obj, "$meta()");
		pfc::string8_fast text;
		metadb_handle_ptr _metadb;

		if (!metadb::g_get_random_handle(_metadb))
		{
			static_api_ptr_t<metadb>()->handle_create(_metadb, make_playable_location("file://C:\\________.ogg", 0));
		}

		if (_metadb.is_valid())
		{
			static_api_ptr_t<playback_control>()->playback_format_title_ex(_metadb,NULL,text,obj,NULL,playback_control::display_level_all);
		}
		
		pfc::stringcvt::string_wide_from_utf8 textw(text);

		if (_wcsicmp(textw, L"[Î´Öªº¯Êý]") == 0)
		{
			lang = t_lang_zh_cn;
		}
		else
		{
			lang = t_lang_en_us;
		}
	}
	return lang;
}

void set_ui_language( t_ui_lang p_lang )
{
	if (helpers::is_vista())
	{
		typedef LANGID (WINAPI *pfSetThreadUILanguage)(LANGID langId);
		pfSetThreadUILanguage pf = (pfSetThreadUILanguage)GetProcAddress(LoadLibrary(_T("kernel32.dll")), "SetThreadUILanguage");
		if (pf)
		{
			pf((p_lang == t_lang_zh_cn) ? MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) : MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
		}
	}
	else
	{
		SetThreadLocale((p_lang == t_lang_zh_cn) ? MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_DEFAULT) : MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
	}
}

const wchar_t* load_lang( UINT id, std::wstring& p_out )
{
	enum { MAX_BUF = 1048, };
	p_out.resize(MAX_BUF);
	wchar_t * buf = const_cast<wchar_t*>(p_out.c_str());
	LoadStringW(core_api::get_my_instance(), id, buf, MAX_BUF);
	return p_out.c_str();
}

const char* load_lang( UINT id, pfc::string_base& p_out )
{
	std::wstring out;
	p_out = pfc::stringcvt::string_utf8_from_wide(load_lang(id, out));
	return p_out;
}
