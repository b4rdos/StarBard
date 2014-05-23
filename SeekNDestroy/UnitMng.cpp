#include "UnitMng.h"
#include <set>
int UnitMng::UnitInnov=0;
void UnitMng::setGroup(GroupMng *newGroup)
{
	UnitGroup=newGroup;
}
GroupMng *UnitMng::getGroup()
{
	return UnitGroup;
}
void UnitMng::generateInputs()
{
	BWAPI::Unit *enemyUnit=0;
	BWAPI::Unit *tempUnit=getUnit();
	
	//Units position

	//mapWidth() is in TilePosition Units which is 32 pixels
	inputs[0]=((tempUnit->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0));
	inputs[1]=(tempUnit->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0);
	//Group leader position
	inputs[2]=(UnitGroup->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0);
	inputs[3]=(UnitGroup->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0);
	//How many hit points?
	inputs[4]=getHitPoints();
	// Is there an enemy within the unit's seek range?
	enemyUnit=enemyExists(tempUnit);
	inputs[5]=enemyUnit?1.0:0;
	// Find closest enemy
	enemyUnit=closestEnemy();
	inputs[6]=enemyUnit?(UnitGroup->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0):1.0;
	inputs[7]=enemyUnit?(UnitGroup->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0):1.0;
	inputs[8]=underAttack();
	//check surroundings
	//east
	inputs[9]=getBlocked(0,1);
	//north-east
	inputs[10]=getBlocked(1,1);
	//north
	inputs[11]=getBlocked(1,0);
	//north-west
	inputs[12]=getBlocked(1,-1);
	//west
	inputs[13]=getBlocked(0,-1);
	//south-west
	inputs[14]=getBlocked(-1,-1);
	//south
	inputs[15]=getBlocked(-1,0);
	//south-east
	inputs[16]=getBlocked(-1,1);
	//Get Group Command;
	inputs[17]=GroupCommand();
	//get last Command or not maybe useless
	inputs[18]=getLastCommand();	
	//bias
	inputs[19]=1.0;
	//return inputs;
}
void UnitMng::generateOutputs()
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

}
void UnitMng::issueCommands()
{
	if(conUnit)
	{
		//double *inputs,*outputs;
		//inputs=generateInputs();
		generateInputs();
		//outputs=generateOutputs(inputs);
		generateOutputs();
		//Keep Last Command incase of CONTINUE
		lastCommand=act;
		if(outputs[0]>outputs[1]&&outputs[0]>outputs[2])//&&outputs[0]>=outputs[3])
		{
			act=ATTACK;
			attack();
		}
		else if(outputs[1]>outputs[0]&&outputs[1]>outputs[2])//&&outputs[1]>=outputs[3])
		{
			act=MOVE;
			move();
		}
		else if(outputs[2]>outputs[0]&&outputs[2]>outputs[1])//&&outputs[2]>=outputs[3])
		{
			act=HOLD;
			hold();
		}
		else
		{ 
			Continue();
		}
	}

}

BWAPI::Position UnitMng::getPosition()
{
	return conUnit->getPosition();
}

bool UnitMng::setUnit(BWAPI::Unit* unit)
{
	if(conUnit)
		DeathCount++;
	pastKC=KillCount+pastKC;
	conUnit=unit;
	if(conUnit)
	{
		KillCount=conUnit->getKillCount();
		lastHP=getHitPoints();
	}
	else
		KillCount=0;
	
	return true;
}

BWAPI::Unit *UnitMng::getUnit()
{
	return conUnit;
}

void UnitMng::attack()
{
	BWAPI::Unit* enemy=enemyExists(this->conUnit);
	if(enemy)
		this->conUnit->rightClick(enemy);
	else
	{
		enemy=closestEnemy();
		//BWAPI::Broodwar->printf("Unit Cannot attack this unit");
		this->conUnit->rightClick(enemy);
		//this->move();
	}
}
void UnitMng::move()
{
	BWAPI::Position* currentPosition= new BWAPI::Position((this->conUnit)->getPosition());
	BWAPI::Position *movePosition;
	int updown=128*(outputs[3]-0.5);
	int leftright=128*(outputs[4]-0.5);
	movePosition= new BWAPI::Position((currentPosition->x() + updown),(currentPosition->y()+leftright));
	movePosition->makeValid();
	this->conUnit->move(*movePosition);
}
void UnitMng::hold()
{
	this->conUnit->issueCommand(BWAPI::UnitCommand::holdPosition(this->conUnit));
}
void UnitMng::Continue()
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

UnitMng::UnitMng():VesselMng()
{
	UnitInnov++;
	UnitID=UnitInnov;
	inputs=new double[UNIT_INPUTS];
	outputs=new double[UNIT_OUTPUTS];
	conUnit=NULL;
	UnitGroup=NULL;
	groupCommand=NOCOMMMAND;
	KillCount=DeathCount=pastKC=0;
}
UnitMng::UnitMng(NEAT::Organism* newOrgan):UnitID(++UnitInnov),VesselMng()
{
	inputs=new double[UNIT_INPUTS];
	outputs=new double[UNIT_OUTPUTS];
	organism=newOrgan;
	brain=organism->net;
	conUnit=NULL;
	UnitGroup=NULL;
	groupCommand=NOCOMMMAND;
	KillCount=DeathCount=pastKC=0;
}

UnitMng::UnitMng(BWAPI::Unit *newUnit, NEAT::Organism *newOrgan):UnitID(++UnitInnov),VesselMng()
{
	//UnitInnov++;
	//UnitID=UnitInnov;
	inputs=new double[UNIT_INPUTS];
	outputs=new double[UNIT_OUTPUTS];
	organism=newOrgan;
	brain=organism->net;
	conUnit=newUnit;
	UnitGroup=NULL;
	groupCommand=NOCOMMMAND;
	KillCount=DeathCount=pastKC=0;
}




BWAPI::Unit* UnitMng::enemyExists(BWAPI::Unit* friendly)
{
	// If there is no enemy within weapon range this function returns 0
	BWAPI::Unit* closestEnemy=0;
	int mindist=10000,dist;
	//Get units in seek range
	std::set<BWAPI::Unit*> UnitsInRange=BWAPI::Broodwar->getUnitsInRadius(friendly->getPosition(),friendly->getType().seekRange());
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
double UnitMng::getLastCommand()
{
		return act/4.0;
}
void UnitMng::setGroupCommand(command com)
{
	groupCommand=com;
}

command UnitMng::getGroupCommand()
{
	return groupCommand;
}

double UnitMng::GroupCommand()
{
	return groupCommand/4.0;
}

double UnitMng::getHitPoints()
{
	int hitpoints=conUnit->getType().maxHitPoints();
	int maxHP=conUnit->getHitPoints();
	return (double)hitpoints/maxHP;
}


double UnitMng::underAttack()
{
	return conUnit->isUnderAttack()?1.0:0;
}
double UnitMng::assessFitness()
{
	// fitness assess for every command issue 
	// takes into mind if the unit is under attack, if there is an enemy around end the NN 
	// issues an attack command, the kill count the rate it loses HP, if it follows group commands
	// and the organism's fitness. 
	if(conUnit)
	{
		double AttF,ComF;
		double nAttF=enemyExists(conUnit)?1.0:0.0;
		AttF=(ATTACK-act)?-5.0:5.0;
		ComF=(groupCommand-act)?5:-5;
		//Maybe 
		KillCount=conUnit->getKillCount();
		double tempDist=(1-(conUnit->getDistance(UnitGroup->getPosition())/96.0))<0?0:(1-(conUnit->getDistance(UnitGroup->getPosition())/96.0));
		double tempFit=underAttack()*AttF+(1.0-nAttF)*AttF+10*(1.0-lastHP+getHitPoints())+40*(KillCount+pastKC)/(1+DeathCount)+15*tempDist+ComF+fitness;
		//double tempFit=3*(1.0-lastHP+getHitPoints())+10*(KillCount+pastKC)/(1+DeathCount)+ComF+6*tempDist;
		fitness=tempFit<0?0:tempFit;
		lastHP=getHitPoints();
	}
	else
	{
		fitness=40*(KillCount+pastKC)/(1+DeathCount)+fitness;
	}
	timeAlive++;
	return fitness;
}

double UnitMng::getBlocked(int vert,int horiz)
{
	double dist=0;
	BWAPI::Position curPosition=conUnit->getPosition();
	int i=1;
	while(BWAPI::Broodwar->isWalkable(curPosition.x()/8+i*horiz,curPosition.y()/8+i*vert)&&i<33)//&&
		   //BWAPI::Broodwar->getUnitsOnTile(curPosition.x()/32+32*i*horiz,curPosition.x()/32+32*i*vert).empty())
		i++;
	if(i>32)
		dist=1.0;
	else
		dist=i/32.0;
	return dist;
}

BWAPI::Unit* UnitMng::closestEnemy()
{
	BWAPI::Unit* closestEnemy=0;
	int mindist=10000,dist;
	
	std::set<BWAPI::Unit*> enemyUnits=BWAPI::Broodwar->enemy()->getUnits();
	// Find Closest enemy
	for (std::set<BWAPI::Unit*>::iterator UnitIter=enemyUnits.begin();UnitIter!=enemyUnits.end();UnitIter++)
	{
		if (BWAPI::Broodwar->self()->isEnemy((*UnitIter)->getPlayer()))
		{
			dist=conUnit->getDistance((*UnitIter));
			if (dist<mindist)
			{
				closestEnemy=(*UnitIter);
				mindist=dist;
			}
		}
	}
	return closestEnemy;
}

int UnitMng::getKillCount()
{
	KillCount=conUnit?conUnit->getKillCount():0;
	return KillCount+pastKC;
}
int UnitMng::getDeathCount()
{
	return DeathCount;
}
bool UnitMng::isDead()
{
	if(conUnit)
		return false;
	else
		return true;
}