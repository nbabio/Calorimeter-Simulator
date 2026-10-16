#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class MyPhysicsList final : public G4VModularPhysicsList
{

public:
    MyPhysicsList(G4bool);
    ~MyPhysicsList();
    //void ConstructProcess() override;

};

#endif
