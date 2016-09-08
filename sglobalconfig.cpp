/***************************************************************************
***************************************************************************
(c) 1999-2006 Ganymede Technologies                 All Rights Reserved
Krakow, Poland                                  www.ganymede.eu
***************************************************************************
***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
bool SGlobalConfig::read_from_json(const char * json_data, INT32 affiliate_id_override)
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
		if (o.Find("affiliates_override") != o.End())
		{
			if (affiliate_id_override != 0)
			{
				char affiliate_id[32];
				sprintf(affiliate_id, "%d", affiliate_id_override);
				json::Object & o_override = o["affiliates_override"][affiliate_id];

				merge_objects(o, o_override, "affiliates_override");
				o["affiliate_override"] = json::Number(affiliate_id_override);
			}
			o.Erase(o.Find("affiliates_override"));
		}

		life_refill_duration = get_INT32_from_json(o, "life_refill_duration", 300);

		initialize_xp_level_config(o);
		initialize_shop_items(o);
		initialize_powerup_items(o);
		initialize_powerup_slot_prices(o);
		initialize_tournament_sets(o);
		initialize_tournament_batches(o);
		initialize_spin_wheel(o);
		initialize_adventure_sets(o);


		if (o.Find("payments_items") != o.End())
		{
			std::stringstream stream;
			json::Writer::Write(o["payments_items"], stream);
			payments_items_json = stream.str();
			const char * p;
			for (p = "\n\r\t"; *p != 0; p++)
			{
				payments_items_json.erase(std::remove(payments_items_json.begin(), payments_items_json.end(), *p), payments_items_json.end());
			}
		}

		if (o.Find("gifts") != o.End())
		{
			std::stringstream stream;
			json::Writer::Write(o["gifts"], stream);
			gifts_json = stream.str();
			const char * p;
			for (p = " \n\r\t"; *p != 0; p++)
			{
				gifts_json.erase(std::remove(gifts_json.begin(), gifts_json.end(), *p), gifts_json.end());
			}
		}

		daily_bonus_day_duration = get_INT64_from_json(o["internal"], "daily_bonus_day_duration", 0);

		// Przygotowujemy 'client_global_config_json'
		if (o.Find("internal") != o.End())
		{
			json::Object internall =  o["internal"];
			json::Array singleModes = (json::Array)internall["singleMods"];
			json::Array paths = json::Array();
			INT32 path_counts = 0;

			for(INT32 i = 0 ; i < (INT32)singleModes.Size(); i++)
			{
				json::Object mode = singleModes[i];
				json::Array level_sets = (json::Array)mode["level_sets"];

				for (INT32 s = 0 ; s < (INT32)level_sets.Size(); s++)
				{
					json::Object level_set = level_sets[s];
					paths[path_counts++] = level_set["totems_data_path"];
				}
			}
			o.Erase(o.Find("internal"));
			o["singleModes"] = paths;
		}
		if (o.Find("spin_wheel") != o.End())
		{
			json::Object & o1 = o["spin_wheel"];
			if (o1.Find("spin_awards") != o1.End())
			{
				json::Array & a1 = o1["spin_awards"];
				INT32 a, count = a1.Size();
				for (a = 0; a < count; a++)
				{
					json::Object & o2 = a1[a];
					if (o2.Find("drawing_weight") != o2.End())
					{
						o2.Erase(o2.Find("drawing_weight"));
					}
				}
			}
		}

		if (o.Find("collection_sizes") != o.End())
		{
			json::Array::const_iterator pos;
			const json::Array & ar = json::Array(o["collection_sizes"]);
			INT32 idx;
			collection_sizes.resize(ar.Size());
			for (pos = ar.Begin(), idx = 0; pos != ar.End(); pos++)
			{
				collection_sizes[idx++] = (DWORD32)json::Number(*pos);
			}
		}

		if (o.Find("payments_items") != o.End())
		{
			json::Array & ar = o["payments_items"];
			INT32 a, count = ar.Size();
			for (a = 0; a < count; a++)
			{
				try_cleanup_json_content("translations", ar[a]);
			}
		}

		if (o.Find("hired_friend_cost") != o.End())
		{
			hired_friend_cost = get_INT32_from_json(o,"hired_friend_cost", 10);
		}

		o["xp_level_req"] = json::Array();
		json::Array & ar = o["xp_level_req"];
		INT32 a, count = xp_level_config.size();
		ar.Resize(count - 1);
		for (a = 0; a < count - 1; a++)
		{
			ar[a] = json::Number(xp_level_config[a + 1].xp_req);
		}

		{
			std::stringstream stream;
			json::Writer::Write(o, stream);
			client_global_config_json = stream.str();
		}
	}
	catch (json::Exception & )
	{
		Zero();
		result = false;
	}

	time_t t;
	time(&t);
	srand((DWORD32)t ^ 0xc2da5b78 ^ strlen(json_data));

	return result;
}
/***************************************************************************/
bool SGlobalConfig::read_from_json_all_affiliates(const char * json_data, hash_map<INT32, SGlobalConfig> & global_configs)
{
	bool result = true;
	global_configs.clear();

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
		global_configs[0].read_from_json(json_data, 0);

		if (o.Find("affiliates_override") != o.End())
		{
			json::Object & o_affiliates_override = o["affiliates_override"];

			json::Object::const_iterator it;
			for (it = o_affiliates_override.Begin(); it != o_affiliates_override.End(); it++)
			{
				INT32 affiliate_id = ATOI(it->name.c_str());
				if (affiliate_id != 0)
				{
					global_configs[affiliate_id].read_from_json(json_data, affiliate_id);
				}
			}
		}
	}
	catch (json::Exception & )
	{
		global_configs.clear();
		result = false;
	}

	return result;
}
/***************************************************************************/
void SGlobalConfig::initialize_xp_level_config(json::Object & o)
{
	this->xp_level_config.clear();

	INT32 max_level = 0;
	json::Object & xp_level_config = o["internal"]["xp_level_config"];
	json::Object::const_iterator it;
	for (it = xp_level_config.Begin(); it != xp_level_config.End(); it++)
	{
		INT32 level;
		level = ATOI(it->name.c_str());
		if (level >= 1 && level <= 1000)
		{
			if ((int)this->xp_level_config.size() < level + 1)
			{
				this->xp_level_config.resize(level * 2 + 1);
			}
			max_level = max(level, max_level);
			SXPLevelConfig & cfg = this->xp_level_config[level];

			cfg.xp_req = get_INT32_from_json(it->element, "xp_req", 0);
			cfg.waka_add = get_INT32_from_json(it->element, "waka_add", 0);
			cfg.gems_add = get_INT32_from_json(it->element, "gems_add", 0);
			cfg.life_refill = true;
			cfg.powerup_unlock = get_string_from_json(it->element, "powerup_unlock");
			cfg.powerup_slot_unlock = get_INT32_from_json(it->element, "powerup_slot_unlock", 0);
			cfg.max_lifes_amount = get_INT32_from_json(it->element, "max_lifes_amount", 0);
		}
	}
	this->xp_level_config.resize(max_level + 1);
}
/***************************************************************************/
void SGlobalConfig::initialize_shop_items(json::Object & o)
{
	shop_items.clear();

	json::Array & shop_items = o["shop_items"];
	json::Array::const_iterator it;
	for (it = shop_items.Begin(); it != shop_items.End(); it++)
	{
		const json::Array & shop_items_group = json::Array(*it);
		json::Array::const_iterator itg;
		for (itg = shop_items_group.Begin(); itg != shop_items_group.End(); itg++)
		{
			SShopItem item;

			item.id = get_string_from_json(*itg, "id");
			item.price = get_INT32_from_json(*itg, "price", 0);
			item.type = get_string_from_json(*itg, "type");
			item.value = get_INT32_from_json(*itg, "value", 0);
			item.time_type = get_string_from_json(*itg, "time_type");
			item.duration = get_INT32_from_json(*itg, "duration", 0);

			this->shop_items[item.id] = item;
		}
	}
}
/***************************************************************************/
void SGlobalConfig::initialize_powerup_items(json::Object & o)
{
	INT32 affiliate_override = get_INT32_from_json(o, "affiliate_override");

	powerup_items.clear();

	json::Array & powerup_items = o["power_ups"];
	json::Array::const_iterator it;
	for (it = powerup_items.Begin(); it != powerup_items.End(); it++)
	{ 
		SPowerUpItem item;
		item.type = get_string_from_json(*it, "type");
		item.cost = get_INT32_from_json(*it, "cost", 0);
		if (affiliate_override == AFF_ID_WIZQ)
		{
			if(item.type == "TOTAL_CURE")
			{
				item.price = 600;
			}
			else if (item.type == "TIME_EXTEND")
			{
				item.price = 300;
			}
		}
		else
		{
			item.price = get_INT32_from_json(*it, "price", 0);
		}
		this->powerup_items[item.type] = item;
	}

	INT32 a, count = xp_level_config.size();
	for (a = 0; a < count; a++)
	{
		string type = xp_level_config[a].powerup_unlock;
		if (type != "" && this->powerup_items.find(type) != this->powerup_items.end())
		{
			this->powerup_items[type].unlock_level = a;
		}
	}
}
/***************************************************************************/
void SGlobalConfig::initialize_powerup_slot_prices(json::Object & o)
{
	INT32 affiliate_override = get_INT32_from_json(o, "affiliate_override");

	json::Array & powerup_slot_prices = o["internal"]["powerup_slot_prices"];
	INT32 a, count = powerup_slot_prices.Size();
	power_up_slot_prices.resize(count);
	for (a = 0; a < count; a++)
	{
		power_up_slot_prices[a].price = 0;
		power_up_slot_prices[a].xp_level_req = 0;
		if (a < (int)powerup_slot_prices.Size())
		{
			if (affiliate_override == AFF_ID_WIZQ)
			{
				power_up_slot_prices[a].price = a == 1 ? 650 : a == 2 ? 1200 : 0;
			}
			else
			{
				power_up_slot_prices[a].price = get_INT32_from_json(powerup_slot_prices[a], "price", 0);				
			}
			power_up_slot_prices[a].xp_level_req = get_INT32_from_json(powerup_slot_prices[a], "xp_level_req", 0);
		}
	}

	count = xp_level_config.size();
	for (a = 0; a < count; a++)
	{
		INT32 slotid = xp_level_config[a].powerup_slot_unlock;
		if (slotid >= 1 && slotid <= (INT32)power_up_slot_prices.size())
		{
			power_up_slot_prices[slotid - 1].unlock_level = a;
		}
	}
}
/***************************************************************************/
void SGlobalConfig::initialize_spin_wheel(json::Object & o)
{
	INT32 affiliate_override = get_INT32_from_json(o, "affiliate_override");

	if (affiliate_override == AFF_ID_WIZQ)
	{
		spin_wheel.spin_cost_gems = 5;
		o["spin_wheel"]["spin_cost_gems"] = json::Number(5);
	}
	else
	{
		spin_wheel.spin_cost_gems = get_INT32_from_json(o["spin_wheel"], "spin_cost_gems", 0);
	}

	spin_wheel.daily_awards.clear();
	spin_wheel.spin_awards.clear();

	json::Array & daily_awards = o["spin_wheel"]["daily_awards"];
	INT32 a, count = daily_awards.Size();
	spin_wheel.daily_awards.resize(count);
	for (a = 0; a < count; a++)
	{
		SWheelItem & item = spin_wheel.daily_awards[a];
		item.type = get_string_from_json(daily_awards[a], "type");
		item.value = get_INT32_from_json(daily_awards[a], "value", 0);
	}

	json::Array & spin_awards = o["spin_wheel"]["spin_awards"];
	count = spin_awards.Size();
	spin_wheel.spin_awards.resize(count);
	for (a = 0; a < count; a++)
	{
		SWheelItem & item = spin_wheel.spin_awards[a];
		item.type = get_string_from_json(spin_awards[a], "type");
		item.value = get_INT32_from_json(spin_awards[a], "value", 0);
		item.drawing_weight = get_INT32_from_json(spin_awards[a], "drawing_weight", 1);
	}
}
/***************************************************************************/
void SGlobalConfig::initialize_adventure_sets(json::Object & o)
{
	json::Object internall =  o["internal"];
	json::Array singleModes = (json::Array)internall["singleMods"];
	json::Array paths = json::Array();

	INT32 path_counts = 0;


	for(INT32 i = 0 ; i < (INT32)singleModes.Size(); i++)
	{
		json::Object mode = singleModes[i];
		json::Array level_sets = (json::Array)mode["level_sets"];
		INT32 stages_count = get_INT32_from_json(mode, "stages_count");

		this->adventure_sets.stages_count = stages_count;
		this->adventure_sets.time_limits.resize(stages_count);
		this->adventure_sets.stages_data_paths.resize((INT32)level_sets.Size());


		if (stages_count > 0)
		{
			if (get_string_from_json(mode,"name") == "ADVENTURE")
			{				
				INT32 stage_index = 0;
				for (INT32 s = 0 ; s < (INT32)level_sets.Size(); s++)
				{
					json::Object level_set = level_sets[s];
					json::Array time_limits =  (json::Array)level_set["time_limits"];
					this->adventure_sets.stages_data_paths[s] = get_string_from_json(level_set,"totems_data_path");

					for (INT32 l = 0; l < (INT32)time_limits.Size(); l ++)
					{							
						this->adventure_sets.time_limits[i++] = (INT32)json::Number(time_limits[l]);
					}
				}					
			}
		}
	}

}

/***************************************************************************/
void SGlobalConfig::initialize_tournament_sets(json::Object & o)
{
	this->tournament_sets.clear();

	json::Object & tournament_sets = o["internal"]["tournament_sets"];
	json::Object::const_iterator it;
	for (it = tournament_sets.Begin(); it != tournament_sets.End(); it++)
	{
		DWORD32 tournamentsetid;
		tournamentsetid = ATOI(it->name.c_str());

		STournamentConfig cfg;

		cfg.totems_data_path = get_string_from_json(it->element, "totems_data_path");

		json::Array::const_iterator pos;
		const json::Array & ar = json::Array(it->element["time_limits"]);
		cfg.totems_count = ar.Size();
		cfg.time_limits.resize(cfg.totems_count);
		INT32 idx;
		for (pos = ar.Begin(), idx = 0; pos != ar.End(); pos++)
		{
			cfg.time_limits[idx++] = (DWORD32)json::Number(*pos);
		}

		this->tournament_sets[tournamentsetid] = cfg;
		if (default_tournamentset_id == 0)
		{
			default_tournamentset_id = tournamentsetid;
		}
	}

	if (default_tournamentset_id == 0)
	{
		STournamentConfig cfg;
		this->tournament_sets[0] = cfg;
	}
}
/***************************************************************************/
struct tournament_batch_comparator
{
	bool operator()(const STournamentBatch & t1, const STournamentBatch & t2) const
	{
		return t1.first_tournament_start_time > t2.first_tournament_start_time;
	}
};
/***************************************************************************/
static time_t my_mkgmtime(struct tm * tdate)
{
	DWORD64 day_offset = tdate->tm_hour * 3600 + tdate->tm_min * 60 + tdate->tm_sec;
	tdate->tm_hour = tdate->tm_min = tdate->tm_sec = 0;

	time_t timestamp0 = mktime(tdate);
	timestamp0 -= timestamp0 % 86400L;

	INT32 pass;
	for (pass = 0; pass < 24; pass++)
	{
		time_t timestamp_guess = timestamp0 + ((pass + 1) / 2) * 86400 * (pass % 2 == 0 ? 1 : -1);

		struct tm * tdate2 = gmtime(&timestamp_guess);
		if (tdate2->tm_year == tdate->tm_year && tdate2->tm_mon == tdate->tm_mon && tdate2->tm_mday == tdate->tm_mday &&
			tdate2->tm_hour == 0 && tdate2->tm_min == 0 && tdate2->tm_sec == 0)
		{
			return timestamp_guess + day_offset;
		}
	}
	return timestamp0 + day_offset;
}

/***************************************************************************/
void SGlobalConfig::initialize_tournament_batches(json::Object & o)
{
	this->tournament_batches.clear();

	json::Array & tournament_batches = o["internal"]["tournaments"];
	json::Array::const_iterator pos;
	INT32 a, count = tournament_batches.Size();
	this->tournament_batches.resize(count);
	for (pos = tournament_batches.Begin(), a = 0; pos != tournament_batches.End(); pos++, a++)
	{
		DWORD64 tmp_date = get_INT64_from_json(*pos, "first_tournament_start_time");

		struct tm tdate;
		tdate.tm_year = (INT32)(tmp_date / 100000000L - 1900);
		tmp_date %= 100000000L;
		tdate.tm_mon = (INT32)(tmp_date / 1000000L - 1);
		tmp_date %= 1000000L;
		tdate.tm_mday = (INT32)(tmp_date / 10000);
		tmp_date %= 10000;
		tdate.tm_hour = (INT32)(tmp_date / 100);
		tmp_date %= 100;
		tdate.tm_min = (INT32)tmp_date;
		tdate.tm_sec = 0;
		time_t timestamp = my_mkgmtime(&tdate);

		this->tournament_batches[a].first_tournament_start_time = timestamp;
		this->tournament_batches[a].tournament_duration = get_INT64_from_json(*pos, "tournament_duration");

		json::Array & tournament_sets_id = tournament_batches[a]["tournament_sets"];
		json::Array::const_iterator tpos;
		for (tpos = tournament_sets_id.Begin(); tpos != tournament_sets_id.End(); tpos++)
		{
			DWORD32 tournament_set_id = (DWORD32)json::Number(*tpos);
			this->tournament_batches[a].tournament_sets_id.push_back(tournament_set_id);
		}
	}

	sort(this->tournament_batches.begin(), this->tournament_batches.end(), tournament_batch_comparator());
}
/***************************************************************************/
DWORD64 SGlobalConfig::current_global_tournament_start_time(DWORD64 timestamp)
{
	INT32 a = 0, count = tournament_batches.size();
	while (a < count && (tournament_batches[a].first_tournament_start_time > timestamp || tournament_batches[a].tournament_duration == 0))
	{
		a++;
	}
	if (a >= count)
	{
		return 0;
	}

	DWORD64 tournament_seconds_elapsed = (timestamp - tournament_batches[a].first_tournament_start_time) % tournament_batches[a].tournament_duration;
	return timestamp - tournament_seconds_elapsed;
}
/***************************************************************************/
DWORD64 SGlobalConfig::get_tournament_duration(DWORD64 tournament_start_time)
{
	INT32 a = 0, count = tournament_batches.size();
	while (a < count && (tournament_batches[a].first_tournament_start_time > tournament_start_time || tournament_batches[a].tournament_duration == 0))
	{
		a++;
	}
	if (a >= count)
	{
		return 0;
	}

	if ((tournament_start_time - tournament_batches[a].first_tournament_start_time) % tournament_batches[a].tournament_duration == 0)
	{
		if (a > 0 && tournament_start_time + tournament_batches[a].tournament_duration > tournament_batches[a - 1].first_tournament_start_time)
		{
			return tournament_batches[a - 1].first_tournament_start_time - tournament_start_time;
		}
		return tournament_batches[a].tournament_duration;
	}
	return 0;
}
/***************************************************************************/
STournamentConfig & SGlobalConfig::get_tournament_config(DWORD64 tournament_start_time)
{
	INT32 a = 0, count = tournament_batches.size();
	while (a < count && (tournament_batches[a].first_tournament_start_time > tournament_start_time || tournament_batches[a].tournament_duration == 0))
	{
		a++;
	}
	if (a < count)
	{
		if ((tournament_start_time - tournament_batches[a].first_tournament_start_time) % tournament_batches[a].tournament_duration == 0)
		{
			count = tournament_batches[a].tournament_sets_id.size();
			if (count > 0)
			{
				DWORD64 idx = (tournament_start_time - tournament_batches[a].first_tournament_start_time) / tournament_batches[a].tournament_duration;
				DWORD32 tournamentsetid = tournament_batches[a].tournament_sets_id[idx % count];
				if (tournament_sets.find(tournamentsetid) != tournament_sets.end())
				{
					return tournament_sets[tournamentsetid];
				}
			}
		}
	}

	return tournament_sets[default_tournamentset_id];
}
/***************************************************************************/
INT32 SGlobalConfig::get_tournament_index(DWORD64 tournament_start_time)
{
	INT32 a = 0, count = tournament_batches.size();
	while (a < count && (tournament_batches[a].first_tournament_start_time > tournament_start_time || tournament_batches[a].tournament_duration == 0))
	{
		a++;
	}
	if (a >= count)
	{
		return 0;
	}

	if ((tournament_start_time - tournament_batches[a].first_tournament_start_time) % tournament_batches[a].tournament_duration == 0)
	{
		DWORD64 idx = (tournament_start_time - tournament_batches[a].first_tournament_start_time) / tournament_batches[a].tournament_duration;
		return (INT32)idx;
	}
	return 0;
}
/***************************************************************************/
