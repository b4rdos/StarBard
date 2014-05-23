#pragma once
#include "rtNEAT.h"
#include <BWAPI.h>
enum command
{
	NOCOMMMAND=0,
	CONTINUE,
	HOLD,
	ATTACK,
	MOVE
	
};
class VesselMng
{
protected:
	double *inputs,*outputs;
	NEAT::Organism *organism;
	NEAT::Network *brain;
	command act;
	command lastCommand;
	double fitness;
	int KillCount,DeathCount,pastKC;
	int timeAlive;
public:
	VesselMng();
	// Translates the game state into a  double array 
	// in order for the network to be able to handle
	virtual void generateInputs()=0;
	//Loads inputs to network and Translates it into commands
	virtual void generateOutputs()=0;
	//virtual bool loadSensors(NEAT::Network *net)=0;
	// Generic function to issue commands
	virtual void issueCommands()=0;
	virtual double assessFitness()=0;
	virtual double groupProximity()=0;
	virtual bool isDead()=0;
	double getNormFitness();
	bool attachBrain(NEAT::Network *net);
	bool changeOrganism(NEAT::Organism *newOrgan);
	NEAT::Organism* getOrganism();
	NEAT::Network* getBrain();
	virtual double getK2Dratio()=0;
	void resetFitness();
	double makeInBound(double out);
	double getAngle(int x1, int y1, int x2, int y2);

};