#include "stdafx.h"
#include "GameIncludes.h"

PlayerPool::PlayerPool() : mLocalPlayer(0)
{
    // Initialize the pool
    for( int i = 0; i < MAX_PLAYERS; i ++ )
    {
        mPlayers[i] = NULL;
    }
}

int PlayerPool::addPlayer( RakNet::RakNetGUID playerid, char *szNickname )
{
    int i = 0, iNew = -1;
    for( i = 0; i < MAX_PLAYERS; i ++ )
    {
        if( mPlayers[i] == NULL )
        {
            iNew = i;
            break;
        }
    }

    if( iNew != -1 )
    {
        mPlayers[iNew] = new Player();
        mPlayers[iNew]->setPlayerGUID(playerid);
        mPlayers[iNew]->setGUID(playerid);
        mGUID[iNew] = playerid;
        mPlayers[iNew]->setNickname( szNickname );

        return iNew;
    }

    return -1;
}

int PlayerPool::getNumberOfPlayers()
{
    int i = 0, count = 0;
    for( i = 0; i < MAX_PLAYERS; i ++ )
    {
        if( mPlayers[i] != NULL )
        {
            count++;
        }
    }

    return count;
}

void PlayerPool::addLocalPlayer( RakNet::RakNetGUID playerid, char *szNickname )
{
    mLocalPlayer = new Player();
    mLocalPlayer->setPlayerGUID(playerid);
    mLocalPlayer->setNickname(szNickname);
    mLocalGUID = playerid;
    
    mPlayers[0] = mLocalPlayer;
    mGUID[0] = playerid;
}

void PlayerPool::delPlayer( RakNet::RakNetGUID playerid )
{
    int iRemove = getPlayerIndex( playerid );
    if( iRemove != -1 )
    {
        delete mPlayers[iRemove];
        mPlayers[iRemove] = NULL;
    }
}

int PlayerPool::getPlayerIndex( RakNet::RakNetGUID playerid )
{
    int i = 0;
    for (int i = 0; i < MAX_PLAYERS; i ++ )
    {
        if( mGUID[i] == playerid )
            return i;
    }

    return -1;
}

Player* PlayerPool::getLocalPlayer() { return mLocalPlayer; }
RakNet::RakNetGUID PlayerPool::getLocalPlayerID() { return mLocalGUID; }
RakNet::RakNetGUID PlayerPool::getPlayerGUID( int index ) { return mGUID[index]; }

Player* PlayerPool::getPlayer( int index ) { return mPlayers[index]; }

Player* PlayerPool::getPlayer( RakNet::RakNetGUID playerid )
{
    int index = getPlayerIndex( playerid );
    if( index != -1 )
        return mPlayers[index];

    return NULL;
}

void PlayerPool::frameEvent( const Ogre::FrameEvent& evt )
{
    int i = 0;
    Player *pPlayer;

    for( i = 0; i < MAX_PLAYERS; i ++ )
    {
        // Local player physics in GraphicsApplication
        if( mGUID[i] == mLocalGUID )
            continue;

        pPlayer = mPlayers[i];
        if( pPlayer == NULL )
            return;

        if( pPlayer->newInput != NULL )
            pPlayer->processControlsFrameEvent( pPlayer->newInput, evt.timeSinceLastFrame, (1.0f / 60.0f));

#ifdef COLLISION_DOMAIN_CLIENT
        if (pPlayer->getVIP())
            GameCore::mGraphicsCore->updateVIPLocation(pPlayer->getTeam(), pPlayer->getCar()->mBodyNode->getPosition());
#endif
    }
}