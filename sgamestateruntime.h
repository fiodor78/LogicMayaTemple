/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#pragma warning(disable: 4511)
#pragma warning(disable: 4355)

/***************************************************************************/
struct SOnlineAndOfflineFriends
{
	SOnlineAndOfflineFriends()
	{
	}
	SOnlineAndOfflineFriends(const SOnlineAndOfflineFriends & r)
	{
		operator=(r);
	}

	SOnlineAndOfflineFriends & operator=(const SOnlineAndOfflineFriends & r)
	{
		online_instances = r.online_instances;
		online_friends_id = r.online_friends_id;
		offline_friends_id = r.offline_friends_id;
		return *this;
	}

	vector<pair<INT32, INT32> >		online_instances;			// <user_id, gamestate_ID>
	set<INT32>						online_friends_id;			// <user_id>
	set<INT32>						offline_friends_id;			// <user_id>
};
typedef hash_map<INT32, SOnlineAndOfflineFriends>	TOnlineAndOfflineFriendsMap;
/***************************************************************************/
struct SUnansweredMessage
{
	SUnansweredMessage()
	{
		timestamp = 0;
		user_id = 0;
		gamestate_ID = 0;
		message = "";
		params = "";
	}
	SUnansweredMessage(const SUnansweredMessage & r)
	{
		operator=(r);
	}

	SUnansweredMessage & operator=(const SUnansweredMessage & r)
	{
		timestamp = r.timestamp;
		user_id = r.user_id;
		gamestate_ID = r.gamestate_ID;
		message = r.message;
		params = r.params;
		return *this;
	}

	DWORD64				timestamp;
	INT32				user_id;
	INT32				gamestate_ID;
	string				message;
	string				params;
};
typedef multimap<string, SUnansweredMessage>	TUnansweredMessagesMap;
/***************************************************************************/
struct SGamestateRuntime
{
public:
	SGamestateRuntime()
	{
		Zero();
	}
	void Zero()
	{
		recovered_from_json = false;
		unanswered_messages.clear();
		online_and_offline_friends.clear();
		cache_temple_0_leaders.clear();
		cache_xp_level_friends.clear();
		debug_current_tournament_end_time = 0;
		waiting_for_user_position_request = 0;
		waiting_for_leaderboard_request = 0;
	}

	bool	read_from_json(json::Object & o);
	void	write_to_json(json::Object & o);

public:
	bool					recovered_from_json;					// czy stan zostal odtworzony za pomoca read_from_json()
	int						waiting_for_user_position_request;
	int						waiting_for_leaderboard_request;

	TUnansweredMessagesMap	unanswered_messages;					// trigger_name -> SUnansweredMessage

	struct _current_trigger
	{
		string				name;

		struct _leaderboard_data
		{
			string							key;
			INT64							subkey;
			INT32							standings_index;
			vector<pair<INT32, DWORD32> >	highscores;				// <user_id, score>
		} leaderboard_data;
	} current_trigger;

	vector<pair<INT32, DWORD32> >			cache_temple_0_leaders;	// <user_id, score> graczy, ktorzy aktualnie sa liderami w poszczegolnych chamberach
																	// w biezacym turnieju
	map<INT32, DWORD32>						cache_xp_level_friends;	// user_id -> level nasz i przyjaciol

	INT64							debug_current_tournament_end_time;		// zmodyfikowany moment konca biezacego turnieju

	TOnlineAndOfflineFriendsMap		online_and_offline_friends;

	bool					ProcessOnlineFriends(INT32 user_id, const string & message, const json::Object & jparams);
	void					GetOnlineFriends(INT32 user_id, vector<INT32> & result);
};
/***************************************************************************/
