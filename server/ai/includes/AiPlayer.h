#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <string>
#include "SteeringBehaviour.h"
#include "utils.h"
#include "RakNetTypes.h"

using namespace std;
using namespace Ogre;

class SteeringBehaviour;
class Player;
enum CarType;

enum level
{
	easy,
	normal,
	hard
};

class AiPlayer
{
public:
	AiPlayer(string name, Ogre::Vector3 startPos, Ogre::SceneManager* sceneManager, int flags, level diff);
	AiPlayer() {};
	~AiPlayer() {};

    void Spawn();
	void Update(double timeSinceLastFrame);
	Vector3 GetFleeTarget() { return mFleeTarget; };
	Vector3 GetSeekTarget() { return mSeekTarget; };
	Vector3 GetPos();
	float GetMaxSpeed() { return mMaxSpeed; };
	Vector3 GetVelocity() { return mVelocity; };
	Quaternion GetHeading();
	string GetName() { return mName; };
	Vector3 getFeelerPos() { return mFeelerPosition; };
	Vector3 getWallHitPosition(void)const{return mWallHitPosition;}
    Vector3 getWallNormal(void)const{return mWallNormal;}
    std::vector<Vector3> getFeelersPosition(void)const{ return mFeelers;}


private:
	string mName;
	Player* mPlayer;
	Vector3 mFleeTarget;
	Vector3 mSeekTarget;
	float mTimeElapsed;
	std::vector<Vector3> feelers;
	float mWallDetectionFeelerLength;
	Vector3 mWallHitPosition;
	Vector3 mFeelerPosition;
	Vector3 mWallNormal;
	float mMaxSpeed;
	Vector3 mVelocity;
	SteeringBehaviour* mSteeringBehaviour;
	RakNet::Packet* mPacket;
	CarType mCarType;
	double mTolerance;
	std::vector<Vector3> mFeelers;
	void CreateFeelers();
    Vector3 FindWalls();
	double mFeelerDectionLength;
	int turn, direction;
	double targetDistance;
	level difficulty;



};

#endif
