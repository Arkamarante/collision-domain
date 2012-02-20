#include "stdafx.h"
#include "GameIncludes.h"

#define MAP_WIDTH	1.0f
#define MAP_HEIGHT	0.8f

#define MARKER_WIDTH 0.05f
#define MARKER_HEIGHT 0.05f
/*
BigScreen::BigScreen(ViewportManager* vpm_P, NetworkCore* networkCore)
{
	mViewportManager	= vpm_P;
	mNetworkCore		= networkCore;
}
*/

BigScreen::BigScreen(ViewportManager* vpm_P)
{
	mViewportManager = vpm_P;

	//Ogre::LogManager::getSingleton().logMessage("Starting BigScreen");
	OutputDebugString("BigScreen Loaded \n");
}

bool BigScreen::declareNewPlayer( RakNet::RakNetGUID playerid )
{
	Player* tmpPlayer = GameCore::mPlayerPool->getPlayer(playerid);
	/*
	ViewCamera* bestCanidateVC = NULL;
	std::vector<ViewCamera*>::iterator itr;
	for(itr = viewCameraVector.begin(); itr<viewCameraVector.end(); ++itr)
	{
		ViewCamera* tmp = *itr;
		if(bestCanidateVC != NULL)
		{
			//Check to see if this new canidate is smaller than the previous canidate
			if(RakNet::LessThan(tmp->getLastUpdateTime(),bestCanidateVC->getLastUpdateTime()))
			{
				bestCanidateVC =  tmp;
			}
		}
		else
		{
			bestCanidateVC=  tmp;
		}
	}
	
	if(bestCanidateVC != NULL)
	{
		//This is important. If you add the camera to two points it dies
		bestCanidateVC->getCamera()->detachFromParent(); 
		tmpPlayer->attachCamera(bestCanidateVC->getCamera());
		OutputDebugString("Attached Camera to new player\n");
		bestCanidateVC->setLastUpdateTime();
	}
	*/
	//Manage new player for bigscreen view
	this->manageNewPlayer(tmpPlayer);
	return mViewportManager->declareNewPlayer(tmpPlayer);
}

void BigScreen::addCamera(Ogre::Camera* cam) 
{
	ViewCamera* tmpVC = new ViewCamera(cam);
	viewCameraVector.push_back(tmpVC);
}

void BigScreen::setupMapView()
{
	Ogre::Overlay *olMap = 
		Ogre::OverlayManager::getSingleton().create( "OVERLAY_MAP" );
	olMap->setZOrder(500);
	olMap->show();

	olcMap = static_cast<Ogre::OverlayContainer*> ( Ogre::OverlayManager::getSingleton().
		createOverlayElement( "Panel", "MAP" ) );

	olcMap->setMetricsMode( Ogre::GMM_RELATIVE );
	olcMap->setHorizontalAlignment( Ogre::GHA_LEFT);
	olcMap->setVerticalAlignment(Ogre::GVA_TOP);
	
	//Set the map
	olcMap->setDimensions(MAP_WIDTH,MAP_HEIGHT);
	olcMap->setMaterialName( "map_top_1" );
	olMap->add2D(olcMap);

	//Start thinking about the cars
	/*
	oleCar = 
		Ogre::OverlayManager::getSingleton().createOverlayElement( 
			"Panel", 
			"Player" );
	oleCar->setMetricsMode( Ogre::GMM_RELATIVE );
	oleCar->setDimensions(0.2f,0.2f);
	oleCar->setMaterialName( "map_top_1" );
	oleCar->setPosition(0.5,0.5);
	olcMap->addChild(oleCar);
	*/
}

void BigScreen::manageNewPlayer(Player* player)
{
	//Timestamp used to fix duplicate named overlays.
	std::stringstream overlayNameSS;
	overlayNameSS << "PlayerOverlay" << RakNet::GetTime();	
	
	Ogre::OverlayElement* tmpOLE = 
		Ogre::OverlayManager::getSingleton().createOverlayElement(
		"Panel",
		overlayNameSS.str());


	tmpOLE->setMetricsMode( Ogre::GMM_RELATIVE );
	tmpOLE->setDimensions(MARKER_WIDTH,MARKER_HEIGHT);

	//Make a copy of the material
	Ogre::MaterialPtr arrowMaterial = Ogre::MaterialManager::getSingleton().getByName("ArrowBlack");
	//Build a new name with GUID so it should be unique
	std::stringstream newMaterialName;
	newMaterialName << "ArrowBlack" << player->getPlayerGUID().ToString();
	//Clone a new instance
	arrowMaterial = arrowMaterial->clone(newMaterialName.str());
	//We can now assign the new material with a new name
	tmpOLE->setMaterialName(newMaterialName.str());
	
	tmpOLE->setPosition(0.5f, 0.5f);
	olcMap->addChild(tmpOLE);

	player->setOverlayElement(tmpOLE);
}

void BigScreen::updateMapView()
{
	//Player* localPlayer = GameCore::mPlayerPool->getLocalPlayer();
	//updatePlayer(localPlayer,oleCar);

	//Loop through all possible players
	for(int i=0;i<MAX_PLAYERS;i++)
	{
		Player* tmpPlayer = GameCore::mPlayerPool->getPlayer(i);
		//Check to see if there is an actual player in this section
		if(tmpPlayer != NULL)
		{
			updatePlayer(tmpPlayer,tmpPlayer->getOverlayElement());
		}
	}
}

void BigScreen::updatePlayer(Player* player, Ogre::OverlayElement* carOverlay)
{
	
	if(player->getCar() != NULL && player->getCar()->getCarSnapshot() != NULL)
	{
		btVector3 localPlayerPos = player->getCar()->getCarSnapshot()->mPosition;
		float xPos = localPlayerPos.getX(); 
		float yPos = localPlayerPos.getZ(); //Z as we're doing a 2D projection
	
		//Correct the position to be relative of top left corner.
		//xPos -= mapCorner.x;
		//yPos -= mapCorner.z;
		//Ogre::Entity* arenaEntity = GameCore::mGraphicsApplication->getArenaEntity();
		//Ogre::Vector3 maxArena = arenaEntity->getBoundingBox().getMaximum();
	
		//Correct to make percentage
		xPos = xPos/(mapSize.x);
		yPos = yPos/(mapSize.z);
	
		xPos += MAP_WIDTH/2;
		yPos += MAP_HEIGHT/2;

		//Corrects it for aspect ratio of drawn map
		xPos *= MAP_WIDTH;
		yPos *= MAP_HEIGHT;

		//Correct for the size of the marker
		xPos -= MARKER_WIDTH;
		yPos -= MARKER_HEIGHT;
		
		//Sort out the rotation
		btQuaternion rotationQuat = player->getCar()->getCarSnapshot()->mRotation; 
		//Convert to ogre representation of quats
		Ogre::Quaternion ogreRotQuat(rotationQuat.getW(), rotationQuat.getX(), rotationQuat.getY(), rotationQuat.getZ());
		//Then we can use the inbuilt function (took forever to get to this point :O)
		Ogre::Radian rot = ogreRotQuat.getYaw();
			

		Ogre::Material* matMarker = carOverlay->getMaterial().get();
		
		Ogre::TextureUnitState* texMarker = 
			matMarker->getTechnique(0)->getPass(0)->getTextureUnitState(0);
		//texMarker->setTextureRotate(Ogre::Radian(rotationQuat.getY()));
		texMarker->setTextureRotate(rot);

		/*
		std::stringstream tmpDebugString;
		tmpDebugString << "XPos: " << xPos;
		tmpDebugString << " yPos: " << yPos;
		tmpDebugString << "\n";
		OutputDebugString(tmpDebugString.str().c_str());
		*/

		//This is not really needed if the above code is working
		if(xPos > 1)
			xPos = 1.0f;
		if(yPos > 1)
			yPos = 1.0f;

		carOverlay->setPosition(xPos,yPos);
		//Ogre::Overlay::rota
	}
}

void BigScreen::setMapCorner(Ogre::Vector3 corner)
{
	mapCorner = corner;
}

void BigScreen::setMapSize(Ogre::Vector3 size)
{
	mapSize = size;
}