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
	if(conUnit)
	{
		BWAPI::Unit *enemyUnit=0;
		BWAPI::Unit *tempUnit=getUnit();
	
		//Units position

		//mapWidth() is in TilePosition Units which is 32 pixels
		inputs[0]=((tempUnit->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0));
		inputs[1]=(tempUnit->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0);
		//Group position
		inputs[2]=(UnitGroup->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0);
		inputs[3]=(UnitGroup->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0);
		//Group Target Position
		const BWAPI::Position* groupTarget=UnitGroup->getTarget();
		inputs[4]=groupTarget->x()/(BWAPI::Broodwar->mapWidth()*32.0);
		inputs[5]=groupTarget->y()/(BWAPI::Broodwar->mapHeight()*32.0);
		//How many hit points?
		inputs[6]=getHitPoints();
		// Is there an enemy within the weapon's range?
		enemyUnit=enemyExists(tempUnit);
		inputs[7]=enemyUnit?1.0:0;
		enemyUnit=closestEnemy();
		inputs[8]=enemyUnit?(enemyUnit->getPosition().x())/(BWAPI::Broodwar->mapWidth()*32.0):1.0;
		inputs[9]=enemyUnit?(enemyUnit->getPosition().y())/(BWAPI::Broodwar->mapHeight()*32.0):1.0;
		inputs[10]=underAttack();
		//check surroundings
		//east
		inputs[11]=getBlocked(0,1);
		//south-east
		inputs[12]=getBlocked(1,1);
		//south
		inputs[13]=getBlocked(1,0);
		//south-west
		inputs[14]=getBlocked(1,-1);
		//west
		inputs[15]=getBlocked(0,-1);
		//north-west
		inputs[16]=getBlocked(-1,-1);
		//north
		inputs[17]=getBlocked(-1,0);
		//north-east
		inputs[18]=getBlocked(-1,1);
		//Get Group Command;
		inputs[19]=GroupCommand();
		//get last Command or not maybe useless
		inputs[20]=getLastCommand();	
		//bias
		inputs[21]=1.0;
		//return inputs;
	}
}
void UnitMng::generateOutputs()
{
	if(conUnit)
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
		move();
}
void UnitMng::move()
{
	BWAPI::Position* currentPosition= new BWAPI::Position((this->conUnit)->getPosition());
	BWAPI::Position *movePosition;
	int updown=128*(makeInBound(outputs[3])-0.5);
	int leftright=128*(makeInBound(outputs[4])-0.5);
	movePosition= new BWAPI::Position((currentPosition->x() + updown),(currentPosition->y()+leftright));
	//int x=BWAPI::Broodwar->mapWidth()*outputs[3];
	//int y=BWAPI::Broodwar->mapHeight()*outputs[4];
	//movePosition = new BWAPI::Position(x,y);
	/*if(!search)
	{
	
	movePosition= new BWAPI::Position((currentPosition->x() + updown),(currentPosition->y()+leftright));

	}
	else
	{
	AUXILIARY::InfluenceMap infMap(this->conUnit);
	int size=infMap.getLength();
	int GoTo=infMap.assessMap(&infMap);
	switch (GoTo)
	{
		
		case 0:
			movePosition=new BWAPI::Position(currentPosition->x()+ updown,(currentPosition->y()+(size-1)*8+leftright));
			break;
		case 1:
			movePosition= new BWAPI::Position((currentPosition->x() +(size-1)*8+ updown),(currentPosition->y()+(size-1)*8+leftright));
			break;
		case 2:
			movePosition=new BWAPI::Position((currentPosition->x()+(size-1)*8+ updown),currentPosition->y()+leftright);
			break;
		case 3:
			movePosition=new BWAPI::Position(currentPosition->x()+ updown,(currentPosition->y()-(size-1)*8+leftright));
			break;
		case 4:
			movePosition=new BWAPI::Position(currentPosition->x()+ updown,(currentPosition->y()-(size-1)*8+leftright));
			break;
		case 5:
			movePosition=new BWAPI::Position((currentPosition->x()-(size-1)*8+ updown),(currentPosition->y()-(size-1)*8+leftright));
			break;
		case 6:
			movePosition=new BWAPI::Position((currentPosition->x()-(size-1)*8+ updown),currentPosition->y()+leftright);
			break;
		default:
			movePosition=new BWAPI::Position((currentPosition->x()-(size-1)*8+ updown),(currentPosition->y()+(size-1)*8+leftright));
	}
	}*/
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
	brain=organism->gnome->genesis(organism->net->net_id);
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
	brain=organism->gnome->genesis(organism->net->net_id);
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
	//Get units in weapon range
	if(friendly)
	{
		std::set<BWAPI::Unit*> UnitsInRange=BWAPI::Broodwar->getUnitsInRadius(friendly->getPosition(),friendly->getType().sightRange());
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
		//compute target factor
		const BWAPI::Position* groupTarget=UnitGroup->getTarget();
		double targFac=getAngle(conUnit->getVelocityX(),groupTarget->x()-conUnit->getPosition().x(),groupTarget->x()-conUnit->getPosition().x(),groupTarget->y()-conUnit->getPosition().y());
		//Maybe 
		KillCount=conUnit->getKillCount();
		double tempDist=groupProximity();
		double tempFit=5*(1.0-lastHP+getHitPoints())+45*(KillCount+pastKC)/(1+DeathCount)+15*tempDist+fitness+10*targFac;
		fitness=tempFit<0?0:tempFit;
		lastHP=getHitPoints();
	}
	else
	{
		fitness=45*(KillCount+pastKC)/(1+DeathCount)+fitness;
	}
	timeAlive++;
	return fitness;
}

double UnitMng::getBlocked(int vert,int horiz)
{
	double dist=0;
	BWAPI::Position curPosition=conUnit->getPosition();
	int i=1;
	while(BWAPI::Broodwar->isWalkable(curPosition.x()/8+i*horiz,curPosition.y()/8+i*vert)&&i<33&&BWAPI::Broodwar->getUnitsOnTile(curPosition.x()/32+32*i*horiz,curPosition.y()/32+32*i*vert).empty())
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


double UnitMng::groupProximity()
{
	if(conUnit)
		return (1-(conUnit->getDistance(UnitGroup->getPosition())/128.0))<0?0:(1-(conUnit->getDistance(UnitGroup->getPosition())/128.0));
	else 
		return 0;
}

double UnitMng::getK2Dratio()
{
	return (getKillCount()/(getDeathCount()+1.0));
}

bool UnitMng::isDead()
{
	if(conUnit)
		return false;
	else
		return true;
}