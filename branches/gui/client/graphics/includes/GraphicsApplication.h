/**
 * @file	GraphicsApplication.h
 * @brief 	Adds objects to the graphics interface and contains the framelistener
 */
#ifndef GRAPHICSAPPLICATION_H
#define GRAPHICSAPPLICATION_H

/*-------------------- INCLUDES --------------------*/
#include "stdafx.h"
#include "GraphicsCore.h"
#include "Player.h"
#include "InputState.h"
#include "GameGUI.h"



/*-------------------- CLASS DEFINITIONS --------------------*/
/**
 *  @brief  Adds objects to the graphics interface.
 *
 *          Derived from the Ogre Tutorial Framework (TutorialApplication.h).
 */
class GraphicsApplication : public GraphicsCore
{
public:
    GraphicsApplication(void);
    virtual ~GraphicsApplication(void);

    int clientID;               ///< The client ID which is assigned by the server.
    bool firstFrameOccurred;

protected:

	CEGUI::OgreRenderer* mGuiRenderer;
	GameGUI* mGui;

    virtual void createScene(void);
    virtual void createFrameListener(void);
	
    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool frameStarted(const Ogre::FrameEvent& evt);
    virtual bool frameEnded(const Ogre::FrameEvent& evt);

private:
    void setupLighting (void);
    void setupArena (void);
    void setupNetworking (void);
	void setupGUI(void);
};

#endif // #ifndef GRAPHICSAPPLICATION_H