/**
 * @file	NetworkCore.h
 * @brief 	Takes notifications and deals with them however networking needs to
 */
#ifndef NETWORKCORE_H
#define NETWORKCORE_H

/*-------------------- INCLUDES --------------------*/

// Things just defined here, need to change later
#define SERVER_PORT 55010
#define SERVER_PASS 0
#define ENCRYPT_DATA 0
#define UPDATE_INTERVAL 20
#define LOG_FILENAME "cdomain.txt"

// Game includes
#include "stdafx.h"
#include "GameIncludes.h"

// RakNet includes
#include "BitStream.h"
#include "GetTime.h"
#include "MessageIdentifiers.h"
#include "RakNetTypes.h"
#include "RakPeerInterface.h"
#include "StringCompressor.h"
#include "WindowsIncludes.h"

// RakNet plugins
#include "RPC4Plugin.h"

// THIS SHOULDN'T BE HERE, FOR LATER, SOMEWHERE MORE GLOBAL (I find it useful)
void log( char *data, ... );

/*-------------------- CLASS DEFINITIONS --------------------*/
/**
 * @brief       A class to take notifications and deal with them however networking needs to
 *
 */

// Define our custom packet ID's
enum
{
    ID_PLAYER_SNAPSHOT = ID_USER_PACKET_ENUM,
    ID_PLAYER_INPUT
};

struct PLAYER_INPUT_DATA
{
    bool frwdPressed;
    bool backPressed;
    bool leftPressed;
    bool rghtPressed;
};

struct PLAYER_SYNC_DATA
{
    RakNet::RakNetGUID playerid;
    RakNet::Time timestamp;
    btVector3 vPosition;
    btQuaternion qRotation;
    btVector3 vAngVel;
    btVector3 vLinVel;
    float fWheelPos;
};

class NetworkCore
{
private:
    static RakNet::RakPeerInterface *m_pRak;
    static RakNet::RPC4 *m_RPC;
    RakNet::RakNetGUID serverGUID;

    static RakNet::TimeMS timeLastUpdate;

public:
    NetworkCore();
    ~NetworkCore (void);

	bool Connect( const char *szHost, int iPort, char *szPass );

    RakNet::RakPeerInterface* getRakInterface();
    void RegisterRPCSlots();

    void frameEvent(InputState *inputSnapshot);
    void ProcessPlayerState( RakNet::Packet *pkt );

    void sendSpawnRequest( CarType iCarType );
    void sendChatMessage( char *szMessage );

    // RPC Calls
    static void GameJoin( RakNet::BitStream *bitStream, RakNet::Packet *pkt );
    static void PlayerJoin( RakNet::BitStream *bitStream, RakNet::Packet *pkt );
    static void PlayerQuit( RakNet::BitStream *bitStream, RakNet::Packet *pkt );
    static void PlayerChat( RakNet::BitStream *bitStream, RakNet::Packet *pkt );
    static void PlayerSpawn( RakNet::BitStream *bitStream, RakNet::Packet *pkt );

    static bool bConnected;

};

#endif // #ifndef NETWORKCORE_H
