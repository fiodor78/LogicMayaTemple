/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#pragma warning(disable: 4511)
#pragma warning(disable: 4355)

/***************************************************************************/
struct SGamestate
{
	SGamestate()
	{
		Zero();
	}
	void Zero()
	{
		INT32 a;

		player_info.gems = 0;
		player_info.waka = 0;
		player_info.lifes = 0;
		player_info.max_lifes = 0;
		player_info.extra_lifes = 5;
		player_info.xp = 0;
		player_info.level = 0;
		player_info.score = 0;
		for (a = 0; a < 10; a++)
		{
			player_info.temple_progress[a].score = 0;
			player_info.temple_progress[a].longest_chain = 0;
			player_info.temple_progress[a].highest_combo = 0;
			player_info.temple_progress[a].time_left = 0;
		}

		player_info.single_player_progress.clear();
		player_info.hired_friends.clear();
		player_info.invited_friends.clear();

		player_info.tournament_prizes.clear();
		player_info.free_spins_available = 0;
		player_info.current_collection = 0;
		player_info.current_collection_progress = 0;
		player_info.last_tournament_completed = 0;

		achievements.enabled = false;
		achievements.awarded.clear();
		achievements.counters.last_chamber_finished = -1;
		achievements.counters.events_24h_game_played.clear();
		achievements.counters.events_24h_reset.clear();
		achievements.counters.last_perfectionist_awarded = 0;

		power_ups.slots.clear();
		power_ups.slots_count = 0;
		power_ups.enabled.clear();

		items.clear();

		settings.sound_enabled = 1;
		settings.music_enabled = 1;
		settings.intro_seen = 0;
		settings.arena_seen = 0;
		settings.tooltips_enabled = 1;
		settings.welcome_tournament_seen = 0;
		settings.challenge_info_seen = 0;
		settings.tutorial_step = 0;
		settings.game_mode = "";

		table.game_in_progress = false;
		table.chamber = 0;
		table.active_powerups.clear();
		table.active_items.clear();
		table.active_friends_requests.clear();
		table.waka_locked = 0;
		table.game_start_timestamp = 0;

		daily_bonus.last_awarded_timestamp = 0;
		daily_bonus.last_awarded_value = 0;

		payments.clear();

		current_tournament_start_time = 0;
		last_life_refill_time = 0;
		saved_at_server_close = false;

		affiliate_id = 0;

		runtime.Zero();
	}

	bool	read_from_json(const char * json_data);
	void	write_to_json(json::Object & o, const string & which_part, bool client_response, bool server_closing, bool logic_reload);

	const json::Object operator[] (const std::string & key);
	operator json::Object()
	{
		return (*this)[""];
	}

	//---------------------------------------------------------------------------

	struct _player_info
	{
		
		DWORD32		gems;
		DWORD32		waka;
		DWORD32		lifes;
		DWORD32		max_lifes;
		DWORD32		extra_lifes;
		DWORD32		xp;
		DWORD32		level;
		DWORD32		score;
		struct _temple_progress
		{
			DWORD32		score;
			DWORD32		longest_chain;
			DWORD32		highest_combo;
			DWORD32		time_left;
		} temple_progress[10];

		typedef		map<string, map<INT32, _temple_progress> > TSinglePlayerProgress;
		TSinglePlayerProgress	single_player_progress;
		map<string, DWORD32>	tournament_prizes;
		DWORD32		free_spins_available;
		DWORD32		current_collection;
		DWORD32		current_collection_progress;
		DWORD64		last_tournament_completed;
		vector<DWORD32> hired_friends;
		vector<string>	invited_friends;
	} player_info;

	struct _achievements
	{
		bool					enabled;
		map<string, DWORD32>	awarded;						// type -> level
		struct _counters
		{
			INT32					last_chamber_finished;
			vector<DWORD64>			events_24h_game_played;
			map<string, DWORD64>	events_24h_reset;
			DWORD64					last_perfectionist_awarded;
		} counters;
	} achievements;

	struct _power_ups
	{
		struct _slot
		{
			string		status;
		};
		vector<_slot>	slots;
		INT32			slots_count;

		set<string>		enabled;
	} power_ups;

	struct _item
	{
		string		id;
		DWORD64		expiration_time;
		DWORD32		expiration_games;
		bool		is_permanent;
	};
	vector<_item>	items;

	struct _settings
	{
		DWORD32		sound_enabled;
		DWORD32		music_enabled;
		DWORD32		intro_seen;
		DWORD32		arena_seen;
		DWORD32		tooltips_enabled;
		DWORD32		welcome_tournament_seen;
		DWORD32		challenge_info_seen;
		DWORD32		tutorial_step;
		string		game_mode;
	} settings;


	struct _friend_request
	{
		_friend_request(string id, DWORD64 time, DWORD64 remaining = 0)
		{
			social_id = id;
			send_time = time;
			remaining_time = remaining;
		}

		string		social_id;
		DWORD64		send_time;
		DWORD64		remaining_time;
	};
	struct _table
	{
		bool					game_in_progress;
		INT32					chamber;
		string					level_pack;
		vector<string>			active_powerups;
		vector<string>			active_items;
		vector<_friend_request>	active_friends_requests;
		DWORD32					waka_locked;
		DWORD64					game_start_timestamp;
	} table;

	struct _daily_bonus
	{
		DWORD64		last_awarded_timestamp;
		DWORD32		last_awarded_value;
	} daily_bonus;

	struct _payment
	{
		DWORD64		paymentid;
	};
	vector<_payment>	payments;

	set<string>	used_promocodes;
	DWORD64		current_tournament_start_time;
	DWORD64		last_life_refill_time;
	bool		saved_at_server_close;

	INT32		affiliate_id;						// uzywane do okreslenia, z ktorego SGlobalConfig gracz korzysta

	SGamestateRuntime		runtime;

	//---------------------------------------------------------------------------

private:
	DWORD64	get_lifes_recover_time(DWORD64 life_refill_duration, DWORD64 timestamp);

public:
	STournamentConfig & get_current_tournament_config(SGlobalConfig & global_config);

	void	validate_gamestate(SGlobalConfig & global_config, DWORD64 timestamp);
	void	add_client_fields_to_response(SGlobalConfig & global_config, DWORD64 timestamp, json::Object & response);
	void	remove_expired_items(DWORD64 timestamp);
	void	check_friends_requests(DWORD64 timestamp);
};
/***************************************************************************/
