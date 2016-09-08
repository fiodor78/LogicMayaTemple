/***************************************************************************
***************************************************************************
(c) 1999-2006 Ganymede Technologies                 All Rights Reserved
Krakow, Poland                                  www.ganymede.eu
***************************************************************************
***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_leaderboard(SMessageIO & io)
{
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	char needed_trigger_name[128];

	if (level_pack == "temple_0")
	{		
		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboard(io.user_id, level_pack, io.gs.current_tournament_start_time);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
		json::Array & ar = io.response["highscores"] = json::Array();
		io.response["level_pack"] = json::String(level_pack);

		INT32 a, count;

		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		count = tournament_config.totems_count;
		ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			ar[a]["user_id"] = json::Number(leaderboard_data.highscores[a + 1].first);
			ar[a]["score"] = json::Number(leaderboard_data.highscores[a + 1].second);
		}
	}
	else
	{
		INT32 stages_range = io.global_config.adventure_sets.stages_count; // get_INT32_from_json(io.jparams, "stages_range");
		INT64 subkey = (INT64)chalenge_mode_leaderboard_data.size();

		if(stages_range == 0)
		{
			io.response["error"] = json::String("Invalid parameters.");
			return;
		}
		else
		{
			stages_range --;
		}

		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);

		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			chalenge_mode_leaderboard_data.push_back(leaderboard_data);

			subkey ++;
		}

		if(subkey <= stages_range / FACTOR)
		{		
			sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);

			if ((io.gs.runtime.waiting_for_leaderboard_request & (1 << subkey)) == 0)
			{
				if (upcalls)
				{
					add_response_trigger(io, needed_trigger_name);
					upcalls->RequestLeaderboard(io.user_id, level_pack, (DWORD64)subkey);
					io.gs.runtime.waiting_for_leaderboard_request |= (1 << subkey);
				}
			}
			return;

		}
		else
		{
			json::Array & ar = io.response["highscores"] = json::Array();
			io.response["level_pack"] = json::String(level_pack);

			INT32 a, count;

			count = stages_range + 1;
			ar.Resize(count);


			for(vector<SGamestateRuntime::_current_trigger::_leaderboard_data>::iterator it = chalenge_mode_leaderboard_data.begin(); it != chalenge_mode_leaderboard_data.end(); it ++)
			{				
				for (a = 0; a < FACTOR; a++)
				{
					ar[(INT32)it->subkey * FACTOR + a]["user_id"] = json::Number(it->highscores[a + 1].first);
					ar[(INT32)it->subkey * FACTOR + a]["score"] = json::Number(it->highscores[a + 1].second);
				}
			}

			chalenge_mode_leaderboard_data.clear();
			io.gs.runtime.waiting_for_leaderboard_request = 0;
		}


		//json::Array & ar = io.response["highscores"] = json::Array();
		//io.response["level_pack"] = json::String(level_pack);

	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_leaderboard_range_opt(SMessageIO & io)
{
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	char needed_trigger_name[128];

	if (level_pack == "temple_0")
	{		
		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboard(io.user_id, level_pack, io.gs.current_tournament_start_time);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;

		json::Object & highscores = io.response["highscores"] = json::Object();
		io.response["level_pack"] = json::String(level_pack);


		INT32 a, count;

		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		io.response["stages_count"] = json::Number(tournament_config.totems_count);
		count = tournament_config.totems_count;
		for (a = 0; a < count; a++)
		{
			if (leaderboard_data.highscores[a + 1].second <= 0) continue;
			highscores[str(boost::format("%1%") % a)]["user_id"] = json::Number(leaderboard_data.highscores[a + 1].first);
			highscores[str(boost::format("%1%") % a)]["score"] = json::Number(leaderboard_data.highscores[a + 1].second);
		}
	}
	else
	{
		INT32 from_offset = get_INT32_from_json(io.jparams, "from", 0);
		INT32 stages_range = get_INT32_from_json(io.jparams, "stages_range", (io.global_config.adventure_sets.stages_count)) - 1;

		if (from_offset < 0) from_offset = 0;
		if (stages_range > (io.global_config.adventure_sets.stages_count - 1)) stages_range = (io.global_config.adventure_sets.stages_count - 1);
		if (from_offset > stages_range) from_offset = stages_range;

		INT64 subkey = -1;

		for (INT32 i = from_offset; i < stages_range; i += FACTOR)
		{
			if ((io.gs.runtime.waiting_for_leaderboard_request & (1 << (i / FACTOR))) == 0) 
			{				
				subkey = i / FACTOR;
				break;
			}
		}


		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);

		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			chalenge_mode_leaderboard_data.push_back(leaderboard_data);
			io.gs.runtime.waiting_for_leaderboard_request |= (1 << subkey);

			subkey = -1;
			for (INT32 i = from_offset; i < stages_range; i += FACTOR)
			{
				if ((io.gs.runtime.waiting_for_leaderboard_request & (1 << (i / FACTOR))) == 0) 
				{				
					subkey = i / FACTOR;
					break;
				}
			}
		}

		if (subkey >= 0)
		{

			sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);

			if ((io.gs.runtime.waiting_for_leaderboard_request & (1 << subkey)) == 0)
			{
				if (upcalls)
				{
					add_response_trigger(io, needed_trigger_name);
					upcalls->RequestLeaderboard(io.user_id, level_pack, (DWORD64)subkey);
				}
			}
			return;

		}
		else
		{
			json::Object & highscores = io.response["highscores"] = json::Object();
			io.response["level_pack"] = json::String(level_pack);

			INT32 a, count;

			count = stages_range + 1;
			io.response["stages_count"] = json::Number(count);

			for(vector<SGamestateRuntime::_current_trigger::_leaderboard_data>::iterator it = chalenge_mode_leaderboard_data.begin(); it != chalenge_mode_leaderboard_data.end(); it ++)
			{				
				for (a = 0; a < FACTOR; a++)
				{
					if (it->highscores[a + 1].second <= 0) continue;
					highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))]["user_id"] = json::Number(it->highscores[a + 1].first);
					highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))]["score"] = json::Number(it->highscores[a + 1].second);
				}
			}

			chalenge_mode_leaderboard_data.clear();
			io.gs.runtime.waiting_for_leaderboard_request = 0;
		}

	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_leaderboard_opt(SMessageIO & io)
{
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	char needed_trigger_name[128];

	if (level_pack == "temple_0")
	{		
		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboard(io.user_id, level_pack, io.gs.current_tournament_start_time);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;

		json::Object & highscores = io.response["highscores"] = json::Object();
		//json::Array & ar = io.response["highscores"] = json::Array();
		io.response["level_pack"] = json::String(level_pack);
		

		INT32 a, count;

		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		io.response["stages_count"] = json::Number(tournament_config.totems_count);
		count = tournament_config.totems_count;
		//ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			if (leaderboard_data.highscores[a + 1].second <= 0) continue;
			//highscores[str(boost::format("%1%") % leaderboard_data.highscores[a + 1].first)] = json::Number(leaderboard_data.highscores[a + 1].second);
			highscores[str(boost::format("%1%") % a)]["user_id"] = json::Number(leaderboard_data.highscores[a + 1].first);
			highscores[str(boost::format("%1%") % a)]["score"] = json::Number(leaderboard_data.highscores[a + 1].second);
			//ar[a]["user_id"] = json::Number(leaderboard_data.highscores[a + 1].first);
			//ar[a]["score"] = json::Number(leaderboard_data.highscores[a + 1].second);
		}
	}
	else
	{
		INT32 stages_range = io.global_config.adventure_sets.stages_count; // get_INT32_from_json(io.jparams, "stages_range");
		INT64 subkey = (INT64)chalenge_mode_leaderboard_data.size();

		if(stages_range == 0)
		{
			io.response["error"] = json::String("Invalid parameters.");
			return;
		}
		else
		{
			stages_range --;
		}

		sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);
		
		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			chalenge_mode_leaderboard_data.push_back(leaderboard_data);

			subkey ++;
		}

		if(subkey <= stages_range / FACTOR)
		{		
			sprintf(needed_trigger_name, "leaderboard:%s:%lld", level_pack.data(), (DWORD64)subkey);
			
			if ((io.gs.runtime.waiting_for_leaderboard_request & (1 << subkey)) == 0)
			{
				if (upcalls)
				{
					add_response_trigger(io, needed_trigger_name);
					upcalls->RequestLeaderboard(io.user_id, level_pack, (DWORD64)subkey);
					io.gs.runtime.waiting_for_leaderboard_request |= (1 << subkey);
				}
			}
			return;
			
		}
		else
		{
			json::Object & highscores = io.response["highscores"] = json::Object();
			//json::Array & ar = io.response["highscores"] = json::Array();
			io.response["level_pack"] = json::String(level_pack);

			INT32 a, count;

			count = stages_range + 1;
			//ar.Resize(count);
			io.response["stages_count"] = json::Number(count);

			for(vector<SGamestateRuntime::_current_trigger::_leaderboard_data>::iterator it = chalenge_mode_leaderboard_data.begin(); it != chalenge_mode_leaderboard_data.end(); it ++)
			{				
				for (a = 0; a < FACTOR; a++)
				{
					if (it->highscores[a + 1].second <= 0) continue;
					//highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))] = json::Number(it->highscores[a + 1].second);
					highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))]["user_id"] = json::Number(it->highscores[a + 1].first);
					highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))]["score"] = json::Number(it->highscores[a + 1].second);
				}
			}

			chalenge_mode_leaderboard_data.clear();
			io.gs.runtime.waiting_for_leaderboard_request = 0;
		}


		//json::Array & ar = io.response["highscores"] = json::Array();
		//io.response["level_pack"] = json::String(level_pack);

	}

}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_user_position(SMessageIO & io)
{
#if defined(SGS_API_VERSION) && SGS_API_VERSION >= 103

	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	char needed_trigger_name[128];

	if (level_pack == "temple_0")
	{	

		sprintf(needed_trigger_name, "user_position:%s:%lld", "temple_0", io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboardUserPosition(io.user_id, "temple_0", io.gs.current_tournament_start_time);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
		json::Array & ar = io.response["highscores"] = json::Array();
		INT32 a, count;

		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		count = tournament_config.totems_count;
		ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			ar[a]["position"] = json::Number(leaderboard_data.highscores[a + 1].second);
		}
	}
	else
	{
		INT32 stages_range = io.global_config.adventure_sets.stages_count; //get_INT32_from_json(io.jparams, "stages_range");
		INT64 subkey = (INT64)chalenge_mode_user_positions_data.size();

		if(stages_range == 0)
		{
			io.response["error"] = json::String("Invalid parameters.");
			return;
		}
		else
		{
			stages_range --;
		}

		sprintf(needed_trigger_name, "user_position:%s:%lld", level_pack.data(), (DWORD64)subkey);

		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			chalenge_mode_user_positions_data.push_back(leaderboard_data);

			subkey ++;
		}

		if(subkey <= stages_range / FACTOR)
		{		
			sprintf(needed_trigger_name, "user_position:%s:%lld", level_pack.data(), (DWORD64)subkey);

			if ((io.gs.runtime.waiting_for_user_position_request & (1 << subkey)) == 0)
			{
				if (upcalls)
				{
					add_response_trigger(io, needed_trigger_name);
					upcalls->RequestLeaderboardUserPosition(io.user_id, level_pack, (DWORD64)subkey);
					io.gs.runtime.waiting_for_user_position_request |= (1 << subkey);
				}
			}
			return;

		}
		else
		{
			json::Array & ar = io.response["highscores"] = json::Array();
			io.response["level_pack"] = json::String(level_pack);

			INT32 a, count;

			count = stages_range + 1;
			ar.Resize(count);


			for(vector<SGamestateRuntime::_current_trigger::_leaderboard_data>::iterator it = chalenge_mode_user_positions_data.begin(); it != chalenge_mode_user_positions_data.end(); it ++)
			{				
				for (a = 0; a < FACTOR; a++)
				{
					ar[(INT32)it->subkey * FACTOR + a]["position"] = json::Number(it->highscores[a + 1].second);
				}
			}

			chalenge_mode_user_positions_data.clear();
			io.gs.runtime.waiting_for_user_position_request = 0;
		}
	}

#endif
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_user_position_optimized(SMessageIO & io)
{
#if defined(SGS_API_VERSION) && SGS_API_VERSION >= 103

	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}

	char needed_trigger_name[128];

	if (level_pack == "temple_0")
	{	

		sprintf(needed_trigger_name, "user_position:%s:%lld", "temple_0", io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboardUserPosition(io.user_id, "temple_0", io.gs.current_tournament_start_time);
			}
			return;
		}

		SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;

		json::Object & highscores = io.response["highscores"] = json::Object();			
		io.response["level_pack"] = json::String(level_pack);

		//json::Array & ar = io.response["highscores"] = json::Array();
		INT32 a, count;

		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		count = tournament_config.totems_count;
		//ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			if (leaderboard_data.highscores[a + 1].second == 0) continue; 

			highscores[str(boost::format("%1%") % a)] = json::Number(leaderboard_data.highscores[a + 1].second);
			//ar[a]["position"] = json::Number(leaderboard_data.highscores[a + 1].second);
		}
	}
	else
	{
		INT32 stages_range = io.global_config.adventure_sets.stages_count; //get_INT32_from_json(io.jparams, "stages_range");
		INT64 subkey = (INT64)chalenge_mode_user_positions_data.size();

		if(stages_range == 0)
		{
			io.response["error"] = json::String("Invalid parameters.");
			return;
		}
		else
		{
			stages_range --;
		}

		sprintf(needed_trigger_name, "user_position:%s:%lld", level_pack.data(), (DWORD64)subkey);

		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			chalenge_mode_user_positions_data.push_back(leaderboard_data);

			subkey ++;
		}

		if(subkey <= stages_range / FACTOR)
		{		
			sprintf(needed_trigger_name, "user_position:%s:%lld", level_pack.data(), (DWORD64)subkey);

			if ((io.gs.runtime.waiting_for_user_position_request & (1 << subkey)) == 0)
			{
				if (upcalls)
				{
					add_response_trigger(io, needed_trigger_name);
					upcalls->RequestLeaderboardUserPosition(io.user_id, level_pack, (DWORD64)subkey);
					io.gs.runtime.waiting_for_user_position_request |= (1 << subkey);
				}
			}
			return;

		}
		else
		{
			//json::Array & ar = io.response["highscores"] = json::Array();
			json::Object & highscores = io.response["highscores"] = json::Object();			
			io.response["level_pack"] = json::String(level_pack);

			//INT32 a, count;

			//count = stages_range + 1;
			//ar.Resize(count);


			for(vector<SGamestateRuntime::_current_trigger::_leaderboard_data>::iterator it = chalenge_mode_user_positions_data.begin(); it != chalenge_mode_user_positions_data.end(); it ++)
			{				
				for (INT32 a = 0; a < FACTOR; a++)
				{
					if (it->highscores[a + 1].second == 0) continue;
					highscores[str(boost::format("%1%") % ((INT32)it->subkey * FACTOR + a))] = json::Number(it->highscores[a + 1].second);
					//ar[(INT32)it->subkey * FACTOR + a]["position"] = json::Number(it->highscores[a + 1].second);
				}
			}

			chalenge_mode_user_positions_data.clear();
			io.gs.runtime.waiting_for_user_position_request = 0;
		}
	}

#endif
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_standings(SMessageIO & io)
{
	INT32 chamber = get_INT32_from_json(io.jparams, "chamber", -1);
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	INT32 request_standings_index = -1;
	DWORD64 subkey = 0;

	if (chamber == -1)
	{
		request_standings_index = 0;
	}
	else
	{
		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		if(level_pack == "temple_0")
		{
			if (chamber >= 0 && chamber < (INT32)tournament_config.totems_count)
			{
				request_standings_index = chamber + 1;
			}
		}
		else
		{
			if (chamber >= 0 )
			{
				request_standings_index = chamber + 1;
			}
		}		
	}

	if (request_standings_index == -1 || level_pack == "not_found")
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
		sprintf(needed_trigger_name, "standings:%s:%lld:%d", "xp", (DWORD64)0, (INT32)0);

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

	if (level_pack == "temple_0")
	{
		subkey = io.gs.current_tournament_start_time;
	}
	else
	{
		if (request_standings_index == 0)
			subkey = (DWORD64)0;
		else
			subkey = (DWORD64)(chamber / (FACTOR));

		request_standings_index = (chamber % FACTOR) + 1;
	}



	char needed_trigger_name[128];
	sprintf(needed_trigger_name, "standings:%s:%lld:%d", level_pack.data(), subkey, request_standings_index % (FACTOR + 1));

	if (io.gs.runtime.current_trigger.name != needed_trigger_name)
	{
		if (upcalls)
		{
			add_response_trigger(io, needed_trigger_name);
			INT32 max_results = -1;
			upcalls->RequestLeaderboardStandings(io.user_id, level_pack, subkey, request_standings_index % (FACTOR + 1), max_results);
		}
		return;
	}

	io.response["chamber"] = json::Number(chamber);
	io.response["level_pack"] = json::String(level_pack);

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
/***************************************************************************/
void GLogicEngineLastTemple::hgm_leaderboard_data(SMessageIO & io)
{
	string key = get_string_from_json(io.jparams, "key");
	INT64 subkey = get_INT64_from_json(io.jparams, "subkey");
	INT32 standings_index = get_INT32_from_json(io.jparams, "standings_index");
	INT32 batch_info = get_INT32_from_json(io.jparams, "batch_info");
	INT32 user_position = get_INT32_from_json(io.jparams, "user_position");
	json::Array highscores = json::Array(io.jparams["highscores"]);

	if (standings_index >= 0 && !batch_info && !user_position)
	{
		// Obcinamy zerowe wyniki.
		INT32 a, size;
		size = highscores.Size();
		for (a = 0; a < size; a++)
		{
			if (get_INT32_from_json(highscores[a], "score") == 0)
			{
				highscores.Resize(a);
				break;
			}
		}
	}

	SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
	leaderboard_data.key = key;
	leaderboard_data.subkey = subkey;
	leaderboard_data.standings_index = standings_index;
	INT32 a, count = highscores.Size();
	leaderboard_data.highscores.resize(count);
	for (a = 0; a < count; a++)
	{
		leaderboard_data.highscores[a].first = get_INT32_from_json(highscores[a], "user_id");
		leaderboard_data.highscores[a].second = get_INT32_from_json(highscores[a], "score");
	}

	if (leaderboard_data.key == "temple_0" && leaderboard_data.subkey == io.gs.current_tournament_start_time && standings_index == -1)
	{
		io.gs.runtime.cache_temple_0_leaders = leaderboard_data.highscores;
	}

	if (standings_index == -1)
	{
		char trigger_name[128];
		sprintf(trigger_name, "%s:%s:%lld", user_position ? "user_position" : "leaderboard", key.c_str(), subkey);
		execute_trigger(io, trigger_name);
	}
	else
	{
		char trigger_name[128];
		sprintf(trigger_name, "%s:%s:%lld:%d", batch_info ? "batch_info" : "standings", key.c_str(), subkey, standings_index);
		execute_trigger(io, trigger_name);
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_check_tournament_end(SMessageIO & io)
{
	io.response["finished_tournament_end_time"] = json::Number(0);

	DWORD64 current_tournament_end_time = io.gs.current_tournament_start_time + io.global_config.get_tournament_duration(io.gs.current_tournament_start_time);
	if (io.gs.runtime.debug_current_tournament_end_time != 0)
	{
		current_tournament_end_time = io.gs.runtime.debug_current_tournament_end_time;
	}

	INT64 to_end = (INT64)current_tournament_end_time - (INT64)io.timestamp;
	if (to_end < 0)
	{
		to_end = 0;
	}
	io.response["to_end"] = json::Number((double)to_end);

	DWORD64 global_tournament_start_time = io.global_config.current_global_tournament_start_time(io.timestamp);
	if ((io.gs.runtime.debug_current_tournament_end_time == 0 && io.gs.current_tournament_start_time == global_tournament_start_time) ||
		(io.gs.runtime.debug_current_tournament_end_time != 0 && (INT64)io.timestamp < io.gs.runtime.debug_current_tournament_end_time))
	{
		return;
	}
	if (io.gs.current_tournament_start_time == 0)
	{
		io.gs.current_tournament_start_time = global_tournament_start_time;
		io.gs.runtime.cache_temple_0_leaders.clear();
		return;
	}

	// Zeby podsumowac turniej musimy najpierw pobrac jego wyniki.
	io.response.Clear();
	if (io.gs.runtime.current_trigger.name == "")
	{
		// Wymuszamy pobranie listy liderow na nowo po stwierdzeniu konca turnieju.
		io.gs.runtime.cache_temple_0_leaders.clear();
	}

	if (io.gs.runtime.cache_temple_0_leaders.size() == 0)
	{
		char needed_trigger_name[128];
		sprintf(needed_trigger_name, "leaderboard:%s:%lld", "temple_0", io.gs.current_tournament_start_time);

		if (io.gs.runtime.current_trigger.name != needed_trigger_name)
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboard(io.user_id, "temple_0", io.gs.current_tournament_start_time);
			}
			return;
		}
	}

	char needed_trigger_name[128];
	sprintf(needed_trigger_name, "standings:%s:%lld:%d", "temple_0", io.gs.current_tournament_start_time, 0);
	if (io.gs.runtime.current_trigger.name != needed_trigger_name)
	{
		if (upcalls)
		{
			add_response_trigger(io, needed_trigger_name);
			upcalls->RequestLeaderboardStandings(io.user_id, "temple_0", io.gs.current_tournament_start_time, 0, -1);
		}
		return;
	}

	// Podsumowujemy turniej.
	if (io.gs.current_tournament_start_time == io.gs.runtime.current_trigger.leaderboard_data.subkey)
	{
		finish_tournament(io, io.gs.runtime.current_trigger.leaderboard_data.highscores);
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::finish_tournament(SMessageIO & io, vector<pair<INT32, DWORD32> > & results)
{
	DWORD64 global_tournament_start_time = io.global_config.current_global_tournament_start_time(io.timestamp);

	DWORD64 current_tournament_end_time = io.gs.current_tournament_start_time + io.global_config.get_tournament_duration(io.gs.current_tournament_start_time);
	if (io.gs.runtime.debug_current_tournament_end_time != 0)
	{
		current_tournament_end_time = io.gs.runtime.debug_current_tournament_end_time;
	}

	if ((io.gs.runtime.debug_current_tournament_end_time == 0 && io.gs.current_tournament_start_time == global_tournament_start_time) ||
		(io.gs.runtime.debug_current_tournament_end_time != 0 && (INT64)io.timestamp < io.gs.runtime.debug_current_tournament_end_time) ||
		io.gs.current_tournament_start_time == 0)
	{
		io.response["finished_tournament_end_time"] = json::Number(0);

		INT64 to_end = (INT64)current_tournament_end_time - (INT64)io.timestamp;
		if (to_end < 0)
		{
			to_end = 0;
		}
		io.response["to_end"] = json::Number((double)to_end);
		return;
	}

	// Konczymy turniej 'current_tournament_start_time'.
	io.response["finished_tournament_end_time"] = json::Number((double)current_tournament_end_time);

	json::Array winners;
	INT32 a, count = results.size(), winners_count = 0, our_position = -1;
	for (a = 0; a < 3 && a < count; a++)
	{
		winners[winners_count]["user_id"] = json::Number(results[a].first);
		winners[winners_count]["score"] = json::Number(results[a].second);
		winners_count++;
		if (results[a].first == io.gamestate_ID)
		{
			our_position = a;
		}
	}
	if (our_position == -1)
	{
		for (; a < count; a++)
		{
			if (results[a].first == io.gamestate_ID)
			{
				winners[winners_count]["user_id"] = json::Number(results[a].first);
				winners[winners_count]["score"] = json::Number(results[a].second);
				winners_count++;
				our_position = a;
				break;
			}
		}
	}
	// Dopelniamy liste zwyciezcow biezacym graczem.
	if (winners_count < 3 && our_position == -1)
	{
		winners[winners_count]["user_id"] = json::Number(io.gamestate_ID);
		winners[winners_count]["score"] = json::Number(0);
		winners_count++;
		our_position = winners_count;
	}
	// Dopelniamy liste zwyciezcow przyjaciolmi z zerowym wynikiem.
	if (winners_count < 3)
	{
		SOnlineAndOfflineFriends & friends = io.gs.runtime.online_and_offline_friends[io.gamestate_ID];
		set<INT32> all_friends = friends.offline_friends_id;
		all_friends.insert(friends.online_friends_id.begin(), friends.online_friends_id.end());
		INT32 a, count = results.size();
		for (a = 0; a < count; a++)
		{
			all_friends.erase(results[a].first);
		}

		while (winners_count < 3 && all_friends.size() > 0)
		{
			INT32 idx = rand() % all_friends.size();
			set<INT32>::iterator pos = all_friends.begin();
			advance(pos, idx);

			INT32 user_id = *pos;
			winners[winners_count]["user_id"] = json::Number(user_id);
			winners[winners_count]["score"] = json::Number(0);
			winners_count++;
			all_friends.erase(pos);
		}
	}

	io.response["winners"] = winners;
	io.response["our_position"] = json::Number((double)our_position + 1);
	if (io.response.Find("highscores") != io.response.End())
	{
		io.response.Erase(io.response.Find("highscores"));
	}

	// Przydzielanie nagrod w zaleznosci od 'our_position'.
	if (our_position == 0)
	{
		io.gs.player_info.tournament_prizes["gold"] += 1;
	}
	if (our_position == 1)
	{
		io.gs.player_info.tournament_prizes["silver"] += 1;
	}
	if (our_position == 2)
	{
		io.gs.player_info.tournament_prizes["bronze"] += 1;
	}

	check_after_tournament_achievements(io);

	// Zerujemy wyniki nowego turnieju (global_tournament_start_time).
	io.gs.player_info.score = 0;
	INT32 c;
	for (c = 0; c < 10; c++)
	{
		io.gs.player_info.temple_progress[c].score = 0;
		io.gs.player_info.temple_progress[c].longest_chain = 0;
		io.gs.player_info.temple_progress[c].highest_combo = 0;
		io.gs.player_info.temple_progress[c].time_left = 0;
	}
	io.gs.achievements.counters.last_chamber_finished = -1;

	//io.gs.settings.welcome_tournament_seen = 0;
	io.gs.settings.challenge_info_seen = 0;

	// Collections
	if (io.gs.player_info.current_collection + 1 < io.global_config.collection_sizes.size() &&
		io.gs.player_info.current_collection_progress == io.global_config.collection_sizes[io.gs.player_info.current_collection])
	{
		io.gs.player_info.current_collection++;
		io.gs.player_info.current_collection_progress = 0;
	}

	io.gs.current_tournament_start_time = global_tournament_start_time;
	io.gs.runtime.cache_temple_0_leaders.clear();
	current_tournament_end_time = io.gs.current_tournament_start_time + io.global_config.get_tournament_duration(io.gs.current_tournament_start_time);
	INT64 to_end = (INT64)current_tournament_end_time - (INT64)io.timestamp;
	if (to_end < 0)
	{
		to_end = 0;
	}
	io.response["to_end"] = json::Number((double)to_end);

	if (io.gs.runtime.debug_current_tournament_end_time != 0)
	{
		// clear leaderboard
		vector<INT32> scores;
		INT32 a;
		for (a = 0; a <= 10; a++)
		{
			scores.push_back(0);
		}
		upcalls->UpdateLeaderboard(io.user_id, "temple_0", io.gs.current_tournament_start_time, scores);

		io.gs.runtime.debug_current_tournament_end_time = 0;
	}

	io.response["player_info"] = io.gs["player_info"];
	io.response["totems_data_path"] = json::String();
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_global_standings(SMessageIO & io)
{
	INT32 chamber = get_INT32_from_json(io.jparams, "chamber", -1);

	INT32 request_standings_index = -1;

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

#if defined(SGS_API_VERSION) && SGS_API_VERSION >= 102

	char global_ranking_key[32];
	sprintf(global_ranking_key, "%lld:%d", io.gs.current_tournament_start_time, chamber);
	SGlobalRankingData & global_ranking_data = global_runtime.global_rankings[global_ranking_key];

	char needed_trigger_name[128];

	if (global_ranking_data.expiration_time <= io.timestamp && !global_ranking_data.have_scores)
	{
		sprintf(needed_trigger_name, "standings:%s:%lld:%d", "GLOBAL:temple_0", io.gs.current_tournament_start_time, request_standings_index);
		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			INT32 a, count = leaderboard_data.highscores.size();
			global_ranking_data.pending_standings.resize(count);
			for (a = 0; a < count; a++)
			{
				global_ranking_data.pending_standings[a].user_id = leaderboard_data.highscores[a].first;
				global_ranking_data.pending_standings[a].score = leaderboard_data.highscores[a].second;
				global_ranking_data.pending_standings[a].xp_level = 0;
			}

			global_ranking_data.have_scores = true;
		}
		else
		{
			if (upcalls)
			{
				add_response_trigger(io, needed_trigger_name);
				INT32 max_results = 25;
				upcalls->RequestLeaderboardStandings(io.user_id, "GLOBAL:temple_0", io.gs.current_tournament_start_time, request_standings_index, max_results);
			}
			return;
		}
	}

	if (global_ranking_data.expiration_time <= io.timestamp && global_ranking_data.have_scores)
	{
		sprintf(needed_trigger_name, "batch_info:%s:%lld:%d", "xp", (DWORD64)0, (INT32)0);
		if (io.gs.runtime.current_trigger.name == needed_trigger_name)
		{
			SGamestateRuntime::_current_trigger::_leaderboard_data & leaderboard_data = io.gs.runtime.current_trigger.leaderboard_data;
			INT32 a, rcount = global_ranking_data.pending_standings.size();
			INT32 b, count = leaderboard_data.highscores.size();
			for (a = 0; a < rcount; a++)
			{
				for (b = 0; b < count; b++)
				{
					if (leaderboard_data.highscores[b].first == global_ranking_data.pending_standings[a].user_id)
					{
						global_ranking_data.pending_standings[a].xp_level = leaderboard_data.highscores[b].second;
						break;
					}
				}
			}

			global_ranking_data.standings = global_ranking_data.pending_standings;
			global_ranking_data.have_scores = false;
			global_ranking_data.expiration_time = io.timestamp + 5 * 60;				// globalne wyniki beda wazne przez 5 minut
		}
		else
		{
			if (upcalls)
			{
				vector<INT32> users_id;
				INT32 a, count = global_ranking_data.pending_standings.size();
				users_id.resize(count);
				for (a = 0; a < count; a++)
				{
					users_id[a] = global_ranking_data.pending_standings[a].user_id;
				}
				add_response_trigger(io, needed_trigger_name);
				upcalls->RequestLeaderboardBatchInfo(io.user_id, "xp", (DWORD64)0, (INT32)0, users_id);
			}
			return;
		}
	}

	if (global_ranking_data.expiration_time > io.timestamp)
	{
		io.response["chamber"] = json::Number(chamber);

		json::Array & ar = io.response["highscores"] = json::Array();
		INT32 a, count = global_ranking_data.standings.size();
		ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			ar[a]["user_id"] = json::Number(global_ranking_data.standings[a].user_id);
			ar[a]["score"] = json::Number(global_ranking_data.standings[a].score);
			ar[a]["level"] = json::Number(global_ranking_data.standings[a].xp_level);
		}
		return;
	}

#endif
}
/***************************************************************************/
