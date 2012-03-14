
#include "stdafx.h"
#include "GameIncludes.h"

#define BASIC_INTERP 1

PlayerPool::PlayerPool() : mLocalPlayer(0)
{
	// Initialize the pool
	for( int i = 0; i < MAX_PLAYERS; i ++ )
	{
		mPlayers[i] = NULL;
	}
}

void PlayerPool::addPlayer( RakNet::RakNetGUID playerid, char *szNickname )
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
		mGUID[iNew] = playerid;
        mPlayers[iNew]->setNickname( szNickname );
	}
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
	mLocalGUID = playerid;
    mLocalPlayer->setNickname( szNickname );
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
Player* PlayerPool::getPlayer( int index ) { return mPlayers[index]; }
Player* PlayerPool::getPlayer( RakNet::RakNetGUID playerid )
{
    if( playerid == mLocalGUID )
        return mLocalPlayer;

	int index = getPlayerIndex( playerid );
	if( index != -1 )
		return mPlayers[index];

	return NULL;
}

void PlayerPool::frameEvent()
{
	int i = 0;

	for( i = 0; i < MAX_PLAYERS; i ++ )
	{
		processPlayer( mPlayers[i] );
		// TODO: add timestamps to snapshots
	}

    processPlayer( getLocalPlayer() );

/*	// Perform local client update
	if( mLocalPlayer != NULL )
	{
        // Check if we've received an update from the server
		if( mLocalPlayer->getCar() != NULL && mLocalPlayer->mSnapshots != NULL )
		{
#if BASIC_INTERP
            CarSnapshot *currentSnap = mLocalPlayer->getCar()->getCarSnapshot();
            
            btVector3 interpPos = mLocalPlayer->mSnapshots->mPosition.lerp( currentSnap->mPosition, 0.5 );
            btQuaternion interpRot = mLocalPlayer->mSnapshots->mRotation.slerp( currentSnap->mRotation, 0.5 );

            btVector3 interpLin = mLocalPlayer->mSnapshots->mLinearVelocity.lerp( currentSnap->mLinearVelocity, 0.5 );
            btVector3 interpAng = mLocalPlayer->mSnapshots->mAngularVelocity.lerp( currentSnap->mAngularVelocity, 0.5 );

            CarSnapshot *restoreSnap = new CarSnapshot( interpPos, interpRot, interpAng, interpLin, currentSnap->mWheelPosition );

			mLocalPlayer->getCar()->restoreSnapshot( restoreSnap );

            delete( restoreSnap );
            restoreSnap = NULL;

            delete( currentSnap );
            currentSnap = NULL;
#else
            mLocalPlayer->getCar()->restoreSnapshot( mLocalPlayer->mSnapshots );
            delete( mLocalPlayer->mSnapshots );
            mLocalPlayer->mSnapshots = NULL;
#endif

			//delete( mLocalPlayer->mSnapshots );
			//mLocalPlayer->mSnapshots = NULL;
		}
	}*/

}

void PlayerPool::processPlayer( Player *pPlayer )
{
    if( pPlayer == NULL )
		return;

	if( pPlayer->mSnapshots != NULL && pPlayer->getCar() != NULL )
	{
#if BASIC_INTERP

        CarSnapshot *currentSnap = pPlayer->getCar()->getCarSnapshot();
        CarSnapshot *restoreSnap = NULL;

        btScalar dist = currentSnap->mPosition.distance( pPlayer->mSnapshots->mPosition );

        if( dist > 3.00f )
        {
            restoreSnap = (CarSnapshot*)malloc( sizeof( CarSnapshot ) );
            memcpy( restoreSnap, pPlayer->mSnapshots, sizeof( CarSnapshot ) );
        }
        else if( dist > 0.20f )
        {          
            btVector3 interpPos = pPlayer->mSnapshots->mPosition.lerp( currentSnap->mPosition, 0.9f );
            btQuaternion interpRot = pPlayer->mSnapshots->mRotation.slerp( currentSnap->mRotation, 0.9f );

            //btVector3 interpLin = pPlayer->mSnapshots->mLinearVelocity.lerp( currentSnap->mLinearVelocity, 0.9 );
            //btVector3 interpAng = pPlayer->mSnapshots->mAngularVelocity.lerp( currentSnap->mAngularVelocity, 0.9 );
                
            //restoreSnap = new CarSnapshot( interpPos, interpRot, interpAng, interpLin, currentSnap->mWheelPosition );

            restoreSnap = new CarSnapshot( 
                interpPos, 
                interpRot, 
                pPlayer->mSnapshots->mAngularVelocity,
                pPlayer->mSnapshots->mLinearVelocity, 
                pPlayer->mSnapshots->mWheelPosition );
        }

        if( restoreSnap != NULL )
        {
	        pPlayer->getCar()->restoreSnapshot( restoreSnap );

            delete( restoreSnap );
            restoreSnap = NULL;
        }

        delete( currentSnap );
        currentSnap = NULL;

#else

        pPlayer->getCar()->restoreSnapshot( mLocalPlayer->mSnapshots );
#endif
        delete( pPlayer->mSnapshots );
        pPlayer->mSnapshots = NULL;

        
        if (pPlayer->getVIP())
            GameCore::mGraphicsCore->updateVIPLocation(pPlayer->getTeam(), pPlayer->getCar()->mBodyNode->getPosition());
    }
}