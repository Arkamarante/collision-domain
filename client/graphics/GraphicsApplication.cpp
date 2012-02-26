/**
 * @file    GraphicsApplication.cpp
 * @brief     Adds objects to the graphics interface.
 *          Derived from the Ogre Tutorial Framework (TutorialApplication.cpp).
 */

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GameIncludes.h"
#include <iostream>
#include <fstream>


/*-------------------- METHOD DEFINITIONS --------------------*/

/// @brief  Constructor.
GraphicsApplication::GraphicsApplication (void) :
    mBenchmarkRunning(false)
{
}


/// @brief  Destructor.
GraphicsApplication::~GraphicsApplication (void)
{
}


/// @brief  Creates and positions the camera.
void GraphicsApplication::createCamera (void)
{
    // Create the camera
    mCamera = GameCore::mSceneMgr->createCamera("PlayerCam");

    // Position it looking back along -Z
    mCamera->setPosition(Ogre::Vector3(0, 3, 60));
    mCamera->lookAt(Ogre::Vector3(0, 0, -300));
    mCamera->setNearClipDistance(5);
    mCamera->setFarClipDistance(2500);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}


/// @brief  Adds a single viewport that spans the entire window.
void GraphicsApplication::createViewports (void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);

	// Set the background colour and match the aspect ratio to the window's.
    vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}


/// @brief  Creates the initial scene prior to the first render pass, adding objects etc.
void GraphicsApplication::createScene (void)
{
	// Save reference
	GameCore::mGraphicsApplication = this;
    
	// Setup the scene
    setupCompositorChain(mCamera->getViewport());
	setupShadowSystem();
    setupLightSystem();
    setupParticleSystem();
    setupArena();
    setupGUI();
}


void GraphicsApplication::setupGUI (void)
{
    // Run the generic GUI setup
    SceneSetup::setupGUI();

    // Attach the GUI components
    GameCore::mGui->setupConnectBox();
    GameCore::mGui->setupConsole();
    GameCore::mGui->setupChatbox();
    GameCore::mGui->setupSpeedo();
    GameCore::mGui->setupGearDisplay();
    GameCore::mGui->updateSpeedo(0.0f, 0);

    GameCore::mGameplay->setupOverlay();
}


/// @brief  Passes the frame listener down to the GraphicsCore.
void GraphicsApplication::createFrameListener (void)
{
    GraphicsCore::createFrameListener();

	// Handle Game play (this isn't the place to do this, this will be moved).
	GameCore::mGameplay = new Gameplay();
	Team* t1 = GameCore::mGameplay->createTeam("Team1Name");
	Team* t2 = GameCore::mGameplay->createTeam("Team2Name");
}


void GraphicsApplication::startBenchmark (uint8_t stage)
{
	Ogre::CompositorManager& cm = Ogre::CompositorManager::getSingleton();
	Ogre::Viewport* vp = mCamera->getViewport();
	switch (stage)
	{
		case 0:	// all off
			OutputDebugString("Starting benchmark...\n");
			cm.removeCompositor(vp, "HDR");
			cm.removeCompositor(vp, "Bloom");
			cm.removeCompositor(vp, "MotionBlur");
			cm.removeCompositor(vp, "RadialBlur");
			break;
		case 1: // just hdr on
			cm.addCompositor(vp, "HDR");
			loadHDR(vp, 0);
			break;
		case 2: // just bloom on
			cm.removeCompositor(vp, "HDR");
			cm.addCompositor(vp, "Bloom");
			loadBloom(vp, 0, 0.15f, 1.0f);
			break;
		case 3: // just MotionBlur on
			cm.removeCompositor(vp, "Bloom");
			cm.addCompositor(vp, "MotionBlur");
			loadMotionBlur(vp, 0, 0.1f);
			break;
		case 4: // just RadialBlur on
			cm.removeCompositor(vp, "MotionBlur");
			cm.addCompositor(vp, "RadialBlur");
			cm.setCompositorEnabled(vp, "RadialBlur", true);
			break;
		case 5: // all on
			cm.addCompositor(vp, "HDR");
			loadHDR(vp, 0);
			cm.addCompositor(vp, "Bloom");
			loadBloom(vp, 0, 0.15f, 1.0f);
			cm.addCompositor(vp, "MotionBlur");
			loadMotionBlur(vp, 0, 0.1f);
			break;
	}
	
	mWindow->resetStatistics();
	mBenchmarkStage = stage;
	mBenchmarkRunning = true;
}

void GraphicsApplication::finishBenchmark (uint8_t stage, float averageTriangles)
{
	static float r[6];
	static float triangles;
	r[stage] = mWindow->getAverageFPS();
	if (stage == 0)
		triangles = averageTriangles;

	if (stage == 5)
	{
		std::ofstream rFile;
		rFile.open("BenchmarkResults.txt", std::ios::out | std::ios::trunc);
		rFile << std::fixed;
		rFile << "              BENCHMARKING RESULTS\n";
		rFile << " Average triangles per frame = " << std::setprecision(0) << triangles << "\n\n";
		rFile << "+-----+-------+-----+-----+-------+-------+\n";
		rFile << "| HDR | Bloom | MoB | RaB |  FPS  |  DIF  |\n";
		rFile << "+-----+-------+-----+-----+-------+-------+\n";
		rFile.precision(2);
		rFile << "|  0  |   0   |  0  |  0  | " << std::setw(5) << std::setfill(' ') << r[0] << " | 00.00 |\n";
		rFile << "|  1  |   0   |  0  |  0  | " << std::setw(5) << std::setfill(' ') << r[1] << " | " << std::setw(5) << std::setfill(' ') << r[0] - r[1] << " |\n";
		rFile << "|  0  |   1   |  0  |  0  | " << std::setw(5) << std::setfill(' ') << r[2] << " | " << std::setw(5) << std::setfill(' ') << r[0] - r[2] << " |\n";
		rFile << "|  0  |   0   |  1  |  0  | " << std::setw(5) << std::setfill(' ') << r[3] << " | " << std::setw(5) << std::setfill(' ') << r[0] - r[3] << " |\n";
		rFile << "|  0  |   0   |  0  |  1  | " << std::setw(5) << std::setfill(' ') << r[4] << " | " << std::setw(5) << std::setfill(' ') << r[0] - r[4] << " |\n";
		rFile << "|  1  |   1   |  1  |  1  | " << std::setw(5) << std::setfill(' ') << r[5] << " | " << std::setw(5) << std::setfill(' ') << r[0] - r[5] << " |\n";
		rFile << "+-----+-------+-----+-----+-------+-------+\n";
		rFile.close();
		OutputDebugString("Benchmark complete. See $(OGRE_HOME)/bin/debug/BenchmarkResults.txt for the results.\n");
		Ogre::CompositorManager::getSingleton().removeCompositor(mCamera->getViewport(), "HDR");
	}
	else
	{
		startBenchmark(stage+1);
	}
}

/// @brief  Called once a frame as the CPU has finished its calculations and the GPU is about to start rendering.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameRenderingQueued (const Ogre::FrameEvent& evt)
{
    if (!GraphicsCore::frameRenderingQueued(evt))
        return false;
    
	if (mBenchmarkRunning)
	{
		static float benchmarkProgress = 0;
		static float CATriangles = 0;
		static uint16_t CAi = 0;
		float benchmarkIncrement = 500 * evt.timeSinceLastFrame;
		benchmarkProgress += benchmarkIncrement;
		CATriangles += ((float) (mWindow->getTriangleCount() - CATriangles)) / ((float) (++CAi));

		// stop the benchmark after 8 seconds
		if (benchmarkProgress > 4000)
		{
			CAi = 0;
			benchmarkProgress = 0;
			mBenchmarkRunning = false;
			finishBenchmark(mBenchmarkStage, CATriangles);
		}

		// rotate the camera
		GameCore::mPlayerPool->getLocalPlayer()->updateCameraFrameEvent(benchmarkIncrement, 0.0f, 0.0f);

		// update fps counter
		float avgfps = mWindow->getAverageFPS(); // update fps
		CEGUI::Window *fps = CEGUI::WindowManager::getSingleton().getWindow( "root_wnd/fps" );
		char szFPS[16];
		sprintf(szFPS, "FPS: %.2f", avgfps);
		fps->setText(szFPS);

		// dont do any of the non-graphics bullshit
		return true;
	}

    // NOW WE WILL DO EVERYTHING BASED OFF THE LATEST KEYBOARD / MOUSE INPUT
	InputState *inputSnapshot = mUserInput.getInputState();
	if( mUserInput.isToggleConsole() )      GameCore::mGui->toggleConsole();
	else if( mUserInput.isToggleChatbox() ) GameCore::mGui->toggleChatbox();
    
    // Process the networking. Sends client's input and receives data
    GameCore::mNetworkCore->frameEvent(inputSnapshot);

	if( NetworkCore::bConnected == true )
	{
		// Process the player pool. Perform updates on other players
		GameCore::mPlayerPool->frameEvent();

		// Apply controls the player (who will be moved on frameEnd and frameStart).
		if( GameCore::mPlayerPool->getLocalPlayer()->getCar() != NULL )
		{
			GameCore::mPlayerPool->getLocalPlayer()->processControlsFrameEvent(inputSnapshot, evt.timeSinceLastFrame, (1.0f / 60.0f));
			GameCore::mPlayerPool->getLocalPlayer()->updateCameraFrameEvent(mUserInput.getMouseXRel(), mUserInput.getMouseYRel(), mUserInput.getMouseZRel());

            // update GUI
			float speedmph = GameCore::mPlayerPool->getLocalPlayer()->getCar()->getCarMph();
            int   curGear  = GameCore::mPlayerPool->getLocalPlayer()->getCar()->getGear();
			float avgfps   = mWindow->getAverageFPS();

			CEGUI::Window *fps = CEGUI::WindowManager::getSingleton().getWindow( "root_wnd/fps" );
			char szFPS[64];
			sprintf( szFPS,   "FPS: %.2f", avgfps);
			fps->setText( szFPS );
			GameCore::mGui->updateSpeedo( speedmph, curGear );

			// 
		}

	}

    /*  NOTE TO SELF (JAMIE)
        Client doesn't want to do PowerupPool::frameEvent() when powerups are networked
        All powerup removals handled by the server's collision events
        In fact, client probably shouldn't register any collision callbacks for powerups
    */
    GameCore::mPowerupPool->frameEvent( evt );

    if( NetworkCore::bConnected && GameCore::mPlayerPool->getLocalPlayer()->getCar() != NULL )
    {
        float rpm = GameCore::mPlayerPool->getLocalPlayer()->getCar()->getRPM();

        GameCore::mAudioCore->frameEvent(rpm);
    }
    else
    {
        GameCore::mAudioCore->frameEvent(800);
    }

    // FUTURE
    // game will run x ticks behind the server
    // when a new snapshot is received, it should be in the client's future
    // interpolate based on snapshot timestamps

    // Cleanup frame specific objects so we don't rape memory. If you want to remember some, delete them later!
    delete inputSnapshot;

    // Minimum of 30 FPS (maxSubsteps=2) before physics becomes wrong
    GameCore::mPhysicsCore->stepSimulation(evt.timeSinceLastFrame, 4, (1.0f / 60.0f));
	
	//Draw info items
	GameCore::mGameplay->drawInfo();
    return true;
}


/// @brief  Called once a frame every time processing for a frame has begun.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameStarted (const Ogre::FrameEvent& evt)
{
    return true;
}


/// @brief  Called once a frame every time processing for a frame has ended.
/// @param  evt  The FrameEvent associated with this frame's rendering.
/// @return Whether the application should continue (i.e.\ false will force a shut down).
bool GraphicsApplication::frameEnded (const Ogre::FrameEvent& evt)
{
    return true;
}


// Main function. Bootstraps the application.
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
            std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
