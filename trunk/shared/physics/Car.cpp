/**
 * @file	Car.cpp
 * @brief 	Contains the core methods and variables common to all different types of car.
                Extend or implement this class as required to create a functioning car.
                This class and its subclasses deal with the physics and the graphics which are
                car related and expose an interface to do stuff to the cars.
 */
#include "stdafx.h"
#include "SharedIncludes.h"


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


/// @brief  Moves the car to the specified position keeping current rotation, velocity etc.
/// @param  position  The position to move to.
void Car::moveTo(const btVector3 &position)
{
    moveTo(position, mbtRigidBody->getOrientation());
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
            if (mSteer >= mSteerToZeroIncrement) calcIncrement = -mSteerToZeroIncrement;
            else resetToZero = true;
        }
        else
        {
            if (mSteer <= -mSteerToZeroIncrement) calcIncrement = mSteerToZeroIncrement;
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
void Car::accelInputTick(bool isForward, bool isBack)
{
    int forwardBack = 0;
    if (isForward) forwardBack += 1;
    if (isBack)    forwardBack -= 1;

    float f = forwardBack < 0 ? mMaxBrakeForce : mMaxAccelForce;

    mEngineForce = f * forwardBack;

    mVehicle->applyEngineForce(mEngineForce, 0);
    mVehicle->applyEngineForce(mEngineForce, 1);

    //mbtRigidBody->setBrake(1500.0f, 0);
    //mbtRigidBody->setBrake(1500.0f, 1);
    //mbtRigidBody->setBrake(1500.0f, 2);
    //mbtRigidBody->setBrake(1500.0f, 3);
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