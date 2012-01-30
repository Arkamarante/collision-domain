/**
 * @file    GraphicsApplication.cpp
 * @brief     Adds objects to the graphics interface.
 *          Derived from the Ogre Tutorial Framework (TutorialApplication.cpp).
 */

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GameIncludes.h"


/*-------------------- METHOD DEFINITIONS --------------------*/

/// @brief  Constructor.
GraphicsApplication::GraphicsApplication (void)
{
}


/// @brief  Destructor.
GraphicsApplication::~GraphicsApplication (void)
{
}


/// @brief  Creates the initial scene prior to the first render pass, adding objects etc.
void GraphicsApplication::createScene (void)
{
	mGuiRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
	GameCore::mGui->setupGUI();

    setupLighting();
    setupArena();
    setupNetworking();

    // Load the ninjas
    Ogre::Entity* ninjaEntity = GameCore::mSceneMgr->createEntity("Ninja", "ninja.mesh");
    ninjaEntity->setCastShadows(true);
    Ogre::SceneNode* ninjaNode = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
    ninjaNode->attachObject(ninjaEntity);
    ninjaNode->translate(0, 0, 0);
    Ogre::Entity* ninjaEntity2 = GameCore::mSceneMgr->createEntity("Ninja2", "ninja.mesh");
    ninjaEntity2->setCastShadows(true);
    Ogre::SceneNode* ninjaNode2 = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode2");
    ninjaNode2->attachObject(ninjaEntity2);
    ninjaNode2->pitch(Ogre::Degree(90));
    ninjaNode2->roll(Ogre::Degree(180));
    ninjaNode2->translate(0, 100, 0);

	GameCore::mGui->displayConsole();
	GameCore::mGui->displayChatbox();
}


/// @brief  Adds and configures lights to the scene.
void GraphicsApplication::setupLighting (void)
{
    // Set the ambient light.
    GameCore::mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25f, 0.25f, 0.25f));
    
    // Add a directional light
    Ogre::Vector3 directionalLightDir(0.55f, -0.3f, 0.75f);
    directionalLightDir.normalise();
    Ogre::Light* directionalLight = GameCore::mSceneMgr->createLight("directionalLight");
    directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
    directionalLight->setDiffuseColour( Ogre::ColourValue::White);
    directionalLight->setSpecularColour(Ogre::ColourValue(0.4f, 0.4f, 0.4f));
    directionalLight->setDirection(directionalLightDir);
    
    // Create the skybox
    GameCore::mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    // Set the shadow renderer
    GameCore::mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
}


/// @brief  Builds the initial arena.
void GraphicsApplication::setupArena (void)
{
    // Load and meshes and create entities
    Ogre::Entity* arenaEntity = GameCore::mSceneMgr->createEntity("Arena", "arena.mesh");
    arenaEntity->setMaterialName("arena_uv");
    arenaEntity->setCastShadows(true); // without shadows you can't see the seating rows
    
    Ogre::SceneNode* arenaNode = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("ArenaNode", Ogre::Vector3(0, 0, 0));
    arenaNode->attachObject(arenaEntity);
    arenaNode->scale(0.15f, 0.15f, 0.15f);



    // ground plane, visible on the top down view only (unless something bad happens!!)
    /*Ogre::Plane groundPlane(Ogre::Vector3::UNIT_Y, 0);
    Ogre::MeshManager::getSingleton().createPlane("GroundPlaneMesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, groundPlane, 5000, 5000, 20, 20, true, 1, 20, 20, Ogre::Vector3::UNIT_Z);

    Ogre::Entity* groundEntity = GameCore::mSceneMgr->createEntity("Ground", "GroundPlaneMesh");
    groundEntity->setMaterialName("Examples/GrassFloor");
    groundEntity->setCastShadows(false);
    
    // Create ground plane at -5 (below lowest arena point) to avoid z-index flickering/showing the plane through arena floor.
    Ogre::SceneNode* groundNode = GameCore::mSceneMgr->getRootSceneNode()->createChildSceneNode("GroundNode", Ogre::Vector3(0, -5, 0));
    groundNode->attachObject(groundEntity);*/



    // create collideable floor so shit doesn't freefall. It will hit the floor.
    GameCore::mPhysicsCore->createFloorPlane();
    GameCore::mPhysicsCore->createWallPlanes();
}


/// @brief  Configures the networking, retreiving the required data from the server.
void GraphicsApplication::setupNetworking (void)
{
}


/// @brief  Passes the frame listener down to the GraphicsCore.
void GraphicsApplication::createFrameListener (void)
{
    firstFrameOccurred = false;
    mClock = new btClock();
    GraphicsCore::createFrameListener();
}


/// @brief  Called once a frame as the CPU has finished its calculations and the GPU is about to start rendering.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameRenderingQueued (const Ogre::FrameEvent& evt)
{
    // MUST BE THE FIRST THING - do the core things (GraphicsCore is extended by this class)
    if (!GraphicsCore::frameRenderingQueued(evt)) return false;

	if( NetworkCore::bConnected == false )
		return true;

    // Toggle on screen widgets
    /*if (mUserInput.isToggleWidget()) 
    {
        mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
        mDetailsPanel->show();
    }*/
    
    // NOW WE WILL DO EVERYTHING BASED OFF THE LATEST KEYBOARD / MOUSE INPUT

    // Process keyboard input and produce an InputState object from this.
    InputState *inputSnapshot;
	
	if( !GameCore::mGui->consoleVisible() && !GameCore::mGui->chatboxVisible() )
	{
		inputSnapshot = mUserInput.getInputState();

		if( mUserInput.isToggleConsole() )
			GameCore::mGui->toggleConsole();
		else if( mUserInput.isToggleChatbox() )
			GameCore::mGui->toggleChatbox();
	}
	else
	{
		// Don't want to capture any keys (typing things)
		inputSnapshot = new InputState( false, false, false, false );
	}
    
    // Process the networking. Sends client's input and receives data
    GameCore::mNetworkCore->frameEvent(inputSnapshot);

	// Process the player pool. Perform updates on other players
	GameCore::mPlayerPool->frameEvent( evt );

    // Apply controls the player (who will be moved on frameEnd and frameStart).
	GameCore::mPlayerPool->getLocalPlayer()->processControlsFrameEvent(inputSnapshot, evt.timeSinceLastFrame, (float) (1./60.));
    //mPlayerPool->getLocalPlayer()->updateCameraFrameEvent(mUserInput.getMouseXRel(), mUserInput.getMouseYRel());

    GameCore::mPowerupPool->frameEvent();

	// LOCAL
	// get new snapshpot from control press - don't move the car though
	// get old snapshot received from server
	// create a new snapshot between new and old
	// apply new snapshot to player

	// REMOTE
	// get snapshot from server
	// apply to player
	
	// FUTURE
	// game will run x ticks behind the server
	// when a new snapshot is received, it should be in the client's future
	// interpolate based on snapshot timestamps

    // Perform Client Side Prediction.
    // Move any players who are out of sync

    /* Deal with all but local player (who's snapshots should be 0ms behind where this client thinks they are)
    for (i : otherPlayerIDs)
    {
        CarSnapshot *carSnapshot = getCarSnapshotIfExistsSincePreviousGet(int playerID);

        if (CSP.needsPushingBackIntoPosition(players[playerID], carSnapshot)
        {
            players[playerID].getCar()->restoreSnapshot(carSnapshot);
        }

        delete carSnapshot;
    }
    
    // Deal with the local player (who's snapshot will be x=latency ms behind the real thing)
    if (CSP.needsMePushedBack(players[clientID], carSnapshot))
    {
        // Calculate a snapshot which doesn't jolt the player harshly if it can be fixed with small movements
        CarSnapshot *latestPlayerSnapshot = getCarSnapshotIfExistsSincePreviousGet(int clientID);
        CarSnapshot *fixSnapshot = new CarSnapshot(...);

        players[clientID].getCar->restoreSnapshot(fixSnapshot);
    }
    */

    // Cleanup frame specific objects so we don't rape memory. If you want to remember some, delete them later!
    delete inputSnapshot;

    // Minimum of 20 FPS (maxSubsteps=3) before physics becomes wrong
    GameCore::mPhysicsCore->stepSimulation(evt.timeSinceLastFrame, 3, (float) (1./60.));

    return true;
}


/// @brief  Called once a frame every time processing for a frame has begun.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameStarted(const Ogre::FrameEvent& evt)
{
    return true;
}


/// @brief  Called once a frame every time processing for a frame has ended.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameEnded(const Ogre::FrameEvent& evt)
{
    return true;
}


// The following code is not understood. Does something for Win32: unknown. Best just leave it alone.
#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        GraphicsApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
