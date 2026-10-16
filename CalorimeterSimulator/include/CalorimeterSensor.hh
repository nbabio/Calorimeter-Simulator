#ifndef CalorimeterSensor_h
#define CalorimeterSensor_h 1

#include "G4VSensitiveDetector.hh"
#include "CalorimeterHit.hh"
#include <map>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class CalorimeterSensor : public G4VSensitiveDetector
{

public:
    CalorimeterSensor(G4String name, G4String collection);
    virtual ~CalorimeterSensor();
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
    virtual void EndOfEvent(G4HCofThisEvent*HCE);

private:
    CalorimeterHitsCollection * hitsCollection;
    std::map<G4int, CalorimeterHit*> hitsByLayer; // key = 2*layer + (scintillator ? 1 : 0)
    G4int HCID;
};


#endif
