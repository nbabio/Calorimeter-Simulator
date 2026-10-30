#ifndef PhinderCell_h
#define PhinderCell_h 1

#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include <map>

//----------------------------------------------------------------------//
// Single scintillator cell with one straight light guide on the +z     //
// face, identical to phinder_bridge/scripts/phinder_stage1.py of the   //
// Scintillator-simulation project. It is placed in the Python          //
// coordinates (cell in [0,5]x[0,5]x[0,1] cm) so that both outputs can  //
// be compared without any transformation.                              //
//----------------------------------------------------------------------//
class PhinderCell {

public:

    PhinderCell(G4String);

    void createG4Objects(G4LogicalVolume *, std::map<G4String, G4Material*> &);

    void Print();

private:

    G4String scintillatorMaterial;

};

#endif