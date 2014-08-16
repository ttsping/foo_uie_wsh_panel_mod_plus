#pragma once


class wsh_process_locations_notify : public process_locations_notify
{
public:
	virtual void on_completion(const pfc::list_base_const_t<metadb_handle_ptr> & p_items);
	virtual void on_aborted(){};
};


class wsh_playlist_lock : public playlist_lock
{
public:

	wsh_playlist_lock(t_int32 flag):m_flag(flag){}

	static bool get_playlist_locker(UINT playlistIndex, service_ptr_t<playlist_lock>& p_lock);
	static void add_playlist_locker(UINT playlistIndex, const service_ptr_t<playlist_lock> &p_lock);

	//! Queries whether specified item insertiion operation is allowed in the locked playlist.
	//! @param p_base Index from which the items are being inserted.
	//! @param p_data Items being inserted.
	//! @param p_selection Caller-requested selection state of items being inserted.
	//! @returns True to allow the operation, false to block it.
	virtual bool query_items_add(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection);
	//! Queries whether specified item reorder operation is allowed in the locked playlist.
	//! @param p_order Pointer to array containing permutation defining requested reorder operation.
	//! @param p_count Number of items in array pointed to by p_order. This should always be equal to number of items on the locked playlist.
	//! @returns True to allow the operation, false to block it.
	virtual bool query_items_reorder(const t_size * p_order,t_size p_count);
	//! Queries whether specified item removal operation is allowed in the locked playlist.
	//! @param p_mask Specifies which items from locked playlist are being removed.
	//! @param p_force If set to true, the call is made only for notification purpose and items are getting removed regardless (after e.g. they have been physically removed).
	//! @returns True to allow the operation, false to block it. Note that return value is ignored if p_force is set to true.
	virtual bool query_items_remove(const bit_array & p_mask,bool p_force);
	//! Queries whether specified item replacement operation is allowed in the locked playlist.
	//! @param p_index Index of the item being replaced.
	//! @param p_old Old value of the item being replaced.
	//! @param p_new New value of the item being replaced.
	//! @returns True to allow the operation, false to block it.
	virtual bool query_item_replace(t_size p_index,const metadb_handle_ptr & p_old,const metadb_handle_ptr & p_new);
	//! Queries whether renaming the locked playlist is allowed.
	//! @param p_new_name Requested new name of the playlist; a UTF-8 encoded string.
	//! @param p_new_name_len Length limit of the name string, in bytes (actual string may be shorter if null terminator is encountered before). Set this to infinite to use plain null-terminated strings.
	//! @returns True to allow the operation, false to block it.
	virtual bool query_playlist_rename(const char * p_new_name,t_size p_new_name_len);
	//! Queries whether removal of the locked playlist is allowed. Note that the lock will be released when the playlist is removed.
	//! @returns True to allow the operation, false to block it.
	virtual bool query_playlist_remove();
	//! Executes "default action" (doubleclick etc) for specified playlist item. When the playlist is not locked, default action starts playback of the item.
	//! @returns True if custom default action was executed, false to fall-through to default one for non-locked playlists (start playback).
	virtual bool execute_default_action(t_size p_item){ return false; };
	//! Notifies lock about changed index of the playlist, in result of user reordering playlists or removing other playlists.
	virtual void on_playlist_index_change(t_size p_new_index);
	//! Notifies lock about the locked playlist getting removed.
	virtual void on_playlist_remove();
	//! Retrieves human-readable name of playlist lock to display.
	virtual void get_lock_name(pfc::string_base & p_out);
	//! Requests user interface of component controlling the playlist lock to be shown.
	virtual void show_ui(){};
	//! Queries which actions the lock filters. The return value must not change while the lock is registered with playlist_manager. The return value is a combination of one or more filter_* constants.
	virtual t_uint32 get_filter_mask();

private:
	t_uint32 m_flag;
	static pfc::map_t<UINT,playlist_lock::ptr> m_lockers;
};

class wsh_playlist_loader_callback : public playlist_loader_callback
{
public:
	
	wsh_playlist_loader_callback() : m_clear(false){}

	virtual void on_progress(const char * p_path){};

	virtual void on_entry(const metadb_handle_ptr & p_item,t_entry_type p_type,const t_filestats & p_stats,bool p_fresh){};
	
	virtual bool want_info(const metadb_handle_ptr & p_item,t_entry_type p_type,const t_filestats & p_stats,bool p_fresh) { return true ;};
	
	virtual void on_entry_info(const metadb_handle_ptr & p_item,t_entry_type p_type,const t_filestats & p_stats,const file_info & p_info,bool p_fresh) {};

	virtual void handle_create(metadb_handle_ptr & p_out,const playable_location & p_location);

	virtual bool is_path_wanted(const char * path, t_entry_type type) { return true;};

	virtual bool want_browse_info(const metadb_handle_ptr & p_item,t_entry_type p_type,t_filetimestamp ts) {return false;};
	
	virtual void on_browse_info(const metadb_handle_ptr & p_item,t_entry_type p_type,const file_info & info, t_filetimestamp ts) {};

private:
	bool m_clear;
};