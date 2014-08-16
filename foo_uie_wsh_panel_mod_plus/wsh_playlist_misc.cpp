#include "stdafx.h"
#include "script_interface_impl.h"
#include "panel_manager.h"
#include "wsh_playlist_misc.h"
#include "user_message.h"
#include "dbgtrace.h"

pfc::map_t<UINT,playlist_lock::ptr> wsh_playlist_lock::m_lockers;

void wsh_process_locations_notify::on_completion( const pfc::list_base_const_t<metadb_handle_ptr> & p_items )
{
	TRACK_FUNCTION();

	simple_callback_data< metadb_handle_list > * on_process_locs_data = 
		new simple_callback_data< metadb_handle_list >(p_items);

	panel_manager::instance().post_msg_to_all_pointer(CALLBACK_UWM_ON_PROCESS_LOCATIONS_DONE, 
		on_process_locs_data);

}

bool wsh_playlist_lock::query_items_add( t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection )
{
	if (m_flag & filter_add)return false;
	return true;
}

bool wsh_playlist_lock::query_items_reorder( const t_size * p_order,t_size p_count )
{
	if (m_flag & filter_reorder)return false;
	return true;
}

bool wsh_playlist_lock::query_items_remove( const bit_array & p_mask,bool p_force )
{
	if (m_flag & filter_remove)return false;
	return true;
}

bool wsh_playlist_lock::query_item_replace( t_size p_index,const metadb_handle_ptr & p_old,const metadb_handle_ptr & p_new )
{
	if (m_flag & filter_replace)return false;
	return true;
}

bool wsh_playlist_lock::query_playlist_rename( const char * p_new_name,t_size p_new_name_len )
{
	if (m_flag & filter_rename)return false;
	return true;
}

bool wsh_playlist_lock::query_playlist_remove()
{
	if (m_flag & filter_remove_playlist)return false;
	return true;
}

void wsh_playlist_lock::on_playlist_index_change( t_size p_new_index )
{
	
}

void wsh_playlist_lock::on_playlist_remove()
{
	
}

void wsh_playlist_lock::get_lock_name( pfc::string_base & p_out )
{
	
}

t_uint32 wsh_playlist_lock::get_filter_mask()
{
	return 	filter_add|filter_remove|filter_reorder|filter_replace|filter_rename|filter_remove_playlist;
}

void wsh_playlist_lock::add_playlist_locker( UINT playlistIndex, const service_ptr_t<playlist_lock> &p_lock )
{
	TRACK_FUNCTION();

	m_lockers.set(playlistIndex,p_lock);
}

bool wsh_playlist_lock::get_playlist_locker( UINT playlistIndex, service_ptr_t<playlist_lock>& p_lock )
{
	TRACK_FUNCTION();
	if (m_lockers.have_item(playlistIndex)){
		p_lock = m_lockers[playlistIndex];
		m_lockers.remove(playlistIndex);
		return true;
	}
	return false;
}

void wsh_playlist_loader_callback::handle_create( metadb_handle_ptr & p_out,const playable_location & p_location )
{
	TRACK_FUNCTION();

	static_api_ptr_t<metadb>()->handle_create(p_out,p_location);

	static_api_ptr_t<metadb_io_v2>()->load_info_async(
		pfc::list_single_ref_t<metadb_handle_ptr>(p_out),
		metadb_io_v2::load_info_default,
		core_api::get_main_window(),
		metadb_io_v2::op_flag_background | metadb_io_v2::op_flag_delay_ui,
		NULL);

	static_api_ptr_t<playlist_manager> plman;

	t_size idx = plman->get_active_playlist();
	if (idx != pfc::infinite_size){
		if (!m_clear){
			plman->activeplaylist_clear();
			m_clear = true;
		}
		plman->activeplaylist_add_items(pfc::list_single_ref_t<metadb_handle_ptr>(p_out),bit_array_true());
		plman->playlist_set_selection(plman->get_active_playlist(),bit_array_true(),bit_array_true());
	}
	else{
		m_clear=true;
		t_size plx = static_api_ptr_t<playlist_manager>()->create_playlist_autoname();
		plman->set_active_playlist(plx);
		plman->playlist_add_items(plx,pfc::list_single_ref_t<metadb_handle_ptr>(p_out),bit_array_true());
	}
}
