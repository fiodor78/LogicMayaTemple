/***************************************************************************
 ***************************************************************************
  (c) 1999-2006 Ganymede Technologies                 All Rights Reserved
      Krakow, Poland                                  www.ganymede.eu
 ***************************************************************************
 ***************************************************************************/

#pragma warning(disable: 4511)
#pragma warning(disable: 4355)

/***************************************************************************/
struct SGlobalRankingData
{
	SGlobalRankingData()
	{
		Zero();
	}
	void Zero()
	{
		standings.clear();
		pending_standings.clear();
		expiration_time = 0;
		have_scores = false;
	}

	struct _position
	{
		INT32				user_id;
		INT32				score;
		INT32				xp_level;
	};
	vector<_position>		standings;
	vector<_position>		pending_standings;					// Ranking w trakcie kompletowania.

	DWORD64					expiration_time;					// Do kiedy ranking 'standings' traktujemy jako aktualny. Po tym czasie pobieramy go na nowo.
	bool					have_scores;						// Czy dostalismy juz wyniki i czekamy teraz na levele xp.
};
typedef map<string, SGlobalRankingData>		TGlobalRankingResults;				// "tournament_start_time:chamber" -> global rankings
/***************************************************************************/
struct SGlobalRuntime
{
public:
	SGlobalRuntime()
	{
		Zero();
	}
	void Zero()
	{
		global_rankings.clear();
	}

public:
	TGlobalRankingResults				global_rankings;
};
/***************************************************************************/
