/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_get_gamestate(SMessageIO & io)
{
	json::Object o;
	io.gs.write_to_json(o, "", false, false, false);

	string gamestate_data = write_json_to_string(o);
	const char * p;
	for (p = " \n\r\t"; *p != 0; p++)
	{
		gamestate_data.erase(std::remove(gamestate_data.begin(), gamestate_data.end(), *p), gamestate_data.end());
	}
	io.response["status"] = json::String(gamestate_data);
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_clear_game_state(SMessageIO & io)
{
	io.gs.Zero();
	io.gs.validate_gamestate(io.global_config, io.timestamp);
	io.gs.player_info.gems = 20;
	io.gs.player_info.lifes = io.gs.player_info.max_lifes;
	io.gs.used_promocodes.clear();
	// clear leaderboard
	if (upcalls)
	{
		vector<INT32> scores;
		INT32 a;
		for (a = 0; a <= (io.global_config.adventure_sets.stages_count / FACTOR) + 1; a++)
		{
			scores.push_back(0);
		}

		// clear Tournament leaderboard
		upcalls->UpdateLeaderboard(io.user_id, "temple_0", io.gs.current_tournament_start_time, scores);

		// clear Adventure leaderboard
		for (INT32 i = 0 ; i <= 10 ; i ++)
		{
			upcalls->UpdateLeaderboard(io.user_id, "adventure", (DWORD64)i, scores);
		}

		vector<INT32> xp_level;
		xp_level.push_back(io.gs.player_info.level);
		upcalls->UpdateLeaderboard(io.user_id, "xp", 0, xp_level);
	}

	io.response = io.gs;
	io.response["totems_data_path"] = json::String();
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_add_waka(SMessageIO & io)
{
	INT32 waka = get_INT32_from_json(io.jparams, "waka");
	if (waka > 0)
	{
		io.gs.player_info.waka += waka;
	}
	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_add_gems(SMessageIO & io)
{
	INT32 gems = get_INT32_from_json(io.jparams, "gems");
	if (gems > 0)
	{
		io.gs.player_info.gems += gems;
	}
	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_clear_items(SMessageIO & io)
{
	io.gs.items.clear();
	io.response["items"] = io.gs["items"]["array"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_reset_powerups(SMessageIO & io)
{
	DWORD32 level = get_INT32_from_json(io.jparams, "level");
	if (level == 0)
	{
		level = io.gs.player_info.level;
	}

	INT32 a;
	for (a = 0; a < io.gs.power_ups.slots_count; a++)
	{
		io.gs.power_ups.slots[a].status = "DISABLED";
	}
	INT32 count = io.global_config.xp_level_config.size();
	for (a = 1; a < count && a <= (INT32)level; a++)
	{
		if (io.global_config.xp_level_config[a].powerup_slot_unlock > 0)
		{
			io.gs.power_ups.slots[io.global_config.xp_level_config[a].powerup_slot_unlock - 1].status = "EMPTY";
		}
	}

	io.gs.power_ups.enabled.clear();
	TPowerUpItemsMap::iterator it;
	for (it = io.global_config.powerup_items.begin(); it != io.global_config.powerup_items.end(); it++)
	{
		if (it->second.unlock_level > 0 && it->second.unlock_level <= level)
		{
			io.gs.power_ups.enabled.insert(it->second.type);
		}
	}

	io.response["power_ups"] = io.gs["power_ups"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_debug_set_xp(SMessageIO & io)
{
	DWORD32 xp = get_INT32_from_json(io.jparams, "xp");

	io.gs.player_info.xp = xp;
	DWORD32 level;
	for (level = 1; level + 1 < io.global_config.xp_level_config.size() && io.global_config.xp_level_config[level + 1].xp_req <= io.gs.player_info.xp ; level++) ;
	io.gs.player_info.level = level;

	if (upcalls)
	{
		vector<INT32> xp_level;
		xp_level.push_back(io.gs.player_info.level);
		upcalls->UpdateLeaderboard(io.user_id, "xp", 0, xp_level);
	}

	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_clear_game_state(SMessageIO & io)
{
	hpm_debug_clear_game_state(io);
	if (upcalls)
	{
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_set_gems(SMessageIO & io)
{
	INT32 gems = get_INT32_from_json(io.jparams, "gems");
	if (gems >= 0)
	{
		io.gs.player_info.gems = gems;
	}
	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_set_xp(SMessageIO & io)
{
	hpm_debug_set_xp(io);
	hpm_debug_reset_powerups(io);

	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_clear_items(SMessageIO & io)
{
	hpm_debug_clear_items(io);

	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_save_gamestate(SMessageIO & io)
{
	if (upcalls)
	{
		upcalls->SaveSnapshot(io.gamestate_ID);
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_load_gamestate(SMessageIO & io)
{
	INT32 snapshot_id = get_INT32_from_json(io.jparams, "snapshot_id");
	if (snapshot_id <= 0)
	{
		io.response["status"] = json::String("FAIL: Invalid snapshot_id.");
		return;
	}

	if (upcalls)
	{
		upcalls->LoadSnapshot(snapshot_id, io.gamestate_ID);

		SGamestate & gs = gamestates[io.gamestate_ID];
		gs.validate_gamestate(io.global_config, io.timestamp);

		// update leaderboard
		vector<INT32> temple_scores;
		INT32 c;
		temple_scores.resize(1 + 10);
		temple_scores[0] = 0;
		for (c = 1; c <= 10; c++)
		{
			INT32 chamber_score = gs.player_info.temple_progress[c - 1].score;
			temple_scores[c] = chamber_score;
			temple_scores[0] += chamber_score;
		}
		upcalls->UpdateLeaderboard(io.user_id, "temple_0", gs.current_tournament_start_time, temple_scores);

		vector<INT32> xp_level;
		xp_level.push_back(gs.player_info.level);
		upcalls->UpdateLeaderboard(io.user_id, "xp", 0, xp_level);

		gs.remove_expired_items(io.timestamp);
		io.response = gs;
		io.response["totems_data_path"] = json::String();
		gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_set_tournament_end_time(SMessageIO & io)
{
	INT64 seconds = get_INT64_from_json(io.jparams, "seconds", 300);
	io.gs.runtime.debug_current_tournament_end_time = io.timestamp + seconds;

	if (upcalls)
	{
		hpm_check_tournament_end(io);
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "check_tournament_end", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_set_waka(SMessageIO & io)
{
	INT32 waka = get_INT32_from_json(io.jparams, "waka");
	if (waka >= 0)
	{
		io.gs.player_info.waka = waka;
	}
	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_set_lifes(SMessageIO & io)
{
	INT32 lifes = get_INT32_from_json(io.jparams, "lifes");
	if (lifes >= 0 && lifes <= (INT32)io.gs.player_info.max_lifes)
	{
		io.gs.player_info.lifes = lifes;
	}
	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_add_waka(SMessageIO & io)
{
	INT32 waka = get_INT32_from_json(io.jparams, "waka");
	if (waka > 0)
	{
		io.gs.player_info.waka += waka;
	}
	if (upcalls)
	{
		io.gs.remove_expired_items(io.timestamp);
		io.response = io.gs;
		io.response["totems_data_path"] = json::String();
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, io.response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(io.response));
	}
	io.response["status"] = json::String("OK");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_debug_unlock_all_adventure_stages(SMessageIO & io)
{
	string game_mode = "adventure";

	if (io.gs.player_info.single_player_progress.count(game_mode) == 0)
	{
		io.gs.player_info.single_player_progress.insert(pair<string, map<INT32, SGamestate::_player_info::_temple_progress> >(game_mode, map<INT32, SGamestate::_player_info::_temple_progress>()));
	}

	try
	{
		INT32 stage_index = 1;
		INT32 last_stage_index = io.global_config.adventure_sets.stages_count;
		map<INT32, SGamestate::_player_info::_temple_progress>& adventure_stages = io.gs.player_info.single_player_progress.at(game_mode);
		map<INT32, SGamestate::_player_info::_temple_progress>::reverse_iterator rit = adventure_stages.rbegin();
		
		if (rit != adventure_stages.rend())
		{
			stage_index = rit->first;
		}

		for (; stage_index < last_stage_index; ++stage_index)
		{
			SGamestate::_player_info::_temple_progress& stage_data = adventure_stages.insert(make_pair(stage_index, SGamestate::_player_info::_temple_progress())).first->second;
			stage_data.highest_combo = 10;
			stage_data.longest_chain = 10;
			stage_data.score = 100;
			stage_data.time_left = 30;
		}


	}
	catch (std::out_of_range&)
	{
		//oops str is too short!!!
	}

	//if (rit != io.gs.player_info.single_player_progress.rend())
	//{
	//	rit->
	//}


	if (io.gs.player_info.single_player_progress.count(io.gs.table.level_pack) == 0)
	{
		io.gs.player_info.single_player_progress.insert(pair<string, map<INT32, SGamestate::_player_info::_temple_progress> >(io.gs.table.level_pack, map<INT32, SGamestate::_player_info::_temple_progress>()));
		io.gs.player_info.single_player_progress[io.gs.table.level_pack].insert(pair<INT32, SGamestate::_player_info::_temple_progress>(io.gs.table.chamber, SGamestate::_player_info::_temple_progress()));
	}

	io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].score;
}
/***************************************************************************/
