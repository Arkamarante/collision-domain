/**
 * @file	Player.h
 * @brief 	Contains the player car and the related data specific to each player.
 */
#ifndef PLAYER_H
#define PLAYER_H

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GameIncludes.h"
#include "CarCam.h"


/*-------------------- CLASS DEFINITIONS --------------------*/
/**
 *  @brief 	Contains the Player nodes and the related data.
 */
class Player
{
public:
    Player (void);
    ~Player (void);
    void createPlayer (CarType carType, CarSkin skin);
    void attachCamera (Ogre::Camera* cam);
    void processControlsFrameEvent (InputState *userInput, Ogre::Real secondsSinceLastFrame, float targetPhysicsFrameRate);
    void updateCameraFrameEvent (int XRotation, int YRotation, int ZDepth);
	float getCameraYaw (void);
    Car* getCar (void);
    void collisionTickCallback (int damage);
    void applyHealthBonus (void);
    
    const char *getNickname (void) { return mNickname; }
    int getCarType (void) { return mCarType; }

    // Probably a better alternative to strdup (could use std::string but I've never been a fan, I like C strings :D )
    void setNickname (char *szNick) { mNickname = strdup( szNick ); }
	int	 getHP (void);
    void setVIP(bool newState) { mIsVIP = newState; };
    bool getVIP(void) { return mIsVIP; };
    void setTeam(int newTeam) { char bob[64]; sprintf(bob, "Client car team set: %d\n", newTeam); OutputDebugString(bob); mCar->updateTeam(newTeam); mTeam = newTeam; };
    int  getTeam(void) { return mTeam; };

	//Just some getters and setters
	void setAlive(bool pAlive);
	bool getAlive();

	//Now have a "Kill" method that will also set the call backs
	void killPlayer();

	CarSnapshot *mSnapshots;

private:
    const float      cameraRotationConstant;
	int              hp;
    int              mTeam;
    char*            mNickname;
    bool             mIsVIP;
    bool             mAlive;
    Car*             mCar;
    CarSnapshot*     mCarSnapshot;
	CarCam*	         mCarCam;
	Ogre::SceneNode* camNode;
	Ogre::SceneNode* camArmNode;
    CarType          mCarType;
};

#endif // #ifndef PLAYER_H