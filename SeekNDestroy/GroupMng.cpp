#include "GroupMng.h"
// look why I have to add the header twice
//#include "bardAI.h"
#include <vector>
#include <algorithm>
//#include "rtNEAT.h"
#include <BWAPI/Unit.h>
#include <BWAPI/Game.h>
#include <math.h>
#include <set>
//Default Constructor - Creates an empty group
	
int GroupMng::groupInnov=0;
GroupMng::GroupMng():VesselMng()
{	
	++groupInnov;
	groupID=groupInnov;
	BWAPI::Broodwar->printf("Group no %d has been created\n",groupID);
	//Group Leader Init
	groupLeader=NULL;
	inputs=new double[GROUP_INPUTS];
	outputs=new double[GROUP_OUTPUTS];
	KillCount=DeathCount=0;
}
GroupMng::GroupMng(NEAT::Organism* newOrgan):groupID(++groupInnov),VesselMng()
{
	++groupInnov;
	groupID=groupInnov;
	BWAPI::Broodwar->printf("Group no %d has been created\n",groupID);
	organism=newOrgan;
	brain=organism->net;
	//Group Leader Init
	groupLeader=NULL;
	inputs=new double[GROUP_INPUTS];
	outputs=new double[GROUP_OUTPUTS];
	KillCount=DeathCount=0;
}

// Creates Group and adds a new member which is appointed as leader of the group
GroupMng::GroupMng(UnitMng* member,NEAT::Organism *newOrgan):groupID(++groupInnov),VesselMng()
{	
	BWAPI::Broodwar->printf("Group No %d has been created\n",groupID);
	addMember(member);
	//Group Leader Init;
	groupLeader=NULL;
	appointLeader();
	organism=newOrgan;
	brain=organism->net;
	inputs=new double[GROUP_INPUTS];
	outputs=new double[GROUP_OUTPUTS];
	KillCount=DeathCount=0;
}
//Adds a new member to the group
void GroupMng::addMember(UnitMng *member)
{
	groupUnits.push_back(member);
	BWAPI::Broodwar->printf("New member added\n");
	if(getSize()==1)
		appointLeader();

}
//Removing just for removing without deleting object
bool GroupMng::removeMember(UnitMng *member)
{
	std::vector<UnitMng*>::iterator toRemove,toReplace;
	toRemove=isMember(member);
	if(toRemove==groupUnits.end())
	{
		return false;
	}
	else
	{
		toReplace=groupUnits.end();
		toReplace--;
		std::iter_swap(toRemove,toReplace);
		groupUnits.pop_back();
		findFittest();
		return true;
	}
}
bool GroupMng::removeMember(BWAPI::Unit *member)
{
	std::vector<UnitMng*>::iterator toRemove,toReplace;
	toRemove=isMember(member);
	if(toRemove==groupUnits.end())
	{
		return false;
	}
	else
	{
		toReplace=groupUnits.end();
		toReplace--;
		std::iter_swap(toRemove,toReplace);
		groupUnits.pop_back();
		findFittest();
		return true;
	}
}

//Appoints group leader by choosing the most fitest
bool GroupMng::appointLeader()
{
	if(groupUnits.size()>0)
	{
		groupIter=findFittest();
		if(groupIter!=groupUnits.end())
			groupLeader=(*groupIter)->getUnit();
		else
 			groupLeader=NULL;

		if(groupLeader){
			BWAPI::Broodwar->printf("Leader has been appointed\n");
			return true;
		}
		else
		{
			BWAPI::Broodwar->printf("No more live Units in Group\n");
			return false;
		}
	}
	else
	{
		BWAPI::Broodwar->printf("Group is empty");
		return false;
	}
}

std::vector<UnitMng*>::iterator GroupMng::findFittest()
{
	std::vector<UnitMng*>::iterator tempIter,fitIter;
	double maxFit=0;
	fitIter=groupUnits.end();
	for(tempIter=groupUnits.begin();tempIter!=groupUnits.end();tempIter++)
	{

		if((*tempIter)->getOrganism()->fitness >= maxFit&&(*tempIter)->getUnit())
		{
			maxFit=(*tempIter)->getOrganism()->fitness;
			fitIter=tempIter;
		}
	}
	return fitIter;
}


std::vector<UnitMng*>::iterator GroupMng::isMember(BWAPI::Unit *member)
{
	std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();
	for(;UnitIter!=groupUnits.end();UnitIter++)
	{
		if((*UnitIter)->getUnit()==member)
			return UnitIter;
	}
	return UnitIter;
}
std::vector<UnitMng*>::iterator GroupMng::isMember(UnitMng* member)
{
	return std::find(groupUnits.begin(),groupUnits.end(),member);
}

int GroupMng::getSize()
{
	return groupUnits.size(); 
}
//Main function for decision making
void GroupMng::issueCommands()
{
	if(!isDead())
	{
		//double *inputs,*outputs;
		//inputs=generateInputs();
		generateInputs();
		//outputs=generateOutputs(inputs);
		generateOutputs();


		if(outputs[0]>outputs[1]&&outputs[0]>outputs[2])//&&outputs[0]>outputs[3])
		{
			this->act=ATTACK;
			attack();
		}
		else if(outputs[1]>outputs[0]&&outputs[1]>outputs[2])//&&outputs[1]>outputs[3])
		{
			this->act=MOVE;
			move();
		
		}
		else if(outputs[2]>outputs[0]&&outputs[2]>outputs[1])//&&outputs[2]>outputs[3])
		{
			this->act=HOLD;
			hold();
		}
		else
		{ 
			Continue();
		}

	//for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	//	(*UnitIter)->issueCommands();

	}
}

//This is a test
//double *GroupMng::generateInputs()
void GroupMng::generateInputs()
{
	// TODO: Change hardcoded inputs size
	//double inputs[NUM_INPUTS];
	BWAPI::Unit *enemyUnit=0;
	BWAPI::Unit *tempunit=groupLeader;
	
	//Units position
	//Group leader position
	//mapWidth() is in TilePosition Units which is 32 pixels
	inputs[0]=((tempunit->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0));
	inputs[1]=(tempunit->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0);
	//Group size, normalized by 10
	inputs[2]=getSize()/10.0;
	// Group hit points normalized
	inputs[3]=getHitPoints();
	//Previous Command
	inputs[4]=getLastCommand();
	// Closest Enemy Threat
	enemyUnit=enemyExists(tempunit);
	//
	inputs[5]=enemyUnit?1.0:-1.0;
	enemyUnit=closestEnemy();

	inputs[6]=enemyUnit?getDistance(enemyUnit):1.0;
	inputs[7]=underAttack();
	// Can them the enemy be attacked? (If it is airborn some units are unable to attack it )- maybe
	//inputs[6]=isAttackable(enemy_unit);
	
	//Get last command maybe I should or maybe not I have to try both ways due 
	// network memory	
	inputs[8]=1.0;
	//return inputs;
}

//Checks for the closest enemy unit end returns it
BWAPI::Unit* GroupMng::enemyExists(BWAPI::Unit *friendly)
{
	BWAPI::Unit* closestEnemy=0;
	int mindist=10000,dist;
	
	
	//Get units in a radius of three times the weapon range
	std::set<BWAPI::Unit*> &UnitsInRange=BWAPI::Broodwar->getUnitsInRadius(friendly->getPosition(),3*(friendly->getType().groundWeapon().maxRange()));
	// Find Closest enemy
	for (std::set<BWAPI::Unit*>::iterator UnitIter=UnitsInRange.begin();UnitIter!=UnitsInRange.end();UnitIter++)
	{
		if (BWAPI::Broodwar->self()->isEnemy((*UnitIter)->getPlayer()))
		{
			dist=friendly->getDistance((*UnitIter));
			if (dist<mindist)
			{
				closestEnemy=(*UnitIter);
				mindist=dist;
			}
		}
	}
	return closestEnemy;
}

BWAPI::Unit* GroupMng::enemyExists(BWAPI::Position* groupPos)
{
	BWAPI::Unit* closestEnemy=0;
	int mindist=10000,dist;
	
	//Get units in a radius of three times the weapon range
	std::set<BWAPI::Unit*> &UnitsInRange=BWAPI::Broodwar->getUnitsInRadius(*groupPos,3*(groupLeader->getType().groundWeapon().maxRange()));
	// Find Closest enemy
	for (std::set<BWAPI::Unit*>::iterator UnitIter=UnitsInRange.begin();UnitIter!=UnitsInRange.end();UnitIter++)
	{
		if (BWAPI::Broodwar->self()->isEnemy((*UnitIter)->getPlayer()))
		{
			dist=groupPos->getDistance((*UnitIter)->getPosition());
			if (dist<mindist)
			{
				closestEnemy=(*UnitIter);
				mindist=dist;
			}
		}
	}
	return closestEnemy;
}

		
/* 11/10 No time for that. Propably, I have to check is a unit is airborn and whether my unit has the apropriate weapon
	Otherwise I can find the closest unit my unit is able to attack.
// Checks if a unit can attack another unit probably needs to be global function or unit specific
double GroupMng::isAttackable(BWAPI::Unit *enemy)
{
	groupLeader->getType().w
	*/
//Last command is stored in class member act to be used as a normalized input it is divided by 4
double GroupMng::getLastCommand()
{
		return act/4.0;
}

// This function loads the inputs to the network 
//double* GroupMng::generateOutputs(double* inputs)
void GroupMng::generateOutputs()
{	
	std::vector<NEAT::NNode*>::iterator outNodeIter;
	bool check;
	brain->load_sensors(inputs);
	check=brain->activate();
	if(check)
	{
		int count=0;
		for (outNodeIter=brain->outputs.begin();outNodeIter!=brain->outputs.end();outNodeIter++)
		{
			outputs[count]=(*outNodeIter)->activation;
			count++;
		}
	}
	else
		BWAPI::Broodwar->printf("!!!!!OUTPUT ERROR!!!!!");

	//return outputs;
}
			
void GroupMng::attack()
{
	//Probably I should assess where it is better to attack
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		(*UnitIter)->setGroupCommand(act);
	}

}


void GroupMng::move()
{
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		(*UnitIter)->setGroupCommand(act);
	}

}
void GroupMng::hold()
{
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		(*UnitIter)->setGroupCommand(act);
	}
}

BWAPI::Position GroupMng::getPosition()
{
	return groupLeader->getPosition();
}


void GroupMng::Continue()
{
	if(lastCommand==NOCOMMMAND)
	{
		
		lastCommand=MOVE;
		act=lastCommand;
		move();
	}
	else
	{
		act=lastCommand;
		switch (lastCommand)
		{
		case ATTACK:
			attack();
			break;
		case MOVE:
			move();
			break;
		case HOLD:
			hold();
		}
	}

}
double GroupMng::underAttack()
{
	bool att=false;
	std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();
	while ((!att)&&(UnitIter!=groupUnits.end()))
	{
		att=(*UnitIter)->getUnit()?(*UnitIter)->getUnit()->isUnderAttack():false;
		UnitIter++;
	}
	return att?1.0:0;
}
double GroupMng::assessFitness()
{
	if(!isDead()){
		double AttF,nAttF;
		AttF=(ATTACK-act)?-5.0:5.0;
		nAttF=enemyExists(groupLeader)?0:1.0;
	
		KillCount=GroupKillCount();
		DeathCount=GroupDeathCount();
		double tempFit=underAttack()*AttF+(1.0-nAttF)*AttF +10*(1.0-lastHP+getHitPoints())+40*KillCount/(1+DeathCount)+fitness;//+organism->fitness;
		//double tempFit=3*(1.0-lastHP+getHitPoints())+10*KillCount/(1+DeathCount);
		fitness=tempFit<0?0:tempFit;
		lastHP=getHitPoints();
	}
	else
	{
		KillCount=GroupKillCount();
		DeathCount=GroupDeathCount();
		double tempFit=40*KillCount/(1+DeathCount)+fitness;//+organism->fitness;

	}
	timeAlive++;
	return fitness;
}
double GroupMng::getHitPoints()
{
	double averHP=0;
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		if((*UnitIter)->getUnit())
			averHP+=(*UnitIter)->getHitPoints();
	}
	return averHP/groupUnits.size();
}
int GroupMng::GroupKillCount()
{
	int kc=0;
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		kc+=(*UnitIter)->getKillCount();
	}
	return kc;
}
int GroupMng::GroupDeathCount()
{
	int dc=0;
	for(std::vector<UnitMng*>::iterator UnitIter=groupUnits.begin();UnitIter!=groupUnits.end();UnitIter++)
	{
		dc+=(*UnitIter)->getDeathCount();
	}
	return dc;
}

double GroupMng::getDistance(BWAPI::Unit* unit)
{
	double diagMap=(double)(BWAPI::Broodwar->mapHeight()*BWAPI::Broodwar->mapHeight()+BWAPI::Broodwar->mapWidth()*BWAPI::Broodwar->mapWidth());
	int dist=groupLeader->getDistance(unit->getPosition());
	return dist/sqrt(diagMap);
}

std::vector<UnitMng*> *GroupMng::getMembers()
{
	return &groupUnits;
}

BWAPI::Unit* GroupMng::getLeader()
{
	return groupLeader;
}

BWAPI::Unit* GroupMng::closestEnemy()
{
	BWAPI::Unit* closestEnemy=0;
	int mindist=10000,dist;
	
	std::set<BWAPI::Unit*> enemyUnits=BWAPI::Broodwar->enemy()->getUnits();
	// Find Closest enemy
	for (std::set<BWAPI::Unit*>::iterator UnitIter=enemyUnits.begin();UnitIter!=enemyUnits.end();UnitIter++)
	{
		if (BWAPI::Broodwar->self()->isEnemy((*UnitIter)->getPlayer()))
		{
			dist=groupLeader->getDistance((*UnitIter));
			if (dist<mindist)
			{
				closestEnemy=(*UnitIter);
				mindist=dist;
			}
		}
	}
	return closestEnemy;
}

bool GroupMng::isDead()
{
	bool dead=true;
	std::vector<UnitMng*>::iterator unitIter=groupUnits.begin();
	while(dead&&unitIter!=groupUnits.end())
	{
		dead=(*unitIter)->isDead();
		unitIter++;
	}
	return dead;
}
	
