/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

/***************************************************************************/
void GLogicEngineLastTemple::hsm_payments_add_item(SMessageIO & io)
{
	string item = get_string_from_json(io.jparams, "item");
	INT32 quantity = get_INT32_from_json(io.jparams, "quantity");
	DWORD64 paymentid = get_INT64_from_json(io.jparams, "paymentid");

	if (quantity <= 0 || quantity > 100000)
	{
		io.response["status"] = json::String("{\"status\":\"FAIL\"}");					// Invalid quantity.
		return;
	}
	
	INT32 a, count = io.gs.payments.size();
	for (a = 0; a < count; a++)
	{
		if (paymentid == io.gs.payments[a].paymentid)
		{
			io.response["status"] = json::String("{\"status\":\"OK\"}");				// Duplicate paymentId.
			return;
		}
	}

	bool payment_success = false;
	
	if (item == "gems")
	{
		io.gs.player_info.gems += quantity;
		if (upcalls)
		{
			json::Object response;
			response["gems_added"] = json::Number(quantity);
			response["player_info"] = io.gs["player_info"];
			io.gs.add_client_fields_to_response(io.global_config, io.timestamp, response);
			upcalls->SendMessage(io.user_id, io.gamestate_ID, "gems_purchased", write_json_to_string(response));
		}
		payment_success = true;
	}
	else
	{
		const char * p = item.c_str();
		const char * s = strchr(p, '|');
		if (s != NULL)
		{
			string command, params;
			command.assign(p, s - p);
			if (s[1] == '{')
			{
				params = "{\"paid_action\": 1, ";
				params += (s + 2);
			}
			else
			{
				params.assign(s + 1);
			}
			handle_player_message(io.timestamp, io.gamestate_ID, io.gamestate_ID, command, params);
			payment_success = true;
		}
	}

	if (!payment_success)
	{
		io.response["status"] = json::String("{\"status\":\"FAIL\"}");			// Transaction failed.
		return;
	}

	SGamestate::_payment payment;
	payment.paymentid = paymentid;
	io.gs.payments.push_back(payment);

	io.response["status"] = json::String("{\"status\":\"OK\"}");				// Transaction finished.
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_rewards_add_waka(SMessageIO & io)
{
	INT32 quantity = get_INT32_from_json(io.jparams, "quantity");
	if (quantity <= 0 || quantity > 100000)
	{
		io.response["status"] = json::String("FAIL: Invalid quantity.");
		return;
	}

	io.gs.player_info.waka += quantity;
	if (upcalls)
	{
		json::Object response;
		response["waka_added"] = json::Number(quantity);
		response["player_info"] = io.gs["player_info"];
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "waka_awarded", write_json_to_string(response));
	}

	io.response["status"] = json::String("OK: Waka added.");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_payments_get_config(SMessageIO & io)
{
	io.response["status"] = json::String(io.global_config.payments_items_json);
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_gifts_get_config(SMessageIO & io)
{
	io.response["status"] = json::String(io.global_config.gifts_json);
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_gifts_add_item(SMessageIO & io)
{
	string item = get_string_from_json(io.jparams, "item");
	INT32 quantity = get_INT32_from_json(io.jparams, "quantity");
	INT32 sender_id = get_INT32_from_json(io.jparams, "sender_id");

	if (quantity <= 0 || quantity > 100000)
	{
		io.response["status"] = json::String("FAIL: Invalid quantity.");
		return;
	}

	bool payment_success = false;
	
	if (item == "gems")
	{
		io.gs.player_info.gems += quantity;
		payment_success = true;
	}
	else
	if (item == "waka")
	{
		io.gs.player_info.waka += quantity;
		payment_success = true;
	}
	else
	if (item == "lifes")
	{
		if (io.gs.player_info.extra_lifes < 5 )
		{
			io.gs.player_info.extra_lifes++;
		}		
		payment_success = true;
	}
	else
	if (item == "hire_friend")
	{
		if(sender_id > 0)
		{
			io.gs.player_info.waka += 500;
			if (upcalls)
			{
				json::Object params;
				params["sender_id"] = json::Number(io.user_id);

				upcalls->SendMessageToGamestate((int)sender_id, "add_hired_friend", write_json_to_string(params));
			}
			payment_success = true;
		}
	}
	else if (item == "invite_friend")
	{
		if(sender_id > 0)
		{
			io.gs.player_info.waka += quantity;

			if (upcalls)
			{
				json::Object o = json::Object();
				o["sender_id"] = json::Number(io.user_id);
				

				upcalls->SendMessageToGamestate((int)sender_id, "add_hired_friend", write_json_to_string(o));
			}


			payment_success = true;
		}
	}

	else
	{
		const char * p = item.c_str();
		const char * s = strchr(p, '|');
		if (s != NULL)
		{
			string command, params;
			command.assign(p, s - p);
			if (s[1] == '{')
			{
				params = "{\"paid_action\": 1, ";
				params += (s + 2);
			}
			else
			{
				params.assign(s + 1);
			}
			handle_player_message(io.timestamp, io.gamestate_ID, io.gamestate_ID, command, params);
			payment_success = true;
		}
	}

	if (!payment_success)
	{
		io.response["status"] = json::String("FAIL: Transaction failed.");
		return;
	}
	else
	{
		if (upcalls)
		{
			json::Object response;
			response["item"] = json::String(item);
			response["quantity"] = json::Number(quantity);
			response["player_info"] = io.gs["player_info"];
			io.gs.add_client_fields_to_response(io.global_config, io.timestamp, response);
			upcalls->SendMessage(io.user_id, io.gamestate_ID, "gift_accepted", write_json_to_string(response));
		}
	}

	io.response["status"] = json::String("OK: Gift processed.");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_gifts_new_gift(SMessageIO & io)
{
	INT32 gifts_pending = get_INT32_from_json(io.jparams, "gifts_pending");

	if (upcalls)
	{
		json::Object response;
		response["gifts_pending"] = json::Number(gifts_pending);
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "new_gift_waiting", write_json_to_string(response));
	}

	io.response["status"] = json::String("OK: Notification sent.");
}
/***************************************************************************/
void GLogicEngineLastTemple::hsm_promocode_add_gems(SMessageIO & io)
{
	INT32 quantity = get_INT32_from_json(io.jparams, "quantity");
	if (quantity <= 0 || quantity > 10000000)
	{
		io.response["status"] = json::String("FAIL: Invalid quantity.");
		return;
	}

	io.gs.player_info.gems += quantity;
	if (upcalls)
	{
		json::Object response;
		response["gems_added"] = json::Number(quantity);
		response["player_info"] = io.gs["player_info"];
		io.gs.add_client_fields_to_response(io.global_config, io.timestamp, response);
		upcalls->SendMessage(io.user_id, io.gamestate_ID, "game_state_load", write_json_to_string(response));
	}

	io.response["status"] = json::String("OK: Gems added.");
}
/***************************************************************************/
