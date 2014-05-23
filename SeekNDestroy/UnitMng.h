/* Class UnitMng 
	This class simulates the AI that controls the unit decisions
	Work on progress */
#pragma once
#include "StarBard.h"
#include "VesselMng.h"
#include <BWAPI/Unit.h>
class GroupMng;
class UnitMng:public VesselMng
{
private:
	int UnitID;
	BWAPI::Unit* conUnit;
	GroupMng *UnitGroup;
	command groupCommand;
	int KillCount,DeathCount,pastKC;
	double lastHP;
public:
	static int UnitInnov;
	UnitMng();
	UnitMng(NEAT::Organism* newOrgan);
	UnitMng(BWAPI::Unit *newUnit,NEAT::Organism *newOrgan);
	~UnitMng(){};
	BWAPI::Unit *getUnit();
	BWAPI::Position getPosition();
	bool setUnit(BWAPI::Unit* unit);
	//Immulates the process of command issueing
	virtual void issueCommands();
	//Network functions
	virtual void generateInputs();
	virtual void generateOutputs();
	//virtual bool loadSensors(NEAT::Network *net);
	virtual double assessFitness();
	void setGroup(GroupMng *newGroup);
	// Each Unit bellongs to a group, this function returns a pointer to that group
	GroupMng *getGroup();
	// Return groupCommand for fitness calculation
	command getGroupCommand();
	// groupCommand for input
	double GroupCommand();
	// Set groupCommand, this will be used as an input to the ANN
	void setGroupCommand(command com);
	//Macro commands
	//The functions below implement the coresponding actions to be followed by the unit
	// Find the nearest enemy and attack it
	void attack();
	// By using an influence map choose the best way to follow - not anymore
	// Use network outputs to define next location to go to 
	void move();
	// Hold position - The unit can attack an enemy in this state
	void hold();
	// Check lastCommand and follow on the same pattern, if there was no last command (NOCOMMAND) move
	// the act, witch shows the current action becomes the action of the lastCommand (or MOVE if NOCOMMAND)
	void Continue();
	// Check and return the nearest enemy within weapon range
	BWAPI::Unit* enemyExists(BWAPI::Unit* friendly);
	// Return lastCommand
	double getLastCommand();
	//Is the unit under attack?
	double underAttack();
	// Return normalized hitpoints
	double getHitPoints();
	// Get normalized distance untill blocked path
	double getBlocked(int vert,int horiz);
	// If there is no enemy within weapon radius, find closest
	BWAPI::Unit* closestEnemy();
	//Get managers killcount - current controled unit's and past's
	int getKillCount();
	int getDeathCount();
	bool isDead();

};
