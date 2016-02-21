#include "stdafx.h"
#include "wsh_panel_window.h"
#include "ui_name_value_edit.h"
#include "ui_property.h"


// Some marcos defined by windowsx.h should be removed
#ifdef _INC_WINDOWSX
#undef SubclassWindow
#endif

#include "PropertyList.h"

LRESULT CDialogProperty::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	DlgResize_Init();
	if (m_parent->ScriptInfo().name.length())
	{
		uSetWindowText(m_hWnd, m_parent->ScriptInfo().name);
	}
	// Subclassing
	m_properties.SubclassWindow(GetDlgItem(IDC_LIST_PROPERTIES));
	m_properties.ModifyStyle(0, LBS_SORT | LBS_HASSTRINGS);
	m_properties.SetExtendedListStyle(PLS_EX_SORTED | PLS_EX_XPLOOK | PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);

	LoadProperties();

	return TRUE; // set focus to default control
}

LRESULT CDialogProperty::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	switch (wID)
	{
	case IDOK:
		Apply();
		break;

	case IDAPPLY:
		Apply();
		return 0;
	}

	EndDialog(wID);
	return 0;
}

LRESULT CDialogProperty::OnPinItemChanged(LPNMHDR pnmh)
{
	LPNMPROPERTYITEM pnpi = (LPNMPROPERTYITEM)pnmh;

	pfc::stringcvt::string_utf8_from_os uname = pnpi->prop->GetName();

	if (m_dup_prop_map.have_item(uname))
	{
		prop_kv_config::t_val & val = m_dup_prop_map[uname];
		_variant_t var;

		if (pnpi->prop->GetValue(&var))
		{
			if (pnpi->prop->GetKind() == PROPKIND_COLOR)
			{
				GenColorStringProp(var.ulVal, val);
			}
			else
			{
				val.ChangeType(val.vt, &var);
			}
			
		}
	}

	return 0;
}

LRESULT CDialogProperty::OnClearallBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	m_dup_prop_map.remove_all();
	m_properties.ResetContent();

	return 0;
}

void CDialogProperty::Apply()
{
	// Copy back
	m_parent->get_config_prop().get_val() = m_dup_prop_map;
	m_parent->update_script();
	LoadProperties();
}

void CDialogProperty::LoadProperties(bool reload /*= true*/)
{
	m_properties.ResetContent();

	if (reload)
	{
		m_dup_prop_map = m_parent->get_config_prop().get_val();
	}

	for (prop_kv_config::t_map::const_iterator iter = m_dup_prop_map.first(); iter.is_valid(); ++iter)
	{
		pfc::stringcvt::string_wide_from_utf8_fast wname = iter->m_key;
		HPROPERTY hProp = NULL;
		const _variant_t & v = iter->m_value;
		_variant_t var;

		switch (v.vt)
		{
		case VT_BOOL:
			hProp = PropCreateSimple(wname, v.boolVal ? true : false);
			break;

		case VT_I1:
		case VT_UI1:
		case VT_I2:
		case VT_UI2:
		case VT_I4:
		case VT_UI4:
		case VT_I8:
		case VT_UI8:
			var.ChangeType(VT_I4, &v);
			hProp = PropCreateSimple(wname, var.lVal);
			break;

		case VT_BSTR:
		default:
			{
				var.ChangeType(VT_BSTR, &v);
				t_property_tag tag;
				AnalyzeStringProp(v.bstrVal, tag);
				switch(tag.type)
				{
				case t_prop_file:
					hProp = PropCreateFileName(wname, tag.data.lpText);
					break;
				case t_prop_color:
					hProp = PropCreateColor(wname, tag.data.color);
					break;
				case t_prop_simple:
				default:
					hProp = PropCreateSimple(wname, tag.data.lpText);
					break;
				}
			}
			break;
		}

		m_properties.AddItem(hProp);
	}
}

LRESULT CDialogProperty::OnDelBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	int idx = m_properties.GetCurSel();

	if (idx >= 0)
	{
		HPROPERTY hproperty = m_properties.GetProperty(idx);
		pfc::stringcvt::string_utf8_from_os uname = hproperty->GetName();

		m_properties.DeleteItem(hproperty);
		m_dup_prop_map.remove(uname);
	}

	return 0;
}

LRESULT CDialogProperty::OnImportBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pfc::string8 filename, lang_filetype, lang_import;

	if (uGetOpenFileName(m_hWnd, load_lang(IDS_PROP_IMPORT_FILETYPE, lang_filetype), 0, "wsp", load_lang(IDS_IMPORT_IMPORT, lang_filetype), NULL, filename, FALSE))
	{
		file_ptr io;
		abort_callback_dummy abort;

		try
		{
			filesystem::g_open_read(io, filename, abort);
			prop_kv_config::g_load(m_dup_prop_map, io.get_ptr(), abort);
		}
		catch (std::exception &)
		{
			return 0;
		}

		// Refresh display
		LoadProperties(false);
	}

	return 0;
}

LRESULT CDialogProperty::OnExportBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pfc::string8 path, lang_filetype, lang_saveas;

	if (uGetOpenFileName(m_hWnd, load_lang(IDS_PROP_IMPORT_FILETYPE, lang_filetype), 0, "wsp", load_lang(IDS_IMPORT_SAVE_AS, lang_saveas), NULL, path, TRUE))
	{
		file_ptr io;
		abort_callback_dummy abort;

		try
		{
			filesystem::g_open_write_new(io, path, abort);
			prop_kv_config::g_save(m_dup_prop_map, io.get_ptr(), abort);
		}
		catch (std::exception &)
		{

		}
	}

	return 0;
}

LRESULT CDialogProperty::OnPinItemBrowse( LPNMHDR pnmh )
{
	LPNMPROPERTYITEM nmp = (LPNMPROPERTYITEM)pnmh;
	switch(nmp->prop->GetKind())
	{
	case PROPKIND_COLOR:
		{
			_variant_t val;
			nmp->prop->GetValue(&val);
			PFC_ASSERT(V_VT(&val) == VT_UI4);

			static COLORREF custom_colors[16] = { 0 };

			CHOOSECOLOR cc = { 0 };
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = m_hWnd;
			cc.rgbResult = val.ulVal;
			cc.lpCustColors = custom_colors;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;
			if (ChooseColor(&cc))
			{
				CComVariant var(cc.rgbResult);
				m_properties.SetItemValue(nmp->prop, &var);
			}
		}
		break;
	case PROPKIND_FILENAME:
		{
			pfc::string8 path, lang_filetype, lang_title;

			if (uGetOpenFileName(m_hWnd, load_lang(IDS_BROWSE_FILETYPE, lang_filetype), 0, 0, load_lang(IDS_BROWSE_TITLE, lang_title), NULL, path, FALSE))
			{
				CComVariant var(pfc::stringcvt::string_wide_from_utf8(path).get_ptr());
				m_properties.SetItemValue(nmp->prop, &var);
			}
		}
		break;
	case PROPKIND_FOLDERNAME:
		{
			pfc::string8 path, lang_title;
			if (uBrowseForFolder(m_hWnd, load_lang(IDS_BROWSE_TITLE, lang_title), path))
			{
				path.fix_dir_separator();
				CComVariant var(pfc::stringcvt::string_wide_from_utf8(path).get_ptr());
				m_properties.SetItemValue(nmp->prop, &var);
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

VOID CDialogProperty::AnalyzeStringProp( LPCTSTR lpText, t_property_tag& tag )
{
	PFC_ASSERT(lpText);
	tag.type = t_prop_simple;
	tag.data.lpText = lpText;

	const TCHAR* text_start = lpText;
	size_t len = _tcslen(lpText);

	if (*text_start == '#')
	{
		if (len > _tcslen(_T("#FFFFFF")))return;
		++text_start;
		unsigned color = 0;
		size_t n, m = len - 1;
		for (n = 0; n < m; ++n)
		{
			unsigned a = 0;
			TCHAR c = text_start[n];
			if (c >= '0' && c <= '9') a = (unsigned)(c - '0');
			else if (c >= 'a' && c <= 'f') a = (unsigned)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F') a = (unsigned)(c - 'A' + 10);
			else return;
			color = (color << 4) | a;
		}

		BYTE r, g, b;
		r = (color & 0x00FF0000) >> 16;
		g = (color & 0x0000FF00) >> 8;
		b = (color & 0x000000FF) >> 0;
		tag.type = t_prop_color;
		tag.data.color = RGB(r, g, b);
	}
	else if (_tcsstr(text_start, _T("RGB")))
	{
		TCHAR * color_text = new TCHAR[len + 1];
		ZeroMemory(color_text, sizeof(TCHAR) * (len + 1));
		//remove spaces
		TCHAR * color_index = color_text;
		for (t_size n = 0; n < len; ++n)
		{
			if(*text_start == ' ')continue;
			*color_index = text_start[n];
			++color_index;
		}
		color_index = color_text;
		color_index += _tcslen(_T("RGB"));
		if (*color_index != '(' || color_index[_tcslen(color_index) - 1] != ')')return;
		++color_index;//skip '('
		color_index[_tcslen(color_index) - 1] = ',';

		unsigned color = 0;
		const TCHAR *color_start = color_index;
		while(color_start && *color_start)
		{
			TCHAR *color_end = const_cast<TCHAR*>(_tcschr(color_start, ','));
			if(!color_end)break;
			*color_end = '\0';
			color = (color << 8) | (unsigned)_ttoi(color_start);
			color_start = color_end + 1;
		}
		delete []color_text;

		BYTE r, g, b;
		r = (color & 0x00FF0000) >> 16;
		g = (color & 0x0000FF00) >> 8;
		b = (color & 0x000000FF) >> 0;

		tag.type = t_prop_color;
		tag.data.color = RGB(r,g,b);
	}
	else
	{
		TCHAR c = *text_start;
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))return;
		++text_start;
		if ((*text_start != ':') || (*(text_start + 1) != '\\'))return;

		tag.type = t_prop_file;
	}
}

VOID CDialogProperty::GenColorStringProp( COLORREF color, _variant_t& var )
{
	//assume var is color string : "#XXXXXX" or "RGB(X,X,X)"
	PFC_ASSERT(V_VT(&var) == VT_BSTR);
	enum { COLOR_BUF = 32, };
	const TCHAR* color_text = var.bstrVal;

	BYTE r, g, b;
	r = GetRValue(color);
	g = GetGValue(color);
	b = GetBValue(color);

	TCHAR color_buf[COLOR_BUF] = { 0 };
	if (*color_text == '#')
	{
		StringCchPrintf(color_buf, sizeof(color_buf)/sizeof(color_buf[0]), _T("#%02X%02X%02X"), r, g, b);
	}
	else
	{
		StringCchPrintf(color_buf, sizeof(color_buf)/sizeof(color_buf[0]), _T("RGB(%d, %d, %d)"), (int)r, (int)g, (int)b);
	}

	var = color_buf;
}
