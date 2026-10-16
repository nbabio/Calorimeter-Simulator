#include "EventAction.hh"
#include "EventActionMessenger.hh"

#include "G4AnalysisManager.hh"

#include "Analysis.hh"
#include "G4Run.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"
#include "G4SDManager.hh"
#include "G4UImanager.hh"
#include "G4ios.hh"

#include "CalorimeterHit.hh"

#include "ConfigurationGeometry.hh"

#include <math.h>

#include <iostream>
#include <map>

#define PI 3.141592653589793238462643383279502884



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
EventAction::EventAction(ConfigurationGeometry *geom_, G4int eventOffset_) {

    eventOffset = eventOffset_;
    geom = geom_;
    G4String colName;
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    G4int numberOfDetectors = SDman->GetCollectionCapacity();
    SDman->ListTree();
    MyRndEngine = CLHEP::HepRandom::getTheEngine();
    myGauss = new CLHEP::RandGauss(MyRndEngine);

    for(auto i : geom->collections) {
        DHCID.push_back(SDman->GetCollectionID(i));
    }
    messenger = new EventActionMessenger(this);

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
EventAction::~EventAction() {
    delete messenger;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//



//----------------------------------------------------------------------//
// Begin of the event                                                   //
//----------------------------------------------------------------------//
void EventAction::BeginOfEventAction(const G4Event*) {}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// End of the event -> the action goes here                             //
//----------------------------------------------------------------------//
void EventAction::EndOfEventAction(const G4Event* evt) {

    auto man = G4AnalysisManager::Instance();
    G4HCofThisEvent * HCE = evt->GetHCofThisEvent();

    //One row per layer with energy: absorber and scintillator deposits of that layer
    struct LayerInfo {
        G4double eAbsorber = 0.0, eScintillator = 0.0;
        G4ThreeVector posScintillator;
    };
    std::map<G4int, LayerInfo> layers;
    for(auto i : DHCID) {
        if(!HCE) break;
        CalorimeterHitsCollection *hc = (CalorimeterHitsCollection*)(HCE->GetHC(i));
        if(!hc) continue;
        for(size_t hit = 0; hit < hc->entries(); hit++) {
            CalorimeterHit* aHit = (*hc)[hit];
            LayerInfo &info = layers[aHit->GetLayerID()];
            if(aHit->IsScintillator()) {
                info.eScintillator = aHit->GetEnergy();
                info.posScintillator = aHit->GetMeanPos();
            } else {
                info.eAbsorber = aHit->GetEnergy();
            }
        }
    }

    for(auto &[layer, info] : layers) {
        man->FillNtupleIColumn(0, 0, eventOffset + evt->GetEventID());
        man->FillNtupleIColumn(0, 1, layer);
        man->FillNtupleDColumn(0, 2, info.eScintillator/CLHEP::MeV);
        man->FillNtupleDColumn(0, 3, info.eAbsorber/CLHEP::MeV);
        man->FillNtupleDColumn(0, 4, info.posScintillator.x()/CLHEP::cm);
        man->FillNtupleDColumn(0, 5, info.posScintillator.y()/CLHEP::cm);
        man->FillNtupleDColumn(0, 6, info.posScintillator.z()/CLHEP::cm);
        man->AddNtupleRow(0);
    }

    //Primary particle (the first one of the first vertex) and total energies
    G4int pdg = 0;
    G4double primaryEnergy = 0.0;
    if(evt->GetNumberOfPrimaryVertex() > 0 && evt->GetPrimaryVertex(0)->GetPrimary(0)) {
        G4PrimaryParticle *primary = evt->GetPrimaryVertex(0)->GetPrimary(0);
        pdg = primary->GetPDGcode();
        primaryEnergy = primary->GetKineticEnergy();
    }
    G4double eScintillator = 0.0, eAbsorber = 0.0;
    for(auto &[layer, info] : layers) {
        eScintillator += info.eScintillator;
        eAbsorber += info.eAbsorber;
    }
    man->FillNtupleIColumn(2, 0, eventOffset + evt->GetEventID());
    man->FillNtupleIColumn(2, 1, pdg);
    man->FillNtupleDColumn(2, 2, primaryEnergy/CLHEP::GeV);
    man->FillNtupleDColumn(2, 3, eScintillator/CLHEP::GeV);
    man->FillNtupleDColumn(2, 4, eAbsorber/CLHEP::GeV);
    man->AddNtupleRow(2);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
