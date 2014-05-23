#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "rtNEAT.h"
#include "GroupMng.h"
#include "UnitMng.h"
#include "PopulationMng.h"

#ifndef VERSION
//#define VERSION SND
#define VERSION SYD
#endif

#if VERSION==SND


#ifndef GROUP_INPUTS
#define GROUP_INPUTS 9
#endif

#ifndef GROUP_OUTPUTS
#define GROUP_OUTPUTS 3
#endif

#ifndef UNIT_INPUTS
#define UNIT_INPUTS 20
#endif

#else

#ifndef GROUP_INPUTS
#define GROUP_INPUTS 10
#endif

#ifndef GROUP_OUTPUTS
#define GROUP_OUTPUTS 5
#endif

#ifndef UNIT_INPUTS
#define UNIT_INPUTS 22
#endif

#endif

#ifndef UNIT_OUTPUTS
#define UNIT_OUTPUTS 5
#endif

#ifndef NUM_HIDDEN
#define NUM_HIDDEN 5
#endif


extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class PopulationMng;

class StarBard : public BWAPI::AIModule
{
private:
	BWAPI::Unit* base;
	int num_worker;
	std::vector<GroupMng*> groupList;
	std::vector<UnitMng*> unitList;
	// for experiment purposes - Start of 50 units when a unit dies another takes its place
	std::vector<BWAPI::Unit*> unitReserve,activeUnit;
	//Self describing actually
	int num_of_groups;
	PopulationMng *groupPop;
	PopulationMng *unitPop;
	bool withEvolution;
	//files to check
	char groupFile[200],unitFile[200],gameStat[200],unitStat[200],groupStat[200];
	int run;
public:
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player* player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player* player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit* unit);
	virtual void onUnitEvade(BWAPI::Unit* unit);
	virtual void onUnitShow(BWAPI::Unit* unit);
	virtual void onUnitHide(BWAPI::Unit* unit);
	virtual void onUnitCreate(BWAPI::Unit* unit);
	virtual void onUnitDestroy(BWAPI::Unit* unit);
	virtual void onUnitMorph(BWAPI::Unit* unit);
	virtual void onUnitRenegade(BWAPI::Unit* unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit *unit);
	//Population Initiation
	void initPopulation();
	//Manager Initialization
	void GroupInit(int numUnits);
	void UnitInit(std::vector<BWAPI::Unit*> Units);
	void savePops();
	//Default
	void showGroupLeader();
	void drawStats();
	void printStats(char* filename);
	void printStats(PopulationMng *popMng,char *filename);
	void drawBullets();
	void drawVisibilityData();
	void drawTerrainData();
	void drawTargetLines();
	void showPlayers();
	void showForces();
	bool show_bullets;
	bool show_visibility_data;
	UnitMng* findMng(BWAPI::Unit* unit);
	void releaseMemory();
};