/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

struct SInt2StringValue
{
	INT32			value;
	const char *	id;
};
typedef SInt2StringValue TInt2StringMap[];

/***************************************************************************/
void GLogicEngineLastTemple::award_achievement(SMessageIO & io, const char * achievement_id)
{
	io.gs.achievements.awarded[achievement_id] += 1;

	json::Object achievement_info;
	achievement_info["type"] = json::String(achievement_id);
	achievement_info["level"] = json::Number(io.gs.achievements.awarded[achievement_id]);

	if (io.response.Find("achievements_awarded") == io.response.End())
	{
		io.response["achievements_awarded"] = json::Array();
	}
	json::Array & oachievements = io.response["achievements_awarded"];
	oachievements.Insert(achievement_info);
}
/***************************************************************************/
void GLogicEngineLastTemple::check_after_game_achievements(SMessageIO & io)
{
	if (!io.gs.achievements.enabled || (io.gs.table.level_pack == "adventure" && io.gs.table.chamber <= 4))
	{
		return;
	}

	INT32 a;
	STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
	string status = get_string_from_json(io.jparams, "status");

	// Chain Master
	if (status == "WON")
	{
		static INT32 id_chain_master[] = { 10, 15, 20, 25, 30 };

		INT32 longest_chain = get_INT32_from_json(io.jparams, "longest_chain");
		for (a = io.gs.achievements.awarded["CHAIN_MASTER"]; a < (int)sizeof(id_chain_master) / sizeof(id_chain_master[0]); a++)
		{
			if (longest_chain >= id_chain_master[a])
			{
				award_achievement(io, "CHAIN_MASTER");
				break;
			}
		}
	}

	// Combo Hero
	if (status == "WON")
	{
		static INT32 id_combo_hero[] = { 10, 15, 20, 25, 30 };
	
		INT32 highest_combo = get_INT32_from_json(io.jparams, "highest_combo");
		for (a = io.gs.achievements.awarded["COMBO_HERO"]; a < (int)sizeof(id_combo_hero) / sizeof(id_combo_hero[0]); a++)
		{
			if (highest_combo >= id_combo_hero[a])
			{
				award_achievement(io, "COMBO_HERO");
				break;
			}
		}
	}

	// Unstoppable
	static TInt2StringMap id_unstoppable =
		{
			{25, "UNSTOPPABLE_25" },
			{50, "UNSTOPPABLE_50" },
			{100, "UNSTOPPABLE_100" },
			{175, "UNSTOPPABLE_175" },
			{250, "UNSTOPPABLE_250" },
		};

	if (status == "WON" || status == "LOST")
	{
		vector<DWORD64> & timestamps = io.gs.achievements.counters.events_24h_game_played;
		timestamps.erase(timestamps.begin(), lower_bound(timestamps.begin(), timestamps.end(), io.timestamp - 24 * 3600));
		timestamps.push_back(io.timestamp);

		for (a = io.gs.achievements.awarded["UNSTOPPABLE"]; a < (int)sizeof(id_unstoppable) / sizeof(id_unstoppable[0]); a++)
		{
			DWORD64 last_time_awarded = io.gs.achievements.counters.events_24h_reset[id_unstoppable[a].id];
			if (last_time_awarded + 23 * 3600 >= io.timestamp)
			{
				continue;
			}

			vector<DWORD64>::iterator pos;
			pos = lower_bound(timestamps.begin(), timestamps.end(), last_time_awarded + 1);
			INT32 games_completed = timestamps.end() - pos;

			if (games_completed >= id_unstoppable[a].value)
			{
				award_achievement(io, "UNSTOPPABLE");
				io.gs.achievements.counters.events_24h_reset[id_unstoppable[a].id] = io.timestamp;
				break;
			}
		}
	}

	// Conqueror
	if (io.gs.table.level_pack == "temple_0" && io.gs.achievements.counters.last_chamber_finished != -2)
	{
		// Gracz musi najpierw ukonczyc chamber 0, potem 1 itd...
		if (status != "WON" || io.gs.table.chamber != io.gs.achievements.counters.last_chamber_finished + 1)
		{
			io.gs.achievements.counters.last_chamber_finished = -1;
		}
		else
		{
			if (io.gs.table.chamber == tournament_config.totems_count - 1)
			{
				if (io.gs.achievements.awarded["CONQUEROR"] < 5)
				{
					award_achievement(io, "CONQUEROR");
				}
				io.gs.achievements.counters.last_chamber_finished = -2;
			}
			else
			{
				io.gs.achievements.counters.last_chamber_finished = io.gs.table.chamber;
			}
		}
	}

	// Godspeed
	if (status == "WON")
	{
		static INT32 id_godspeed[] = { 2, 3, 4, 5, 6 };
	
		INT32 time_left = get_INT32_from_json(io.jparams, "time_left");
		INT32 time_limit = 0;
		if (io.gs.table.chamber >= 0 && io.gs.table.chamber < (int)tournament_config.totems_count)
		{
			time_limit = tournament_config.time_limits[io.gs.table.chamber];
		}

		for (a = io.gs.achievements.awarded["GODSPEED"]; a < (int)sizeof(id_godspeed) / sizeof(id_godspeed[0]); a++)
		{
			if ((time_limit - time_left) * id_godspeed[a] <= time_limit)
			{
				award_achievement(io, "GODSPEED");
				break;
			}
		}
	}

	// Swift Glory
	if (status == "WON" && io.gs.achievements.awarded["SWIFT_GLORY"] < 5)
	{
		INT32 last_chamber = tournament_config.totems_count - 1;
		if ( io.gs.table.level_pack == "temple_0" && io.gs.table.chamber == last_chamber)
		{
			if (last_chamber >= 0 && last_chamber + 1 < (INT32)io.gs.runtime.cache_temple_0_leaders.size())
			{
				if (io.gs.runtime.cache_temple_0_leaders[last_chamber + 1].first == 0)
				{
					award_achievement(io, "SWIFT_GLORY");
				}
			}
		}
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::check_after_tournament_achievements(SMessageIO & io)
{
	if (!io.gs.achievements.enabled)
	{
		return;
	}

	INT32 a;
	STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);

	// Sacred Warrior
	INT32 our_position = get_INT32_from_json(io.response, "our_position");
	if (our_position >= 1 && our_position <= 3)
	{
		static INT32 id_sacred_warrior[] = { 1, 5, 10, 15, 20 };

		INT32 total_medals = 0;
		map<string, DWORD32>::iterator pos;
		for (pos = io.gs.player_info.tournament_prizes.begin(); pos != io.gs.player_info.tournament_prizes.end(); pos++)
		{
			total_medals += pos->second;
		}

		for (a = io.gs.achievements.awarded["SACRED_WARRIOR"]; a < (int)sizeof(id_sacred_warrior) / sizeof(id_sacred_warrior[0]); a++)
		{
			if (total_medals >= id_sacred_warrior[a])
			{
				award_achievement(io, "SACRED_WARRIOR");
				break;
			}
		}
	}

	// Totem Sweeper
	if (io.gs.table.level_pack == "temple_0" && io.gs.achievements.awarded["TOTEM_SWEEPER"] < 5)
	{
		INT32 a, boards_count = tournament_config.totems_count;
		if (boards_count > 0 && (INT32)io.gs.runtime.cache_temple_0_leaders.size() > boards_count)
		{
			for (a = 1; a <= boards_count; a++)
			{
				if (io.gs.runtime.cache_temple_0_leaders[a].first != io.user_id)
				{
					break;
				}
			}
			if (a > boards_count)
			{
				award_achievement(io, "TOTEM_SWEEPER");
			}
		}
	}

	// Temple Hunter
	if (io.gs.achievements.awarded["TEMPLE_HUNTER"] < 5)
	{
		INT32 boards_count = tournament_config.totems_count;
		if (boards_count > 0 && (INT32)io.gs.runtime.cache_temple_0_leaders.size() > boards_count)
		{
			if (io.gs.runtime.cache_temple_0_leaders[boards_count].first == io.user_id)
			{
				award_achievement(io, "TEMPLE_HUNTER");
			}
		}
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_set_achievements_enabled(SMessageIO & io)
{
	if (!io.gs.achievements.enabled)
	{
		io.gs.achievements.enabled = true;		
	}
	io.response["achievements"] = io.gs["achievements"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_set_client_type(SMessageIO & io)
{
	if (!io.gs.achievements.enabled)
	{
		return;
	}
	string client_type = get_string_from_json(io.jparams, "client_type");
	if (client_type == "ipad" && io.gs.achievements.awarded["MOBILIZED"] == 0)
	{
		award_achievement(io, "MOBILIZED");
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_award_achievement(SMessageIO & io)
{
	string type = get_string_from_json(io.jparams, "type");

	if (!io.gs.achievements.enabled)
	{
		io.response["error"] = json::String("Achievements not enabled yet.");
		return;
	}

	if (type == "PERFECTIONIST")
	{
		INT32 current_level = io.gs.achievements.awarded[type];
		if (io.gs.table.level_pack == "temple_0" && current_level < 5)
		{
			if (io.gs.achievements.counters.last_perfectionist_awarded != io.gs.current_tournament_start_time)
			{
				award_achievement(io, type.c_str());
				io.gs.achievements.counters.last_perfectionist_awarded = io.gs.current_tournament_start_time;
			}
		}
	}
	else if (type == "CHOSEN_ONE")
	{
		if (io.gs.achievements.awarded["CHOSEN_ONE"] == 0)
		{
			award_achievement(io, "CHOSEN_ONE");
		}
	}
	else
	{
		io.response["error"] = json::String("Achievement type not allowed.");
	}

	io.response["achievements"] = io.gs["achievements"];
}
/***************************************************************************/
