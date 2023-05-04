// experiment0.cpp
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 2.12.2022

#include "simlib.h"

#define ZAHONY_TRAVEL_TIME 1

#define ZAHONY_WAGOONS 29
#define ZAHONY_DOCKING_TIME 2
#define ZAHONY_UNLOADING_TIME 1.4
#define ZAHONY_UNDOCKING_TIME 3


Store EmptyWagoons("Empty wagoons", 1000000000000);

// deklarace globálních objektů
Facility  KolajeTam("Cesta k terminalom");
Facility  KolajeSpat("Cesta k terminalom");

Store Zahony("Zahony", 3);
Stat STAT_Zahony("Priemerny cas na obsluhu 1 vlaku");

Store Zahony24tCraneTrack("Zahony track with 24t crane", 2);
Facility  Zahony18tCraneTrack("Zahony track with 18t crane");

Stat STAT_Zahony24tCraneTrack1("Pocet prelozenych konrajnerov 24t zeriavom 1");
Stat STAT_Zahony24tCraneTrack2("Pocet prelozenych konrajnerov 24t zeriavom 2");
Stat STAT_Zahony18tCraneTrack("Pocet prelozenych konrajnerov 18t zeriavom");


class Train : public Process {
  void ZahonyTerminal()
  {
    int wagoons = ZAHONY_WAGOONS;
    int wagons_weight = (Uniform(0, 10) < 3) ? 18 : 24;

    Enter(EmptyWagoons, wagoons);
    // cesta k terminalu
    Seize(KolajeTam);
    Wait(ZAHONY_TRAVEL_TIME);
    Release(KolajeTam);

    // obsadenie terminalu
    //Enter(Zahony);

    if (wagons_weight == 18)
      Seize(Zahony18tCraneTrack);
    else
      Enter(Zahony24tCraneTrack);


    int ent_time = Time;
    Wait(ZAHONY_DOCKING_TIME);
    for (int i = 0; i < wagoons; i++)
    {
      Wait(ZAHONY_UNLOADING_TIME);
      if (wagons_weight == 18)
        STAT_Zahony18tCraneTrack(1);
      else
      {
        if (Uniform(0, 1) < 0.5) // statistically, utilizations of both cranes are same
          STAT_Zahony24tCraneTrack1(1);
        else
          STAT_Zahony24tCraneTrack2(1);
      }
    }

    STAT_Zahony(Time - ent_time);


    // vlak je prazdny, posielanie naspat
    Wait(ZAHONY_UNDOCKING_TIME);
    Seize(KolajeSpat);

    if (wagons_weight == 18)
      Release(Zahony18tCraneTrack);
    else
      Leave(Zahony24tCraneTrack);

    //Leave(Zahony);
    Wait(ZAHONY_TRAVEL_TIME);
    Release(KolajeSpat);

    // je naspat
    Leave(EmptyWagoons, wagoons);
  }
  void Behavior() {
    // na meranie priepustnosti 1 terminalu je treba spustat len 1 terminal...
    this->ZahonyTerminal();
  }
};

class Generator : public Event {  // generátor vlakov
  void Behavior() {
    (new Train)->Activate();
    Activate(Time+ 1);  // nelimituj prichod casom, testuje sa priepustnost
  }
};

int main() {
  //DebugON();
  Init(0,24*365*60);
  (new Generator)->Activate();
  Run();                     // simulace

  Zahony.Output();
  STAT_Zahony.Output();

  STAT_Zahony24tCraneTrack1.Output();
  STAT_Zahony24tCraneTrack2.Output();
  STAT_Zahony18tCraneTrack.Output();

  //Zahony24tCraneTrack.Output();
  //Zahony18tCraneTrack.Output();
  return 0;
}
