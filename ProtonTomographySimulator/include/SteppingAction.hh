#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4OpBoundaryProcess;

class SteppingAction : public G4UserSteppingAction {

public:
    SteppingAction();
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step *);

private:
    G4OpBoundaryProcess *boundary;
};

#endif
