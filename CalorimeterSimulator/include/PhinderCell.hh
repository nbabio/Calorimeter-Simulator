#ifndef PhinderCell_h
#define PhinderCell_h 1

#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include <map>

//----------------------------------------------------------------------//
// Single scintillator cell with two geometry modes:                    //
// 1) stage1: one straight light guide on the +z face, identical to     //
//    phinder_bridge/scripts/phinder_stage1.py of the                   //
// Scintillator-simulation project                                      //
// 2) crossedGuides: two light guides on the +z and +x faces            //
//----------------------------------------------------------------------//
class PhinderCell {

public:

    PhinderCell(G4String, G4String);

    void createG4Objects(G4LogicalVolume *, std::map<G4String, G4Material*> &);

    void Print();

private:

    void createStage1Guide(G4LogicalVolume *, std::map<G4String, G4Material*> &);
    void createCrossedGuides(G4LogicalVolume *, std::map<G4String, G4Material*> &);

    G4String scintillatorMaterial;
    G4String geometry;              // "stage1" or "crossedGuides"

};


#endif