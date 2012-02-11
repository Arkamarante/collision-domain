/**
 * @file	Car.cpp
 * @brief 	Contains the core methods and variables common to all different types of car.
                Extend or implement this class as required to create a functioning car.
                This class and its subclasses deal with the physics and the graphics which are
                car related and expose an interface to do stuff to the cars.
 */
#include "stdafx.h"
#include "SharedIncludes.h"
#include "boost/algorithm/string.hpp"

#define WHEEL_FRICTION_CFM 0.05

/// @brief  Takes the given CarSnapshot and positions this car as it specifies (velocity etc.).
/// @param  carSnapshot  The CarSnapshot specifying where and how to place the car.
void Car::restoreSnapshot(CarSnapshot *carSnapshot)
{
    moveTo(carSnapshot->mPosition, carSnapshot->mRotation);

    // After this the car will be moved and rotated as specified, but the current velocity
    // will be pointing in the wrong direction (if car is now rotated differently).
    mbtRigidBody->setAngularVelocity(carSnapshot->mAngularVelocity);
    mbtRigidBody->setLinearVelocity(carSnapshot->mLinearVelocity);

    mSteer = carSnapshot->mWheelPosition;
    applySteeringValue();
}


/// @brief  The CarSnapshot specifying the current location of this car which can be restored later if need be.
/// @return The CarSnapshot specifying where and how to place the car at its current location.
CarSnapshot *Car::getCarSnapshot()
{
    return new CarSnapshot(
        btVector3(mBodyNode->getPosition().x, mBodyNode->getPosition().y, mBodyNode->getPosition().z),
        mbtRigidBody->getOrientation(),
        mbtRigidBody->getAngularVelocity(),
        mbtRigidBody->getLinearVelocity(),
        mSteer);
}

float Car::getCarMph()
{
	return mVehicle->getBulletVehicle()->getCurrentSpeedKmHour() * 0.621371192;
}


/// @brief  Moves the car to the specified position keeping current rotation, velocity etc.
/// @param  position  The position to move to.
void Car::moveTo(const btVector3 &position)
{
    moveTo(position, mbtRigidBody->getOrientation());

    // now stop the car moving
    mbtRigidBody->setAngularVelocity(btVector3(0,0,0));
    mbtRigidBody->setLinearVelocity(btVector3(0,0,0));

    mSteer = 0;
    applySteeringValue();
}


/// @brief  This is PRIVATE for a reason. Without giving angular and linear velocity along with a new rotation
///         the rotation won't be applied nicely so the car will still keep moving in the original direction.
/// @param  position  The position to move to.
/// @param  rotation  The rotation to move to.
void Car::moveTo(const btVector3 &position, const btQuaternion &rotation)
{
    btTransform transform(rotation, position);
    mbtRigidBody->proceedToTransform(transform);
    //mbtRigidBody->setWorldTransform(transform);
}


/// @brief  Called once every frame with new user input and updates steering from this.
/// @param  isLeft                  User input specifying if the left control is pressed.
/// @param  isRight                 User input specifying if the right control is pressed.
/// @param  secondsSinceLastFrame   For framerate independence as the wheel turning "accelerate" with keypresses.
/// @param  targetPhysicsFrameRate  The target framerate in seconds anything other than 1/60 will result in an
///         unexpected steering rate. This does not mean the controls aren't framerate independent, its just the
///         fixed frame length in seconds which is taken as "base" for applying normalised steering increments.
void Car::steerInputTick(bool isLeft, bool isRight, Ogre::Real secondsSinceLastFrame, float targetPhysicsFrameRate)
{
    // process steering on both wheels (+1 = left, -1 = right)
    int leftRight = 0;
    if (isLeft)  leftRight += 1;
    if (isRight) leftRight -= 1;

    float calcIncrement = 0.0f;
    bool resetToZero = false;

    // we don't want to go straight to this steering value (i.e. apply acceleration to steer value)
    if (leftRight != 0)
    {
        // Apply steering increment normally
        // Else We are steering against the current wheel direction (i.e. back towards 0). Steer faster.
        if (mSteer * leftRight >= 0) calcIncrement = mSteerIncrement * leftRight;
        else calcIncrement = mSteerToZeroIncrement * leftRight;
    }
    else
    {
        // go back to zero
        if (mSteer >= 0)
        {
            if (mSteer >= mSteerToZeroIncrement * (secondsSinceLastFrame / targetPhysicsFrameRate)) calcIncrement = -mSteerToZeroIncrement;
            else resetToZero = true;
        }
        else
        {
            if (mSteer <= -mSteerToZeroIncrement * (secondsSinceLastFrame / targetPhysicsFrameRate)) calcIncrement = mSteerToZeroIncrement;
            else resetToZero = true;
        }
    }

    if (resetToZero) mSteer = 0;
    else
    {
        // Framerate independent wheel turning acceleration
        calcIncrement *= secondsSinceLastFrame / targetPhysicsFrameRate;
        mSteer += calcIncrement;
    }

    applySteeringValue();
}


/// @brief  Sets the wheel position from the mSteer variable, and clamps mSteer to its bounds.
void Car::applySteeringValue()
{
    // don't steer too far! Use the clamps.
    float steer = mSteer > mSteerClamp ? mSteerClamp : (mSteer < -mSteerClamp ? -mSteerClamp : mSteer);

    mVehicle->setSteeringValue(steer, 0);
    mVehicle->setSteeringValue(steer, 1);

    mSteer = steer;
}


/// @brief  Called once every frame with new user input and updates forward/back engine forces from this.
/// @param  isForward  User input specifying if the forward control is pressed.
/// @param  isBack     User input specifying if the back control is pressed.
void Car::accelInputTick(bool isForward, bool isBack, bool isHand)
{
    int forwardBack = 0;
    if (isForward) forwardBack += 1;
    if (isBack)    forwardBack -= 1;

    //float f = forwardBack < 0 ? mMaxBrakeForce : mMaxAccelForce;
    //mEngineForce = f * forwardBack;

    mEngineForce  = ( isForward ) ? mMaxAccelForce : 0;
    mBrakingForce = ( isBack    ) ? mMaxBrakeForce : 0;

    int doBrake = 0;

    if( isHand )
    {
        mVehicle->applyEngineForce( 0, 0 );
        mVehicle->applyEngineForce( 0, 1 );
        mVehicle->applyEngineForce( 0, 2 );
        mVehicle->applyEngineForce( 0, 3 );

        mVehicle->getBulletVehicle()->setBrake( mMaxAccelForce * 2, 2 );
        mVehicle->getBulletVehicle()->setBrake( mMaxAccelForce * 2, 3 );

    }
    else
    {
        // Reset brakes to 0
        for( int i = 0; i < 4; i ++ )
            mVehicle->getBulletVehicle()->setBrake( 0 , i );
    }

    // Loop through each wheel
    for( int i = 0; i < 4; i ++ )
    {
        // Skip wheels depending on car driving mode
        if( i < 2 && !mFrontWheelDrive ) continue;
        if( i > 1 && !mRearWheelDrive  ) continue;

        if( isHand ) continue;

        // This code is a bit of a mess to avoid really tight brake / reverse checks
        // on exact float values but it works!

        float fSpeed = this->mVehicle->getBulletVehicle()->getCurrentSpeedKmHour();
        if( fSpeed < 2 )                                                                // Brake / Reverse threshold between 0 and 1 kph
        {
            if( isBack )
            {
                mVehicle->applyEngineForce( mMaxAccelForce * -0.6, i );                 // Press brake - assume we want to reverse
                mCurrentGear = -1;
            }
            else
            {
                mVehicle->applyEngineForce( 0, i );                                     // Turn off assumed reverse
                if( mCurrentGear == -1 )
                    mCurrentGear = 1;
            }

            if( isForward )
            {
                if( fSpeed >= -2 )
                {
                    mVehicle->applyEngineForce( mEngineForce, i );                      // Press accel & moving forwards - accelerate
                    doBrake = 1;
                    //mVehicle->getBulletVehicle()->setBrake( mBrakingForce, i );       // and apply the brake if had been pressed
                }
                else
                {
                    mVehicle->applyEngineForce( 0, i );                                 // Press accell & moving backwards - turn off accel
                    //mVehicle->getBulletVehicle()->setBrake( mMaxBrakeForce, i );      // and apply the brake if had been pressed
                    doBrake = 2;
                }
            }
            else
            {
                mVehicle->getBulletVehicle()->setBrake( 0, i );                         // Moving backwards and not pressing accel - turn off brake
            }

        }
        else                                                                            // Speed above threshold - driving forwards
        {
            if( isBack )
                mVehicle->applyEngineForce( 0, i );                                     // Turn off accel if you're pressing brake temporarily
            else
                mVehicle->applyEngineForce( mEngineForce, i );                          // otherwise normal force (simulate accel & brake together)

            //mVehicle->getBulletVehicle()->setBrake( mBrakingForce, i );               // Set brake on if we're pressing it
            doBrake = 1;
        }
    }

    if( doBrake > 0 && !isHand )
    {
        for( int i = 0; i < 4; i ++ )
            mVehicle->getBulletVehicle()->setBrake( doBrake == 1 ? mBrakingForce : mMaxBrakeForce , i );
    }
    else if( !isHand )
    {
        // Reset brakes to 0
        for( int i = 0; i < 4; i ++ )
            mVehicle->getBulletVehicle()->setBrake( 0 , i );
    }

	// update exhaust. whee this is the wrong place to do this.
	float speedmph = getCarMph();
	float emissionRate = 0;
	if (isForward)
	{
		if (speedmph < 50.0f)
			emissionRate = (50 - speedmph) * 15;
	}
	for (int i = 0; i < mExhaustSystem->getNumEmitters(); i++)
		mExhaustSystem->getEmitter(i)->setEmissionRate(emissionRate);
	
#ifdef COLLISION_DOMAIN_CLIENT
	float blurAmount = 0;
	if (speedmph > 40.0f)
	{
		// calculate blurring as a function of speed, then scale it back depending on where you
		// are looking at the car from (effect strongest from behind and infront (3 maxima at 
		// +/-180 and 0, hence the double abs() reduction)).
		blurAmount = (speedmph - 40) / 30;
		blurAmount *= abs((abs(GameCore::mPlayerPool->getLocalPlayer()->getCameraYaw()) - 90)) / 90;
	}
	GameCore::mGraphicsApplication->setRadialBlur(blurAmount);
#endif

    updateRPM();
}

/*
mph = (rpm * cir) / (gear * final * 88)
where rpm = engine rpm
cir = tire cicumference, in feet
gear = gear ratio of your car
final = final drive ratio of your car
88 = combines several conversion factors
*/

void Car::updateRPM()
{
    // Check if we're in neutral (revving at start of race)
    if( mCurrentGear == 0 )
    {
        // Linearly increase engine RPM
        mEngineRPM += mMaxAccelForce / mRevLimit;

        // Fluctuate around the rev limiter value
        if( mEngineRPM > mRevLimit )
            mEngineRPM -= 500;
    }
    else if( mCurrentGear == -1 )
    {
        // Calculate MPH -> RPM for current gear
        float fRPMCir = 
            getCarMph() * ( mReverseRatio * mFinalDriveRatio * 88 );

        // Previous calcs give RPM * WheelCircumference in feet, so get RPM
        mEngineRPM = fRPMCir / (Ogre::Math::PI * (2 * mWheelRadius*3.2808399));

        if( mEngineRPM < mRevTick  ) mEngineRPM = ( mRevTick  - 200 ) + ( rand() % 400 );
        if( mEngineRPM > mRevLimit ) mEngineRPM = ( mRevLimit - 200 ) + ( rand() % 400 );
    }
    else
    {
        // Calculate MPH -> RPM for current gear
        float fRPMCir = 
            getCarMph() * ( mGearRatio[mCurrentGear-1] * mFinalDriveRatio * 88 );

        // Calculate MPH -> RPM for current gear (for shift-down check)
        float fPrevGear = mCurrentGear <= 1 ? fRPMCir :
            getCarMph() * ( mGearRatio[mCurrentGear-2] * mFinalDriveRatio * 88 );

        // Previous calcs give RPM * WheelCircumference in feet, so get RPM
        mEngineRPM = fRPMCir / (Ogre::Math::PI * (2 * mWheelRadius*3.2808399));
        fPrevGear /= (Ogre::Math::PI * (2 * mWheelRadius*3.2808399));

        // Check for shift-up
        if( mEngineRPM > mRevLimit -1500 )
        {
            mEngineRPM = ( mRevLimit - 200 ) + ( rand() % 400 );
            if( mCurrentGear < mGearCount )
                mCurrentGear ++;
        }
        
        // Check for shift-down
        if( fPrevGear < mRevLimit -1500 && mCurrentGear > 1 )
        {
            mCurrentGear --;
        }
    }

    // Fluctuate about the engin rest level if RPM is lower
    if( mEngineRPM < mRevTick )
        mEngineRPM = ( mRevTick - 200 ) + ( rand() % 400 );

#ifdef DEBUG_SHOW_REVS
    CEGUI::Window *fps = CEGUI::WindowManager::getSingleton().getWindow( "root_wnd/fps" );
    char szFPS[64];
    sprintf( szFPS,   "RPM: %f    Gear: %i", mEngineRPM, mCurrentGear );
    fps->setText( szFPS );
#endif

}


/// @brief  If a node isnt already attached, attaches a new one, otherwise returns the current one
/// @return The node onto which a camera can be attached to observe the car. The parent of this node is
///         guaranteed to be the arm node.
Ogre::SceneNode *Car::attachCamNode()
{
    if (mCamNode != NULL) return mCamNode;

    // else we need to make a new camera
    mCamArmNode = mBodyNode->createChildSceneNode("CamArmNode" + boost::lexical_cast<std::string>(mUniqueCarID));
    mCamNode = mCamArmNode->createChildSceneNode("CamNode" + boost::lexical_cast<std::string>(mUniqueCarID));

    return mCamNode;
}


void Car::attachCollisionTickCallback(Player* player)
{
    mbtRigidBody->setUserPointer(player);
}


/// @brief  Loads the given mesh and attaches it to the given node. The given entity name is used, but appended
///         with this car's unique ID so that (forbidden) name collisions don't occur.
/// @param  entityName  Name which the imported mesh will be given.
/// @param  meshName    Name of the mesh which is to be imported.
/// @param  toAttachTo  The SceneNode which the mesh should be imported and attached to.
void Car::createGeometry(
                    const std::string &entityName,
                    const std::string &meshName,
                    Ogre::SceneNode *toAttachTo)
{
    createGeometry(entityName, meshName, false, "", toAttachTo);
}


/// @brief  Loads the given mesh and attaches it to the given node. The given entity name is used, but appended
///         with this car's unique ID so that (forbidden) name collisions don't occur.
/// @param  entityName    Name which the imported mesh will be given.
/// @param  meshName      Name of the mesh which is to be imported.
/// @param  materialName  The name of the material file which is to be imported and applied to the mesh.
/// @param  toAttachTo    The SceneNode which the mesh should be imported and attached to.
void Car::createGeometry(
                    const std::string &entityName,
                    const std::string &meshName,
                    const std::string &materialName,
                    Ogre::SceneNode *toAttachTo)
{
    createGeometry(entityName, meshName, true, materialName, toAttachTo);
}


/// @brief  Loads the given mesh and attaches it to the given node. The given entity name is used, but appended
///         with this car's unique ID so that (forbidden) name collisions don't occur.
/// @param  entityName     Name which the imported mesh will be given.
/// @param  meshName       Name of the mesh which is to be imported.
/// @param  applyMaterial  Specified whether to apply the given material (which may be NULL) or not.
/// @param  materialName   The name of the material file which is to be imported and applied to the mesh.
/// @param  toAttachTo     The SceneNode which the mesh should be imported and attached to.
void Car::createGeometry(
                    const std::string &entityName,
                    const std::string &meshName,
                    bool applyMaterial,
                    const std::string &materialName,
                    Ogre::SceneNode *toAttachTo)
{
    Ogre::Entity* entity;
    entity = mSceneMgr->createEntity(entityName + boost::lexical_cast<std::string>(mUniqueCarID), meshName);
    if (applyMaterial) entity->setMaterialName(materialName);

    int GEOMETRY_QUERY_MASK = 1<<2;
    entity->setQueryFlags(GEOMETRY_QUERY_MASK); // lets raytracing hit this object (for physics)
#if (OGRE_VERSION < ((1 << 16) | (5 << 8) | 0))
    entity->setNormaliseNormals(true);
#endif // only applicable before shoggoth (1.5.0)

    //DOESNT WORKmSceneMgr->setFlipCullingOnNegativeScale(false); // make sure that the culling mesh gets flipped for negatively scaled nodes
    entity->setCastShadows(true);
    toAttachTo->attachObject(entity);
}


/// @brief  Move the debug chassis outline of the car (does not affect physical location at all)
/// @param  chassisShift    Ogre::Vector3 containing translation relative to car origin

// THIS DOES NOT WORK SO DO NOT USE IT. SO RETARDID.
void Car::shiftDebugShape( Ogre::Vector3 chassisShift )
{
    Ogre::Matrix4 matShift;
    matShift.getTrans( chassisShift );

    OgreBulletCollisions::DebugCollisionShape *dbg = mCarChassis->getDebugShape();
    
    int numt = dbg->getNumWorldTransforms();


    Ogre::Matrix4 *matTest;
    matTest = (Ogre::Matrix4*)malloc( numt * sizeof( Ogre::Matrix4 ) );

    dbg->getWorldTransforms( matTest );

    
    for( int i = 0; i < numt; i ++ )
    {
        matTest[i].setTrans( matTest[i].getTrans() + chassisShift );
    }

    dbg->setWorldTransform( matTest[0] );
}

WheelFrictionConstraint::WheelFrictionConstraint( OgreBulletDynamics::RaycastVehicle *v, btRigidBody *r )
    : btTypedConstraint( btTypedConstraintType::POINT2POINT_CONSTRAINT_TYPE, *v->getBulletVehicle()->getRigidBody() )
{
    mVehicle = v; mbtRigidBody = r;
}

void WheelFrictionConstraint::getInfo1( btTypedConstraint::btConstraintInfo1* info )
{
    // Add two constraint rows for each wheel on the ground
    info->m_numConstraintRows = 0;
    for (int i = 0; i < mVehicle->getBulletVehicle()->getNumWheels(); ++i)
    {
        const btWheelInfo& wheel_info = mVehicle->getBulletVehicle()->getWheelInfo(i);
        info->m_numConstraintRows += 2 * ( wheel_info.m_raycastInfo.m_isInContact != 0 );
    }
}

void WheelFrictionConstraint::getInfo2( btTypedConstraint::btConstraintInfo2* info )
{
    int row = 0;

    // Setup sideways friction.

    for( int i = 0; i < mVehicle->getBulletVehicle()->getNumWheels(); ++i )
    {
        const btWheelInfo& wheel_info = mVehicle->getBulletVehicle()->getWheelInfo(i);

        // Only if the wheel is on the ground:
        if( !wheel_info.m_raycastInfo.m_isInContact )
            continue;

        int row_index = row++ * info->rowskip;

        // Set axis to be the direction of motion:
        const btVector3& axis = wheel_info.m_raycastInfo.m_wheelAxleWS;
        info->m_J1linearAxis[row_index]   = axis[0];
        info->m_J1linearAxis[row_index+1] = axis[1];
        info->m_J1linearAxis[row_index+2] = axis[2];

        // Set angular axis.
        btVector3 rel_pos = wheel_info.m_raycastInfo.m_contactPointWS - mbtRigidBody->getCenterOfMassPosition();
        info->m_J1angularAxis[row_index]   = rel_pos.cross(axis)[0];
        info->m_J1angularAxis[row_index+1] = rel_pos.cross(axis)[1];
        info->m_J1angularAxis[row_index+2] = rel_pos.cross(axis)[2];

        // Set constraint error (target relative velocity = 0.0)
        info->m_constraintError[row_index] = 0.0f;

        info->cfm[row_index] = WHEEL_FRICTION_CFM; // Set constraint force mixing

        // Set maximum friction force according to Coulomb's law
        // Substitute Pacejka here
        btScalar max_friction = wheel_info.m_wheelsSuspensionForce * wheel_info.m_frictionSlip / info->fps;

        // Set friction limits.
        info->m_lowerLimit[row_index] = -max_friction;
        info->m_upperLimit[row_index] =  max_friction;
    }

    // Setup forward friction.
    for (int i = 0; i < mVehicle->getBulletVehicle()->getNumWheels(); ++i)
    {
        const btWheelInfo& wheel_info = mVehicle->getBulletVehicle()->getWheelInfo(i);

        // Only if the wheel is on the ground:
        if (!wheel_info.m_raycastInfo.m_isInContact)
            continue;

        int row_index = row++ * info->rowskip;

        // Set axis to be the direction of motion:
        btVector3 axis = wheel_info.m_raycastInfo.m_wheelAxleWS.cross( wheel_info.m_raycastInfo.m_wheelDirectionWS );

        info->m_J1linearAxis[row_index]   = axis[0];
        info->m_J1linearAxis[row_index+1] = axis[1];
        info->m_J1linearAxis[row_index+2] = axis[2];

        // Set angular axis.
        btVector3 rel_pos = wheel_info.m_raycastInfo.m_contactPointWS - mbtRigidBody->getCenterOfMassPosition();

        info->m_J1angularAxis[row_index]   = rel_pos.cross(axis)[0];
        info->m_J1angularAxis[row_index+1] = rel_pos.cross(axis)[1];
        info->m_J1angularAxis[row_index+2] = rel_pos.cross(axis)[2];

        // FIXME: Calculate the speed of the contact point on the wheel spinning.

        // Estimate the wheel's angular velocity = m_deltaRotation
        btScalar wheel_velocity = wheel_info.m_deltaRotation * wheel_info.m_wheelsRadius;
        //btScalar wheel_velocity = wheel_info.m_rotation * wheel_info.m_wheelsRadius;

        // Set constraint error (target relative velocity = 0.0)
        info->m_constraintError[row_index] = wheel_velocity;

        // Set constraint force mixing
        info->cfm[row_index] = WHEEL_FRICTION_CFM; 

        // Set maximum friction force
        btScalar max_friction = wheel_info.m_wheelsSuspensionForce * wheel_info.m_frictionSlip / info->fps;

        // Set friction limits.
        info->m_lowerLimit[row_index] = -max_friction;
        info->m_upperLimit[row_index] =  max_friction;
    }
}

void	WheelFrictionConstraint::setParam(int num, btScalar value, int axis) { return; }
///return the local value of parameter
btScalar WheelFrictionConstraint::getParam(int num, int axis) const { return 0.0; }




void Car::readTuning( char *szFile )
{
    std::ifstream ifs( szFile );
    if( !ifs )
        return;

    std::ostringstream oss;
    oss << ifs.rdbuf();

    if( !ifs && ifs.eof() )
        return;

    std::string strContent( oss.str() );

    std::vector<std::string> vecLines;
    boost::split( vecLines, strContent, boost::is_any_of( "\n" ) );

    for( uint32_t i = 0; i < vecLines.size(); i ++ )
    {
        std::vector<std::string> tokens;
        boost::split( tokens, vecLines.at(i), boost::is_any_of( "\t " ) );

        log( "line is %s and numtok is %i", vecLines.at(i).c_str(), tokens.size() );

        if( tokens.size() < 2 )
            continue;

        if( tokens.at(0) == "mSuspensionStiffness" )
            mSuspensionStiffness = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSuspensionDamping" )
            mSuspensionDamping = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSuspensionCompression" )
            mSuspensionCompression = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mMaxSuspensionForce" )
            mMaxSuspensionForce = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mRollInfluence" )
            mRollInfluence = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSuspensionRestLength" )
            mSuspensionRestLength = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mMaxSuspensionTravelCm" )
            mMaxSuspensionTravelCm = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mFrictionSlip" )
            mFrictionSlip = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mChassisLinearDamping" )
            mChassisLinearDamping = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mChassisAngularDamping" )
            mChassisAngularDamping = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mChassisRestitution" )
            mChassisRestitution = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mChassisFriction" )
            mChassisFriction = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mChassisMass" )
            mChassisMass = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mWheelRadius" )
            mWheelRadius = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mWheelWidth" )
            mWheelWidth = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mWheelFriction" )
            mWheelFriction = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mConnectionHeight" )
            mConnectionHeight = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSteerIncrement" )
            mSteerIncrement = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSteerToZeroIncrement" )
            mSteerToZeroIncrement = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mSteerClamp" )
            mSteerClamp = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mMaxAccelForce" )
            mMaxAccelForce = boost::lexical_cast<float>( tokens.at(1) );
        if( tokens.at(0) == "mMaxBrakeForce" )
            mMaxBrakeForce = boost::lexical_cast<float>( tokens.at(1) );
    }
}