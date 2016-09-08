/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#pragma warning(disable: 4511)
#pragma warning(disable: 4355)

#include <sgs_api.h>

#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"
/*********************************Affiliates********************************/
#define AFF_ID_FB	400190
#define AFF_ID_NK	400191
#define AFF_ID_WIZQ 400238
#define AFF_ID_GD	500000
/*********************************Powerups**********************************/
#define POWERUP_GEMS_COST_ALL 50
#define POWERUP_GEMS_COST_WIZQ 100
#define powerup_gems_cost(affiliate) (affiliate == AFF_ID_WIZQ ? POWERUP_GEMS_COST_WIZQ : POWERUP_GEMS_COST_ALL)
/***************************************************************************/

// json_io.cpp
void try_cleanup_json_content(const std::string & name, json::Object & json_data);
json::Object get_json_from_string(const std::string & json_data);
string write_json_to_string(json::Object & o);
INT32 get_INT32_from_json(const json::Object & o, const char * key, INT32 default_value = 0);
INT64 get_INT64_from_json(const json::Object & o, const char * key, INT64 default_value = 0);
string get_string_from_json(const json::Object & o, const char * key, const char * default_value = "");
void merge_objects(json::Object & o, const json::Object & new_fields, const string & preserve_field);

#include "sglobalconfig.h"
#include "sgamestateruntime.h"
#include "sgamestate.h"
#include "sglobalruntime.h"

const INT32 FACTOR = 10;

struct SMessageIO
{
	SMessageIO(DWORD64 _timestamp, INT32 _user_id, INT32 _gamestate_ID, const string & _message, const string & _params, SGamestate & _gs, const json::Object & _jparams, SGlobalConfig & _global_config, json::Object & _response) :
		timestamp(_timestamp), user_id(_user_id), gamestate_ID(_gamestate_ID), message(_message), params(_params), gs(_gs), jparams(_jparams), global_config(_global_config), response(_response)
	{
	}

	// input
	DWORD64					timestamp;
	INT32					user_id;
	INT32					gamestate_ID;
	const string &			message;
	const string &			params;
	const json::Object &	jparams;
	SGlobalConfig &			global_config;
	// input/output
	SGamestate &			gs;

	// output
	json::Object &			response;
};
/***************************************************************************/
class GLogicEngineLastTemple : public ILogicEngine
{
public:
	GLogicEngineLastTemple(class ILogicUpCalls * upcalls);

	bool					on_init_global_config(DWORD64 timestamp, const string & global_config);
	void					on_cleanup();
	bool					on_put_gamestate(DWORD64 timestamp, INT32 gamestate_ID, const string & gamestate);
	void					on_get_gamestate(INT32 gamestate_ID, string & gamestate, bool server_closing, bool logic_reload);
	void					on_erase_gamestate(INT32 gamestate_ID);
	void					on_print_statistics(strstream & s);

	/*
		Obsluga polecen logiki.
	*/
	void					handle_player_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
													const std::string &message, const std::string &params);
	void					handle_friend_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
													const std::string &message, const std::string &params);
	void					handle_stranger_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
													const std::string &message, const std::string &params);
	void					handle_system_message(DWORD64 timestamp, INT32 gamestate_ID,
													const std::string &message, const std::string &params, std::string & output);
	void					handle_gamestate_message(DWORD64 timestamp, INT32 gamestate_ID,
												const std::string &message, const std::string &params);

	void					on_disconnect_player(INT32 user_id, INT32 gamestate_ID);

private:
	void					add_response_trigger(SMessageIO & io, const char * trigger_name);
	void					execute_trigger(SMessageIO & io, const char * trigger_name);

	// glogic_debug.cpp
	void					hpm_debug_clear_game_state(SMessageIO & io);
	void					hpm_debug_add_waka(SMessageIO & io);
	void					hpm_debug_add_gems(SMessageIO & io);
	void					hpm_debug_clear_items(SMessageIO & io);
	void					hpm_debug_reset_powerups(SMessageIO & io);
	void					hpm_debug_set_xp(SMessageIO & io);

	void					hsm_debug_get_gamestate(SMessageIO & io);
	void					hsm_debug_clear_game_state(SMessageIO & io);
	void					hsm_debug_set_gems(SMessageIO & io);
	void					hsm_debug_set_xp(SMessageIO & io);
	void					hsm_debug_clear_items(SMessageIO & io);
	void					hsm_debug_save_gamestate(SMessageIO & io);
	void					hsm_debug_load_gamestate(SMessageIO & io);
	void					hsm_debug_set_tournament_end_time(SMessageIO & io);
	void					hsm_debug_set_waka(SMessageIO & io);
	void					hsm_debug_set_lifes(SMessageIO & io);
	void					hsm_debug_add_waka(SMessageIO & io);

	// glogic_payments_gifts.cpp
	void					hsm_payments_add_item(SMessageIO & io);
	void					hsm_payments_get_config(SMessageIO & io);
	void					hsm_rewards_add_waka(SMessageIO & io);
	void					hsm_gifts_get_config(SMessageIO & io);
	void					hsm_gifts_add_item(SMessageIO & io);
	void					hsm_gifts_new_gift(SMessageIO & io);
	void					hsm_promocode_add_gems(SMessageIO & io);
	void					hsm_debug_unlock_all_adventure_stages(SMessageIO & io);

	// glogic_game.cpp
	void					calculate_game_bonus(DWORD32 icons_busted, DWORD32 time_spent_percent, DWORD32 & xp_add, DWORD32 & waka_add);
	void					hpm_game_state_load(SMessageIO & io);
	void					hpm_get_lifes_info(SMessageIO & io);
	void					hpm_get_player_profile(SMessageIO & io);
	void					hgm_get_player_profile(SMessageIO & io);
	void					hgm_player_profile_data(SMessageIO & io);
	void					hpm_game_start(SMessageIO & io);
	void					hpm_game_ended(SMessageIO & io);
	void					hpm_change_settings(SMessageIO & io);
	void					hpm_add_friend_request(SMessageIO & io);
	void					hpm_collect_friend(SMessageIO & io);
	void					hpm_realize_promocode(SMessageIO & io);
	void					hpm_buy_time(SMessageIO & io);

	// globic_achievements.cpp
	void					award_achievement(SMessageIO & io, const char * achievement_id);
	void					check_after_game_achievements(SMessageIO & io);
	void					check_after_tournament_achievements(SMessageIO & io);
	void					hpm_set_achievements_enabled(SMessageIO & io);
	void					hpm_set_client_type(SMessageIO & io);
	void					hpm_award_achievement(SMessageIO & io);

	// glogic_powerups_items.cpp
	void					hpm_power_up_slot_changed(SMessageIO & io);
	void					hpm_power_up_unlock(SMessageIO & io);
	void					hpm_buy_item(SMessageIO & io);

	// glogic_tournament.cpp
	void					finish_tournament(SMessageIO & io, vector<pair<INT32, DWORD32> > & results);
	
	void					hpm_get_leaderboard(SMessageIO & io);
	void					hpm_get_leaderboard_opt(SMessageIO & io);
	void					hpm_get_leaderboard_range_opt(SMessageIO & io);

	void					hpm_get_standings(SMessageIO & io);
	void					hpm_get_user_position(SMessageIO & io);
	void					hpm_get_global_standings(SMessageIO & io);
	void					hpm_check_tournament_end(SMessageIO & io);
	void					hgm_leaderboard_data(SMessageIO & io);

	void					hpm_get_user_position_optimized(SMessageIO & io);


	// glogic_daily_spin.cpp
	void					hpm_check_daily_spin(SMessageIO & io);
	void					hpm_run_spin(SMessageIO & io);

	// glogic_single_player.cpp
	void					hpm_get_single_player_standings(SMessageIO & io);
	void					hpm_buy_hired_friend(SMessageIO & io);
	void					hpm_add_hired_friend(SMessageIO & io);
	void					hpm_friend_hired(SMessageIO & io);



	void					add_item_by_id(SMessageIO & io, string item_id);
private:
	class ILogicUpCalls * 			upcalls;

	hash_map<INT32, SGlobalConfig>	global_configs;

	hash_map<INT32, SGamestate>		gamestates;				// gamestate_ID -> SGamestate

	SGlobalRuntime					global_runtime;

	vector<SGamestateRuntime::_current_trigger::_leaderboard_data> chalenge_mode_leaderboard_data;

	vector<SGamestateRuntime::_current_trigger::_leaderboard_data> chalenge_mode_user_positions_data;

};
