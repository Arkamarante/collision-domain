
#ifndef __Car_h_
#define __Car_h_

#include "stdafx.h"
#include "boost\lexical_cast.hpp"
#include "CarSnapshot.h"

class Car
{
public:
    // = 0 methods not implemented by Car yet!

    // Overrideable methods, but you can use the generic Car method with all cars
    virtual Ogre::SceneNode *attachCamNode();
    virtual void steerInputTick(bool isLeft, bool isRight);
    virtual void accelInputTick(bool isForward, bool isBack);
    virtual void moveTo(const btVector3 &position);
    virtual void restoreSnapshot(CarSnapshot *carSnapshot);
    virtual CarSnapshot *getCarSnapshot();

protected:
    void createGeometry(
        const std::string &entityName,
        const std::string &meshName,
        Ogre::SceneNode *toAttachTo);
    void createGeometry(
        const std::string &entityName,
        const std::string &meshName,
        const std::string &materialName,
        Ogre::SceneNode *toAttachTo);
    
    // Camera node (if not null its attached to bodyNode) (attachCamNode())
    Ogre::SceneNode *mCamArmNode;
    Ogre::SceneNode *mCamNode;

    // Car related Nodes (initNodes())
    Ogre::SceneNode *mPlayerNode;
    
    Ogre::SceneNode *mBodyNode;
    Ogre::SceneNode *mWheelsNode;

    // Data for whole class
    int mUniqueCarID;
    Ogre::SceneManager* mSceneMgr; // Graphics object
    OgreBulletDynamics::DynamicsWorld *mWorld; // Collisions object

    // mTuning related values
    float mSteer;
    float mEngineForce;

    // mTuning fixed properties
    float mSuspensionStiffness;
    float mSuspensionDamping;
    float mSuspensionCompression;
    float mRollInfluence;
    float mSuspensionRestLength;
    float mMaxSuspensionTravelCm;
    float mFrictionSlip;

    float mWheelRadius;
    float mWheelWidth;
    float mWheelFriction;
    float mConnectionHeight;
    
    float mSteerIncrement;
    float mSteerToZeroIncrement;
    float mSteerClamp;

    float mMaxAccelForce;
    float mMaxBrakeForce;

    // Car physics objects
    OgreBulletCollisions::BoxCollisionShape      *chassisShape;
    OgreBulletCollisions::CompoundCollisionShape *compoundChassisShape;
    OgreBulletDynamics::WheeledRigidBody         *mCarChassis;
    OgreBulletDynamics::VehicleTuning            *mTuning;
    OgreBulletDynamics::VehicleRayCaster         *mVehicleRayCaster;
    OgreBulletDynamics::RaycastVehicle           *mVehicle;

private:
    void applySteeringValue();
    void moveTo(const btVector3 &position, const btQuaternion &rotation);
    void createGeometry(
        const std::string &entityName,
        const std::string &meshName,
        bool applyMaterial,
        const std::string &materialName,
        Ogre::SceneNode *toAttachTo);
};

#endif // #ifndef __Car_h_
