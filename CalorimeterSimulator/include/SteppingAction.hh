#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4OpBoundaryProcess;

//----------------------------------------------------------------------//
// Optical photons in the PHINDER cell: a photon that crosses into the  //
// readout is stored in the "arrivals" ntuple and killed. A photon that //
// crosses into the air is killed, as the Python simulator does (LOST). //
//----------------------------------------------------------------------//
class SteppingAction : public G4UserSteppingAction {

public:
    SteppingAction(G4int);
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step *);

private:
    G4int eventOffset;
    G4OpBoundaryProcess *boundary = nullptr;
    //Same values as PhotonStatus in Photon_Simulator. OTHER: killed by anything else (should be 0)
    enum Fate { CAUGHT = 2, ABSORBED_LAMBDA_AT_Scintillator = 7, ABSORBED_LAMBDA_AT_Guide = 8,
                LOST_AT_Scintillator = 9, LOST_AT_Guide = 10, OTHER = 99 };
    void fillFate(G4int fate);
};

#endif