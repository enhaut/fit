// experiment0.cpp
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 2.12.2022

#include "simlib.h"
#include "time.h"

#define CIERNA_NAD_TISOU_TRAVEL_TIME 1
#define ZAHONY_TRAVEL_TIME 1
#define MEDYKA_TRAVEL_TIME 1
#define DOROHURSK_TRAVEL_TIME 1
#define DORNESTI_TRAVEL_TIME 1
#define HALMEU_TRAVEL_TIME 1

#define CIERNA_NAD_TISOU_WAGOONS 70
#define CIERNA_NAD_TISOU_DOCKING_TIME 4
#define CIERNA_NAD_TISOU_UNLOADING_TIME 2
#define CIERNA_NAD_TISOU_UNDOCKING_TIME 5

#define ZAHONY_WAGOONS 29
#define ZAHONY_DOCKING_TIME 2
#define ZAHONY_UNLOADING_TIME 1.4
#define ZAHONY_UNDOCKING_TIME 3

#define MEDYKA_WAGOONS 39
#define MEDYKA_DOCKING_TIME 4
#define MEDYKA_UNLOADING_TIME 5
#define MEDYKA_UNDOCKING_TIME 4

#define DOROHURSK_WAGOONS 30
#define DOROHURSK_DOCKING_TIME 2
#define DOROHURSK_UNLOADING_TIME 5
#define DOROHURSK_UNDOCKING_TIME 3

#define DORNESTI_WAGOONS 15
#define DORNESTI_DOCKING_TIME 1
#define DORNESTI_UNLOADING_TIME 2
#define DORNESTI_UNDOCKING_TIME 1

#define HALMEU_WAGOONS 19
#define HALMEU_DOCKING_TIME 2
#define HALMEU_UNLOADING_TIME 2
#define HALMEU_UNDOCKING_TIME 2


Store EmptyWagoons("Empty wagoons", 1000000000000);

// deklarace globálních objektů
Facility  KolajeTam("Cesta k terminalom");
Facility  KolajeSpat("Cesta k terminalom");

// terminaly
Facility  CiernaNadTisou("Cierna nad Tisou");  // kapacita len 1 vlaku
Stat STAT_CiernaNadTisou("Priemerny cas na obsluhu 1 vlaku");

Store Zahony("Zahony", 3);
Stat STAT_Zahony("Priemerny cas na obsluhu 1 vlaku");

Facility  Dorohursk("Dorohursk");  // kapacita len 1 vlaku
Stat STAT_Dorohursk("Priemerny cas na obsluhu 1 vlaku");

Facility  Medyka("Medyka");  // kapacita len 1 vlaku
Stat STAT_Medyka("Priemerny cas na obsluhu 1 vlaku");

Store Dornesti("Dornesti", 2);
Stat STAT_Dornesti("Priemerny cas na obsluhu 1 vlaku");

Stat STAT_DornestiCraneWaiting1("Cakanie na zeriav - kolaj 1");
Stat STAT_DornestiCraneWaiting2("Cakanie na zeriav - kolaj 2");
//Facility DornestiCrane("Dornesti crane");
Facility DornestiTrack1("Kolaj 1");
Facility DornestiTrack2("Kolaj 2");


Store DornestiCrate("Dornesti crane", 2);

Facility  Halmeu("Halmeu");  // kapacita len 1 vlaku
Stat STAT_Halmeu("Priemerny cas na obsluhu 1 vlaku");


class Train : public Process {
  void DornestiTerminal()
  {
    int wagoons = DORNESTI_WAGOONS;
    Enter(EmptyWagoons, wagoons);
    // cesta k terminalu
    Seize(KolajeTam);
    Wait(DORNESTI_TRAVEL_TIME);
    Release(KolajeTam);

    // obsadenie terminalu
    Enter(Dornesti);

    int ent_time = Time;
    Wait(DORNESTI_DOCKING_TIME);

    int rail_index = 0;
    if (DornestiTrack1.Busy())
    {
      rail_index = 1;
      Seize(DornestiTrack2);
    }else
      Seize(DornestiTrack1);


    int waiting_time1 = 0;
    int waiting_time2 = 0;

    for (int i = 0; i < wagoons; i++)
    {
      int waiting_time = Time;
      Enter(DornestiCrate);
        if (rail_index == 0)
          waiting_time1 += Time - waiting_time;
        else
          waiting_time2 += (Time - waiting_time);


      Wait(DORNESTI_UNLOADING_TIME);
      Leave(DornestiCrate);
    }

    STAT_Dornesti(Time - ent_time);
    if (rail_index == 0) {
      STAT_DornestiCraneWaiting1(waiting_time1);
      Release(DornestiTrack1);
    } else {
      STAT_DornestiCraneWaiting2(waiting_time2);
      Release(DornestiTrack2);
    }

    // vlak je prazdny, posielanie naspat
    Wait(DORNESTI_UNDOCKING_TIME);
    Seize(KolajeSpat);
    Leave(Dornesti);

    Wait(DORNESTI_TRAVEL_TIME);
    Release(KolajeSpat);

    // je naspat
    Leave(EmptyWagoons, wagoons);
  }
  void Behavior() {
    // na meranie priepustnosti 1 terminalu je treba spustat len 1 terminal...
    this->DornestiTerminal();
  }
};

class Generator : public Event {  // generátor vlakov
  void Behavior() {
    (new Train)->Activate();
    Activate(Time+ 1);  // nelimituj prichod casom, testuje sa priepustnost
  }
};

int main() {
  srand(time(0));
  //DebugON();
  Init(0,24*365*60);
  (new Generator)->Activate();
  Run();                     // simulace

  Dornesti.Output();
  STAT_Dornesti.Output();
  DornestiCrate.Output();
  STAT_DornestiCraneWaiting1.Output();
  STAT_DornestiCraneWaiting2.Output();
  DornestiTrack1.Output();
  DornestiTrack2.Output();

  return 0;
}
