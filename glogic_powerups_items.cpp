/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hpm_power_up_slot_changed(SMessageIO & io)
{
	string action = get_string_from_json(io.jparams, "action");
	string type = get_string_from_json(io.jparams, "type");
	bool paid_action = get_INT32_from_json(io.jparams, "paid_action", 0) > 0;

	if (action != "select" && action != "deselect" && action != "unlock" && action != "unlock_premium")
	{
		io.response["error"] = json::String("Invalid action.");
		return;
	}

	if (action == "unlock" || action == "unlock_premium")
	{
		bool only_premium_slot = (action == "unlock_premium");
		INT32 a, price = 0, slot_to_unlock = -1;

		for (a = 0; a < io.gs.power_ups.slots_count; a++)
		{
			if (io.gs.power_ups.slots[a].status != "DISABLED")
			{
				continue;
			}
			if (only_premium_slot && io.global_config.power_up_slot_prices[a].unlock_level != 0)
			{
				continue;
			}
			if (io.global_config.power_up_slot_prices[a].xp_level_req > io.gs.player_info.level)
			{
				continue;
			}
			if (!paid_action && io.global_config.power_up_slot_prices[a].price > io.gs.player_info.gems)
			{
				io.response["error"] = json::String("Not enough gems.");
				return;
			}
			break;
		}
		if (a >= io.gs.power_ups.slots_count)
		{
			io.response["error"] = json::String("No slot to unlock.");
			return;
		}

		io.gs.player_info.gems -= (!paid_action) ? io.global_config.power_up_slot_prices[a].price : 0;
		io.gs.power_ups.slots[a].status = "EMPTY";

		io.response["power_ups"] = io.gs["power_ups"];
		io.response["player_info"] = io.gs["player_info"];
		return;
	}

	INT32 selected_in_slot = -1;
	INT32 free_slot = -1;

	INT32 a;
	for (a = 0; a < io.gs.power_ups.slots_count; a++)
	{
		string slot_content = io.gs.power_ups.slots[a].status;
		if (slot_content == "")
		{
			continue;
		}
		if (selected_in_slot == -1 && slot_content == type)
		{
			selected_in_slot = a;
		}
		if (free_slot == -1 && slot_content == "EMPTY")
		{
			free_slot = a;
		}
	}

	bool is_in_repo = (io.gs.power_ups.enabled.find(type) != io.gs.power_ups.enabled.end());

	if (action == "select")
	{
		if (selected_in_slot != -1)
		{
			io.response["error"] = json::String("Powerup is already selected.");
			return;
		}
		if (io.global_config.powerup_items.find(type) == io.global_config.powerup_items.end())
		{
			io.response["error"] = json::String("Invalid powerup type.");
			return;
		}
		if (!is_in_repo)
		{
			io.response["error"] = json::String("Powerup is not available in repository.");
			return;
		}
		if (free_slot == -1)
		{
			io.response["error"] = json::String("No free slot for powerup.");
			return;
		}

		io.gs.power_ups.slots[free_slot].status = type;
	}
	else
	{
		if (selected_in_slot == -1)
		{
			io.response["error"] = json::String("Powerup is not selected.");
			return;
		}
		io.gs.power_ups.slots[selected_in_slot].status = "EMPTY";
	}

	io.response["power_ups"] = io.gs["power_ups"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_power_up_unlock(SMessageIO & io)
{
	string type = get_string_from_json(io.jparams, "type");
	bool paid_action = get_INT32_from_json(io.jparams, "paid_action", 0) > 0;

	if (io.gs.power_ups.enabled.find(type) != io.gs.power_ups.enabled.end())
	{
		io.response["error"] = json::String("Powerup already unlocked.");
		return;
	}

	TPowerUpItemsMap::iterator pos = io.global_config.powerup_items.find(type);
	if (pos == io.global_config.powerup_items.end())
	{
		io.response["error"] = json::String("Invalid powerup type.");
		return;
	}

	DWORD32 price = pos->second.price;
	if (price == 0)
	{
		price = powerup_gems_cost(io.gs.affiliate_id);// POWERUP_GEMS_COST_ALL;// 100 + pos->second.unlock_level - io.gs.player_info.level;
	}
	if (!paid_action && io.gs.player_info.gems < price)
	{
		io.response["error"] = json::String("Not enough gems.");
		return;
	}

	io.gs.player_info.gems -= (!paid_action) ? price : 0;
	io.gs.power_ups.enabled.insert(type);

	io.response["power_ups"] = io.gs["power_ups"];
	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_buy_item(SMessageIO & io)
{
	string item = get_string_from_json(io.jparams, "item");
	bool paid_action = get_INT32_from_json(io.jparams, "paid_action", 0) > 0;

	TShopItemsMap::iterator pos = io.global_config.shop_items.find(item);
	if (pos == io.global_config.shop_items.end())
	{
		io.response["error"] = json::String("Invalid item.");
		return;
	}

	SShopItem & item_data = pos->second;
	if (item_data.price <= 0)
	{
		io.response["error"] = json::String("Invalid item price.");
		return;
	}
	if (!paid_action && item_data.price > io.gs.player_info.gems)
	{
		io.response["error"] = json::String("Not enough gems.");
		return;
	}

	if (item_data.type == "add_waka")
	{
		io.gs.player_info.gems -= (!paid_action) ? item_data.price : 0;
		io.gs.player_info.waka += item_data.value;
	}

	if (item_data.type == "add_life_slot")
	{
		if (io.gs.player_info.max_lifes >= 8)
		{
			io.response["error"] = json::String("Max. lifes slot already reached.");
			return;
		}

		io.gs.player_info.gems -= (!paid_action) ? item_data.price : 0;
		io.gs.player_info.max_lifes = min(io.gs.player_info.max_lifes + (DWORD32)item_data.value, (DWORD32)8);
		io.gs.player_info.lifes = io.gs.player_info.max_lifes;
		io.gs.last_life_refill_time = io.timestamp;
	}

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
			io.gs.player_info.gems -= (!paid_action) ? item_data.price : 0;
			io.gs.items.push_back(new_item);
		}
		else
		{
			// Item "life_refill" moze miec podane value = 0, tzw. 'instant refill'.
			if (item_data.type == "life_refill")
			{
				io.gs.player_info.gems -= (!paid_action) ? item_data.price : 0;
			}
		}

		if (item_data.type == "life_refill")
		{
			io.gs.player_info.lifes = io.gs.player_info.max_lifes;
			io.gs.last_life_refill_time = io.timestamp;
		}
	}

	io.response["player_info"] = io.gs["player_info"];
	io.response["items"] = io.gs["items"]["array"];
	io.response["item"] = json::String(item);
}
/***************************************************************************/
