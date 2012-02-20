/**
 * @file	SimpleCoupeCar.cpp
 * @brief 	A Car object with specific mesh and texture to create a simple coupe car
 */
#include "stdafx.h"
#include "SharedIncludes.h"

#include "BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h"
#include "BulletDynamics/ConstraintSolver/btHingeConstraint.h"

using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;
using namespace Ogre;

/*  Width:    2490mm (not inc wing mirrors, 2866mm with them)
    Height:   2816mm (inc wheels, cab alone is 2589mm)
    Length:   5725mm
    Wheelbase:    3526mm (this class uses 3575 as its 5cm off)
    Weight:    7396kg
    Engine:    12 litre, 420bhp
    Tyre Diameter: 1046mm
    Tyre Width: 243mm (bit that touches ground, not bounding box)
*/

#define CRITICAL_DAMPING_COEF       0.3f

#define TRUCK_VTX_COUNT 20

static btScalar TruckVtx[] = {
    -124.954f, -56.6976f, 76.8711f,
    -124.954f, 24.5182f, 79.2062f,
    130.475f, 24.5182f, 79.2062f,
    130.475f, -56.6976f, 76.8711f,
    -124.954f, -25.7872f, -478.201f,
    -124.954f, 24.5182f, -478.201f,
    130.475f, 24.5182f, -478.201f,
    130.475f, -25.7872f, -478.201f,
    130.475f, -42.6501f, -133.55f,
    130.475f, 24.5182f, -133.55f,
    -124.954f, 24.5182f, -133.55f,
    -124.954f, -42.6501f, -133.55f,
    114.383f, 197.797f, -133.55f,
    -108.862f, 197.797f, -133.55f,
    -108.862f, 188.796f, 44.8438f,
    114.383f, 188.796f, 44.8438f,
    121.33f, 86.4506f, 79.3882f,
    -115.809f, 86.4506f, 79.3882f,
    -115.809f, 86.4506f, -133.55f,
    121.33f, 86.4506f, -133.55f,
};

/// @brief  Tuning values to create a car which handles well and matches the "type" of car we're trying to create.
void TruckCar::initTuning()
{
    // mTuning related values
    mSteer = 0.0f;
    mEngineForce = 0.0f;
    mBrakingForce = 0.0f;
    
    // mTuning fixed properties
    mSuspensionStiffness    =   50.0f;
    mSuspensionDamping      =   CRITICAL_DAMPING_COEF * 2 * btSqrt(mSuspensionStiffness);
    mSuspensionCompression  =   CRITICAL_DAMPING_COEF * 2 * btSqrt(mSuspensionStiffness) + 0.2;
    mMaxSuspensionForce     =   70000.0f;
    mRollInfluence          =   0.1f;
    mSuspensionRestLength   =   0.25f;
    mMaxSuspensionTravelCm  =   7.5f;
    mFrictionSlip           =   3.0f;
	mChassisLinearDamping   =   0.2f;
	mChassisAngularDamping  =   0.2f;
	mChassisRestitution		=   0.6f;
	mChassisFriction        =   0.6f;
	mChassisMass            =   7000.0f;

    mWheelRadius      =  0.523f; // this is actually diameter!!
    mWheelWidth       =  0.243f;
    mWheelFriction    =  5.0f;//1000;//1e30f;
    mConnectionHeight =  0.6f; // this connection point lies at the very bottom of the suspension travel
    
    mSteerIncrement = 0.015f;
    mSteerToZeroIncrement = 0.05f; // when no input is given steer back to 0
    mSteerClamp = 0.75f;

    mMaxAccelForce = 15000.0f;
    mMaxBrakeForce = 300.0f;

	mFrontWheelDrive = true;
	mRearWheelDrive  = true;
    
    mGearCount = 9;
    mCurrentGear = 1;
    mGearRatio[0] = 14.00f;
    mGearRatio[1] = 12.00f;
    mGearRatio[2] = 10.00f;
    mGearRatio[3] = 07.00f;
    mGearRatio[4] = 05.60f;
    mGearRatio[5] = 04.20f;
    mGearRatio[6] = 02.25f;
    mGearRatio[7] = 01.20f;
    mGearRatio[8] = 00.80f;
    mReverseRatio = 03.00f;
    mFinalDriveRatio = 1.5f;

    mRevTick  = 500;
    mRevLimit = 3500;

    readTuning( "spec_truck.txt" );
}

/// @brief  Constructor to create a car, add its graphical model to ogre and add its physics model to bullet.
/// @param  sceneMgr     The Ogre graphics world.
/// @param  world        The bullet physics world.
/// @param  uniqueCarID  A unique ID for the car so that generated nodes do not have (forbidden) name collisions.
TruckCar::TruckCar(OgreBulletDynamics::DynamicsWorld *world, int uniqueCarID)
{
    mWorld = world;
    mUniqueCarID = uniqueCarID;
    
    Ogre::Vector3 carPosition(16, 13, -15);
    Ogre::Vector3 chassisShift(0, 0.81f, 1.75f);

    initTuning();
    initNodes();
    initGraphics();
    initBody(carPosition, chassisShift);
    initWheels();

    testCar = NULL; /*new SmallCar( sceneMgr, world, GameCore::mPhysicsCore->getUniqueEntityID() );

    btPoint2PointConstraint *constr = new btPoint2PointConstraint( 
        *getVehicle()->getBulletVehicle()->getRigidBody(), 
        *testCar->getVehicle()->getBulletVehicle()->getRigidBody(), 
        btVector3(0,0.2,-2.9), btVector3(0,0.2,1.7) );

    GameCore::mPhysicsCore->mWorld->getBulletDynamicsWorld()->addConstraint( constr, true );*/

    fricConst = new WheelFrictionConstraint( mVehicle, mbtRigidBody );
    fricConst->enableFeedback( true );

    GameCore::mPhysicsCore->mWorld->getBulletDynamicsWorld()->addConstraint( fricConst );

    mHornSound = GameCore::mAudioCore->getSoundInstance(HORN_LOW, mUniqueCarID);
}


/// @brief  Destructor to clean up. Doesn't currently remove the car from the physics world though.
TruckCar::~TruckCar(void)
{
    // Cleanup Bodies:
    delete mVehicle;
    delete mVehicleRayCaster;
    delete mTuning;
    delete mCarChassis;

    // Cleanup Shapes:
    delete compoundChassisShape;
    delete chassisShape;

    GameCore::mAudioCore->deleteSoundInstance(mHornSound);
}


void TruckCar::playCarHorn()
{
    GameCore::mAudioCore->playSoundOrRestart(mHornSound);
}


/// @brief  Initialises the node tree for this car.
void TruckCar::initNodes()
{
    mPlayerNode  = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    
    mBodyNode    = mPlayerNode->createChildSceneNode("BodyNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mWheelsNode  = mPlayerNode->createChildSceneNode("WheelsNode" + boost::lexical_cast<std::string>(mUniqueCarID));

    mChassisNode = mBodyNode->createChildSceneNode("ChassisNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    //mLDoorNode   = mBodyNode->createChildSceneNode("LDoorNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mLDoorNode   = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("LDoorNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    //mRDoorNode   = mBodyNode->createChildSceneNode("RDoorNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mRDoorNode   = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("RDoorNode" + boost::lexical_cast<std::string>(mUniqueCarID));

    mFBumperNode = mBodyNode->createChildSceneNode("FBumperNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mRBumperNode = mBodyNode->createChildSceneNode("RBumperNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mLWingmirrorNode = mBodyNode->createChildSceneNode("LWingmirrorNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mRWingmirrorNode = mBodyNode->createChildSceneNode("RWingmirrorNode" + boost::lexical_cast<std::string>(mUniqueCarID));

    mFLWheelNode = mWheelsNode->createChildSceneNode("FLWheelNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mFRWheelNode = mWheelsNode->createChildSceneNode("FRWheelNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mRLWheelNode = mWheelsNode->createChildSceneNode("RLWheelNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mRRWheelNode = mWheelsNode->createChildSceneNode("RRWheelNode" + boost::lexical_cast<std::string>(mUniqueCarID));
	
	// setup particles. This needs to be propogated.
    mExhaustSystem = GameCore::mSceneMgr->createParticleSystem("Exhaust" + boost::lexical_cast<std::string>(mUniqueCarID), "CollisionDomain/Truck/Exhaust");
	//mDustSystem    = mSceneMgr->createParticleSystem("Dust"    + boost::lexical_cast<std::string>(mUniqueCarID), "CollisionDomain/Dust");
	//mSparkSystem   = mSceneMgr->createParticleSystem("Spark"   + boost::lexical_cast<std::string>(mUniqueCarID), "CollisionDomain/Spark");
	mBodyNode->attachObject(mExhaustSystem);
	//mBodyNode->attachObject(mDustSystem);
	//mBodyNode->attachObject(mSparkSystem);

    // The variables which aren't yet to be used
    mCamArmNode  = NULL;
    mCamNode     = NULL;
}


/// @brief  Loads the car parts' meshes and attaches them to the (already initialised) nodes.
void TruckCar::initGraphics()
{
    // Load the car mesh and attach it to the car node (this will be a large if statement for all models/meshes)
    createGeometry("CarBody", "truck_body.mesh", "truck_body_uv", mChassisNode);
    PhysicsCore::auto_scale_scenenode(mChassisNode);
   // mChassisNode->setPosition(chassisShift); - Doesn't work well with this mesh!!!

    // load the left door baby
    createGeometry("CarEntity_LDoor", "truck_ldoor.mesh", "truck_door_uv", mLDoorNode);
    PhysicsCore::auto_scale_scenenode(mLDoorNode);
    //mLDoorNode->translate(1.0 * 0.019, 20.0 * 0.019, 22.0 * 0.019);
    
    // lets get a tasty right door
    createGeometry("CarEntity_RDoor", "truck_rdoor.mesh", "truck_door_uv", mRDoorNode);
    PhysicsCore::auto_scale_scenenode(mRDoorNode);
    //mRDoorNode->translate(-46.0 * 0.019, 20.0 * 0.019, 22.0 * 0.019);

    // and now a regular rear bumper
    createGeometry("CarEntity_RBumper", "truck_rbumper.mesh", "truck_bumper_uv", mRBumperNode);
    PhysicsCore::auto_scale_scenenode(mRBumperNode);
   // mRBumperNode->translate(0, 20.0 * 0.019, -135.0 * 0.019);

    // Wingmirrors
    createGeometry("CarEntity_LWingmirror", "truck_lwingmirror.mesh", "truck_wingmirror_mirror", mLWingmirrorNode);
    PhysicsCore::auto_scale_scenenode(mLWingmirrorNode);
    createGeometry("CarEntity_RWingmirror", "truck_rwingmirror.mesh", "truck_wingmirror_mirror", mRWingmirrorNode);
    PhysicsCore::auto_scale_scenenode(mRWingmirrorNode);

    
    // tidy front left wheel
    createGeometry("CarEntity_FLWheel", "truck_lwheel.mesh", "truck_wheel_uv", mFLWheelNode);
    PhysicsCore::auto_scale_scenenode(mFLWheelNode);

    // delightful front right wheel
    createGeometry("CarEntity_FRWheel", "truck_rwheel.mesh", "truck_wheel_uv", mFRWheelNode);
    PhysicsCore::auto_scale_scenenode(mFRWheelNode);

    // and now an arousing rear left wheel
    createGeometry("CarEntity_RLWheel", "truck_lwheel.mesh", "truck_wheel_uv", mRLWheelNode);
    PhysicsCore::auto_scale_scenenode(mRLWheelNode);

    // and finally a rear right wheel to seal the deal. beaut.
    createGeometry("CarEntity_RRWheel", "truck_rwheel.mesh", "truck_wheel_uv", mRRWheelNode);
    PhysicsCore::auto_scale_scenenode(mRRWheelNode);
}


/// @brief  Creates a physics car using the nodes (with attached meshes) and adds it to the physics world
void TruckCar::initBody(Ogre::Vector3 carPosition, Ogre::Vector3 chassisShift)
{
    // Load the collision mesh and create a collision shape out of it
    Ogre::Entity* entity = GameCore::mSceneMgr->createEntity("SmallCarCollisionMesh" + boost::lexical_cast<std::string>(mUniqueCarID), "truck_collision.mesh");
    entity->setDebugDisplayEnabled( false );
    compoundChassisShape = new OgreBulletCollisions::CompoundCollisionShape();

     OgreBulletCollisions::ConvexHullCollisionShape *convexHull = new OgreBulletCollisions::ConvexHullCollisionShape( TruckVtx, TRUCK_VTX_COUNT, 3*sizeof(btScalar) );
    convexHull->getBulletShape()->setLocalScaling( btVector3( MESH_SCALING_CONSTANT, MESH_SCALING_CONSTANT, MESH_SCALING_CONSTANT ) );

    // Shift the mesh (this does work in a physical sense, but the wireframe is still drawn in the wrong place)
    compoundChassisShape->addChildShape( convexHull, chassisShift );

    mCarChassis = (OgreBulletDynamics::WheeledRigidBody*) (
        new FuckOgreBulletWheeledRigidBody(
            "CarRigidBody" + boost::lexical_cast<std::string>(mUniqueCarID),
            mWorld,
            COL_CAR,
            COL_CAR | COL_ARENA | COL_POWERUP));
    
    // attach physics shell to mBodyNode
    mCarChassis->setShape(mBodyNode, compoundChassisShape, mChassisRestitution, mChassisFriction, mChassisMass, carPosition, Ogre::Quaternion::IDENTITY);
    mCarChassis->setDamping(mChassisLinearDamping, mChassisAngularDamping);

    mCarChassis->disableDeactivation();
    mTuning = new OgreBulletDynamics::VehicleTuning(
        mSuspensionStiffness, mSuspensionCompression, mSuspensionDamping, mMaxSuspensionTravelCm, mFrictionSlip);

    // OGREBULLET Y U NOT TAKE THIS IN CONSTRUCTOR?!!!?!
    mTuning->getBulletTuning()->m_maxSuspensionForce = mMaxSuspensionForce;

    mVehicleRayCaster = new OgreBulletDynamics::VehicleRayCaster(mWorld);
    
    mVehicle = new OgreBulletDynamics::RaycastVehicle(mCarChassis, mTuning, mVehicleRayCaster);
    
    // This line is needed otherwise the model appears wrongly rotated.
    mVehicle->setCoordinateSystem(0, 1, 2); // rightIndex, upIndex, forwardIndex
    
    mbtRigidBody = mCarChassis->getBulletRigidBody();

    initDoors( chassisShift );

    mCarChassis->showDebugShape( false );
}

void TruckCar::initDoors( Ogre::Vector3 chassisShift )
{
    BoxCollisionShape *doorShape = new BoxCollisionShape( Ogre::Vector3( 0.005f, 0.82f, 0.55f ) );

    CompoundCollisionShape *leftDoor  = new CompoundCollisionShape();
    CompoundCollisionShape *rightDoor = new CompoundCollisionShape();

    leftDoor->addChildShape ( doorShape, Ogre::Vector3(  1.118f, 1.714f , 1.75f ) );
    rightDoor->addChildShape( doorShape, Ogre::Vector3( -1.062f, 1.714f , 1.75f ) );

    mLeftDoorBody = new RigidBody(
        "CarLeftDoor" + boost::lexical_cast<std::string>(mUniqueCarID),
        GameCore::mPhysicsCore->mWorld,
        COL_CAR,
        COL_ARENA );

    mRightDoorBody = new RigidBody(
        "CarRightDoor" + boost::lexical_cast<std::string>(mUniqueCarID),
        GameCore::mPhysicsCore->mWorld,
        COL_CAR,
        COL_ARENA );

    mLeftDoorBody->setShape ( mLDoorNode,  leftDoor, 1.0f, 0.6f, 10.0f, mChassisNode->getPosition() );
    mLeftDoorBody->setDamping( 0.2f, 0.5f );
    mLeftDoorBody->disableDeactivation();

    mRightDoorBody->setShape( mRDoorNode, rightDoor, 0.6f, 0.6f, 5.0f, mChassisNode->getPosition() );
    mRightDoorBody->setDamping( 0.2f, 0.5f );
    mRightDoorBody->disableDeactivation();

    btContactSolverInfo& solverInfo = 
        GameCore::mPhysicsCore->mWorld->getBulletDynamicsWorld()->getSolverInfo();

    solverInfo.m_numIterations = 160;

    leftDoorHinge = new btHingeConstraint( 
        *mbtRigidBody,
        *mLeftDoorBody->getBulletRigidBody(),
        btVector3( 1.118f, 1.714f, 2.315f ),
        btVector3( 1.118f, 1.714f, 2.315f ),
        btVector3( 0.000f, 1.000f, 0.000f ),
        btVector3( 0.000f, 1.000f, 0.000f ) );


    rightDoorHinge = new btHingeConstraint( 
        *mbtRigidBody,
        *mRightDoorBody->getBulletRigidBody(),
        btVector3( -1.062f, 1.714f, 2.315f ),
        btVector3( -1.062f, 1.714f, 2.315f ),
        btVector3( 0.0f, 1.0f, 0.0f ),
        btVector3( 0.0f, 1.0f, 0.0f ) );

    //leftDoorHinge->setParam( BT_CONSTRAINT_STOP_CFM, 0.6f );
    leftDoorHinge->setParam( BT_CONSTRAINT_STOP_ERP, 0.6f );

    leftDoorHinge->setLimit( 0.0f, (Ogre::Math::PI * 0.25f), 0.9f, 0.3f, 1.0f );
    rightDoorHinge->setLimit( -(Ogre::Math::PI * 0.25f), 0.0f, 0.9f, 0.01f, 0.0f );

    leftDoorHinge->enableFeedback( true );
    rightDoorHinge->enableFeedback( true );

    GameCore::mPhysicsCore->mWorld->
        getBulletDynamicsWorld()->addConstraint( leftDoorHinge, true );

    GameCore::mPhysicsCore->mWorld->
        getBulletDynamicsWorld()->addConstraint( rightDoorHinge, true );
}


/// @brief  Attaches 4 wheels to the car chassis.
void TruckCar::initWheels()
{
    float wheelBaseLength = 1.788f;
    float wheelBaseHalfWidth  = 1.05f;

    // anything you add onto wheelbase, adjust this to take care of it
    float wheelBaseShiftZ = -0.575f;

    Ogre::Vector3 wheelDirectionCS0(0,-1,0);
    Ogre::Vector3 wheelAxleCS(-1,0,0);

    bool isFrontWheel = true;
    
    // Wheel 0 - Front Left
    Ogre::Vector3 connectionPointCS0 (wheelBaseHalfWidth, mConnectionHeight, wheelBaseShiftZ + wheelBaseLength );
    mVehicle->addWheel(mFLWheelNode, connectionPointCS0, wheelDirectionCS0, wheelAxleCS, mSuspensionRestLength, mWheelRadius,
        isFrontWheel, mWheelFriction, mRollInfluence);
    
    // Wheel 1 - Front Right
    connectionPointCS0 = Ogre::Vector3(-wheelBaseHalfWidth, mConnectionHeight, wheelBaseShiftZ + wheelBaseLength );
    mVehicle->addWheel(mFRWheelNode, connectionPointCS0, wheelDirectionCS0, wheelAxleCS, mSuspensionRestLength, mWheelRadius,
        isFrontWheel, mWheelFriction, mRollInfluence);
    
    isFrontWheel = false;

    // Wheel 2 - Rear Right
    connectionPointCS0 = Ogre::Vector3(-wheelBaseHalfWidth, mConnectionHeight, wheelBaseShiftZ - wheelBaseLength);
    mVehicle->addWheel(mRRWheelNode, connectionPointCS0, wheelDirectionCS0, wheelAxleCS, mSuspensionRestLength, mWheelRadius,
        isFrontWheel, mWheelFriction, mRollInfluence);

    // Wheel 3 - Rear Left
    connectionPointCS0 = Ogre::Vector3(wheelBaseHalfWidth, mConnectionHeight, wheelBaseShiftZ - wheelBaseLength);
    mVehicle->addWheel(mRLWheelNode, connectionPointCS0, wheelDirectionCS0, wheelAxleCS, mSuspensionRestLength, mWheelRadius,
        isFrontWheel, mWheelFriction, mRollInfluence);
}
