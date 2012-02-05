/**
 * @file	Player.cpp
 * @brief 	Contains the player car and the related data specific to each player.
 */

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GameIncludes.h"

/*-------------------- METHOD DEFINITIONS --------------------*/

/// @brief  Constructor, setting the player constants and zeroing the PlayerState.
Player::Player (void) : cameraRotationConstant(0.08f)
{
    // PlayerState state configures constants and zeros values upon creation.
    mCarSnapshot = NULL;
	newInput = NULL;
	mCar = NULL;
}


/// @brief   Deconstructor.
Player::~Player (void)
{
	if( mCar )
		delete( mCar );
}


/// @brief  Creates and positions the 3D player object (and space for a camera).
/// @param  sm  The SceneManager to which the 3D player object is attached.
/// @param  t   The car model to load as the player object.
/// @param  s   The texture to apply to the car model.
/// @param  physicsCore   The class containing the physics world.
void Player::createPlayer (Ogre::SceneManager* sm, CarType iCarType, CarSkin s, PhysicsCore *physicsCore)
{
    mCarType = iCarType;

    // TODO: something with iCarType
    switch( iCarType )
    {
    case CAR_BANGER:
        mCar = (Car*) new SimpleCoupeCar(sm, physicsCore->mWorld, physicsCore->getUniqueEntityID());
        break;
    case CAR_SMALL:
        mCar = (Car*) new SmallCar(sm, physicsCore->mWorld, physicsCore->getUniqueEntityID());
        break;
    case CAR_TRUCK:
        mCar = (Car*) new TruckCar(sm, physicsCore->mWorld, physicsCore->getUniqueEntityID());
        break;
    default:
        mCar = (Car*) new SimpleCoupeCar(sm, physicsCore->mWorld, physicsCore->getUniqueEntityID());
        break;
    }

	//Set HP. More clever damage might be implemented in the future
	hp = 100;

    mCar->moveTo(btVector3(0,0.5,0));
}


/// @brief  Called back every substep of physics stepSim (so potentially multiple times a frame)
///         In total this will even out to 60 calls per second :-)
/// @param  damage   0 if no damage was done to this player in the collision, else 1.
void Player::collisionTickCallback(int damage)
{
    // p1 and p2 might not be the only two players who collided this physics step.
    OutputDebugString("Server: Player collision\n");
	hp-=damage*10; //Apply damage to player
	GameCore::mGraphicsCore->mGameplay->notifyDamage(this);\
}


/// @brief  Attaches a camera to the player.
/// @param  cam   The camera object to attach to the player.
void Player::attachCamera (Ogre::Camera* cam)
{
    // only attach a camera to one of them!! Imagine the carnage if there were more
    Ogre::SceneNode *camNode = mCar->attachCamNode();
    Ogre::SceneNode *camArmNode = camNode->getParentSceneNode();

    camArmNode->translate(0, 0.5, 0); // place camera y above car node
    camArmNode->pitch(Ogre::Degree(25));
    camNode->yaw(Ogre::Degree(180));
    camNode->translate(0, 0, 62); // zoom in!! (50 is a fair way behind the car, 75 is in the car)

	camNode->detachAllObjects(); //Program crashes if you attach more than one cam to each objet.
    camNode->attachObject(cam);
}


/// @brief  Applies the player controls to the car so it will move on next stepSimulation.
/// @param  userInput               The latest user keypresses.
/// @param  secondsSinceLastFrame   The time in seconds since the last frame, for framerate independence.
/// @param  targetPhysicsFrameRate  The target framerate to normalise acceleration to.
void Player::processControlsFrameEvent(
        InputState *userInput,
        Ogre::Real secondsSinceLastFrame,
        float targetPhysicsFrameRate)
{
    // process steering
    mCar->steerInputTick(userInput->isLeft(), userInput->isRight(), secondsSinceLastFrame, targetPhysicsFrameRate);
    
    // apply acceleration 4wd style
    mCar->accelInputTick(userInput->isForward(), userInput->isBack());

    // TELEPORT TESTING
    /*if (userInput->isLeft() && userInput->isRight())
    {
        // teleport to the previously set point!
        if (mCarSnapshot != NULL) mCar->restoreSnapshot(mCarSnapshot);
    }
    
    if (userInput->isLeft() && !userInput->isRight())
    {
        // set the new teleport point
        if (mCarSnapshot != NULL) delete mCarSnapshot;
        mCarSnapshot = mCar->getCarSnapshot();
    }*/
}


/// @brief  Updates the camera's rotation based on the values given.
/// @param  XRotation   The amount to rotate the camera by in the X direction (relative to its current rotation).
/// @param  YRotation   The amount to rotate the camera by in the Y direction (relative to its current rotation).
void Player::updateCameraFrameEvent (int XRotation, int YRotation)
{
    Ogre::SceneNode *camArmNode = mCar->attachCamNode()->getParentSceneNode();
    camArmNode->yaw(Ogre::Degree(-cameraRotationConstant * XRotation), Ogre::Node::TS_PARENT);
    camArmNode->pitch(Ogre::Degree(cameraRotationConstant * 0.5f * YRotation), Ogre::Node::TS_LOCAL);
}


/// @brief  Supplies the Car object which contains player position and methods on that. 
/// @return The Car object which allows forcing a player to a given CarSnapshot or getting a CarSnapshot.
Car* Player::getCar()
{
    return mCar;
}

int Player::getHP()
{
	return hp;
}