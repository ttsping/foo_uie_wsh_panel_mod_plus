#pragma once

class eslyric_callback_impl : public eslyric_callback
{
public:
	virtual void on_lyric_set(const t_para_callback_data& p_info);
	virtual void on_lyric_offset(t_int32 p_offset);
};