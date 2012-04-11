/**
 * @file    PowerupHealth.h
 * @brief     Health powerup.
 */
#ifndef POWERUPHEALTH_H
#define POWERUPHEALTH_H

#include "stdafx.h"
#include "SharedIncludes.h"

class Player;
class GameCore;

/**
 *  @brief     Random powerup
 */
class PowerupHealth : public Powerup
{
public:
    PowerupHealth();
    ~PowerupHealth();
    void playerCollision(Player* player);
    void frameEvent( const Ogre::FrameEvent& evt );
    bool isPendingDelete();
    void spawn(Ogre::Vector3 createAboveAt);

private:
    void createGraphic();
    void createCollideable();
    void removeGraphic();
    void removeCollideable();

    Ogre::Entity *entity;
    bool mHasSpawned;
    int mUniqueID;
};

#endif // #ifndef POWERUPHEALTH_H