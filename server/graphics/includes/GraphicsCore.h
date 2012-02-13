/**
 * @file	GraphicsCore.h
 * @brief 	Configures the graphical settings and provides the common graphical functionality.
 */
#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GameIncludes.h"


/*-------------------- CLASS DEFINITIONS --------------------*/
/**
 * @brief 	Builds the window and provides the necessary low level functions.
 *
 *          Loads the required Ogre libraries and provides the Graphics Application
 *          with underlying functions to keep it tidy.
 *          Derived from the Ogre Tutorial Framework (BaseApplication.h).
 */
class SpawnScreen;

class GraphicsCore : public Ogre::FrameListener, public Ogre::WindowEventListener, OgreBites::SdkTrayListener
{
public:
    GraphicsCore (void);
    virtual ~GraphicsCore (void);
    virtual void go (void);
	void shutdown() { mShutDown = true; }

    Ogre::Camera* mCamera;
    SpawnScreen *mSpawnScreen;	// This shouldn't be here (Input.cpp is shared). Will be purged.

	// Extra server gameplay shit. This shouldn't be here. Will be purged.
    Ogre::Camera* mViewCam1;
    Ogre::Camera* mViewCam2;
    BigScreen* bigScreen;
    ViewportManager* vpm;
    Gameplay* mGameplay;

protected:
    virtual bool initApplication (void); // This shouldn't be here. Will be purged.
    virtual bool configureRenderer (void);
    virtual void createCamera (void) = 0;
    virtual void createFrameListener (void);
    virtual void createScene (void) = 0;     // This needs to be overridden to display anything
    virtual void destroyScene (void);
    virtual void createViewports (void) = 0;
    virtual void setupResources (void);
    virtual void createResourceListener (void);
    virtual void loadResources (void);

    // Ogre::FrameListener overrides.
    virtual bool frameRenderingQueued (const Ogre::FrameEvent& evt);
    
    // Ogre::WindowEventListener overrides.
    virtual void windowResized (Ogre::RenderWindow* rw);
    virtual void windowClosed (Ogre::RenderWindow* rw);

    // OIS user input
    Input mUserInput;

    Ogre::Root*         mRoot;
    Ogre::RenderWindow* mWindow;
    Ogre::String        mResourcesCfg;
    Ogre::String        mPluginsCfg;

    // OgreBites
    OgreBites::SdkCameraMan*   mCameraMan;     // basic camera controller
    bool mCursorWasVisible;                    // Was the cursor visible before dialog appeared
    bool mShutDown;
};

#endif // #ifndef GRAPHICSCORE_H
