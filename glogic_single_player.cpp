/***************************************************************************
 ***************************************************************************
  (c) 1999-2012 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

 /**CI TEST*****************************************************************/
 
#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_single_player_standings(SMessageIO & io)
{
	INT32 chamber = get_INT32_from_json(io.jparams, "chamber", -1);
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");
	INT32 pack_page = get_INT32_from_json(io.jparams, "pack_page", -1);
	
	INT32 request_standings_index = -1;

	if(level_pack == "not_found" || pack_page == -1 )
		return;

	if (chamber == -1)
	{
		request_standings_index = 0;
	}
	else
	{
		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		if (chamber >= 0 && chamber < (INT32)tournament_config.totems_count)
		{
			request_standings_index = chamber + 1;
		}
	}

	if (request_standings_index == -1)
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	if (io.gs.runtime.current_trigger.name == "")
	{
		// Wymuszamy pobranie leveli przyjaciol na nowo.
		io.gs.runtime.cache_xp_level_friends.clear();
	}

	if (io.gs.runtime.cache_xp_level_friends.size() == 0)
	{
		char needed_trigger_name[128];
		sprintf(needed_trigger_name, "single_player_standings:%s:%lld:%d", "xp", (DWORD64)0, (INT32)0);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				INT32 max_results = -1;
				upcalls->RequestLeaderboardStandings(io.user_id, "xp", (DWORD64)0, (INT32)0, max_results);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
		INT32 a, count = leaderboard_data.highscores.size();
		for (a = 0; a < count; a++)
		{
			io.gs.runtime.cache_xp_level_friends[leaderboard_data.highscores[a].first] = leaderboard_data.highscores[a].second;
		}
		// Upewniamy sie, ze w nastepnym przebiegu cache_xp_level_friends bedzie niepuste.
		io.gs.runtime.cache_xp_level_friends[io.gamestate_ID] = io.gs.player_info.level;
	}

	char needed_trigger_name[128];
	sprintf(needed_trigger_name, "single_player_standings:%s:%lld:%d", level_pack.data(), (DWORD64)pack_page, request_standings_index);

	if (io.gs.runtime.current_trigger.name != needed_trigger_name)
	{
		if (upcalls)
		{
			add_response_trigger(io, needed_trigger_name);
			INT32 max_results = -1;
			upcalls->RequestLeaderboardStandings(io.user_id, level_pack, pack_page, request_standings_index, max_results);
		}
		return;
	}


	io.response["chamber"] = json::Number(chamber);
	io.response["level_pack"] = json::String(level_pack);
	io.response["pack_page"] = json::Number(pack_page);

	SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
	json::Array & ar = io.response["highscores"] = json::Array();
	INT32 a, count, hs_count = leaderboard_data.highscores.size();
	count = max(hs_count, (INT32)io.gs.runtime.cache_xp_level_friends.size());
	ar.Resize(count);
	for (a = 0; a < hs_count; a++)
	{
		INT32 user_id = leaderboard_data.highscores[a].first;
		ar[a]["user_id"] = json::Number(user_id);
		ar[a]["score"] = json::Number(leaderboard_data.highscores[a].second);
		ar[a]["level"] = json::Number(io.gs.runtime.cache_xp_level_friends[user_id]);
		io.gs.runtime.cache_xp_level_friends[user_id] = 0;
	}
	// Dodajemy przyjaciol z wynikiem 0.
	map<INT32, DWORD32>::iterator it;
	for (it = io.gs.runtime.cache_xp_level_friends.begin(); a < count && it != io.gs.runtime.cache_xp_level_friends.end(); it++)
	{
		if (it->second != 0)
		{
			ar[a]["user_id"] = json::Number(it->first);
			ar[a]["score"] = json::Number(0);
			ar[a]["level"] = json::Number(it->second);
			a++;
		}
	}

	if (a != count)
	{
		ar.Resize(a);
	}

	
}

void GLogicEngineLastTemple::hpm_buy_hired_friend(SMessageIO & io)
{
	if(io.gs.player_info.gems >= io.global_config.hired_friend_cost)
	{
		INT32 stages_played = 0;
		INT32 max_hired_friends_count = 0;
		INT32 current_hired_friends_count = 0;
		INT32 available_hired_friends_slots = 0;

		map<INT32, SGamestate::_player_info::_temple_progress> adventure_progress = io.gs.player_info.single_player_progress["adventure"];
		
		// Sprawdzamy czy mozemy dodac frienda. Limit to 3 id na ka¿de zagrane 30 plansz adventure.
		for(map<INT32, SGamestate::_player_info::_temple_progress>::iterator it = adventure_progress.begin(); it != adventure_progress.end(); ++it )
		{
			if (stages_played < it->first )
			{
				stages_played = it->first;
			}
		}
		max_hired_friends_count = stages_played;
		max_hired_friends_count /= 30;
		max_hired_friends_count += 1;
		max_hired_friends_count *=3;

		current_hired_friends_count = io.gs.player_info.hired_friends.size();
		
		available_hired_friends_slots = max_hired_friends_count - current_hired_friends_count;

		if (available_hired_friends_slots > 0)
		{
			io.gs.player_info.gems -= io.global_config.hired_friend_cost;
			io.gs.player_info.hired_friends.push_back((DWORD32)0);

			io.response["player_info"] = io.gs["player_info"];
		}
		else
		{
			io.response["error"] = json::String("Hired friends limit reached.");
		}		
	}
	else
	{
		io.response["error"] = json::String("Not enough gems.");
	}	
}

void GLogicEngineLastTemple::hpm_add_hired_friend(SMessageIO & io)
{
	INT32 sender_id = get_INT32_from_json(io.jparams, "sender_id");

	INT32 stages_played = 0;
	INT32 max_hired_friends_count = 0;
	INT32 current_hired_friends_count = 0;
	INT32 available_hired_friends_slots = 0;

	map<INT32, SGamestate::_player_info::_temple_progress> adventure_progress = io.gs.player_info.single_player_progress["adventure"];
		
	// Sprawdzamy czy mozemy dodac frienda. Limit to 3 id na ka¿de zagrane 30 plansz adventure.
	for(map<INT32, SGamestate::_player_info::_temple_progress>::iterator it = adventure_progress.begin(); it != adventure_progress.end(); ++it )
	{
		if (stages_played < it->first )
		{
			stages_played = it->first;
		}
	}
	max_hired_friends_count = stages_played;
	max_hired_friends_count /= 30;
	max_hired_friends_count += 1;
	max_hired_friends_count *=3;

	current_hired_friends_count = io.gs.player_info.hired_friends.size();		
	available_hired_friends_slots = max_hired_friends_count - current_hired_friends_count;

	if (available_hired_friends_slots > 0)
	{
		bool already_contains = false;

		for (DWORD32 i = 0; i < io.gs.player_info.hired_friends.size(); i++)
		{
			if (io.gs.player_info.hired_friends[i] == (DWORD32)sender_id)
			{
				already_contains = true;
				break;
			}
		}

		if (!already_contains)
		{
			io.gs.player_info.hired_friends.push_back((DWORD32)sender_id);

			json::Object response;
			response["player_info"] = io.gs["player_info"];

			if (upcalls)
			{		
				upcalls->SendMessage(io.gamestate_ID, io.gamestate_ID, "friend_hired", write_json_to_string(response));
			}	
		}
		else
		{
			io.response["error"] = json::String("Friend already added.");
		}		
	}
	else
	{
		io.response["error"] = json::String("Hired friends limit reached.");
	}		
	
}

void GLogicEngineLastTemple::hpm_friend_hired(SMessageIO & io)
{
	io.response["player_info"] = io.gs["player_info"];
}