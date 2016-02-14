#pragma once
#include "resource.h"

static inline void print_obsolete_message(const char * message)
{
	pfc::string8 lang_msg;
	console::formatter() << load_lang(IDS_WSHM_NAME, lang_msg) << ": " << load_lang(IDS_OBSOLETE_WARNING, lang_msg) << ": " << message;
}

static inline void print_obsolete_message_once(bool& reported, const char * message)
{
    if (reported) return;
    print_obsolete_message(message);
    //reported = true;
}

#define PRINT_OBSOLETE_MESSAGE_ONCE(message) \
    do {\
        static bool ___obsolete_message_reported__  = false;\
        print_obsolete_message_once(___obsolete_message_reported__, message);\
    } while(0)
