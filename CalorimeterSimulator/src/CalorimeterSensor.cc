#include "CalorimeterSensor.hh"
#include "CalorimeterHit.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "G4OpticalPhoton.hh"


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
CalorimeterSensor::CalorimeterSensor(G4String name, G4String collection)
    :G4VSensitiveDetector(name)
{
    collectionName.insert(collection);
    HCID = -1;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
CalorimeterSensor::~CalorimeterSensor() {
    ;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// New collection for every event                                       //
//----------------------------------------------------------------------//
void CalorimeterSensor::Initialize(G4HCofThisEvent*HCE)
{
    hitsCollection = new CalorimeterHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(HCID<0) HCID = G4SDManager::GetSDMpointer()->GetCollectionID(hitsCollection);
    HCE->AddHitsCollection(HCID,hitsCollection);
    hitsByLayer.clear();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accumulates the deposited energy per layer                           //
//----------------------------------------------------------------------//
G4bool CalorimeterSensor::ProcessHits(G4Step*aStep,G4TouchableHistory*  /*ROhist*/) {

    // Optical photons are simulated, but not stored as hits.
    if(aStep->GetTrack()->GetDefinition() == G4OpticalPhoton::Definition()) return false;

    G4double edep = aStep->GetTotalEnergyDeposit();
    if(edep <= 0) return false;

    //The copy number of the absorber/scintillator is the layer index
    G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
    const G4VTouchable *touchable = preStepPoint->GetTouchable();
    G4int layer = touchable->GetCopyNumber();
    G4bool isScintillator = G4StrUtil::starts_with(touchable->GetVolume()->GetName(), "scintillatorPhys");

    G4int key = 2*layer + (isScintillator ? 1 : 0);
    auto it = hitsByLayer.find(key);
    CalorimeterHit *aHit;
    if(it == hitsByLayer.end()) {
        aHit = new CalorimeterHit(layer, isScintillator);
        hitsCollection->insert(aHit);
        hitsByLayer[key] = aHit;
    } else {
        aHit = it->second;
    }

    //Mid point of the step
    G4ThreeVector pos = 0.5 * (preStepPoint->GetPosition() + aStep->GetPostStepPoint()->GetPosition());
    aHit->AddStep(edep, pos);

    return true;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Needed by the parent class                                           //
//----------------------------------------------------------------------//
void CalorimeterSensor::EndOfEvent(G4HCofThisEvent* /*HCE*/) {
    ;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
