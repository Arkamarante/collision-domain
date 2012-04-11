#ifndef PLAYERPOOL_H
#define PLAYERPOOL_H

#define MAX_PLAYERS 100

#include "stdafx.h"
#include "GameIncludes.h"

// RakNet includes
#include "RakNetTypes.h"
#include <vector>
#include <limits>

class Player;

class PlayerPool
{
private:
	//Player* mPlayers[MAX_PLAYERS];
	std::vector<Player*> mPlayers;
	std::vector<RakNet::RakNetGUID> mGUID;
	Player* mLocalPlayer;
	RakNet::RakNetGUID mLocalGUID;
	//RakNet::RakNetGUID mGUID[MAX_PLAYERS];
	int getPlayerIndex( RakNet::RakNetGUID playerid );

public:
	PlayerPool();
	~PlayerPool();
	
	int addPlayer( RakNet::RakNetGUID playerid, char *szNickname );
	void addLocalPlayer( RakNet::RakNetGUID playerid, char *szNickname );
	void delPlayer( RakNet::RakNetGUID playerid );
	int getNumberOfPlayers();
	Player* getRandomPlayer();
	Player* getClosestPlayer(Player* player);
	std::vector<Player*> getPlayers() { return mPlayers;};
	static bool cmp(Player* a, Player* b);
	std::vector<Player*> getScoreOrderedPlayers();

	Player* getPlayer( int index );
	Player* getPlayer( RakNet::RakNetGUID playerid );
	Player* getLocalPlayer();
	RakNet::RakNetGUID getLocalPlayerID();
	RakNet::RakNetGUID getPlayerGUID( int index );


	void frameEvent( const Ogre::FrameEvent& evt );
	//LocalPlayer *getLocalPlayer() { return static_cast<LocalPlayer>(m_pPlayers[iLocalPlayer]); }
};

#endif