#include "VesselMng.h"
VesselMng::VesselMng()
{
	act=NOCOMMMAND;
	lastCommand=NOCOMMMAND;
	fitness=0;
	brain=NULL;
	organism=NULL;
	KillCount=DeathCount=pastKC=0;
}
bool VesselMng::attachBrain(NEAT::Network *net)
{
	if(brain)
		delete brain;
	brain = net;
	if(brain)
		return true;
	else
		return false;
}
bool VesselMng::changeOrganism(NEAT::Organism *newOrgan)
{
	fitness=0;
	organism=newOrgan;
	pastKC=0;
	DeathCount=0;
	timeAlive=0;
	return attachBrain(organism->gnome->genesis(organism->net->net_id));
}

NEAT::Organism* VesselMng::getOrganism()
{
	return organism;
}
NEAT::Network* VesselMng::getBrain()
{
	return brain;
}
void VesselMng::resetFitness()
{
	fitness=0;
}

double VesselMng::makeInBound(double out)
{
	double inB=out;
	if(inB>1.0)
		inB=1.0;
	else if(inB<0)
		inB=0;
	return inB;
}
double VesselMng::getAngle(int x1, int y1, int x2, int y2)
{
	double abs1,abs2,angle;
	abs1=sqrt((double)(x1*x1+y1*y1));
	abs2=sqrt((double)(x2*x2+y2*y2));
	angle= (x1*x2+y1*y2)/(abs1*abs2);
	return angle;
}

double VesselMng::getNormFitness()
{
	int curFrame = BWAPI::Broodwar->getFrameCount();
	return fitness/timeAlive;
}