/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#include "headers_all.h"
#include "glogic_lasttemple.h"

#pragma warning(disable: 4355)
/***************************************************************************/
bool SGamestateRuntime::ProcessOnlineFriends(INT32 user_id, const string & message, const json::Object & jparams)
{
	if (message == "online_and_offline_friends")
	{
		INT32 a, count;
		SOnlineAndOfflineFriends friends;

		const json::Array & online_friends = jparams["online_friends"];
		json::Array::const_iterator it;
		count = online_friends.Size();
		friends.online_instances.resize(count);
		for (a = 0; a < count; a++)
		{
			INT32 iuser_id = get_INT32_from_json(online_friends[a], "user_id");
			INT32 igamestate_ID = get_INT32_from_json(online_friends[a], "gamestate_ID");
			friends.online_instances[a] = make_pair(iuser_id, igamestate_ID);
			friends.online_friends_id.insert(iuser_id);
		}

		const json::Array & offline_friends = jparams["offline_friends"];
		count = offline_friends.Size();
		for (a = 0; a < count; a++)
		{
			INT32 iuser_id = get_INT32_from_json(offline_friends[a], "user_id");
			friends.offline_friends_id.insert(iuser_id);
		}

		online_and_offline_friends[user_id] = friends;
		return true;
	}

	if (message == "online_friend")
	{
		const json::Object & user_instance = jparams["user_instance"];
		INT32 iuser_id = get_INT32_from_json(user_instance, "user_id");
		INT32 igamestate_ID = get_INT32_from_json(user_instance, "gamestate_ID");

		TOnlineAndOfflineFriendsMap::iterator pos = online_and_offline_friends.find(user_id);
		if (pos == online_and_offline_friends.end())
		{
			return false;
		}
		SOnlineAndOfflineFriends & friends = pos->second;

		INT32 a, count = friends.online_instances.size();
		for (a = 0; a < count; a++)
		{
			if (friends.online_instances[a].first == iuser_id &&
				friends.online_instances[a].second == igamestate_ID)
			{
				return false;
			}
		}

		friends.online_instances.push_back(make_pair(iuser_id, igamestate_ID));
		friends.online_friends_id.insert(iuser_id);
		friends.offline_friends_id.erase(iuser_id);
		return true;
	}

	if (message == "online_friend_remove")
	{
		const json::Object & user_instance = jparams["user_instance"];
		INT32 iuser_id = get_INT32_from_json(user_instance, "user_id");
		INT32 igamestate_ID = get_INT32_from_json(user_instance, "gamestate_ID");

		TOnlineAndOfflineFriendsMap::iterator pos = online_and_offline_friends.find(user_id);
		if (pos == online_and_offline_friends.end())
		{
			return false;
		}
		SOnlineAndOfflineFriends & friends = pos->second;

		INT32 a, count = friends.online_instances.size();
		for (a = 0; a < count; a++)
		{
			if (friends.online_instances[a].first == iuser_id &&
				friends.online_instances[a].second == igamestate_ID)
			{
				friends.online_instances.erase(friends.online_instances.begin() + a);

				count--;
				for (a = 0; a < count; a++)
				{
					if (friends.online_instances[a].first == iuser_id)
					{
						return false;
					}
				}

				friends.online_friends_id.erase(iuser_id);
				friends.offline_friends_id.insert(iuser_id);
				return true;
			}
		}
		return false;
	}

	return false;
}
/***************************************************************************/
void SGamestateRuntime::GetOnlineFriends(INT32 user_id, vector<INT32> & result)
{
	result.clear();

	TOnlineAndOfflineFriendsMap::iterator pos = online_and_offline_friends.find(user_id);
	if (pos == online_and_offline_friends.end())
	{
		return;
	}
	SOnlineAndOfflineFriends & friends = pos->second;

	INT32 a, count;
	count = friends.online_friends_id.size();
	result.resize(count);
	a = 0;
	set<INT32>::iterator it;
	for (it = friends.online_friends_id.begin(); it != friends.online_friends_id.end(); it++)
	{
		result[a++] = *it;
	}
}
/***************************************************************************/
bool SGamestateRuntime::read_from_json(json::Object & o)
{
	Zero();

	json::Object & ofriends = o["online_and_offline_friends"];
	json::Object::iterator pos;
	for (pos = ofriends.Begin(); pos != ofriends.End(); pos++)
	{
		SOnlineAndOfflineFriends friends_data;
		INT32 key = ATOI(pos->name.c_str());

		json::Array & online_instances = ofriends[pos->name]["online_instances"];
		INT32 a, count = online_instances.Size();
		friends_data.online_instances.resize(count);
		for (a = 0; a < count; a++)
		{
			INT32 user_id = get_INT32_from_json(online_instances[a], "user_id");
			INT32 gamestate_ID = get_INT32_from_json(online_instances[a], "gamestate_ID");
			friends_data.online_instances[a] = make_pair(user_id, gamestate_ID);
		}

		json::Array & online_friends_id = ofriends[pos->name]["online_friends_id"];
		count = online_friends_id.Size();
		for (a = 0; a < count; a++)
		{
			friends_data.online_friends_id.insert((INT32)json::Number(online_friends_id[a]));
		}

		json::Array & offline_friends_id = ofriends[pos->name]["offline_friends_id"];
		count = offline_friends_id.Size();
		for (a = 0; a < count; a++)
		{
			friends_data.offline_friends_id.insert((INT32)json::Number(offline_friends_id[a]));
		}

		online_and_offline_friends[key] = friends_data;
	}

	// unanswered_messages
	{
		json::Array & aumessages = o["unanswered_messages"];
		json::Array::iterator pos;
		for (pos = aumessages.Begin(); pos != aumessages.End(); pos++)
		{
			string trigger_name = get_string_from_json(*pos, "trigger_name");
			SUnansweredMessage unanswered_message;
			unanswered_message.timestamp = get_INT64_from_json(*pos, "timestamp");
			unanswered_message.user_id = get_INT32_from_json(*pos, "user_id");
			unanswered_message.gamestate_ID = get_INT32_from_json(*pos, "gamestate_ID");
			unanswered_message.message = get_string_from_json(*pos, "message");
			unanswered_message.params = get_string_from_json(*pos, "params");

			unanswered_messages.insert(make_pair(trigger_name, unanswered_message));
		}
	}

	recovered_from_json = true;
	return true;
}
/***************************************************************************/
void SGamestateRuntime::write_to_json(json::Object & o)
{
	o.Clear();

	json::Object & ofriends = o["online_and_offline_friends"] = json::Object();
	TOnlineAndOfflineFriendsMap::iterator pos;
	for (pos = online_and_offline_friends.begin(); pos != online_and_offline_friends.end(); pos++)
	{
		char key[32];
		sprintf(key, "%d", pos->first);
		json::Object & friends = ofriends[key] = json::Object();

		json::Array & online_instances = friends["online_instances"] = json::Array();
		INT32 a, count = pos->second.online_instances.size();
		online_instances.Resize(count);
		for (a = 0; a < count; a++)
		{
			online_instances[a]["user_id"] = json::Number((double)pos->second.online_instances[a].first);
			online_instances[a]["gamestate_ID"] = json::Number((double)pos->second.online_instances[a].second);
		}

		set<INT32>::iterator it;
		json::Array & online_friends_id = friends["online_friends_id"] = json::Array();
		count = pos->second.online_friends_id.size();
		online_friends_id.Resize(count);
		for (it = pos->second.online_friends_id.begin(), a = 0; it != pos->second.online_friends_id.end(); it++, a++)
		{
			online_friends_id[a] = json::Number((double)*it);
		}

		json::Array & offline_friends_id = friends["offline_friends_id"] = json::Array();
		count = pos->second.offline_friends_id.size();
		offline_friends_id.Resize(count);
		for (it = pos->second.offline_friends_id.begin(), a = 0; it != pos->second.offline_friends_id.end(); it++, a++)
		{
			offline_friends_id[a] = json::Number((double)*it);
		}
	}

	// unanswered_messages
	{
		json::Array & aumessages = o["unanswered_messages"] = json::Array();
		aumessages.Resize(unanswered_messages.size());
		INT32 idx;
		TUnansweredMessagesMap::iterator pos;
		for (pos = unanswered_messages.begin(), idx = 0; pos != unanswered_messages.end(); pos++, idx++)
		{
			aumessages[idx]["trigger_name"] = json::String(pos->first);
			aumessages[idx]["timestamp"] = json::Number((double)pos->second.timestamp);
			aumessages[idx]["user_id"] = json::Number(pos->second.user_id);
			aumessages[idx]["gamestate_ID"] = json::Number(pos->second.gamestate_ID);
			aumessages[idx]["message"] = json::String(pos->second.message);
			aumessages[idx]["params"] = json::String(pos->second.params);
		}
	}
}
/***************************************************************************/
