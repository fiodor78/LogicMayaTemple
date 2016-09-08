/***************************************************************************
***************************************************************************
(c) 1999-2006 Ganymede Technologies                 All Rights Reserved
Krakow, Poland                                  www.ganymede.eu
***************************************************************************
***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hpm_game_state_load(SMessageIO & io)
{
	io.gs.check_friends_requests(io.timestamp);
	io.gs.remove_expired_items(io.timestamp);
	io.response = io.gs;
	io.response["totems_data_path"] = json::String();
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_lifes_info(SMessageIO & io)
{
	io.response["player_info"] = io.gs["player_info"];
	try_cleanup_json_content("temple_progress", io.response["player_info"]);
	try_cleanup_json_content("single_player_progress", io.response["player_info"]);
	try_cleanup_json_content("tournament_prizes", io.response["player_info"]);
	try_cleanup_json_content("hired_friends", io.response["player_info"]);
	try_cleanup_json_content("invited_friends", io.response["player_info"]);
	try_cleanup_json_content("current_tournament_id", io.response["player_info"]);
	try_cleanup_json_content("gems", io.response["player_info"]);
	try_cleanup_json_content("waka", io.response["player_info"]);
	try_cleanup_json_content("xp", io.response["player_info"]);
	try_cleanup_json_content("level", io.response["player_info"]);
	try_cleanup_json_content("score", io.response["player_info"]);
	try_cleanup_json_content("free_spins_available", io.response["player_info"]);
	try_cleanup_json_content("current_collection", io.response["player_info"]);
	try_cleanup_json_content("current_collection_progress", io.response["player_info"]);
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_get_player_profile(SMessageIO & io)
{
	INT32 user_id = get_INT32_from_json(io.jparams, "user_id");
	if (user_id <= 0)
	{
		io.response["error"] = json::String("Invalid user_id.");
		return;
	}
	// TODO(Marian): Usunac to po tym jak naprawiony zostanie blad z wywalaniem sie "get_player_profile(SELF)".
	// Prawdopodobnie blad wynika z przekazywania string & do handle_gamestate_message().
	if (user_id == io.gamestate_ID)
	{
		io.response["player_info"] = io.gs["player_info"];
		io.response["achievements"] = io.gs["achievements"];
		io.response["items"] = io.gs["items"];
		io.response["power_ups"] = io.gs["power_ups"];
		return;
	}

	if (upcalls)
	{
		json::Object params;
		params["source_user_id"] = json::Number(io.user_id);
		params["source_gamestate_ID"] = json::Number(io.gamestate_ID);
		upcalls->SendMessageToGamestate(user_id, "get_player_profile", write_json_to_string(params));
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hgm_get_player_profile(SMessageIO & io)
{
	INT32 source_user_id = get_INT32_from_json(io.jparams, "source_user_id");
	INT32 source_gamestate_ID = get_INT32_from_json(io.jparams, "source_gamestate_ID");
	if (source_user_id <= 0 || source_gamestate_ID <= 0)
	{
		return;
	}

	if (upcalls)
	{
		json::Object params;
		params["source_gamestate_ID"] = json::Number(io.gamestate_ID);
		params["target_user_id"] = json::Number(source_user_id);
		params["player_info"] = io.gs["player_info"];
		params["achievements"] = io.gs["achievements"];
		params["items"] = io.gs["items"];
		params["power_ups"] = io.gs["power_ups"];

		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, params);

		json::Object & power_ups = params["power_ups"];
		if (power_ups.Find("repo") != power_ups.End())
		{
			power_ups.Erase(power_ups.Find("repo"));
		}

		if (io.gs.current_tournament_start_time != io.global_config.current_global_tournament_start_time(io.timestamp))
		{
			params["player_info"]["score"] = json::Number(0);
		}

		upcalls->SendMessageToGamestate(source_gamestate_ID, "player_profile_data", write_json_to_string(params));
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hgm_player_profile_data(SMessageIO & io)
{
	INT32 source_gamestate_ID = get_INT32_from_json(io.jparams, "source_gamestate_ID");
	INT32 target_user_id = get_INT32_from_json(io.jparams, "target_user_id");
	if (source_gamestate_ID <= 0 || target_user_id <= 0)
	{
		return;
	}

	if (upcalls)
	{
		json::Object params;
		params["user_id"] = json::Number(source_gamestate_ID);
		params["player_info"] = io.jparams["player_info"];
		params["achievements"] = io.jparams["achievements"];
		params["items"] = io.jparams["items"];
		params["power_ups"] = io.jparams["power_ups"];

		upcalls->SendMessage(target_user_id, io.gamestate_ID, "get_player_profile", write_json_to_string(params));
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_game_start(SMessageIO & io)
{
	string level_pack = get_string_from_json(io.jparams, "level_pack", "not_found");

	INT32 a;

	// Sprawdzamy czy wszystkie parametry s¹ OK
	if (level_pack == "not_found")
	{
		io.response["error"] = json::String("Wrong level pack.");
		return;
	}

	if (io.gs.table.game_in_progress)
	{
		io.response["error"] = json::String("Game already in progress.");
		return;
	}

	if (io.gs.player_info.lifes < 1)
	{
		if (io.gs.player_info.extra_lifes == 0)
		{
			io.response["error"] = json::String("Not enough lifes.");
			return;
		}
	}

	STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);

	INT32 chamber = get_INT32_from_json(io.jparams, "chamber");

	if(level_pack == "temple_0")
	{
		if (chamber < 0 || chamber > 9 || chamber >= (INT32)tournament_config.totems_count)
		{
			io.response["error"] = json::String("Invalid chamber.");
			return;
		}

		// Na ostatniej planszy mozna grac, nawet jesli sie jeszcze nie przeszlo poprzednich (dostep do niej kontroluje klient).

		if (chamber < (INT32)tournament_config.totems_count - 1)
		{
			for (a = 0; a < chamber; a++)
			{
				if (io.gs.player_info.temple_progress[a].score == 0)
				{
					io.response["error"] = json::String("Chamber not enabled yet.");
					return;
				}
			}
		}
	}

	io.gs.table.active_powerups.clear();

	DWORD32 waka_cost = 0;
	for (a = 0; a < io.gs.power_ups.slots_count; a++)
	{
		string slot_content = io.gs.power_ups.slots[a].status;
		if (slot_content != "DISABLED" && slot_content != "EMPTY")
		{
			TPowerUpItemsMap::iterator pos;
			pos = io.global_config.powerup_items.find(slot_content);
			if (pos != io.global_config.powerup_items.end())
			{
				INT32 price = pos->second.cost;
				if (price > 0)
				{
					waka_cost += price;
				}
				io.gs.table.active_powerups.push_back(slot_content);
			}
		}
	}
	if (waka_cost > io.gs.player_info.waka)
	{
		io.gs.table.active_powerups.clear();
		io.response["error"] = json::String("Not enough waka. Please change powerups configuration.");
		return;
	}

	// Ustawiamy aktywne itemy.
	io.gs.table.active_items.clear();
	bool have_life_refill = false;
	{
		io.gs.remove_expired_items(io.timestamp);
		INT32 a, count = io.gs.items.size();
		for (a = 0; a < count; a++)
		{
			TShopItemsMap::iterator posa = io.global_config.shop_items.find(io.gs.items[a].id);
			if (posa != io.global_config.shop_items.end())
			{
				if (posa->second.type == "multiply_waka" ||
					posa->second.type == "multiply_xp" ||
					posa->second.type == "life_refill")
				{
					io.gs.table.active_items.push_back(io.gs.items[a].id);
					if (posa->second.type == "life_refill")
					{
						have_life_refill = true;
					}
				}
			}
		}
	}

	if (!have_life_refill)
	{
		if (io.gs.player_info.lifes < 1)
		{
			if (io.gs.player_info.extra_lifes > 0)
			{
				io.gs.player_info.extra_lifes--;
			}
		}
		else
		{
			if (io.gs.player_info.lifes == io.gs.player_info.max_lifes)
			{
				io.gs.last_life_refill_time = io.timestamp;
			}
			io.gs.player_info.lifes--;
		}
	}
	if (waka_cost > 0)
	{
		io.gs.player_info.waka -= waka_cost;
		io.gs.table.waka_locked = waka_cost;
	}

	io.gs.table.level_pack = level_pack;
	io.gs.table.game_in_progress = true;
	io.gs.table.chamber = chamber;
	io.gs.table.game_start_timestamp = io.timestamp;



	io.response["player_info"] = io.gs["player_info"];
	io.response["table"] = io.gs["table"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_game_ended(SMessageIO & io)
{
	string level_pack = io.gs.table.level_pack;

	if (!io.gs.table.game_in_progress)
	{
		io.response["error"] = json::String("Game has not started yet.");
		return;
	}
	string status = get_string_from_json(io.jparams, "status");
	if (status != "WON" && status != "LOST" && status != "QUIT")
	{
		io.response["error"] = json::String("Invalid status.");
		return;
	}

	INT32 time_limit = 0;

	if (io.gs.table.level_pack == "temple_0")
	{
		STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);
		if (io.gs.table.chamber >= 0 && io.gs.table.chamber < (int)tournament_config.totems_count)
		{
			time_limit = tournament_config.time_limits[io.gs.table.chamber];
		}
	}
	else if (io.gs.table.level_pack == "adventure")
	{
		if(io.gs.table.chamber >= 0 && io.gs.table.chamber < io.global_config.adventure_sets.stages_count )
		{
			time_limit = io.global_config.adventure_sets.time_limits[io.gs.table.chamber];
		}
	}

	INT32 basic_score = get_INT32_from_json(io.jparams, "score");
	INT32 icons_busted = get_INT32_from_json(io.jparams, "icons_busted");
	INT32 highest_combo = get_INT32_from_json(io.jparams, "highest_combo");
	INT32 longest_chain = get_INT32_from_json(io.jparams, "longest_chain");
	INT32 time_left = get_INT32_from_json(io.jparams, "time_left");
	if (icons_busted < 0 || icons_busted > 1000000000 ||
		basic_score < 0 || basic_score > 1000000000 ||
		highest_combo < 0 || highest_combo > 1000 ||
		longest_chain < 0 || longest_chain > 1000 ||
		time_left < 0 || time_left > 10000 || (time_limit != 0 && time_left > time_limit))
	{
		io.response["error"] = json::String("Invalid parameters.");
		return;
	}
	if (level_pack == "temple_0")
	{
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
	}

	INT32 score = basic_score + highest_combo * 90 + longest_chain * 150 + time_left * 30;

	DWORD32 xp_add = 0, waka_add = 0;
	calculate_game_bonus(icons_busted, time_limit == 0 ? 100 : ((time_limit - time_left) * 100) / time_limit, xp_add, waka_add);

	io.gs.table.game_in_progress = false;


	if (status == "WON")
	{
		INT32 current_high_score = 0;

		if(io.gs.table.level_pack == "temple_0")
		{
			// Heighscore on current (tournament) chamber
			current_high_score = io.gs.player_info.temple_progress[io.gs.table.chamber].score;

			// Collections		
			STournamentConfig & tournament_config = io.gs.get_current_tournament_config(io.global_config);

			if (current_high_score == 0 && score > 0 && io.gs.table.chamber == tournament_config.totems_count - 1)
			{
				if (io.gs.player_info.current_collection + 1 <= io.global_config.collection_sizes.size())
				{
					if (io.gs.player_info.current_collection_progress < io.global_config.collection_sizes[io.gs.player_info.current_collection])
					{
						io.gs.player_info.last_tournament_completed = io.gs.current_tournament_start_time;
						io.gs.player_info.current_collection_progress++;
						io.response["collection_updated"] = json::Number(1);
					}
				}
			}
		}
		else
		{
			if(io.gs.player_info.single_player_progress.count(io.gs.table.level_pack) == 0 )
			{
				io.gs.player_info.single_player_progress.insert(pair<string, map<INT32,SGamestate::_player_info::_temple_progress> >(io.gs.table.level_pack, map<INT32,SGamestate::_player_info::_temple_progress>()) );
				io.gs.player_info.single_player_progress[io.gs.table.level_pack].insert(pair<INT32,SGamestate::_player_info::_temple_progress>(io.gs.table.chamber,SGamestate::_player_info::_temple_progress()));
			}

			current_high_score = io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].score;
		}


		if (score > current_high_score)
		{
			if(io.gs.table.level_pack == "temple_0")
			{
				io.gs.player_info.temple_progress[io.gs.table.chamber].score = score;
				io.gs.player_info.temple_progress[io.gs.table.chamber].longest_chain = longest_chain;
				io.gs.player_info.temple_progress[io.gs.table.chamber].highest_combo = highest_combo;
				io.gs.player_info.temple_progress[io.gs.table.chamber].time_left = time_left;
				io.gs.player_info.score += (score - current_high_score);

				if (upcalls)
				{
					vector<INT32> temple_scores;
					INT32 c;
					temple_scores.resize(1 + 10);
					temple_scores[0] = 0;
					for (c = 1; c <= 10; c++)
					{
						INT32 chamber_score = io.gs.player_info.temple_progress[c - 1].score;
						temple_scores[c] = chamber_score;
						temple_scores[0] += chamber_score;
					}

					upcalls->UpdateLeaderboard(io.user_id, "temple_0", io.gs.current_tournament_start_time, temple_scores);


				}
			}
			else
			{
				io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].score = score;
				io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].highest_combo = highest_combo;
				io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].longest_chain = longest_chain;
				io.gs.player_info.single_player_progress[io.gs.table.level_pack][io.gs.table.chamber].time_left = time_left;

				if (upcalls)
				{					
					map<INT32, SGamestate::_player_info::_temple_progress> current_progress = io.gs.player_info.single_player_progress[level_pack];
					INT32 total_stars = get_INT32_from_json(io.jparams, "total_stars", -1);

					if	(total_stars == -1)
					{
						io.response["error"] = json::String("Invalid parameters.");
						return;
					}

					int factor = FACTOR;					// Liczba rekordów w bazie
					vector<INT32> temple_scores;
					temple_scores.resize(1 + factor);
					temple_scores[0] = total_stars;

					INT32 current_progress_size = 0;

					for (map<INT32, SGamestate::_player_info::_temple_progress>::iterator it = current_progress.begin(); it != current_progress.end(); it++ )
					{
						if (it->first > current_progress_size)
						{
							current_progress_size = it->first;
						}
					}


					for (int i = 0; i <= current_progress_size; )
					{
						for (INT32 c = 0 ; c < factor; c++)
						{
							INT32 chamber_score = 0;
							if (current_progress.count(c + i) == 1)
							{
								chamber_score = current_progress[c + i].score;
							}

							temple_scores[c + 1] = chamber_score;
						}

						DWORD64 subkey = (DWORD64)(i / factor);

						upcalls->UpdateLeaderboard(io.user_id, io.gs.table.level_pack, (DWORD64)subkey, temple_scores);

						i += factor;
					}
				}
			}

			// Rozsylamy notyfikacje do przyjaciol online o nowym wyniku gracza.
			{
				vector<INT32> online_friends;
				io.gs.runtime.GetOnlineFriends(io.user_id, online_friends);

				json::Object params;
				params["user_id"] = json::Number((double)io.user_id);
				params["score"] = json::Number(io.gs.player_info.score);
				params["tournament_start_time"] = json::Number((double)io.gs.current_tournament_start_time);
				params["level_pack"] = json::String(io.gs.table.level_pack);

				INT32 a, count = online_friends.size();
				for (a = 0; a < count; a++)
				{
					INT32 friend_id = online_friends[a];
					params["target_user_id"] = json::Number((double)friend_id);
					upcalls->SendMessageToGamestate(friend_id, "friend_score_update", write_json_to_string(params));
				}
			}
		}


		vector<string>::iterator it;
		for (it = io.gs.table.active_items.begin(); it != io.gs.table.active_items.end(); it++)
		{
			TShopItemsMap::iterator posa = io.global_config.shop_items.find(*it);
			if (posa != io.global_config.shop_items.end())
			{
				if (posa->second.type == "multiply_xp" && posa->second.value >= 1)
				{
					xp_add *= posa->second.value;
				}
				if (posa->second.type == "multiply_waka" && posa->second.value >= 1)
				{
					waka_add *= posa->second.value;
				}
			}
		}
		io.gs.player_info.xp += xp_add;
		io.gs.player_info.waka += waka_add;

		// Sprawdzamy czy gracz nie robi levelup.
		{
			DWORD32 current_xp = io.gs.player_info.xp;
			DWORD32 level;

			json::Array levelup_prize;
			for (level = io.gs.player_info.level; level + 1 < io.global_config.xp_level_config.size() && io.global_config.xp_level_config[level + 1].xp_req <= current_xp ; level++)
			{
				// Wyplacamy nagrody za osiagniecie danego levelu.
				SXPLevelConfig & cfg = io.global_config.xp_level_config[level + 1];
				json::Object prizes;

				prizes["waka_add"] = json::Number((double)cfg.waka_add);
				if (cfg.waka_add > 0)
				{
					io.gs.player_info.waka += cfg.waka_add;
				}
				prizes["gems_add"] = json::Number((double)cfg.gems_add);
				if (cfg.gems_add > 0)
				{
					io.gs.player_info.gems += cfg.gems_add;
				}

				bool add_powerup = false;
				if (cfg.powerup_unlock != "")
				{
					if (io.gs.power_ups.enabled.find(cfg.powerup_unlock) == io.gs.power_ups.enabled.end())
					{
						io.gs.power_ups.enabled.insert(cfg.powerup_unlock);
						add_powerup = true;
					}
				}
				prizes["power_up_unlocked"] = json::String(add_powerup ? cfg.powerup_unlock : "");

				prizes["power_up_slot_unlocked"] = json::Number(0);
				if (cfg.powerup_slot_unlock >= 1 && cfg.powerup_slot_unlock <= io.gs.power_ups.slots_count)
				{
					if (io.gs.power_ups.slots[cfg.powerup_slot_unlock - 1].status == "DISABLED")
					{
						io.gs.power_ups.slots[cfg.powerup_slot_unlock - 1].status = "EMPTY";
						prizes["power_up_slot_unlocked"] = json::Number((double)cfg.powerup_slot_unlock);
					}
				}

				prizes["max_lifes_amount"] = json::Number((double)(cfg.max_lifes_amount > io.gs.player_info.max_lifes ? cfg.max_lifes_amount : 0));
				if (cfg.max_lifes_amount > io.gs.player_info.max_lifes)
				{
					io.gs.player_info.max_lifes = cfg.max_lifes_amount;
				}

				prizes["life_refill"] = json::Number((double)(cfg.life_refill && io.gs.player_info.lifes < io.gs.player_info.max_lifes ? 1 : 0));
				if (cfg.life_refill && io.gs.player_info.lifes < io.gs.player_info.max_lifes)
				{
					io.gs.player_info.lifes = io.gs.player_info.max_lifes;
					io.gs.last_life_refill_time = io.timestamp;
				}

				levelup_prize.Insert(prizes);
			}
			if (io.gs.player_info.level != level && upcalls)
			{
				vector<INT32> xp_level;
				xp_level.push_back(level);
				upcalls->UpdateLeaderboard(io.user_id, "xp", 0, xp_level);
			}
			io.gs.player_info.level = level;

			if (levelup_prize.Size() > 0)
			{
				io.response["levelup_prize"] = levelup_prize;
				io.response["power_ups"] = io.gs["power_ups"];
			}
		}

		// Zmniejszamy licznik boosterow liczonych per gra.
		{
			INT32 a, count = io.gs.items.size();
			for (a = 0; a < count; a++)
			{
				TShopItemsMap::iterator posa = io.global_config.shop_items.find(io.gs.items[a].id);
				if (posa != io.global_config.shop_items.end())
				{
					if (posa->second.time_type == "GAMES")
					{
						io.gs.items[a].expiration_games--;
					}
				}
			}
		}
	}



	check_after_game_achievements(io);
	if (level_pack == "temple_0")
	{
		if (io.gs.player_info.temple_progress[io.gs.table.chamber].score > io.gs.runtime.cache_temple_0_leaders[io.gs.table.chamber + 1].second)
		{
			io.gs.runtime.cache_temple_0_leaders[io.gs.table.chamber + 1].first = io.user_id;
			io.gs.runtime.cache_temple_0_leaders[io.gs.table.chamber + 1].second =  io.gs.player_info.temple_progress[io.gs.table.chamber].score;
		}
	}

	io.gs.table.active_powerups.clear();
	io.gs.table.active_items.clear();
	io.gs.table.waka_locked = 0;

	io.gs.remove_expired_items(io.timestamp);

	io.response["player_info"] = io.gs["player_info"];
	io.response["items"] = io.gs["items"]["array"];
	io.response["table"] = io.gs["table"];
	io.response["settings"] = io.gs["settings"];
}
/***************************************************************************/
void GLogicEngineLastTemple::calculate_game_bonus(DWORD32 icons_busted, DWORD32 time_spent_percent, DWORD32 & xp_add, DWORD32 & waka_add)
{
	xp_add = 0;
	waka_add = 0;

	if (icons_busted > 0)
	{
		int tiles_dropped_factor = (int)( min(icons_busted, (DWORD32)111) / 10) + 1;
		int time_factor = (int)( (100 - time_spent_percent) / 25);

		xp_add +=  min((DWORD32)tiles_dropped_factor * 15, (DWORD32)180);
		xp_add += min((DWORD32)time_factor * 5, (DWORD32)15);

		waka_add += min((DWORD32)tiles_dropped_factor * 45, (DWORD32)540);
		waka_add += min((DWORD32)time_factor * 15, (DWORD32)45);

		waka_add = (int)(waka_add / 20) * 10;
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_change_settings(SMessageIO & io)
{
	INT32 value;
	string game_mode = get_string_from_json(io.jparams, "game_mode");
	value = get_INT32_from_json(io.jparams, "sound_enabled", -100);
	if (value != -100)
	{
		io.gs.settings.sound_enabled = value;
	}
	value = get_INT32_from_json(io.jparams, "music_enabled", -100);
	if (value != -100)
	{
		io.gs.settings.music_enabled = value;
	}
	value = get_INT32_from_json(io.jparams, "intro_seen", -100);
	if (value != -100)
	{
		io.gs.settings.intro_seen = value;
	}
	value = get_INT32_from_json(io.jparams, "arena_seen", -100);
	if (value != -100)
	{
		io.gs.settings.arena_seen = value;
	}
	value = get_INT32_from_json(io.jparams, "tooltips_enabled", -100);
	if (value != -100)
	{
		io.gs.settings.tooltips_enabled = value;
	}
	value = get_INT32_from_json(io.jparams, "welcome_tournament_seen", -100);
	if (value != -100)
	{
		io.gs.settings.welcome_tournament_seen = value;
	}
	value = get_INT32_from_json(io.jparams, "challenge_info_seen", -100);
	if (value != -100)
	{
		io.gs.settings.challenge_info_seen = value;
	}
	value = get_INT32_from_json(io.jparams, "tutorial_step", -100);
	if (value != -100)
	{
		io.gs.settings.tutorial_step = value;
	}
	if (game_mode != "")
	{
		io.gs.settings.game_mode = game_mode;
	}

	io.response["settings"] = io.gs["settings"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_add_friend_request(SMessageIO & io)
{
	const json::Array & otmp = json::Array(io.jparams["social_ids"]);
	json::Array::const_iterator pos;
	DWORD64 send_time = io.timestamp;

	for (INT32 i = 0; i < (INT32)otmp.Size(); i++)
	{
		string social_id = (string)json::String(otmp[i]);
		if (social_id == "")
		{
			continue;
		}

		if(std::find(io.gs.player_info.invited_friends.begin(), io.gs.player_info.invited_friends.end(), social_id) == io.gs.player_info.invited_friends.end()) 
		{
			SGamestate::_friend_request new_friend_request = SGamestate::_friend_request(social_id, send_time);

			io.gs.table.active_friends_requests.push_back(new_friend_request);
			io.gs.player_info.invited_friends.push_back(social_id);
		}		
	}

	io.gs.check_friends_requests(io.timestamp);
	io.response = io.gs;
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_collect_friend(SMessageIO & io)
{
	string social_id = get_string_from_json(io.jparams, "social_id", "");

	if (social_id == "")
	{
		io.response["error"] = json::String("social_id is empty or null");
		return;
	}

	INT32 a, count = io.gs.table.active_friends_requests.size();
	for (a = 0; a < count; a++)
	{
		if (io.gs.table.active_friends_requests[a].social_id == social_id)
		{
			io.gs.table.active_friends_requests.erase(io.gs.table.active_friends_requests.begin() + a);
			break;
		}
	}

	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_realize_promocode(SMessageIO & io)
{
	const string & promocode = get_string_from_json(io.jparams, "promocode");

	if (promocode == "")
	{
		io.response["error"] = json::String("Invalid promocode");
	}
	else if (io.gs.used_promocodes.count(promocode) > 0)
	{
		io.response["error"] = json::String("Promocode already used");
	}
	else
	{
		io.gs.used_promocodes.insert(promocode);

		if (promocode == "promo-double-xp-mailing")
		{
			add_item_by_id(io, "SHOP_ITEM_MULTI_XP_2");
		}
		else if (promocode == "promo-x3-waka-2013")
		{			
			add_item_by_id(io, "SHOP_ITEM_MULTI_WAKA_1");
		}
		else if (promocode == "valentines-day-2014" && io.timestamp < 1392591600)
		{
			add_item_by_id(io, "SHOP_ITEM_LIFES_24");
		}

		else 
		{
			io.response["error"] = json::String("Promocode not recognized");
		}
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::add_item_by_id(SMessageIO & io, string item_id)
{
	TShopItemsMap::iterator pos = io.global_config.shop_items.find(item_id);

	if (pos == io.global_config.shop_items.end())
	{
		io.response["error"] = json::String("Invalid item.");
		return;
	}
	SShopItem & item_data = pos->second;


	io.gs.remove_expired_items(io.timestamp);

	if (item_data.type == "multiply_waka" ||
		item_data.type == "multiply_xp" ||
		item_data.type == "life_refill")
	{
		bool have_active_item = false;
		INT32 a, count = io.gs.items.size();
		for (a = 0; a < count; a++)
		{
			TShopItemsMap::iterator posa = io.global_config.shop_items.find(io.gs.items[a].id);
			if (posa != io.global_config.shop_items.end())
			{
				if (posa->second.type == item_data.type)
				{
					have_active_item = true;
				}
			}
		}

		if (have_active_item)
		{
			io.response["error"] = json::String("Item of this type is already active.");
			return;
		}

		SGamestate::_item new_item;
		if (item_data.time_type == "GAMES" && item_data.duration > 0)
		{
			new_item.id = item_data.id;
			new_item.expiration_time = 0;
			new_item.expiration_games = item_data.duration;
			new_item.is_permanent = false;
		}
		if (item_data.time_type == "MINUTES" && item_data.duration > 0)
		{
			new_item.id = item_data.id;
			new_item.expiration_time = io.timestamp + (INT64)item_data.duration * 60;
			new_item.expiration_games = 0;
			new_item.is_permanent = false;
		}
		if (item_data.time_type == "HOURS" && item_data.duration > 0)
		{
			new_item.id = item_data.id;
			new_item.expiration_time = io.timestamp + (INT64)item_data.duration * 3600;
			new_item.expiration_games = 0;
			new_item.is_permanent = false;
		}
		if (item_data.time_type == "PERM")
		{
			new_item.id = item_data.id;
			new_item.expiration_time = 0;
			new_item.expiration_games = 0;
			new_item.is_permanent = true;
		}

		if (new_item.id != "")
		{
			io.gs.items.push_back(new_item);
		}

		if (item_data.type == "life_refill")
		{
			io.gs.player_info.lifes = io.gs.player_info.max_lifes;
			io.gs.last_life_refill_time = io.timestamp;
		}
	}
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_buy_time(SMessageIO & io)
{
	INT32 price = get_INT32_from_json(io.jparams, "price", 20);

	if (price <= 0)
	{
		io.response["error"] = json::String("Incorrect price value.");
		return;
	}

	if (io.gs.player_info.gems >= (DWORD32)price)
	{
		io.gs.player_info.gems -= price;
		io.response["player_info"] = io.gs["player_info"];
		io.response["status"] = json::String("Ok");
	}
	else
	{
		io.response["error"] = json::String("Not enough gems.");
	}
}
/***************************************************************************/