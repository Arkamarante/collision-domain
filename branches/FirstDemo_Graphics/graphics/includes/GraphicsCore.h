/**
 * @file	GraphicsApplication.h
 * @brief 	Builds the window, loading the Ogre necessary libraries and providing 
 *          the Graphics Application with underlying functions to keep it tidy.
 *          Derived from the Ogre Tutorial Framework (BaseApplication.h).
 */
#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"



/*-------------------- CLASS DEFINITIONS --------------------*/
/**
 * @brief 	Builds the window and provides the necessary low level functions.
 *
 *          Loads the required Ogre libraries and provides the Graphics Application
 *          with underlying functions to keep it tidy.
 *          Derived from the Ogre Tutorial Framework (BaseApplication.h).
 */
class GraphicsCore : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
    GraphicsCore (void);
    virtual ~GraphicsCore (void);
    virtual void go (void);

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
    virtual void createScene(void) = 0;     // This needs to be overridden to display anything
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);

    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    // Ogre::WindowEventListener
    // Adjust mouse clipping area
    virtual void windowResized(Ogre::RenderWindow* rw);
    // Unattach OIS before window shutdown (very important under Linux)
    virtual void windowClosed(Ogre::RenderWindow* rw);

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

    // OgreBites
    OgreBites::SdkTrayManager* mTrayMgr;
    OgreBites::SdkCameraMan* mCameraMan;       // basic camera controller
    OgreBites::ParamsPanel* mDetailsPanel;     // sample details panel
    bool mCursorWasVisible;                    // was cursor visible before dialog appeared
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;
};

#endif // #ifndef GRAPHICSCORE_H
