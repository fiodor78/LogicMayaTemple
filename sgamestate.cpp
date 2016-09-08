/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
bool SGamestate::read_from_json(const char * json_data)
{
	bool result = true;
	Zero();

	json::Object o;
	try
	{
		std::istringstream is(json_data);
		json::Reader::Read(o, is);
	}
	catch (json::Reader::ScanException & e)
	{
		std::cout << "Caught json::ScanException: " << e.what() << ", Line/offset: " << e.m_locError.m_nLine + 1
				<< '/' << e.m_locError.m_nLineOffset + 1 << std::endl << std::endl;
		result = false;
	}
	catch (json::Reader::ParseException & e)
	{
		std::cout << "Caught json::ParseException: " << e.what() << ", Line/offset: " << e.m_locTokenBegin.m_nLine + 1
				 << '/' << e.m_locTokenBegin.m_nLineOffset + 1 << std::endl << std::endl;
		result = false;
	}

	if (!result)
	{
		return false;
	}

	try
	{
		// player_info
		player_info.gems = get_INT32_from_json(o["player_info"], "gems", 20);
		player_info.waka = get_INT32_from_json(o["player_info"], "waka", 0);
		player_info.lifes = get_INT32_from_json(o["player_info"], "lifes", 5);
		player_info.max_lifes = get_INT32_from_json(o["player_info"], "max_lifes", 5);
		player_info.extra_lifes = get_INT32_from_json(o["player_info"], "extra_lifes", 5);
		player_info.xp = get_INT32_from_json(o["player_info"], "xp", 0);
		player_info.level = get_INT32_from_json(o["player_info"], "level", 1);
		player_info.score = get_INT32_from_json(o["player_info"], "score", 0);
		player_info.free_spins_available = get_INT32_from_json(o["player_info"], "free_spins_available", 0);
		player_info.current_collection = get_INT32_from_json(o["player_info"], "current_collection", 0);
		player_info.current_collection_progress = get_INT32_from_json(o["player_info"], "current_collection_progress", 0);
		player_info.last_tournament_completed = get_INT64_from_json(o["player_info"], "last_tournament_completed", 0);

		{
			const json::Array & ar = json::Array(o["player_info"]["temple_progress"]);
			json::Array::const_iterator pos;
			INT32 idx;
			for (pos = ar.Begin(), idx = 0; pos != ar.End() && idx < 10; pos++, idx++)
			{
				player_info.temple_progress[idx].score = get_INT32_from_json(*pos, "score");
				player_info.temple_progress[idx].longest_chain = get_INT32_from_json(*pos, "longest_chain");
				player_info.temple_progress[idx].highest_combo = get_INT32_from_json(*pos, "highest_combo");
				player_info.temple_progress[idx].time_left = get_INT32_from_json(*pos, "time_left");
			}
		}

		{
			const json::Array & ar = json::Array(o["player_info"]["single_player_progress"]);
			json::Array::const_iterator pos;
		
			for (pos = ar.Begin(); pos != ar.End(); pos++)
			{
				string level_pack = get_string_from_json(*pos, "level_pack");
				const json::Array & ar_score = (*pos)["progress"];

				json::Array::const_iterator pos_score;
			
				
				if (player_info.single_player_progress.count(level_pack) == 0)
				{
					player_info.single_player_progress.insert(pair<string, map<INT32,SGamestate::_player_info::_temple_progress> >(level_pack,map<INT32,SGamestate::_player_info::_temple_progress>()) );
				}

				for (pos_score = ar_score.Begin(); pos_score != ar_score.End() ; pos_score++)
				{
					json::Object obj =  json::Object(*pos_score);


					INT32 idx_score = 0;
					idx_score = get_INT32_from_json(*pos_score, "chamber");
					

					if(player_info.single_player_progress[level_pack].count(idx_score) == 0)
					{						
						player_info.single_player_progress[level_pack].insert(pair<INT32,SGamestate::_player_info::_temple_progress>(idx_score, SGamestate::_player_info::_temple_progress()));
					}

					player_info.single_player_progress[level_pack][idx_score].score = get_INT32_from_json(*pos_score, "score");
					player_info.single_player_progress[level_pack][idx_score].longest_chain = get_INT32_from_json(*pos_score, "longest_chain");
					player_info.single_player_progress[level_pack][idx_score].highest_combo = get_INT32_from_json(*pos_score, "highest_combo");
					player_info.single_player_progress[level_pack][idx_score].time_left = get_INT32_from_json(*pos_score, "time_left");
				}
			}
		
		}

		{
			const json::Object & otmp = json::Object(o["player_info"]["tournament_prizes"]);
			json::Object::const_iterator pos;
			for (pos = otmp.Begin(); pos != otmp.End(); pos++)
			{
				player_info.tournament_prizes[pos->name] = (INT32)json::Number(pos->element);
			}
		}

		{
			const json::Array & otmp = json::Array(o["player_info"]["hired_friends"]);
			json::Array::const_iterator pos;
			
			for (INT32 i = 0; i < (INT32)otmp.Size(); i++)
			{
				player_info.hired_friends.push_back((INT32)json::Number(otmp[i]));
			}
		}

		// Disabling invited_friends
		{
			json::Object & p_info = o["player_info"];
			if (p_info.Find("invited_friends") != p_info.End())
			{
				p_info.Erase(p_info.Find("invited_friends"));
			}

			const json::Array & otmp = json::Array(o["player_info"]["invited_friends"]);
			json::Array::const_iterator pos;
			
			for (INT32 i = 0; i < (INT32)otmp.Size(); i++)
			{
				player_info.invited_friends.push_back((string)json::String(otmp[i]));
			}
		}

		

		// achievements
		achievements.enabled = get_INT32_from_json(o["achievements"], "enabled", 0) > 0 ? true : false;
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["achievements"]["awarded"]);
			for (pos = ar.Begin(); pos != ar.End(); pos++)
			{
				string type = get_string_from_json(*pos, "type");
				DWORD32 level = get_INT32_from_json(*pos, "level");
				if (level > 0)
				{
					achievements.awarded[type] = level;
				}
			}
		}
		achievements.counters.last_chamber_finished = get_INT32_from_json(o["achievements"]["counters"], "last_chamber_finished", -1);
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["achievements"]["counters"]["events_24h_game_played"]);
			INT32 idx;
			achievements.counters.events_24h_game_played.resize(ar.Size());
			for (pos = ar.Begin(), idx = 0; pos != ar.End(); pos++)
			{
				achievements.counters.events_24h_game_played[idx++] = (DWORD64)json::Number(*pos);
			}
		}
		{
			const json::Object & otmp = json::Object(o["achievements"]["counters"]["events_24h_reset"]);
			json::Object::const_iterator pos;
			for (pos = otmp.Begin(); pos != otmp.End(); pos++)
			{
				achievements.counters.events_24h_reset[pos->name] = (DWORD64)json::Number(pos->element);
			}
		}
		achievements.counters.last_perfectionist_awarded = get_INT64_from_json(o["achievements"]["counters"], "last_perfectionist_awarded", 0);

		// power_ups
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["power_ups"]["slots"]);
			INT32 a, count = ar.Size();
			power_ups.slots.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				string slot_status = get_string_from_json(*pos, "status");
				power_ups.slots[a].status = (slot_status == "") ? "DISABLED" : slot_status;
			}
			power_ups.slots_count = count;
		}
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["power_ups"]["enabled"]);
			for (pos = ar.Begin(); pos != ar.End(); pos++)
			{
				power_ups.enabled.insert(json::String(*pos));
			}
		}

		// items
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["items"]);
			INT32 a, count = ar.Size();
			items.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				items[a].id = get_string_from_json(*pos, "id");
				items[a].expiration_time = get_INT64_from_json(*pos, "expiration_time", 0);
				items[a].expiration_games = get_INT32_from_json(*pos, "expiration_games", 0);
				items[a].is_permanent = get_INT32_from_json(*pos, "is_permanent", 0) > 0;
			}
		}

		// settings
		settings.sound_enabled = get_INT32_from_json(o["settings"], "sound_enabled", 1);
		settings.music_enabled = get_INT32_from_json(o["settings"], "music_enabled", 1);
		settings.intro_seen = get_INT32_from_json(o["settings"], "intro_seen", 0);
		settings.arena_seen = get_INT32_from_json(o["settings"], "arena_seen", 0);
		settings.tooltips_enabled = get_INT32_from_json(o["settings"], "tooltips_enabled", 1);
		settings.welcome_tournament_seen = get_INT32_from_json(o["settings"], "welcome_tournament_seen", 0);
		settings.challenge_info_seen = get_INT32_from_json(o["settings"], "challenge_info_seen", 0);
		settings.tutorial_step = get_INT32_from_json(o["settings"], "tutorial_step", 0);
		settings.game_mode = get_string_from_json(o["settings"], "game_mode");


		// table
		table.game_in_progress = get_INT32_from_json(o["table"], "game_in_progress", 0) > 0;
		table.chamber = get_INT32_from_json(o["table"], "chamber", -1);
		table.level_pack = get_string_from_json(o["table"], "level_pack", "temple_0");
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["table"]["active_powerups"]);
			INT32 a, count = ar.Size();
			table.active_powerups.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				table.active_powerups[a] = json::String(*pos);
			}
		}
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["table"]["active_items"]);
			INT32 a, count = ar.Size();
			table.active_items.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				table.active_items[a] = json::String(*pos);
			}
		}
		{
			json::Array::const_iterator pos;
			// Disabling active_friends_requests

			json::Object & tbl = o["table"];
			if (tbl.Find("active_friends_requests") != tbl.End())
			{
				tbl.Erase(tbl.Find("active_friends_requests"));
			}

			const json::Array & ar = json::Array(o["table"]["active_friends_requests"]);
			// Disable firend active_friends_requests
			INT32 a, count = ar.Size();
			table.active_items.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				string social_id = get_string_from_json((*pos), "social_id", "");
				DWORD64 send_time = get_INT64_from_json((*pos), "send_time");

				if (social_id == "") continue;

				table.active_friends_requests.push_back(SGamestate::_friend_request(social_id, send_time));
			}
		}
		if (o.Find("used_promocodes") != o.End())
		{ // promocodes
			const json::Array & ar = json::Array(o["used_promocodes"]);
			INT32 a = ar.Size() - 1;

			for (; a >= 0 ; a--)
			{
				used_promocodes.insert(json::String(ar[a]));
			}
		}

		
		table.waka_locked = get_INT32_from_json(o["table"], "waka_locked", 0);
		table.game_start_timestamp = get_INT64_from_json(o["table"], "game_start_timestamp", 0);

		// daily_bonus
		daily_bonus.last_awarded_timestamp = get_INT64_from_json(o["daily_bonus"], "last_awarded_timestamp", 0);
		daily_bonus.last_awarded_value = get_INT32_from_json(o["daily_bonus"], "last_awarded_value", 0);

		// payments
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["payments"]);
			INT32 a, count = ar.Size();
			payments.resize(count);
			for (pos = ar.Begin(), a = 0; pos != ar.End(); pos++, a++)
			{
				payments[a].paymentid = get_INT64_from_json(*pos, "paymentid", 0);
			}
		}

		// inne
		current_tournament_start_time = get_INT64_from_json(o, "current_tournament_start_time", 0);
		last_life_refill_time = get_INT64_from_json(o, "last_life_refill_time", 0);
		saved_at_server_close = get_INT32_from_json(o, "saved_at_server_close", 0) > 0;

		affiliate_id = get_INT32_from_json(o, "affiliate_id", 0);

		if (o.Find("logic_reload") != o.End())
		{
			runtime.read_from_json(o["logic_reload"]);
		}
	}
	catch (json::Exception & )
	{
		Zero();
		result = false;
	}

	return result;
}
/***************************************************************************/
void SGamestate::write_to_json(json::Object & o, const string & which_part, bool client_response, bool server_closing, bool logic_reload)
{
	INT32 a, b;

	o.Clear();

	if (which_part == "" || which_part == "player_info")
	{
		o["player_info"]["gems"] = json::Number(player_info.gems);
		o["player_info"]["waka"] = json::Number(player_info.waka);
		o["player_info"]["lifes"] = json::Number(player_info.lifes);
		o["player_info"]["max_lifes"] = json::Number(player_info.max_lifes);
		o["player_info"]["extra_lifes"] = json::Number(player_info.extra_lifes);
		o["player_info"]["xp"] = json::Number(player_info.xp);
		o["player_info"]["level"] = json::Number(player_info.level);
		o["player_info"]["score"] = json::Number(player_info.score);
		o["player_info"]["free_spins_available"] = json::Number(player_info.free_spins_available);
		o["player_info"]["current_collection"] = json::Number(player_info.current_collection);
		o["player_info"]["current_collection_progress"] = json::Number(player_info.current_collection_progress);
		if (!client_response)
		{
			o["player_info"]["last_tournament_completed"] = json::Number((double)player_info.last_tournament_completed);
		}

		{
			json::Array & ar = o["player_info"]["temple_progress"] = json::Array();
			ar.Resize(10);
			for (b = 0; b < 10; b++)
			{
				ar[b]["score"] = json::Number(player_info.temple_progress[b].score);
				ar[b]["longest_chain"] = json::Number(player_info.temple_progress[b].longest_chain);
				ar[b]["highest_combo"] = json::Number(player_info.temple_progress[b].highest_combo);
				ar[b]["time_left"] = json::Number(player_info.temple_progress[b].time_left);
			}
		}

		{
			json::Array & ar = o["player_info"]["single_player_progress"] = json::Array();
			INT32 i = 0; 


			for (map<string, map<INT32,SGamestate::_player_info::_temple_progress> >::iterator it = player_info.single_player_progress.begin(); it != player_info.single_player_progress.end(); ++it)
			{
				string level_pack = it->first;
				map<INT32,SGamestate::_player_info::_temple_progress> progress = it->second;


				json::Object & row = ar[i++] = json::Object();
				json::Array & progress_ar = row["progress"] = json::Array();				
				row["level_pack"] = json::String(level_pack);
				
				INT32 idx = 0;
				for (map<INT32,SGamestate::_player_info::_temple_progress>::iterator prog_it = progress.begin(); prog_it != progress.end(); ++prog_it, ++idx )
				{
					progress_ar[idx]["chamber"] = json::Number(prog_it->first);
					progress_ar[idx]["score"] = json::Number(prog_it->second.score);
					progress_ar[idx]["longest_chain"] = json::Number(prog_it->second.longest_chain);
					progress_ar[idx]["highest_combo"] = json::Number(prog_it->second.highest_combo);
					progress_ar[idx]["time_left"] = json::Number(prog_it->second.time_left);
				}

			}
		}

		{
			o["player_info"]["tournament_prizes"] = json::Object();
			json::Object & otmp = o["player_info"]["tournament_prizes"];
			map<string, DWORD32>::iterator pos;
			for (pos = player_info.tournament_prizes.begin(); pos != player_info.tournament_prizes.end(); pos++)
			{
				otmp[pos->first] = json::Number(pos->second);
			}
		}

		{
			json::Array & ar = o["player_info"]["hired_friends"] = json::Array();
			
			for (INT32 i = 0; i < (INT32)player_info.hired_friends.size(); i++)
			{
				ar[i] = json::Number(player_info.hired_friends[i]);
			}
		}
		{
			json::Array & ar = o["player_info"]["invited_friends"] = json::Array();
			
			/*for (INT32 i = 0; i < (INT32)player_info.invited_friends.size(); i++)
			{
				ar[i] = json::String(player_info.invited_friends[i]);
			}*/
		}
		
	}





	if (which_part == "" || which_part == "achievements")
	{
		o["achievements"]["enabled"] = json::Number(achievements.enabled ? 1 : 0);
		{
			json::Array & ar = o["achievements"]["awarded"] = json::Array();
			ar.Resize(achievements.awarded.size());
			map<string, DWORD32>::iterator pos;
			for (pos = achievements.awarded.begin(), a = 0; pos != achievements.awarded.end(); pos++)
			{
				if (pos->second > 0)
				{
					ar[a]["type"] = json::String(pos->first);
					ar[a]["level"] = json::Number(pos->second);
					a++;
				}
			}
			ar.Resize(a);
		}

		if (!client_response)
		{
			json::Object & ocounters = o["achievements"]["counters"] = json::Object();
			ocounters["last_chamber_finished"] = json::Number(achievements.counters.last_chamber_finished);
			{
				json::Array & ar = ocounters["events_24h_game_played"] = json::Array();
				INT32 count = achievements.counters.events_24h_game_played.size();
				ar.Resize(count);
				for (a = 0; a < count; a++)
				{
					ar[a] = json::Number((double)achievements.counters.events_24h_game_played[a]);
				}
			}
			{
				json::Object & otmp = ocounters["events_24h_reset"] = json::Object();
				map<string, DWORD64>::iterator pos;
				for (pos = achievements.counters.events_24h_reset.begin(); pos != achievements.counters.events_24h_reset.end(); pos++)
				{
					otmp[pos->first] = json::Number((double)pos->second);
				}
			}
			ocounters["last_perfectionist_awarded"] = json::Number((double)achievements.counters.last_perfectionist_awarded);
		}
	}

	if (which_part == "" || which_part == "power_ups")
	{
		o["power_ups"]["slots"] = json::Array();
		json::Array & ar = o["power_ups"]["slots"];
		ar.Resize(power_ups.slots_count);
		for (a = 0; a < power_ups.slots_count; a++)
		{
			ar[a]["status"] = json::String(power_ups.slots[a].status);
		}

		if (!client_response)
		{
			o["power_ups"]["enabled"] = json::Array();
			json::Array & ar2 = o["power_ups"]["enabled"];
			ar2.Resize(power_ups.enabled.size());
			set<string>::iterator pos;
			for (pos = power_ups.enabled.begin(), a = 0; pos != power_ups.enabled.end(); pos++, a++)
			{
				ar2[a] = json::String(*pos);
			}
		}
	}

	if (which_part == "" || which_part == "items")
	{
		o["items"] = json::Array();
		json::Array & ar = o["items"];
		INT32 count = items.size();
		ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			ar[a]["id"] = json::String(items[a].id);
			if (!client_response)
			{
				ar[a]["expiration_time"] = json::Number((double)items[a].expiration_time);
			}
			ar[a]["expiration_games"] = json::Number(items[a].expiration_games);
			ar[a]["is_permanent"] = json::Number(items[a].is_permanent ? 1 : 0);
		}
	}

	if (which_part == "" || which_part == "settings")
	{
		o["settings"]["sound_enabled"] = json::Number(settings.sound_enabled);
		o["settings"]["music_enabled"] = json::Number(settings.music_enabled);
		o["settings"]["intro_seen"] = json::Number(settings.intro_seen);
		o["settings"]["arena_seen"] = json::Number(settings.arena_seen);
		o["settings"]["tooltips_enabled"] = json::Number(settings.tooltips_enabled);
		o["settings"]["welcome_tournament_seen"] = json::Number(settings.welcome_tournament_seen);
		o["settings"]["challenge_info_seen"] = json::Number(settings.challenge_info_seen);
		o["settings"]["tutorial_step"] = json::Number(settings.tutorial_step);
		o["settings"]["game_mode"] = json::String(settings.game_mode);
	}

	if (which_part == "" || which_part == "table")
	{
		o["table"]["game_in_progress"] = json::Number(table.game_in_progress ? 1 : 0);
		o["table"]["level_pack"] = json::String(table.level_pack);
		o["table"]["chamber"] = json::Number(table.chamber);
		o["table"]["active_powerups"] = json::Array();
		{
			json::Array & ar = o["table"]["active_powerups"];
			INT32 count = table.active_powerups.size();
			ar.Resize(count);
			for (a = 0; a < count; a++)
			{
				ar[a] = json::String(table.active_powerups[a]);
			}
		}
		o["table"]["active_items"] = json::Array();
		{
			json::Array & ar = o["table"]["active_items"];
			INT32 count = table.active_items.size();
			ar.Resize(count);
			for (a = 0; a < count; a++)
			{
				ar[a] = json::String(table.active_items[a]);
			}
		}
		o["table"]["active_friends_requests"] = json::Boolean();
		{
			json::Array & ar = o["table"]["active_friends_requests"];
			//INT32 count = table.active_friends_requests.size();
			//ar.Resize(count);
			//for (a = 0; a < count; a++)
			//{
			//	ar[a] = json::Object();
			//	ar[a]["social_id"] = json::String(table.active_friends_requests[a].social_id);
			//	ar[a]["send_time"] = json::Number(table.active_friends_requests[a].send_time);
			//}
		}

		if (!client_response)
		{
			o["table"]["waka_locked"] = json::Number(table.waka_locked);
			o["table"]["game_start_timestamp"] = json::Number((double)table.game_start_timestamp);
		}
	}

	// promocodes
	if ((which_part == "" || which_part == "used_promocodes") && !client_response)
	{
		json::Array & ar = o["used_promocodes"] = json::Array();
		ar.Resize(used_promocodes.size());
		INT32 a = 0;
		
		for (set<string>::const_iterator it = used_promocodes.begin(); it != used_promocodes.end(); it++)
		{
			ar[a++] = json::String(*it);
		}
	}

	if ((which_part == "" || which_part == "payments") && !client_response)
	{
		o["payments"] = json::Array();
		json::Array & ar = o["payments"];
		INT32 count = payments.size();
		ar.Resize(count);
		for (a = 0; a < count; a++)
		{
			ar[a]["paymentid"] = json::Number((double)payments[a].paymentid);
		}
	}

	if (which_part == "")
	{
		if (!client_response)
		{
			o["daily_bonus"]["last_awarded_timestamp"] = json::Number((double)daily_bonus.last_awarded_timestamp);
			o["daily_bonus"]["last_awarded_value"] = json::Number(daily_bonus.last_awarded_value);

			o["current_tournament_start_time"] = json::Number((double)current_tournament_start_time);
			o["last_life_refill_time"] = json::Number((double)last_life_refill_time);
			o["saved_at_server_close"] = json::Number(server_closing ? 1 : 0);

			o["affiliate_id"] = json::Number(affiliate_id);

			if (logic_reload)
			{
				o["logic_reload"] = json::Object();
				runtime.write_to_json(o["logic_reload"]);
			}
		}
	}
}
/***************************************************************************/
const json::Object SGamestate::operator[] (const std::string & key)
{
	json::Object result;
	write_to_json(result, key, true, false, false);

	if (key == "")
	{
		return result;
	}
	if (key == "player_info" ||
		key == "achievements" ||
		key == "power_ups" ||
		key == "settings" ||
		key == "table")
	{
		return result[key];
	}
	if (key == "items" ||
		key == "payments")
	{
		json::Object tmp;
		tmp["array"] = result[key];
		return tmp;
	}
	return result;
}
/***************************************************************************/
void SGamestate::validate_gamestate(SGlobalConfig & global_config, DWORD64 timestamp)
{
	// inne
	if (current_tournament_start_time == 0)
	{
		current_tournament_start_time = global_config.current_global_tournament_start_time(timestamp);
		player_info.last_tournament_completed = 0;
	}

	// player_info
	player_info.max_lifes = CLAMP(player_info.max_lifes, (DWORD32)5, (DWORD32)8);
	player_info.lifes = CLAMP(player_info.lifes, 0, player_info.max_lifes);
	DWORD32 level;
	for (level = 1; level + 1 < global_config.xp_level_config.size() && global_config.xp_level_config[level + 1].xp_req <= player_info.xp ; level++) ;
	player_info.level = max(player_info.level, level);
	player_info.score = 0;
	INT32 a;
	for (a = 0; a < 10; a++)
	{
		player_info.score += player_info.temple_progress[a].score;
	}
	player_info.tournament_prizes["gold"] += 0;
	player_info.tournament_prizes["silver"] += 0;
	player_info.tournament_prizes["bronze"] += 0;
	if (global_config.collection_sizes.size() == 0)
	{
		player_info.current_collection = 0;
		player_info.current_collection_progress = 0;
	}
	else
	{
		player_info.current_collection = CLAMP(player_info.current_collection, 0, global_config.collection_sizes.size() - 1);
		player_info.current_collection_progress = CLAMP(player_info.current_collection_progress, (DWORD32)0, global_config.collection_sizes[player_info.current_collection]);

		// Sprawdzamy czy w miedzyczasie nie zostala dodana jakas nowa kolekcja.
		if (player_info.current_collection + 1 < global_config.collection_sizes.size() &&
			player_info.current_collection_progress == global_config.collection_sizes[player_info.current_collection] &&
			player_info.last_tournament_completed != current_tournament_start_time)
		{
			player_info.current_collection++;
			player_info.current_collection_progress = 0;
		}
	}

	// achievements
	if (!achievements.enabled)
	{
		achievements.awarded.clear();
		achievements.counters.last_chamber_finished = -1;
		achievements.counters.events_24h_game_played.clear();
		achievements.counters.events_24h_reset.clear();
		achievements.counters.last_perfectionist_awarded = 0;
	}
	else
	{
		vector<DWORD64> & timestamps = achievements.counters.events_24h_game_played;
		sort(timestamps.begin(), timestamps.end());
		timestamps.erase(timestamps.begin(), lower_bound(timestamps.begin(), timestamps.end(), timestamp - 24 * 3600));
	}

	// power_ups
	if (power_ups.slots_count != global_config.power_up_slot_prices.size())
	{
		power_ups.slots_count = global_config.power_up_slot_prices.size();
		power_ups.slots.resize(power_ups.slots_count);
	}
	for (a = 0; a < power_ups.slots_count; a++)
	{
		if (power_ups.slots[a].status == "")
		{
			power_ups.slots[a].status = "DISABLED";
			continue;
		}
		if (power_ups.slots[a].status != "DISABLED" && power_ups.slots[a].status != "EMPTY")
		{
			if (global_config.powerup_items.find(power_ups.slots[a].status) == global_config.powerup_items.end())
			{
				power_ups.slots[a].status = "EMPTY";
			}
		}
	}

	// daily_bonus
	if (daily_bonus.last_awarded_timestamp > timestamp + 100)
	{
		daily_bonus.last_awarded_timestamp = 0;
		daily_bonus.last_awarded_value = 0;
	}
	daily_bonus.last_awarded_value = CLAMP(daily_bonus.last_awarded_value, 0, global_config.spin_wheel.daily_awards.size());

	// nagrody za wczesniejsze levele
	for (level = 1; level <= player_info.level && level < (INT32)global_config.xp_level_config.size(); level++)
	{
		INT32 slotid = global_config.xp_level_config[level].powerup_slot_unlock;
		if (slotid >= 1 && slotid <= power_ups.slots_count && power_ups.slots[slotid - 1].status == "DISABLED")
		{
			power_ups.slots[slotid - 1].status = "EMPTY";
		}
		power_ups.enabled.insert(global_config.xp_level_config[level].powerup_unlock);
		player_info.max_lifes = max(player_info.max_lifes, global_config.xp_level_config[level].max_lifes_amount);
	}



	// table
	if (!table.game_in_progress)
	{
		table.chamber = -1;
		table.active_powerups.clear();
		table.active_items.clear();
		table.waka_locked = 0;
		table.game_start_timestamp = 0;
	}
	else
	{
		if (!runtime.recovered_from_json)
		{
			if (saved_at_server_close)
			{
				// Traktujemy gra jak niebyla, anulowana z winy serwera.
				if (table.waka_locked > 0)
				{
					player_info.waka += table.waka_locked;
				}
			}
			else
			{
				// Traktujemy gra jak zakonczona z powodu rozlaczenia gracza.
			}

			table.game_in_progress = false;
			table.chamber = -1;
			table.active_powerups.clear();
			table.active_items.clear();
			table.waka_locked = 0;
			table.game_start_timestamp = 0;
		}
	}

	runtime.recovered_from_json = false;
}
/***************************************************************************/
void SGamestate::check_friends_requests(DWORD64 timestamp)
{
	INT32 a, count = table.active_friends_requests.size();
	for (a = 0; a < count; a++)
	{
		table.active_friends_requests[a].remaining_time = timestamp - table.active_friends_requests[a].send_time;

		if (table.active_friends_requests[a].remaining_time >= (24 * 60 * 60 * 1000))
		{
			table.active_friends_requests.erase(table.active_friends_requests.begin() + a);
			a--;
			count--;
			continue;
		}
	}
}
/***************************************************************************/
void SGamestate::remove_expired_items(DWORD64 timestamp)
{
	INT32 a, count = items.size();
	for (a = 0; a < count; a++)
	{
		if (items[a].is_permanent)
		{
			continue;
		}
		
		if ((items[a].expiration_time == 0 && items[a].expiration_games == 0) ||
			(items[a].expiration_time > 0 && items[a].expiration_time <= timestamp))
		{
			items.erase(items.begin() + a);
			a--;
			count--;
			continue;
		}
	}
}
/***************************************************************************/
DWORD64 SGamestate::get_lifes_recover_time(DWORD64 life_refill_duration, DWORD64 timestamp)
{
	if (player_info.max_lifes < 5)
	{
		player_info.max_lifes = 5;
		player_info.lifes = player_info.max_lifes;
		last_life_refill_time = timestamp;
	}

	DWORD64 time_recover = 0;
	if (player_info.lifes < player_info.max_lifes)
	{
		INT32 available_lifes = 0;
		INT64 duration_since_last_refill = (INT64)timestamp - (INT64)last_life_refill_time;
		if (duration_since_last_refill >= 0)
		{
			available_lifes = (life_refill_duration == 0) ? player_info.max_lifes : (INT32)(duration_since_last_refill / life_refill_duration);
			if (available_lifes > 0)
			{
				if (player_info.lifes + available_lifes >= player_info.max_lifes)
				{
					player_info.lifes = player_info.max_lifes;
					last_life_refill_time = timestamp;
				}
				else
				{
					player_info.lifes += available_lifes;
					last_life_refill_time += available_lifes * life_refill_duration;
					duration_since_last_refill -= available_lifes * life_refill_duration;
					time_recover = life_refill_duration - duration_since_last_refill;
				}
			}
			else
			{
				time_recover = life_refill_duration - duration_since_last_refill;
			}
		}
	}
	return time_recover;
}
/***************************************************************************/
STournamentConfig & SGamestate::get_current_tournament_config(SGlobalConfig & global_config)
{
	return global_config.get_tournament_config(current_tournament_start_time);
}
/***************************************************************************/
void SGamestate::add_client_fields_to_response(SGlobalConfig & global_config, DWORD64 timestamp, json::Object & response)
{
	if (response.Find("player_info") != response.End())
	{
		response["player_info"]["next_life_recover_time"] = json::Number((double)get_lifes_recover_time(global_config.life_refill_duration, timestamp));
		response["player_info"]["lifes"] = json::Number(player_info.lifes);
		response["player_info"]["max_lifes"] = json::Number(player_info.max_lifes);
		response["player_info"]["current_tournament_id"] = json::Number(global_config.get_tournament_index(current_tournament_start_time));
		response["server_timestamp"] = json::Number((double)timestamp);
	}

	if (response.Find("items") != response.End())
	{
		INT32 a, count = items.size();
		for (a = 0; a < count; a++)
		{
			if (items[a].expiration_time == 0)
			{
				response["items"][a]["time_left"] = json::Number(-1);
			}
			else
			{
				response["items"][a]["time_left"] = json::Number((double)max((INT64)items[a].expiration_time - (INT64)timestamp, (INT64)0));
			}
		}
	}

	if (response.Find("power_ups") != response.End())
	{
		response["power_ups"]["repo"] = json::Array();
		json::Array & repo = response["power_ups"]["repo"];
		INT32 a, count = global_config.powerup_items.size();
		repo.Resize(count);
		TPowerUpItemsMap::iterator pos;
		for (pos = global_config.powerup_items.begin(), a = 0; pos != global_config.powerup_items.end(); pos++, a++)
		{
			repo[a]["type"] = json::String(pos->second.type);
			if (power_ups.enabled.find(pos->second.type) != power_ups.enabled.end())
			{
				repo[a]["price"] = json::Number(0);
				repo[a]["unlock_level"] = json::Number(0);
			}
			else
			{
				if (pos->second.price == 0)
				{
					repo[a]["price"] = json::Number((double)(powerup_gems_cost(this->affiliate_id)));
				}
				else
				{
					repo[a]["price"] = json::Number(pos->second.price);
				}
				repo[a]["unlock_level"] = json::Number((double)pos->second.unlock_level);
			}
		}

		json::Array & slots = response["power_ups"]["slots"];
		for (a = 0; a < power_ups.slots_count; a++)
		{
			if (power_ups.slots[a].status != "DISABLED")
			{
				slots[a]["price"] = json::Number(0);
				slots[a]["unlock_level"] = json::Number(0);
			}
			else
			{
				if (player_info.level < global_config.power_up_slot_prices[a].xp_level_req)
				{
					slots[a]["price"] = json::Number(0);
				}
				else
				{
					slots[a]["price"] = json::Number(global_config.power_up_slot_prices[a].price);
				}
				if (global_config.power_up_slot_prices[a].unlock_level != 0)
				{
					slots[a]["unlock_level"] = json::Number(global_config.power_up_slot_prices[a].unlock_level);
				}
				else
				{
					if (player_info.level < global_config.power_up_slot_prices[a].xp_level_req)
					{
						slots[a]["unlock_level"] = json::Number(global_config.power_up_slot_prices[a].xp_level_req);
					}
					else
					{
						slots[a]["unlock_level"] = json::Number(0);
					}
				}
			}
		}
	}

	if (response.Find("totems_data_path") != response.End())
	{
		STournamentConfig & cfg = get_current_tournament_config(global_config);
		response["totems_data_path"] = json::String(cfg.totems_data_path);
	}
}
/***************************************************************************/
