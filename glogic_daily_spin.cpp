/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hpm_check_daily_spin(SMessageIO & io)
{
	DWORD32 current_day = 0;

	DWORD64 current_day_start_timestamp = 0;
	DWORD64 first_epoch_day_offset = 6 * 3600;				// Ustalamy poczatek dnia wg strefy UTC-6

	bool waka_is_useful = (io.gs.power_ups.slots.size() > 0) && (io.gs.power_ups.slots[0].status != "DISABLED");

	if (waka_is_useful && io.global_config.daily_bonus_day_duration > 0)
	{
		if (io.gs.daily_bonus.last_awarded_timestamp == 0)
		{
			current_day = 1;
		}
		else
		{
			current_day_start_timestamp = io.timestamp - ((io.timestamp - first_epoch_day_offset) % io.global_config.daily_bonus_day_duration);
			if (io.gs.daily_bonus.last_awarded_timestamp >= current_day_start_timestamp)
			{
				current_day = 0;
			}
			else
			{
				if (io.gs.daily_bonus.last_awarded_timestamp < current_day_start_timestamp - io.global_config.daily_bonus_day_duration)
				{
					current_day = 1;
				}
				else
				{
					current_day = io.gs.daily_bonus.last_awarded_value + 1;
					if (current_day > io.global_config.spin_wheel.daily_awards.size())
					{
						current_day = 1;
					}
				}
			}
		}
	}

	if (current_day > 0)
	{
		if (current_day - 1 < io.global_config.spin_wheel.daily_awards.size())
		{
			SWheelItem & award = io.global_config.spin_wheel.daily_awards[current_day - 1];
			if (award.type == "add_waka" && award.value > 0)
			{
				io.gs.player_info.waka += award.value;
			}
			if (award.type == "add_gems" && award.value > 0)
			{
				io.gs.player_info.gems += award.value;
			}
		}

		if (io.gs.player_info.free_spins_available == 0)
		{
			io.gs.player_info.free_spins_available = 1;
		}
		io.gs.daily_bonus.last_awarded_value = current_day;
		io.gs.daily_bonus.last_awarded_timestamp = io.timestamp;
	}

	io.response["current_day"] = json::Number(current_day);
	//io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
void GLogicEngineLastTemple::hpm_run_spin(SMessageIO & io)
{
	INT32 result_spin_offset = -1;

	if (io.gs.player_info.free_spins_available > 0)
	{
		io.gs.player_info.free_spins_available--;
	}
	else
	{
		if (io.global_config.spin_wheel.spin_cost_gems == 0)
		{
			io.response["error"] = json::String("No more free spins available.");
			return;
		}
		if (io.gs.player_info.gems < io.global_config.spin_wheel.spin_cost_gems)
		{
			io.response["error"] = json::String("Not enough gems.");
			return;
		}

		io.gs.player_info.gems -= io.global_config.spin_wheel.spin_cost_gems;
	}

	double weight_sum = 0.0f;
	INT32 a, count = io.global_config.spin_wheel.spin_awards.size();
	for (a = 0; a < count; a++)
	{
		if (io.global_config.spin_wheel.spin_awards[a].drawing_weight > 0)
		{
			weight_sum += 1.0f / io.global_config.spin_wheel.spin_awards[a].drawing_weight;
		}
	}
	if (weight_sum > 0.0f)
	{
		double result = (double)(rand() % RAND_MAX) / (double)RAND_MAX;

		double basic_probability = 1.0f / weight_sum;
		weight_sum = 0.0f;
		for (a = 0; a < count; a++)
		{
			if (io.global_config.spin_wheel.spin_awards[a].drawing_weight > 0)
			{
				weight_sum += basic_probability / io.global_config.spin_wheel.spin_awards[a].drawing_weight;
				if (weight_sum >= result)
				{
					result_spin_offset = a;
					break;
				}
			}
		}
	}

	if (result_spin_offset >= 0 && result_spin_offset < (INT32)io.global_config.spin_wheel.spin_awards.size())
	{
		SWheelItem & award = io.global_config.spin_wheel.spin_awards[result_spin_offset];
		if (award.type == "add_waka" && award.value > 0)
		{
			io.gs.player_info.waka += award.value;
		}
		if (award.type == "add_gems" && award.value > 0)
		{
			io.gs.player_info.gems += award.value;
		}
	}

	io.response["result_spin_offset"] = json::Number(result_spin_offset);
	io.response["player_info"] = io.gs["player_info"];
}
/***************************************************************************/
