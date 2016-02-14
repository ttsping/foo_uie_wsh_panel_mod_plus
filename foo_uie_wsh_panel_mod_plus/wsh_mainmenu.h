#pragma once
#include "resource.h"

class help_commands : public mainmenu_commands
{
public:
	t_uint32 get_command_count()
	{
		return 1;
	}

	GUID get_command(t_uint32 p_index)
	{
		if (p_index == 0)
		{
			return g_guid_mainmenu_help;
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	void get_name(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
		{
			load_lang(IDS_WSHM_DOCS, p_out);
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	bool get_description(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
		{
			p_out = pfc::stringcvt::string_utf8_from_wide(_T("Open WSH Mod Documents¡£"));
			return true;
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}

	GUID get_parent()
	{
		return mainmenu_groups::help;
	}

	t_uint32 get_sort_priority() { return sort_priority_dontcare; }

	virtual bool get_display(t_uint32 p_index,pfc::string_base & p_text,t_uint32 & p_flags) 
	{
		p_flags = flag_disabled;
		get_name(p_index,p_text);
		return true;
	}

	void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0)
		{
			//TODO
		}
		else throw pfc::exception_bug_check("Command index out of range");
	}
};

static mainmenu_commands_factory_t<help_commands> g_mainmenu_commands_factory;