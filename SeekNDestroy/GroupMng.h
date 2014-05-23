/*Class GroupMng
	The point of this class is to divide the millitary units into groups. One member of the group
	is the appointed the leader, the other units will have to stay close to it. The goal of this class is to provide 
	better control over the unit through delegation.

	Traits:
		member units
		leader
		possition
	Functions:
		add member
		remove member
		appoint leader

*/
#pragma once
#include "StarBard.h"
#include "../rtNEAT/network.h"
#include "VesselMng.h"
class UnitMng;

class GroupMng:public VesselMng
{	
private:
	int groupID;
	std::vector<UnitMng*> groupUnits;
	std::vector<UnitMng*>::iterator groupIter;
	BWAPI::Unit* groupLeader;
	int KillCount,DeathCount;
	double lastHP;
	//Address error Checking alternative options
	//double *inputs,*outputs;
public:
	static int groupInnov;
	// Group initialization
	GroupMng();
	GroupMng(NEAT::Organism* newOrgan);
	GroupMng(UnitMng* member,NEAT::Organism *newOrgan);
	~GroupMng(){};
	BWAPI::Position getPosition();
	//Self explaned
	void addMember(UnitMng* member);
	bool removeMember(UnitMng* member);
	bool removeMember(BWAPI::Unit* member);
	bool appointLeader();
	std::vector<UnitMng*> *getMembers();
	BWAPI::Unit* getLeader();
	std::vector<UnitMng*>::iterator findFittest();
	// Checks whether a unit is meber of the group.
	// Returns an iterator pointing to its position if it is or at the end of the vector otherwise
	std::vector<UnitMng*>::iterator isMember(BWAPI::Unit* member);
	std::vector<UnitMng*>::iterator isMember(UnitMng* member);
	int getSize();
	//help functions
	virtual void issueCommands();
	// Translates the game state into a  double array 
	// in order for the network to be able to handle
	//Same here
	/*double* generateInputs();
	double* generateOutputs(double* inputs);*/
	virtual void generateInputs();
	virtual void generateOutputs();
	//virtual void loadSensors(NEAT::Network *network);
	//Check if a unit can attack an enemy unit-maybe I should have it in double isAttackable(BWAPI::Unit* friendly,BWAPI::Unit* enemy);
	//double isAttackable(BWAPI::Unit* enemy);
	//Check if there is an enemy within weapon range
	BWAPI::Unit* enemyExists(BWAPI::Unit* friendly);
	BWAPI::Unit* enemyExists(BWAPI::Position* groupPos);
	double getLastCommand();
	// Is the group under attack at the moment ?
	double underAttack();
	//Network Functions
	//virtual bool changeOrganism(NEAT::Organism *newOrgan);
	//virtual bool attachBrain(NEAT::Network *network);
	virtual double assessFitness();
	//Commands Macros
	void attack();
	void hold();
	void move();
	void Continue();
	double getHitPoints();
	int GroupKillCount();
	int GroupDeathCount();
	double getDistance(BWAPI::Unit* unit);
	// If there is no enemy within weapon radius, find closest
	BWAPI::Unit* closestEnemy();
	virtual bool isDead();


};

