/**
 * @file	Powerup.h
 * @brief 	Powerups, with callbacks from collisions.
 */
#ifndef POWERUP_H
#define POWERUP_H

#include "stdafx.h"
#include "SharedIncludes.h"

/**
 *  @brief 	Headers for powerup
 */
class Powerup
{
public:
    Powerup(Ogre::Vector3 spawnAt, int poolIndex);
	Powerup(PowerupType powerupType, Ogre::Vector3 spawnAt, int poolIndex);
    ~Powerup();

    void playerCollision(Player* player);
    void frameEvent(const float timeSinceLastFrame);
    bool isPendingDelete();

    PowerupType getType();
    int  getIndex();

    Ogre::Vector3 getPosition();
    Ogre::OverlayElement* getBigScreenOverlayElement();

private:
    bool mHasBeenCollected;
    OgreOggISound *mSound;

    PowerupType mPowerupType;
    int mPoolIndex;
    int mUniqueID;
    
    Ogre::Vector3 position;
    Ogre::SceneNode *mNode;
    Ogre::OverlayElement* mBigScreenOverlayElement;
    
    // Used in the server only
    btRigidBody *mRigidBody;
};

#endif // #ifndef POWERUP_H
