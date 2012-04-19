/**
 * @file	SimpleCoupeCar.h
 * @brief 	A Car object with specific mesh and texture to create a simple coupe car
 */
#ifndef __SimpleCoupeCar_h_
#define __SimpleCoupeCar_h_

#include "stdafx.h"
#include "SharedIncludes.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#include "boost\lexical_cast.hpp"
#else
	#include "boost/lexical_cast.hpp"
#endif

class SimpleCoupeCar : Car
{
public:
    SimpleCoupeCar(int uniqueCarID, TeamID tid);
    virtual ~SimpleCoupeCar(void);
    void updateAudioPitchFrameEvent();
    void playCarHorn();
    void louderLocalSounds();
    virtual void updateTeam (TeamID tid);
    virtual void loadDestroyedModel (void);
    void startEngineSound();

    virtual void makeBitsFallOff();

private:
    void initTuning();
    void initNodes();
    void initGraphics(TeamID tid);
    void initBody(Ogre::Vector3 carPosition, btTransform& chassisShift);
    void initWheels();
    
    // Car related Nodes (initNodes())
    Ogre::SceneNode *mChassisNode;
    Ogre::SceneNode *mFLDoorNode;
    Ogre::SceneNode *mFRDoorNode;
    Ogre::SceneNode *mRLDoorNode;
    Ogre::SceneNode *mRRDoorNode;
    Ogre::SceneNode *mFBumperNode;
    Ogre::SceneNode *mRBumperNode;

    btRigidBody     *mChassisBody;
    btRigidBody     *mFLDoorBody;
    btRigidBody     *mFRDoorBody;
    btRigidBody     *mRLDoorBody;
    btRigidBody     *mRRDoorBody;
    btRigidBody     *mFBumperBody;
    btRigidBody     *mRBumperBody;

    Ogre::SceneNode *mFLWheelNode;
    Ogre::SceneNode *mFRWheelNode;
    Ogre::SceneNode *mRLWheelNode;
    Ogre::SceneNode *mRRWheelNode;
    
    OgreOggISound *mHornSound;
    OgreOggISound *mEngineSound;
};

#endif // #ifndef __SimpleCoupeCar_h_

