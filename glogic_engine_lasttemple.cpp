/***************************************************************************
***************************************************************************
(c) 1999-2006 Ganymede Technologies                 All Rights Reserved
Krakow, Poland                                  www.ganymede.eu
***************************************************************************
***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/*
*  GLogicEngineLastTemple
*/

// Uwaga!
// Podczas wywolania tej funkcji nie mozemy jeszcze wolac zadnej funkcji z 'upcalls'.
GLogicEngineLastTemple::GLogicEngineLastTemple(class ILogicUpCalls * upcalls)
{
	this->upcalls = upcalls;

	global_configs.clear();
	global_runtime.Zero();
}

bool GLogicEngineLastTemple::on_init_global_config(DWORD64 timestamp, const string & global_config)
{
	return SGlobalConfig::read_from_json_all_affiliates(global_config.c_str(), this->global_configs);
}

void GLogicEngineLastTemple::on_cleanup()
{
	global_configs.clear();
	global_runtime.Zero();
}

bool GLogicEngineLastTemple::on_put_gamestate(DWORD64 timestamp, INT32 gamestate_ID, const string & gamestate)
{
	SGamestate gs;
	if (!gs.read_from_json(gamestate.c_str()))
	{
		return false;
	}
	SGlobalConfig * global_config = &global_configs[0];
	if (global_configs.find(gs.affiliate_id) != global_configs.end())
	{
		global_config = &global_configs[gs.affiliate_id];
	}
	gs.validate_gamestate(*global_config, timestamp);
	gamestates[gamestate_ID] = gs;
	return true;
}

void GLogicEngineLastTemple::on_get_gamestate(INT32 gamestate_ID, string & gamestate, bool server_closing, bool logic_reload)
{
	SGamestate & gs = gamestates[gamestate_ID];

	json::Object o;
	gs.write_to_json(o, "", false, server_closing, logic_reload);

	gamestate = write_json_to_string(o);
}

void GLogicEngineLastTemple::on_erase_gamestate(INT32 gamestate_ID)
{
	gamestates.erase(gamestate_ID);
}

void GLogicEngineLastTemple::handle_player_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
												   const std::string &message, const std::string &params)
{
	SGamestate & gs = gamestates[gamestate_ID];

	json::Object jparams = get_json_from_string(params);
	json::Object response;

	SGlobalConfig * global_config = &global_configs[0];
	if (global_configs.find(gs.affiliate_id) != global_configs.end())
	{
		global_config = &global_configs[gs.affiliate_id];
	}
	SMessageIO io(timestamp, user_id, gamestate_ID, message, params, gs, jparams, *global_config, response);

	for (;;)
	{
		if (message == "load_config")
		{
			INT32 affiliate_id = get_INT32_from_json(io.jparams, "affiliate_id", -1);
			if (affiliate_id != -1 && global_configs.find(affiliate_id) != global_configs.end())
			{
				io.gs.affiliate_id = affiliate_id;
			}
			SGlobalConfig * global_config = &global_configs[0];
			if (global_configs.find(gs.affiliate_id) != global_configs.end())
			{
				global_config = &global_configs[gs.affiliate_id];
			}
			upcalls->SendMessage(user_id, gamestate_ID, message, global_config->client_global_config_json);
			return;
		}

		if (message == "game_state_load")
		{
			hpm_game_state_load(io);
			break;
		}
		if (message == "get_lifes_info")
		{
			hpm_get_lifes_info(io);
			break;
		}
		if (message == "get_player_profile")
		{
			hpm_get_player_profile(io);
			break;
		}

		if (message == "game_start")
		{
			hpm_game_start(io);
			break;
		}
		if (message == "game_ended")
		{
			hpm_game_ended(io);
			break;
		}
		if (message == "power_up_slot_changed")
		{
			hpm_power_up_slot_changed(io);
			break;
		}
		if (message == "buy_item")
		{
			hpm_buy_item(io);
			break;
		}
		if (message == "power_up_unlock")
		{
			hpm_power_up_unlock(io);
			break;
		}

		if (message == "set_achievements_enabled")
		{
			hpm_set_achievements_enabled(io);
			break;
		}
		if (message == "set_client_type")
		{
			hpm_set_client_type(io);
			break;
		}
		if (message == "award_achievement")
		{
			hpm_award_achievement(io);
			break;
		}
		if (message == "change_settings")
		{
			hpm_change_settings(io);
			break;
		}

		if (message == "get_leaderboard")
		{
			hpm_get_leaderboard(io);
			break;
		}
		if (message == "get_leaderboard_opt")
		{
			hpm_get_leaderboard_opt(io);
			break;
		}
		if (message == "get_leaderboard_range_opt")
		{
			hpm_get_leaderboard_range_opt(io);
			break;
		}

		if (message == "get_standings")
		{
			hpm_get_standings(io);
			break;
		}
		if (message == "get_user_position")
		{
			hpm_get_user_position(io);
			break;
		}
		if (message == "get_user_position_opt")
		{
			hpm_get_user_position_optimized(io);
			break;
		}
		if (message == "get_global_standings")
		{
			hpm_get_global_standings(io);
			break;
		}
		if (message == "check_tournament_end")
		{
			hpm_check_tournament_end(io);
			break;
		}
		if (message == "check_daily_spin")
		{
			hpm_check_daily_spin(io);
			break;
		}
		if (message == "run_spin")
		{
			hpm_run_spin(io);
			break;
		}
		if (message == "get_single_player_standings")
		{
			hpm_get_single_player_standings(io);
			break;
		}
		if (message == "friend_hired")
		{
			hpm_friend_hired(io);
			break;
		}
		if (message == "buy_hired_friend")
		{
			hpm_buy_hired_friend(io);
			break;
		}
		if (message == "add_hired_friend")
		{
			hpm_add_hired_friend(io);
			break;
		}

		if (message == "add_friend_request")
		{
			hpm_add_friend_request(io);
			break;
		}

		if (message == "realize_promocode")
		{
			hpm_realize_promocode(io);
			break;
		}

		if (message == "buy_time")
		{
			hpm_buy_time(io);
			break;
		}

#ifndef LINUX
		if (message == "debug/clear_game_state")
		{
			hpm_debug_clear_game_state(io);
			break;
		}
		if (message == "debug/add_waka")
		{
			hpm_debug_add_waka(io);
			break;
		}
		if (message == "debug/add_gems")
		{
			hpm_debug_add_gems(io);
			break;
		}
		if (message == "debug/clear_items")
		{
			hpm_debug_clear_items(io);
			break;
		}
		if (message == "debug/reset_powerups")
		{
			hpm_debug_reset_powerups(io);
			break;
		}
		if (message == "debug/set_xp")
		{
			hpm_debug_set_xp(io);
			break;
		}
#endif

		break;
	}

	if (io.response.Empty())
	{
		return;
	}

	gs.add_client_fields_to_response(io.global_config, timestamp, response);

	if (upcalls)
	{
		upcalls->SendMessage(user_id, gamestate_ID, message, write_json_to_string(response));
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::handle_friend_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
												   const std::string &message, const std::string &params)
{
	SGamestate & gs = gamestates[gamestate_ID];

	json::Object jparams = get_json_from_string(params);
	json::Object response;

	SGlobalConfig * global_config = &global_configs[0];
	if (global_configs.find(gs.affiliate_id) != global_configs.end())
	{
		global_config = &global_configs[gs.affiliate_id];
	}
	SMessageIO io(timestamp, user_id, gamestate_ID, message, params, gs, jparams, *global_config, response);

	for (;;)
	{
		if (message == "load_config")
		{
			upcalls->SendMessage(user_id, gamestate_ID, message, io.global_config.client_global_config_json);
			return;
		}

		if (message == "game_state_load")
		{
			hpm_game_state_load(io);
			break;
		}

		break;
	}

	if (io.response.Empty())
	{
		return;
	}

	if (upcalls)
	{
		upcalls->SendMessage(user_id, gamestate_ID, message, write_json_to_string(response));
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::handle_stranger_message(DWORD64 timestamp, INT32 user_id, INT32 gamestate_ID,
													 const std::string &message, const std::string &params)
{
}
/***************************************************************************/
void GLogicEngineLastTemple::handle_system_message(DWORD64 timestamp, INT32 gamestate_ID,
												   const std::string &message, const std::string &params, std::string & output)
{
	SGamestate & gs = gamestates[gamestate_ID];

	json::Object jparams = get_json_from_string(params);
	json::Object response;

	INT32 affiliate_id = gs.affiliate_id;
	if (message == "payments/get_config" || message == "gifts/get_config")
	{
		INT32 params_affiliate_id = get_INT32_from_json(jparams, "affiliate_id", -1);
		if (params_affiliate_id != -1 && global_configs.find(params_affiliate_id) != global_configs.end())
		{
			affiliate_id = params_affiliate_id;
		}
	}

	SGlobalConfig * global_config = &global_configs[0];
	if (global_configs.find(affiliate_id) != global_configs.end())
	{
		global_config = &global_configs[affiliate_id];
	}
	SMessageIO io(timestamp, gamestate_ID, gamestate_ID, message, params, gs, jparams, *global_config, response);

	for (;;)
	{
		if (message == "debug/get_gamestate")
		{
			hsm_debug_get_gamestate(io);
			break;
		}
		if (message == "debug/clear_game_state")
		{
			hsm_debug_clear_game_state(io);
			break;
		}
		if (message == "debug/set_gems")
		{
			hsm_debug_set_gems(io);
			break;
		}
		if (message == "debug/set_xp")
		{
			hsm_debug_set_xp(io);
			break;
		}
		if (message == "debug/clear_items")
		{
			hsm_debug_clear_items(io);
			break;
		}
		if (message == "debug/save_gamestate")
		{
			hsm_debug_save_gamestate(io);
			break;
		}
		if (message == "debug/load_gamestate")
		{
			hsm_debug_load_gamestate(io);
			break;
		}
		if (message == "debug/set_tournament_end_time")
		{
			hsm_debug_set_tournament_end_time(io);
			break;
		}
		if (message == "debug/set_waka")
		{
			hsm_debug_set_waka(io);
			break;
		}
		if (message == "debug/set_lifes")
		{
			hsm_debug_set_lifes(io);
			break;
		}
		if (message == "debug/buy_item")
		{
			hpm_buy_item(io);
			break;
		}
		if (message == "payments/add_item")
		{
			hsm_payments_add_item(io);
			break;
		}
		if (message == "payments/add_item_gd")
		{
			hsm_payments_add_item(io);
			break;
		}
		if (message == "payments/get_config")
		{
			hsm_payments_get_config(io);
			break;
		}
		if (message == "rewards/add_waka")
		{
			hsm_rewards_add_waka(io);
			break;
		}
		if (message == "gifts/get_config")
		{
			hsm_gifts_get_config(io);
			break;
		}
		if (message == "gifts/add_item")
		{
			hsm_gifts_add_item(io);
			break;
		}
		if (message == "gifts/new_gift")
		{
			hsm_gifts_new_gift(io);
			break;
		}
		if (message == "promocode/add_gems")
		{
			hsm_promocode_add_gems(io);
			break;
		}
		if (message == "add_hired_friend")
		{
			hpm_add_hired_friend(io);
			break;
		}
		if (message == "get_standings")
		{
			hpm_get_standings(io);
			break;
		}
		if (message == "debug/add_waka")
		{
			hsm_debug_add_waka(io);
			break;
		}
		if (message == "debug/add_gems")
		{
			hsm_promocode_add_gems(io);
			break;
		}
		if (message == "debug/unlock_adventure_stages")
		{
			hsm_debug_unlock_all_adventure_stages(io);
			break;
		}

		break;
	}

	output = "";
	if (io.response.Find("status") != io.response.End())
	{
		output = get_string_from_json(io.response, "status");
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::on_print_statistics(strstream & s)
{
	s << "Gamestates in memory:\t\t" << gamestates.size() << "\r\n";
}
/***************************************************************************/
void GLogicEngineLastTemple::on_disconnect_player(INT32 user_id, INT32 gamestate_ID)
{
	SGamestate & gs = gamestates[gamestate_ID];

	if (gs.table.game_in_progress)
	{
		gs.table.game_in_progress = false;
		gs.table.waka_locked = 0;
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::handle_gamestate_message(DWORD64 timestamp, INT32 gamestate_ID,
													  const std::string &message, const std::string &params)
{
	SGamestate & gs = gamestates[gamestate_ID];

	json::Object jparams = get_json_from_string(params);
	json::Object response;

	SGlobalConfig * global_config = &global_configs[0];
	if (global_configs.find(gs.affiliate_id) != global_configs.end())
	{
		global_config = &global_configs[gs.affiliate_id];
	}
	SMessageIO io(timestamp, 0, gamestate_ID, message, params, gs, jparams, *global_config, response);

	if (message == "online_and_offline_friends" ||
		message == "online_friend" ||
		message == "online_friend_remove")
	{
		INT32 target_user_id = get_INT32_from_json(jparams, "target_user_id");
		if (target_user_id > 0)
		{
			bool changed = gs.runtime.ProcessOnlineFriends(target_user_id, message, jparams);

			if (changed)
			{
				vector<INT32> online_friends;
				gs.runtime.GetOnlineFriends(target_user_id, online_friends);
				INT32 a, count = online_friends.size();

				json::Array jonline_friends;
				jonline_friends.Resize(count);
				for (a = 0; a < count; a++)
				{
					jonline_friends[a] = json::Number(online_friends[a]);
				}
				json::Object response;
				response["online_friends_id"] = jonline_friends;

				if (upcalls)
				{
					upcalls->SendMessage(target_user_id, gamestate_ID, "online_friends", write_json_to_string(response));
				}
			}
		}
		return;
	}

	if (message == "leaderboard_data")
	{
		hgm_leaderboard_data(io);
		return;
	}
	if (message == "get_player_profile")
	{
		hgm_get_player_profile(io);
		return;
	}
	if (message == "player_profile_data")
	{
		hgm_player_profile_data(io);
		return;
	}

	if (message == "add_hired_friend")
	{
		hpm_add_hired_friend(io);
		return;
	}

	if (message == "friend_score_update")
	{
		INT32 target_user_id = get_INT32_from_json(jparams, "target_user_id");
		DWORD64 tournament_start_time = get_INT64_from_json(jparams, "tournament_start_time");
		string level_pack = get_string_from_json(jparams, "level_pack","temple_0");

		if(level_pack == "temple_0")
		{
			if (tournament_start_time == gs.current_tournament_start_time)
			{
				json::Object response;
				response["user_id"] = jparams["user_id"];
				response["score"] = jparams["score"];
				response["level_pack"] = jparams["level_pack"];

				if (upcalls)
				{
					upcalls->SendMessage(target_user_id, gamestate_ID, "friend_score_update", write_json_to_string(response));
				}

				gs.runtime.cache_temple_0_leaders.clear();
			}
			return;
		}
		else
		{
			json::Object response;
			response["user_id"] = jparams["user_id"];
			response["score"] = jparams["score"];
			response["level_pack"] = jparams["level_pack"];

			if (upcalls)
			{
				upcalls->SendMessage(target_user_id, gamestate_ID, "friend_score_update", write_json_to_string(response));
			}
		}
	}

	if (message == "collect_friend")
	{
		hpm_collect_friend(io);
		return;
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::add_response_trigger(SMessageIO & io, const char * trigger_name)
{
	SUnansweredMessage unanswered_message;
	unanswered_message.timestamp = io.timestamp;
	unanswered_message.user_id = io.user_id;
	unanswered_message.gamestate_ID = io.gamestate_ID;
	unanswered_message.message = io.message;
	unanswered_message.params = io.params;

	io.gs.runtime.unanswered_messages.insert(make_pair(trigger_name, unanswered_message));
}
/***************************************************************************/
void GLogicEngineLastTemple::execute_trigger(SMessageIO & io, const char * trigger_name)
{
	io.gs.runtime.current_trigger.name = trigger_name;

	TUnansweredMessagesMap::iterator pos;
	// Nie iterujemy po equal_range() bo w trakcie wykonywania petli moga dojsc jakies inne elementy.
	for (;;)
	{
		pos = io.gs.runtime.unanswered_messages.find(trigger_name);
		if (pos == io.gs.runtime.unanswered_messages.end())
		{
			break;
		}
		SUnansweredMessage umsg = pos->second;
		io.gs.runtime.unanswered_messages.erase(pos);

		this->handle_player_message(umsg.timestamp, umsg.user_id, umsg.gamestate_ID, umsg.message, umsg.params);
	}

	io.gs.runtime.current_trigger.name = "";
}
/***************************************************************************/
