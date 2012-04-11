/**
 * @file    SimpleCoupeCar.h
 * @brief     A Car object with specific mesh and texture to create a simple coupe car
 */
#ifndef __TRUCKCAR_H
#define __TRUCKCAR_H

#include "stdafx.h"
#include "SharedIncludes.h"
#include "boost\lexical_cast.hpp"

class TruckCar : public Car
{
public:
    TruckCar(int uniqueCarID, TeamID team);
    virtual ~TruckCar(void);
    void playCarHorn();
    virtual void updateTeam (TeamID team);
    virtual void loadDestroyedModel (void);

    virtual void makeBitsFallOff();

private:
    void initTuning();
    void initNodes();
    void initGraphics(TeamID team);
    void initBody(Ogre::Vector3 carPosition, btTransform& chassisShift);
    void initDoors( btTransform& chassisShift );
    void initWheels();
    
    // Car related Nodes (initNodes())
    Ogre::SceneNode *mChassisNode;
    Ogre::SceneNode *mLDoorNode;
    Ogre::SceneNode *mRDoorNode;
    Ogre::SceneNode *mRBumperNode;
    Ogre::SceneNode *mLWingmirrorNode;
    Ogre::SceneNode *mRWingmirrorNode;

    btRigidBody     *mLDoorBody;
    btRigidBody     *mRDoorBody;
    btRigidBody     *mRBumperBody;
    btRigidBody     *mLWingmirrorBody;
    btRigidBody     *mRWingmirrorBody;

    Ogre::SceneNode *mFLWheelNode;
    Ogre::SceneNode *mFRWheelNode;
    Ogre::SceneNode *mRLWheelNode;
    Ogre::SceneNode *mRRWheelNode;
};

#endif // #ifndef __TRUCKCAR_H
