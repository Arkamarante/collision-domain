/**
 * @file	Car.h
 * @brief 	Contains the core methods and variables common to all different types of car.
                Extend or implement this class as required to create a functioning car.
                This class and its subclasses deal with the physics and the graphics which are
                car related and expose an interface to do stuff to the cars.
 */
#ifndef __Car_h_
#define __Car_h_

#include "stdafx.h"
#include "SharedIncludes.h"

class Player;

class Car
{
public:
    // = 0 methods not implemented by Car yet!

    // Overrideable methods, but you can use the generic Car method with all cars
    virtual Ogre::SceneNode *attachCamNode();
    virtual void steerInputTick(
        bool isLeft,
        bool isRight,
        Ogre::Real secondsSinceLastFrame,
        float targetPhysicsFrameRate);
    virtual void accelInputTick(bool isForward, bool isBack);
    virtual void moveTo(const btVector3 &position);
    virtual void restoreSnapshot(CarSnapshot *carSnapshot);
    virtual CarSnapshot *getCarSnapshot();
	virtual float getCarMph();
    void attachCollisionTickCallback(Player* player);
    void shiftDebugShape( const Ogre::Vector3 chassisShift );
	Ogre::SceneNode *mBodyNode;
	// Car related Nodes (initNodes())
    Ogre::SceneNode *mPlayerNode;
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

	// Wheel node
    Ogre::SceneNode *mWheelsNode;

	// Exhaust system
	Ogre::ParticleSystem* mExhaustSystem;

    // Data for whole class
    int mUniqueCarID;
    Ogre::SceneManager* mSceneMgr; // Graphics object
    OgreBulletDynamics::DynamicsWorld *mWorld; // Collisions object

    // mTuning related values
    float mSteer;
    float mEngineForce;
    float mBrakingForce;

    // mTuning fixed properties
    float mSuspensionStiffness;
    float mSuspensionDamping;
    float mSuspensionCompression;
    float mRollInfluence;
    float mSuspensionRestLength;
    float mMaxSuspensionTravelCm;
    float mFrictionSlip;
	float mChassisLinearDamping;
	float mChassisAngularDamping;
	float mChassisRestitution;
	float mChassisFriction;
	float mChassisMass;

    float mWheelRadius;
    float mWheelWidth;
    float mWheelFriction;
    float mConnectionHeight;
    
    float mSteerIncrement;
    float mSteerToZeroIncrement;
    float mSteerClamp;

    float mMaxAccelForce;
    float mMaxBrakeForce;

	bool mFrontWheelDrive;
	bool mRearWheelDrive;

    // Car physics objects
    OgreBulletCollisions::BoxCollisionShape      *chassisShape;
    OgreBulletCollisions::CompoundCollisionShape *compoundChassisShape;
    OgreBulletDynamics::WheeledRigidBody         *mCarChassis;
    OgreBulletDynamics::VehicleTuning            *mTuning;
    OgreBulletDynamics::VehicleRayCaster         *mVehicleRayCaster;
    OgreBulletDynamics::RaycastVehicle           *mVehicle;
    btRigidBody                                  *mbtRigidBody;

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
