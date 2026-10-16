#include "StackingAction.hh"
#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4VTouchable.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
StackingAction::StackingAction(G4bool kill, G4bool store, G4int offset) {
    eventOffset = offset;
    killOpticalPhotons = kill;
    storeOpticalPhotons = store;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
StackingAction::~StackingAction() {}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Called for every new track: optical photons born in a scintillator  //
// are stored in the "photons" ntuple (if storeOpticalPhotons)          //
//----------------------------------------------------------------------//
G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track *track) {

    if(track->GetDefinition() != G4OpticalPhoton::Definition()) return fUrgent;

    G4ClassificationOfNewTrack status = killOpticalPhotons ? fKill : fUrgent;
    if(!storeOpticalPhotons) return status;

    //Only photons born in a scintillator
    const G4VTouchable *touchable = track->GetTouchable();
    if(touchable == nullptr || !G4StrUtil::starts_with(touchable->GetVolume()->GetName(), "scintillatorPhys")) {
        return status;
    }

    //The layer is the copy number of the scintillator
    G4int layer = touchable->GetCopyNumber();

    //Creator process: 0 = Scintillation, 1 = Cerenkov
    const G4VProcess *creator = track->GetCreatorProcess();
    G4int process = (creator != nullptr && creator->GetProcessName() == "Scintillation") ? 0 : 1;

    G4ThreeVector pos = track->GetPosition();
    G4ThreeVector dir = track->GetMomentumDirection();
    G4ThreeVector pol = track->GetPolarization();
    G4double energy = track->GetKineticEnergy();

    auto man = G4AnalysisManager::Instance();
    G4int id = 1;
    man->FillNtupleIColumn(id, 0, eventOffset + G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
    man->FillNtupleIColumn(id, 1, layer);
    man->FillNtupleIColumn(id, 2, process);
    man->FillNtupleIColumn(id, 3, track->GetTrackID());
    man->FillNtupleIColumn(id, 4, track->GetParentID());
    man->FillNtupleDColumn(id, 5, pos.x()/CLHEP::cm);
    man->FillNtupleDColumn(id, 6, pos.y()/CLHEP::cm);
    man->FillNtupleDColumn(id, 7, pos.z()/CLHEP::cm);
    man->FillNtupleDColumn(id, 8, dir.x());
    man->FillNtupleDColumn(id, 9, dir.y());
    man->FillNtupleDColumn(id, 10, dir.z());
    man->FillNtupleDColumn(id, 11, pol.x());
    man->FillNtupleDColumn(id, 12, pol.y());
    man->FillNtupleDColumn(id, 13, pol.z());
    man->FillNtupleDColumn(id, 14, energy/CLHEP::eV);
    man->FillNtupleDColumn(id, 15, CLHEP::h_Planck*CLHEP::c_light/energy/CLHEP::nm);
    man->FillNtupleDColumn(id, 16, track->GetGlobalTime()/CLHEP::ns);
    man->AddNtupleRow(id);

    return status;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
