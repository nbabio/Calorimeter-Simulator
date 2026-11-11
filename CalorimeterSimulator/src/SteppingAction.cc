#include "SteppingAction.hh"
#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"

//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
SteppingAction::SteppingAction(G4int offset) {
    eventOffset = offset;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
SteppingAction::~SteppingAction() {}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Called at every step                                                 //
//----------------------------------------------------------------------//
void SteppingAction::UserSteppingAction(const G4Step *step) {

    G4Track *track = step->GetTrack();
    if(track->GetDefinition() != G4OpticalPhoton::Definition()) return;

    G4StepPoint *post = step->GetPostStepPoint();
    //Same split as Python: the scintillator or anything else (core and cladding = guide)
    G4bool inCell = step->GetPreStepPoint()->GetPhysicalVolume()->GetName() == "cellPhys";

    //Bulk absorption: the step ends inside the volume where it started
    const G4VProcess *process = post->GetProcessDefinedStep();
    if(process != nullptr && process->GetProcessName() == "OpAbsorption") {
        fillFate(inCell ? ABSORBED_LAMBDA_AT_Scintillator : ABSORBED_LAMBDA_AT_Guide);
        return;
    }

    if(post->GetStepStatus() == fGeomBoundary && post->GetPhysicalVolume() != nullptr) {

        //The boundary process is looked up once
        if(boundary == nullptr) {
            G4ProcessVector *processes = track->GetDefinition()->GetProcessManager()->GetProcessList();
            for(size_t i = 0; i < processes->size(); i++) {
                if((*processes)[i]->GetProcessName() == "OpBoundary") boundary = (G4OpBoundaryProcess*)(*processes)[i];
            }
        }

        //The post step volume is the one across the boundary, even if the photon was reflected:
        //only photons that really crossed it count
        if(boundary != nullptr && boundary->GetStatus() == FresnelRefraction) {
            if(post->GetPhysicalVolume()->GetName() == "readoutPhys") {
                G4ThreeVector pos = post->GetPosition();
                G4ThreeVector dir = post->GetMomentumDirection();
                auto man = G4AnalysisManager::Instance();
                G4int id = 4;
                man->FillNtupleIColumn(id, 0, eventOffset + G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
                man->FillNtupleIColumn(id, 1, track->GetTrackID());
                man->FillNtupleDColumn(id, 2, pos.x()/cm);
                man->FillNtupleDColumn(id, 3, pos.y()/cm);
                man->FillNtupleDColumn(id, 4, pos.z()/cm);
                man->FillNtupleDColumn(id, 5, dir.x());
                man->FillNtupleDColumn(id, 6, dir.y());
                man->FillNtupleDColumn(id, 7, dir.z());
                man->FillNtupleDColumn(id, 8, post->GetGlobalTime()/ns);
                man->FillNtupleIColumn(id, 9, post->GetPhysicalVolume()->GetCopyNo());
                man->AddNtupleRow(id);
                fillFate(CAUGHT);
                track->SetTrackStatus(fStopAndKill);
                return;
            }
            if(post->GetMaterial()->GetName() == "G4_AIR") {
                fillFate(inCell ? LOST_AT_Scintillator : LOST_AT_Guide);
                track->SetTrackStatus(fStopAndKill);
                return;
            }
        }
    }

    //Any other way of dying: it should never happen; if it does, it shows up in the fates as 99
    if(track->GetTrackStatus() == fStopAndKill) fillFate(OTHER);
}


//----------------------------------------------------------------------//
// One row of the "fates" ntuple per photon                             //
//----------------------------------------------------------------------//
void SteppingAction::fillFate(G4int fate) {
    auto man = G4AnalysisManager::Instance();
    G4int id = 5;
    man->FillNtupleIColumn(id, 0, eventOffset + G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
    man->FillNtupleIColumn(id, 1, fate);
    man->AddNtupleRow(id);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//