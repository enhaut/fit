// experiment0.cpp
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 2.12.2022

#include "simlib.h"

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
Facility DornestiCrane("Dornesti crane");

Facility  Halmeu("Halmeu");  // kapacita len 1 vlaku
Stat STAT_Halmeu("Priemerny cas na obsluhu 1 vlaku");


class Train : public Process {
    void CiernaNadTisouTerminal()
    {
      int wagoons = CIERNA_NAD_TISOU_WAGOONS;
      Enter(EmptyWagoons, wagoons);
      // cesta k terminalu
      Seize(KolajeTam);
      Wait(CIERNA_NAD_TISOU_TRAVEL_TIME);
      Release(KolajeTam);

      // obsadenie terminalu
      Seize(CiernaNadTisou);
      int ent_time = Time;
      Wait(CIERNA_NAD_TISOU_DOCKING_TIME);
      for (int i = 0; i < wagoons; i++)
        Wait(Uniform(2, 3));

      STAT_CiernaNadTisou(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(CIERNA_NAD_TISOU_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Release(CiernaNadTisou);
      Wait(CIERNA_NAD_TISOU_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }
    void ZahonyTerminal()
    {
      int wagoons = ZAHONY_WAGOONS;
      Enter(EmptyWagoons, wagoons);
      // cesta k terminalu
      Seize(KolajeTam);
      Wait(ZAHONY_TRAVEL_TIME);
      Release(KolajeTam);
      
      // obsadenie terminalu
      Enter(Zahony);
      int ent_time = Time;
      Wait(ZAHONY_DOCKING_TIME);
      for (int i = 0; i < wagoons; i++)
        Wait(2);

      STAT_Zahony(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(ZAHONY_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Leave(Zahony);
      Wait(ZAHONY_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }
    void MedykaTerminal()
    {
      int wagoons = MEDYKA_WAGOONS;
      Enter(EmptyWagoons, wagoons);
      // cesta k terminalu
      Seize(KolajeTam);
      Wait(MEDYKA_TRAVEL_TIME);
      Release(KolajeTam);

      // obsadenie terminalu
      Seize(Medyka);
      int ent_time = Time;
      Wait(MEDYKA_DOCKING_TIME);
      for (int i = 0; i < wagoons / 3; i++)  // they have 3 cranes that can work together on the same train
        Wait(MEDYKA_UNLOADING_TIME);

      STAT_Medyka(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(MEDYKA_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Release(Medyka);
      Wait(MEDYKA_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }
    void DorohurskTerminal()
    {
      int wagoons = DOROHURSK_WAGOONS;
      Enter(EmptyWagoons, wagoons);
      // cesta k terminalu
      Seize(KolajeTam);
      Wait(DOROHURSK_TRAVEL_TIME);
      Release(KolajeTam);

      // obsadenie terminalu
      Seize(Dorohursk);
      int ent_time = Time;
      Wait(DOROHURSK_DOCKING_TIME);
      for (int i = 0; i < wagoons; i++)  // they have 3 cranes that can work together on the same train
        Wait(DOROHURSK_UNLOADING_TIME);

      STAT_Dorohursk(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(DOROHURSK_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Release(Dorohursk);
      Wait(DOROHURSK_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }
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
      for (int i = 0; i < wagoons; i++)
      {
        Seize(DornestiCrane);
        Wait(DORNESTI_UNLOADING_TIME);
        Release(DornestiCrane);
      }

      STAT_Dornesti(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(DORNESTI_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Leave(Dornesti);
      Wait(DORNESTI_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }
    void HalmeuTerminal()
    {
      int wagoons = HALMEU_WAGOONS;
      Enter(EmptyWagoons, wagoons);
      // cesta k terminalu
      Seize(KolajeTam);
      Wait(HALMEU_TRAVEL_TIME);
      Release(KolajeTam);

      // obsadenie terminalu
      Seize(Halmeu);
      int ent_time = Time;
      Wait(HALMEU_DOCKING_TIME);
      for (int i = 0; i < wagoons; i++)  // they have 3 cranes that can work together on the same train
        Wait(HALMEU_UNLOADING_TIME);

      STAT_Halmeu(Time - ent_time);

      // vlak je prazdny, posielanie naspat
      Wait(HALMEU_UNDOCKING_TIME);
      Seize(KolajeSpat);
      Release(Halmeu);
      Wait(HALMEU_TRAVEL_TIME);
      Release(KolajeSpat);

      // je naspat
      Leave(EmptyWagoons, wagoons);
    }


  void Behavior() {
    // na meranie priepustnosti 1 terminalu je treba spustat len 1 terminal...
    //this->CiernaNadTisouTerminal();
    //this->ZahonyTerminal();
    //this->MedykaTerminal();
    this->DorohurskTerminal();
    //this->DornestiTerminal();
    //this->HalmeuTerminal();
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

  CiernaNadTisou.Output();
  STAT_CiernaNadTisou.Output();

  Zahony.Output();
  STAT_Zahony.Output();

  Medyka.Output();
  STAT_Medyka.Output();

  Dorohursk.Output();
  STAT_Dorohursk.Output();

  Dornesti.Output();
  STAT_Dornesti.Output();
  DornestiCrane.Output();

  Halmeu.Output();
  STAT_Halmeu.Output();

  return 0;
}
