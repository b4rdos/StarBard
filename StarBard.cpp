/* StarBard
	Η παρακάτω κλάση άποτελεί την υλοποίηση της διεπαφής με το παιχνίδι
	Starcraft. Επικαλύπτωντας τις συναρτήσεις που κληρoνομεί από την 
	κλάση AIModule, ο πράκτορας StarBard, λαμβάνει πληροφορίες για το
	την κατάσταση του παιχνιδιού και λαμβάνει αποφάσεις για τις επόμενες
	κινήσεις.

	Οι κύριες συναρτήσεις που εκμεταλευόμαστε είναι οι:
		1. onStart()	- Αρχικοποίηση 
		2. onFrame()	- Ενέργειες κατά την εκτέλεση του παιχνιδιού
		3. onEnd()		- Ενέργειες κατά τη λήξη του παιχνιδιού
		4. onUnitDestroy() - Τι γίνεται όταν μία μονάδα καταστραφεί

	Περισσότερες λεπτομέριες ως προς τη λειτουργία του πράκτορα υπάρχουν στα 
	σχολιασμένα κομμάτια του κώδικα και στο κείμενο της διπλωματικής εργασίας.

*** Το αρχείο αυτό αποτελεί μέρος του κώδικα που χρησιμοποιήθηκε για την 
*** εκπώνηση της διπλωματικής εργασίας :
***		" Αυτόνομη Δημιουργία Τακτικών Μάχης σε Παχνλιδια Στρατηγικής 
***		  Πραγματικού Χρόνου με Χρήση Νευροεξέλιξης "
*** από τον προπτυχιακό φοιτητή του Τμήματος Ηλεκτρολόγων Μηχανικών 
***	και Μηχανικών Υπολογιστών του Αριστοτελείου Πανεπιστημίου Θεσσαλονίκης,
*** Κωνσταστίνο Αναστασίου
	
*/


#include "StarBard.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "time.h"

using namespace BWAPI;
using namespace NEAT;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;
//This function is meant for generating automatic restarts of the game
//  sec after the previous game has ended a new one will start
void autoRestart()
{
	 char procName[200];
  sprintf_s(procName, 
    "StarBard\\scripts\\startGame.exe");
  // Initialize StartupInfo structure
  STARTUPINFO    StartupInfo;
  memset(&StartupInfo, 0, sizeof(StartupInfo));
  StartupInfo.cb = sizeof(StartupInfo);

  // This will contain the information about the newly created process
  PROCESS_INFORMATION ProcessInformation;

  BOOL results = CreateProcess(0,
         procName, // Process and arguments
         0, // Process Attributes
         0, // Thread Attributes
         FALSE, // Inherit Handles
         0, // CreationFlags,
         0, // Environment
         0, // Current Directory
         &StartupInfo, // StartupInfo
         &ProcessInformation // Process Information
         );
  // Cleanup
  CloseHandle(ProcessInformation.hProcess);
  CloseHandle(ProcessInformation.hThread);

}
bool orderUnit(BWAPI::Unit* u1,BWAPI::Unit* u2);
void StarBard::onStart()
{
	// Should the agents learn?
	withEvolution=true;
	//Load NEAT parameters file
	char param[50];
	sprintf_s(param,50,"StarBard\\paramFile.ne");
	NEAT::load_neat_params(param,false);
	int activeCnt=0;
	this->num_of_groups=0;
	// the Broodwar object is representing the current game and the interface with Stracraft
	Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	
	Broodwar->setLocalSpeed(0);
	Broodwar->enableFlag(Flag::UserInput);
	Broodwar->enableFlag(Flag::CompleteMapInformation);
	
	BWTA::readMap();
	analyzed=false;
	analysis_just_finished=false;

	show_bullets=false;
	show_visibility_data=false;
	// Get enemies name for Polulation Generation
	sprintf_s(groupFile,200,"StarBard\\groupPop_%s",(*Broodwar->enemies().begin())->getName().c_str());
	sprintf_s(unitFile,200,"StarBard\\unitPop_%s",(*Broodwar->enemies().begin())->getName().c_str());
	// Create experiment specific stat file
	char experiment[200];
	sprintf_s(experiment,200,"_%s_VS_%s_%s.dat",Broodwar->self()->getName().c_str(),Broodwar->enemy()->getName().c_str(),Broodwar->mapFileName().c_str());
	std::string *exper = new std::string(experiment,200);
	sprintf_s(gameStat,200,"StarBard//stats//gameStat%s",exper->c_str());
	sprintf_s(groupStat,200,"StarBard//stats//groupStat%s",exper->c_str());
	sprintf_s(unitStat,200,"StarBard//stats//unitStat%s",exper->c_str());
	//Population initilisation
	initPopulation();
	//Go through all units
	if (Broodwar->isReplay())
	{
		Broodwar->printf("The following players are in this replay:");
		for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
		{
			if (!(*p)->getUnits().empty()&&!(*p)->isNeutral())
			{
				Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
			}
		}
	}
	else
		//Live play
	{
		Broodwar->printf("The match is up is %s vs %s",Broodwar->self()->getRace().getName().c_str(),Broodwar->enemy()->getRace().getName().c_str());

		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
		if(!(*i)->getType().isBuilding()&&!(*i)->getType().isWorker())
			{
				 //if not a building or a worker, then it is a soldier
				unitReserve.push_back((*i));
			}

		}

	}
	std::sort(unitReserve.begin(),unitReserve.end(),orderUnit);
	for(int i=0;i<50;i++)
	{
		BWAPI::Unit *unit = (*unitReserve.rbegin());
		activeUnit.push_back(unit);
		unitReserve.pop_back();
	}

	GroupInit(activeUnit.size());
	UnitInit(activeUnit);
	//analyze map
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
	
	//increase experiment count
	run=0;
	std::fstream runFile;
	runFile.open("StarBard\\runNum",std::ios::in);
	if(runFile.is_open())
	{
		char line[128];
		runFile.getline(line,sizeof(line));
		std::stringstream ss(line);
		ss >> run;
		runFile.close();
	}
	++run;

 }


void StarBard::onEnd(bool isWinner)
{
	//save stats and results
	if(isWinner)
	{
		char tempFile[50];
		sprintf_s(tempFile,50,"StarBard\\winner\\groupPop_%d",run);
		groupPop->savePopulation(tempFile);
		sprintf_s(tempFile,50,"StarBard\\winner\\unitPop_%d",run);
		unitPop->savePopulation(tempFile);
	}
	// game Stats 
	char gameStats[100];
	sprintf_s(gameStats,100,"StarBard\\stats\\gameStats.dat");
	printStats(gameStats);
	// Refresh run
	std::fstream runFile;
	runFile.open("StarBard\\runNum",std::ios::out);
	if(runFile.is_open())
		runFile<<run<<std::endl;
	runFile.close();
	releaseMemory();
	if(run<300)
		autoRestart();
	
}

void StarBard::onFrame()
{
	
  if (show_visibility_data)
    drawVisibilityData();

  if (show_bullets)
    drawBullets();

  if (Broodwar->isReplay())
    return;

  drawTargetLines();
  showGroupLeader();

  drawStats();
  if (analyzed && Broodwar->getFrameCount()%30==0)
  {
    //order one of our workers to guard our chokepoint.
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
      if ((*i)->getType().isWorker())
      {
        //get the chokepoints linked to our home region
        std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
        double min_length=10000;
        BWTA::Chokepoint* choke=NULL;

        //iterate through all chokepoints and look for the one with the smallest gap (least width)
        for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++)
        {
          double length=(*c)->getWidth();
          if (length<min_length || choke==NULL)
          {
            min_length=length;
            choke=*c;
          }
        }

        //order the worker to move to the center of the gap
        (*i)->rightClick(choke->getCenter());
        break;
      }
    }
  }
  if (analyzed)
    drawTerrainData();

  if (analysis_just_finished)
  {
    Broodwar->printf("Finished analyzing map.");
    analysis_just_finished=false;
  }

  //Added byme on 28/9
  //Issuing commands every 30 frames
  
  if(Broodwar->getFrameCount()>10000)
  {
	  //savePops();
	  Broodwar->leaveGame();
  }
  //First Commands
  if(Broodwar->getFrameCount()==15)
  {
	for(std::vector<GroupMng*>::iterator groupIter=groupList.begin();groupIter!=groupList.end();groupIter++)
	{
		  (*groupIter)->issueCommands();
		  for(std::vector<UnitMng*>::iterator unitIter=(*groupIter)->getMembers()->begin();unitIter!=(*groupIter)->getMembers()->end();unitIter++)
		  {
			  (*unitIter)->issueCommands();
		  }
	}
  }
  //Issue Unit Commands
  if(!(Broodwar->getFrameCount()%30)&&Broodwar->getFrameCount()>0)
  {
	  for(std::vector<GroupMng*>::iterator groupIter=groupList.begin();groupIter!=groupList.end();groupIter++)
		  for(std::vector<UnitMng*>::iterator unitIter=(*groupIter)->getMembers()->begin();unitIter!=(*groupIter)->getMembers()->end();unitIter++)
		  {
			  (*unitIter)->issueCommands();
		  }
  }
  //Compute Unit Fitness Every 30 frames, 5 frames after command issuing
  if(withEvolution&&(Broodwar->getFrameCount()%30==5)&&Broodwar->getFrameCount()>30)
  {
	  unitPop->evaluatePop();
	  printStats(unitPop,unitStat);
  }
  // Evolve Unit Population And Save Stats
  if(withEvolution&&!(Broodwar->getFrameCount()%60)&&Broodwar->getFrameCount()>0)
  {
	  if(unitPop->evolve())
	  {
		  unitPop->savePopulation(unitFile);
	  }
  }
  //Issue Group Commands Every 90 frames
  if(!(Broodwar->getFrameCount()%90)&&Broodwar->getFrameCount()>0)
  {
	for(std::vector<GroupMng*>::iterator groupIter=groupList.begin();groupIter!=groupList.end();groupIter++)
	{
		  (*groupIter)->issueCommands();
	}
  }
  // Evaluate Group Population Every 90 Frames
  if(withEvolution&&(Broodwar->getFrameCount()%90==15)&&Broodwar->getFrameCount()>90)
  {
	  groupPop->evaluatePop();
	  printStats(groupPop,groupStat);
  }
	  


  //Evolve Group Population every 450 frames
  if(withEvolution&&!(Broodwar->getFrameCount()%450)&&Broodwar->getFrameCount()>150)
  {
	  if(groupPop->evolve())
		  groupPop->savePopulation(groupFile);
  }
}

void StarBard::onSendText(std::string text)
{
  if (text=="/show bullets")
  {
    show_bullets = !show_bullets;
  } else if (text=="/show players")
  {
    showPlayers();
  } else if (text=="/show forces")
  {
    showForces();
  } else if (text=="/show visibility")
  {
    show_visibility_data=!show_visibility_data;
  } else if (text=="/analyze")
  {
    if (analyzed == false)
    {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }
  }
  else if(text == "/leave game")
  {
	  //savePops();
	  Broodwar->leaveGame();
  } else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
    Broodwar->sendText("%s",text.c_str());
  }
}

void StarBard::onReceiveText(BWAPI::Player* player, std::string text)
{
  Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void StarBard::onPlayerLeft(BWAPI::Player* player)
{
  Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void StarBard::onNukeDetect(BWAPI::Position target)
{
  if (target!=Positions::Unknown)
    Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
  else
    Broodwar->printf("Nuclear Launch Detected");
}

void StarBard::onUnitDiscover(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
    Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void StarBard::onUnitEvade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
    Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void StarBard::onUnitShow(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
    Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void StarBard::onUnitHide(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
    Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}

void StarBard::onUnitCreate(BWAPI::Unit* unit)
{
  if (Broodwar->getFrameCount()>1)
  {
    if (!Broodwar->isReplay())
      Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
    else
    {
      /*if we are in a replay, then we will print out the build order
      (just of the buildings, not the units).*/
      if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
      {
        int seconds=Broodwar->getFrameCount()/24;
        int minutes=seconds/60;
        seconds%=60;
        Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
	  }
	  

    }
  }
}

void StarBard::onUnitDestroy(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	if(unit->getPlayer()==Broodwar->self())
	{
		UnitMng* curMng=findMng(unit);
		if(curMng)
		{	// If the destroyed unit is one of mine...
			if(!unitReserve.empty())
			{		
				// Take last unit in unitReserve
				std::vector<BWAPI::Unit*>::reverse_iterator unitIter=unitReserve.rbegin();
				// Change controled unit 
				curMng->setUnit((*unitIter));
				// Does Not matter if the group leader is dead or not appoint a new one
				curMng->getGroup()->appointLeader();
				std::vector<BWAPI::Unit*>::iterator deadUnit=std::find(activeUnit.begin(),activeUnit.end(),unit);
				// Remove dead unit from activeUnit vector
				activeUnit.erase(deadUnit);
				// Add new unit to activeUnit vector
				activeUnit.push_back((*unitIter));
				// Remove newly activated unit from reserve
				unitReserve.pop_back();
			}
			else
			{
				// Should I stop? check if all organisms have a live unit to represent?
				// I will try stopping and printing pop to file
				//savePops();
				//Broodwar->leaveGame();

				//Just Remove from active unit vector and put NULL pointer to unitMng
				curMng->setUnit(NULL);
				//curMng->getGroup()->removeMember(curMng);

				curMng->getGroup()->appointLeader();
				std::vector<BWAPI::Unit*>::iterator deadUnit=std::find(activeUnit.begin(),activeUnit.end(),unit);
				// Remove dead unit from activeUnit vector
				activeUnit.erase(deadUnit);

				
			}
		}
	}
  }
}

void StarBard::savePops()
{
	
	//Save populations
	unitPop->savePopulation(unitFile);
	groupPop->savePopulation(groupFile);
	//save stats no matter what
	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	strftime (buffer,80,"StarBard\\pastTests\\groupPop%d%m%y_%H%M",timeinfo);
	groupPop->savePopulation(buffer);
	strftime (buffer,80,"StarBard\\pastTests\\unitPop%d%m%y_%H%M",timeinfo);
	unitPop->savePopulation(buffer);
	if (Broodwar->self()->isVictorious())
	{
		//save Pops as is
		char tempFile[50];
		sprintf_s(tempFile,50,"StarBard\\groupPop");
		groupPop->savePopulation(tempFile);
		sprintf_s(tempFile,50,"StarBard\\unitPop");
		unitPop->savePopulation(tempFile);
	}

}



void StarBard::onUnitMorph(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
    }
  }
}

void StarBard::onUnitRenegade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

void StarBard::onSaveGame(std::string gameName)
{
  Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread()
{
  BWTA::analyze();

  //self start location only available if the map has base locations
  if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
  {
    home       = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
  }
  //enemy start location only available if Complete Map Information is enabled.
  if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
  {
    enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
  }
  analyzed   = true;
  analysis_just_finished = true;
  return 0;
}

void StarBard::drawStats()
{
  std::set<Unit*> myUnits = Broodwar->self()->getUnits();
  Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
  {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
    {
      unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
  Broodwar->drawTextScreen(5,16*line,"Active Units: %d ",activeUnit.size());
  line++;
  //added by me
  Broodwar->drawTextScreen(5,16*line,"- This is frame %d", Broodwar->getFrameCount());
  line++;
  Broodwar->drawTextScreen(5,16*line,"- There are %d Groups and %d organisms in group population",groupList.size(),groupPop->getPop()->organisms.size());
  line++;
  Broodwar->drawTextScreen(5,16*line,"Group Population Offsprings: %d ",groupPop->getOffspringNum());
  line++;
  Broodwar->drawTextScreen(5,16*line,"Unit Population Offsprings: %d ",unitPop->getOffspringNum());
  line++;

}

void StarBard::drawBullets()
{
  std::set<Bullet*> bullets = Broodwar->getBullets();
  for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();i++)
  {
    Position p=(*i)->getPosition();
    double velocityX = (*i)->getVelocityX();
    double velocityY = (*i)->getVelocityY();
    if ((*i)->getPlayer()==Broodwar->self())
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
      Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
    }
    else
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
      Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
    }
  }
}

void StarBard::drawVisibilityData()
{
  for(int x=0;x<Broodwar->mapWidth();x++)
  {
    for(int y=0;y<Broodwar->mapHeight();y++)
    {
      if (Broodwar->isExplored(x,y))
      {
        if (Broodwar->isVisible(x,y))
          Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
        else
          Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
      }
      else
        Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
    }
  }
}

void StarBard::drawTerrainData()
{
  //we will iterate through all the base locations, and draw their outlines.
  for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
  {
    TilePosition p=(*i)->getTilePosition();
    Position c=(*i)->getPosition();

    //draw outline of center location
    Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

    //draw a circle at each mineral patch
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
    {
      Position q=(*j)->getInitialPosition();
      Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
    }

    //draw the outlines of vespene geysers
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
    {
      TilePosition q=(*j)->getInitialTilePosition();
      Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
    }

    //if this is an island expansion, draw a yellow circle around the base location
    if ((*i)->isIsland())
      Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
  }

  //we will iterate through all the regions and draw the polygon outline of it in green.
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    BWTA::Polygon p=(*r)->getPolygon();
    for(int j=0;j<(int)p.size();j++)
    {
      Position point1=p[j];
      Position point2=p[(j+1) % p.size()];
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
    }
  }

  //we will visualize the chokepoints with red lines
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
    {
      Position point1=(*c)->getSides().first;
      Position point2=(*c)->getSides().second;
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
    }
  }
}


void StarBard::drawTargetLines()
{
	std::vector<BWAPI::Unit*>::iterator unitIter=activeUnit.begin();
	while(unitIter!=activeUnit.end())
	{
		BWAPI::Unit* tmpUnit=(*unitIter);
		BWAPI::Position unitPosition=tmpUnit->getPosition();
		BWAPI::Position targetPosition=tmpUnit->getTargetPosition();
		Broodwar->drawLineMap(unitPosition.x(),unitPosition.y(),targetPosition.x(),targetPosition.y(),Colors::Green);
		unitIter++;
	}
}
void StarBard::showPlayers()
{
  std::set<Player*> players=Broodwar->getPlayers();
  for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
  {
    Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
  }
}

void StarBard::showForces()
{
  std::set<Force*> forces=Broodwar->getForces();
  for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
  {
    std::set<Player*> players=(*i)->getPlayers();
    Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
    for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
    {
      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
    }
  }
}

void StarBard::showGroupLeader()
{
	for (std::vector<GroupMng*>::iterator groupIter=groupList.begin();groupIter!=groupList.end();groupIter++)
	{
		BWAPI::Unit* leader=(*groupIter)->getLeader();
		if(leader)
			Broodwar->drawCircleMap(leader->getPosition().x(),leader->getPosition().y(),5,BWAPI::Colors::Green);
	}
	// Draw an ellipse around active units - different color for every group
	int teamColor=0;
	int teamCount=0;
	for (std::vector<GroupMng*>::iterator groupIter=groupList.begin();groupIter!=groupList.end();groupIter++)
	{
		++teamCount;
		teamColor=teamCount*256/groupList.size();
		for (std::vector<UnitMng*>::iterator unitIter=(*groupIter)->getMembers()->begin();unitIter!=(*groupIter)->getMembers()->end();++unitIter)
		{
			if((*unitIter)->getUnit())
			{
				Broodwar->drawEllipseMap( (*unitIter)->getUnit()->getPosition().x(),(*unitIter)->getUnit()->getPosition().y(),10,5,BWAPI::Color(teamColor));
			}
		}
	}
}
void StarBard::onUnitComplete(BWAPI::Unit *unit)
{
	if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1){
		Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	}
}
void StarBard::initPopulation()
{
	char filename[50];
	std::ifstream iFile(groupFile);
	if (!iFile.is_open()) {
		sprintf_s(filename,50,"StarBard\\groupPop");
		iFile.open(filename);
		if(!iFile.is_open()){
			//Random Group population generation 
			this->groupPop = new PopulationMng(GROUP_INPUTS,GROUP_OUTPUTS,NUM_HIDDEN,10);
		}
		else
		{
			iFile.close();
			this->groupPop = new PopulationMng(filename);
		}

	}
	else
	{
		iFile.close();
		this->groupPop = new PopulationMng(groupFile);
	}
	
	iFile.open(unitFile);
	if (!iFile.is_open()) {
		sprintf_s(filename,50,"StarBard\\unitPop");
		iFile.open(filename);
		if(!iFile.is_open()){
		//Random Unit population generation
		this->unitPop = new PopulationMng(UNIT_INPUTS,UNIT_OUTPUTS,NUM_HIDDEN,25);
		}
		else
		{
			iFile.close();
			this->unitPop = new PopulationMng(filename);
		}

	}
	else
	{
		iFile.close();
		this->unitPop = new PopulationMng(unitFile);
	}
}

void StarBard::GroupInit(int numUnits)
{
	//Attach brain to groups, !!need to think if the number of units is greater than number of population
	//No need they will never be :)
	if(numUnits==0)
		Broodwar->printf("ERROR:No units to control");
	else
	{
		for(std::vector<NEAT::Species*>::iterator SpecIter=(groupPop->getPop())->species.begin();SpecIter!=(groupPop->getPop())->species.end();SpecIter++)
		{
			for(std::vector<NEAT::Organism*>::iterator OrganIter=(*SpecIter)->organisms.begin();OrganIter!=(*SpecIter)->organisms.end();OrganIter++)
			{
				// Appoint a group manager to each organism - means that the groups are always ten
				// if they become less I need to think of a way to make sure that every net is evaluated
				GroupMng *tempGroup=new GroupMng();
				tempGroup->changeOrganism((*OrganIter));
				groupList.push_back(tempGroup);
				groupPop->createAssociation((*OrganIter),tempGroup);
			}
		}
	}
}

void StarBard::UnitInit(std::vector<BWAPI::Unit*> Units)
{
	
	if(Units.size()==0)
		Broodwar->printf("Error: No Units to control");
	else
	{
		int cntGroup,cntSpec;
		cntGroup=cntSpec=0;
		NEAT::Population* pop=unitPop->getPop();
		int numGroups=groupList.size();
		int numSpec=unitPop->getPop()->species.size();
		std::vector<Organism*>::iterator organIter=pop->species[cntSpec%numSpec]->organisms.begin();
		std::vector<BWAPI::Unit*>::iterator UnitIter=Units.begin();
		while(UnitIter!=Units.end())
		{
			// Assign all Units to groups circularly and organisms to Units as well by using modular.
			if(organIter!=pop->species[cntSpec%numSpec]->organisms.end())
			{
				UnitMng* tempUnit=new UnitMng((*UnitIter),(*organIter));
				tempUnit->setGroup(groupList[cntGroup%numGroups]);
				unitList.push_back(tempUnit);
				groupList[cntGroup%numGroups]->addMember(tempUnit);
				unitPop->createAssociation((*organIter),tempUnit);
				UnitIter++;
				organIter++;
				cntGroup++;
			}
			else
			{
				cntSpec++;
				organIter=pop->species[cntSpec%numSpec]->organisms.begin();
			}
		}
	}
}

UnitMng* StarBard::findMng(BWAPI::Unit * unit)
{
	UnitMng *mngPoint=NULL;
	// if it is in the activeUnit than it is one of mine
	std::vector<BWAPI::Unit*>::iterator unitIter=activeUnit.begin();
	bool stop=false;
	while(unitIter!=activeUnit.end()&&!stop)
	{
		if((*unitIter)==unit)
		{
			stop=true;
			continue;
		}
		unitIter++;
	}
	bool found=false;
	if(unitIter!=activeUnit.end())
	{
		std::vector<UnitMng*>::iterator mngIter=unitList.begin();
		while (!found && mngIter!=unitList.end())
		{
			if(((*mngIter)->getUnit())==unit)
			{
				found=true;
				mngPoint=(*mngIter);
				continue;
			}
			mngIter++;
		}
	}
	return mngPoint;
}

void StarBard::printStats(char *filename)
{
	std::fstream statFile(filename,std::ios::out|std::ios::app);
	if(statFile.is_open())
	{	
		int kc=Broodwar->self()->killedUnitCount(BWAPI::UnitTypes::Terran_Marine);
		int dc=Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Terran_Marine);
		double ratio=kc/(dc+1.0);
		statFile<<run<<" "<<ratio<<" "<< Broodwar->self()->getKillScore()<<" "<<Broodwar->enemy()->getKillScore()<<" "<<Broodwar->getFrameCount()<<std::endl;
	}
}

void StarBard::printStats(PopulationMng *popMng,char *filename)
{
	std::fstream StFile(filename,std::ios::out|std::ios::app);
	if(StFile.is_open())
	{
		//NEAT::Population *pop=popMng->getPop();
		/*for(std::vector<NEAT::Species*>::iterator spIter=pop->species.begin();spIter!=pop->species.end();spIter++)
		{
			NEAT::Organism* organ=(*spIter)->get_champ();
			std::vector<VesselMng*> *vessels = popMng->getVessel(organ);
			double prox=0,K2Dratio=0;
			if(vessels)
			{
				int count=0;
				for(std::vector<VesselMng*>::iterator vesIter=vessels->begin();vesIter!=vessels->end();vesIter++)
				{
					prox+=(*vesIter)->groupProximity();
					K2Dratio+=(*vesIter)->getK2Dratio();
					count++;
				}
				prox/=count;
				K2Dratio/=count;
			}
			int offCount=popMng->getOffspringNum();
			// For each species print to file offspring num id average proximity of champ controled units
			// average kill to death ratio and organisms fitness
			StFile<<run<<" "<<offCount<<" "<<(*spIter)->id<<" "<<prox<<" "<<K2Dratio<<" "<<organ->fitness<<std::endl;
		}*/

		double maxAver=-1;
		double maxFit = -1;
		int nodNum = 0;
		NEAT::Organism *bestOrgan=NULL;
		for(std::vector<NEAT::Species*>::iterator specIter=popMng->getPop()->species.begin();specIter!=popMng->getPop()->species.end();specIter++)
		{
			NEAT::Organism *champ = (*specIter)->get_champ();
			if(maxAver<(*specIter)->average_est)
				maxAver=(*specIter)->average_est;
			if(maxFit< champ->fitness)
			{
				maxFit=champ->fitness;
				bestOrgan=champ;
			}
		}
		int offCount = popMng->getOffspringNum();
		StFile<<run<<" "<<offCount<<" "<<maxAver<<" "<<maxFit<<" "<<std::endl;
		StFile.close();
	}

}

bool orderUnit(BWAPI::Unit *u1,BWAPI::Unit* u2)
{
	BWAPI::Position enemyStart = BWAPI::Position(Broodwar->enemy()->getStartLocation());
	bool order = u1->getDistance(enemyStart)>u2->getDistance(enemyStart);
	return order;
}

void StarBard::releaseMemory()
{
	delete groupPop;
	delete unitPop;
	for(std::vector<GroupMng*>::iterator groupIter = groupList.begin();groupIter!=groupList.end();groupIter++ )
		delete *groupIter;

	for(std::vector<UnitMng*>::iterator unitIter = unitList.begin();unitIter!=unitList.end();unitIter++ )
		delete *unitIter;

	unitReserve.clear();
	activeUnit.clear();
}