/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#pragma warning(disable: 4511)
#pragma warning(disable: 4355)

/***************************************************************************/
struct SShopItem
{
	SShopItem()
	{
		id = "";
		price = 0;
		type = "";
		value = 0;
		time_type = "";
		duration = 0;
	}
	SShopItem(const SShopItem & t)
	{
		operator=(t);
	}

	SShopItem & operator=(const SShopItem & t)
	{
		id = t.id;
		price = t.price;
		type = t.type;
		value = t.value;
		time_type = t.time_type;
		duration = t.duration;
		return *this;
	}

	string				id;
	DWORD32				price;
	string				type;
	INT32				value;
	string				time_type;
	DWORD32				duration;
};
typedef map<string, SShopItem>	TShopItemsMap;
/***************************************************************************/
struct SPowerUpItem
{
	SPowerUpItem()
	{
		type = "";
		cost = 0;
		unlock_level = 0;
		price = 0;
	}
	SPowerUpItem(const SPowerUpItem & t)
	{
		operator=(t);
	}

	SPowerUpItem & operator=(const SPowerUpItem & t)
	{
		type = t.type;
		cost = t.cost;
		unlock_level = t.unlock_level;
		price = t.price;
		return *this;
	}

	string				type;
	DWORD32				cost;
	DWORD32				unlock_level;
	DWORD32				price;
};
typedef map<string, SPowerUpItem>	TPowerUpItemsMap;
/***************************************************************************/
struct SXPLevelConfig
{
	SXPLevelConfig()
	{
		xp_req = 0;
		waka_add = 0;
		gems_add = 0;
		powerup_unlock = "";
		life_refill = false;
		powerup_slot_unlock = 0;
		max_lifes_amount = 0;
	}
	SXPLevelConfig(const SXPLevelConfig & t)
	{
		operator=(t);
	}

	SXPLevelConfig & operator=(const SXPLevelConfig & t)
	{
		xp_req = t.xp_req;
		waka_add = t.waka_add;
		gems_add = t.gems_add;
		life_refill = t.life_refill;
		powerup_unlock = t.powerup_unlock;
		powerup_slot_unlock = t.powerup_slot_unlock;
		max_lifes_amount = t.max_lifes_amount;
		return *this;
	}

	DWORD32				xp_req;
	DWORD32				waka_add;
	DWORD32				gems_add;
	string				powerup_unlock;
	bool				life_refill;
	INT32				powerup_slot_unlock;
	DWORD32				max_lifes_amount;
};
/***************************************************************************/
struct SStagesConfig
{
	SStagesConfig()
	{
		stages_data_paths.clear();
		stages_count = 0;
		time_limits.clear();
	}
	SStagesConfig(const SStagesConfig & t)
	{
		operator=(t);
	}

	SStagesConfig & operator=(const SStagesConfig & t)
	{
		stages_data_paths = t.stages_data_paths;
		stages_count = t.stages_count;
		time_limits = t.time_limits;
		return *this;
	}

	vector<string>		stages_data_paths;
	INT32				stages_count;
	vector<INT32>		time_limits;
};
/***************************************************************************/
struct STournamentConfig
{
	STournamentConfig()
	{
		totems_data_path = "";
		totems_count = 0;
		time_limits.clear();
	}
	STournamentConfig(const STournamentConfig & t)
	{
		operator=(t);
	}

	STournamentConfig & operator=(const STournamentConfig & t)
	{
		totems_data_path = t.totems_data_path;
		totems_count = t.totems_count;
		time_limits = t.time_limits;
		return *this;
	}

	string				totems_data_path;
	INT32				totems_count;
	vector<INT32>		time_limits;
};
typedef map<DWORD32, STournamentConfig>	TTournamentConfigsMap;
/***************************************************************************/
struct STournamentBatch
{
	STournamentBatch()
	{
		first_tournament_start_time = 0;
		tournament_duration = 0;
		tournament_sets_id.clear();
	}

	DWORD64				first_tournament_start_time;							// unixtime
	DWORD64				tournament_duration;
	vector<INT32>		tournament_sets_id;
};
/***************************************************************************/
struct SWheelItem
{
	SWheelItem()
	{
		type = "";
		value = 0;
		drawing_weight = 1;
	}
	SWheelItem(const SWheelItem & t)
	{
		operator=(t);
	}

	SWheelItem & operator=(const SWheelItem & t)
	{
		type = t.type;
		value = t.value;
		drawing_weight = t.drawing_weight;
		return *this;
	}

	string				type;
	INT32				value;
	DWORD32				drawing_weight;				// im wieksza wartosc tym mniejsza szansa na wylosowanie
};
/***************************************************************************/
struct SGlobalConfig
{
	SGlobalConfig()
	{
		Zero();
	}
	void Zero()
	{
		life_refill_duration = 0;
		hired_friend_cost = 10;

		power_up_slot_prices.clear();

		shop_items.clear();
		powerup_items.clear();
		xp_level_config.clear();
		tournament_sets.clear();
		tournament_batches.clear();
		default_tournamentset_id = 0;

		spin_wheel.spin_cost_gems = 0;
		spin_wheel.daily_awards.clear();
		spin_wheel.spin_awards.clear();

		collection_sizes.clear();

		daily_bonus_day_duration = 0;

		payments_items_json = "";
		gifts_json = "";

		client_global_config_json = "";
	}

	bool	read_from_json(const char * json_data, INT32 affiliate_id_override = 0);
	static bool	read_from_json_all_affiliates(const char * json_data, hash_map<INT32, SGlobalConfig> & global_configs);

	STournamentConfig & get_tournament_config(DWORD64 tournament_start_time);
	DWORD64 get_tournament_duration(DWORD64 tournament_start_time);
	INT32	get_tournament_index(DWORD64 tournament_start_time);
	DWORD64	current_global_tournament_start_time(DWORD64 timestamp);

private:

	void	initialize_xp_level_config(json::Object & o);
	void	initialize_shop_items(json::Object & o);
	void	initialize_powerup_items(json::Object & o);
	void	initialize_powerup_slot_prices(json::Object & o);
	void	initialize_tournament_sets(json::Object & o);
	void	initialize_tournament_batches(json::Object & o);
	void	initialize_spin_wheel(json::Object & o);
	void	initialize_adventure_sets(json::Object & o);

	//---------------------------------------------------------------------------

public:
	DWORD32						life_refill_duration;
	DWORD32						hired_friend_cost;

	struct _power_up_slot_price
	{
		DWORD32					price;
		DWORD32					xp_level_req;
		DWORD32					unlock_level;
	};
	vector<_power_up_slot_price>	power_up_slot_prices;

	TShopItemsMap				shop_items;
	TPowerUpItemsMap			powerup_items;
	vector<SXPLevelConfig>		xp_level_config;
	TTournamentConfigsMap		tournament_sets;
	SStagesConfig				adventure_sets;
	vector<STournamentBatch>	tournament_batches;
	DWORD32						default_tournamentset_id;

	struct _spin_wheel
	{
		DWORD32					spin_cost_gems;
		vector<SWheelItem>		daily_awards;
		vector<SWheelItem>		spin_awards;
	} spin_wheel;

	vector<DWORD32>				collection_sizes;

	DWORD64						daily_bonus_day_duration;

	string						payments_items_json;
	string						gifts_json;

	// cache ostatnio wczytanego jsona z global_config w postaci do wyslania do klienta.
	string						client_global_config_json;
};
/***************************************************************************/
