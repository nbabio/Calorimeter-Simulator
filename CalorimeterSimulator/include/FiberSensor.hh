#ifndef FiberSensor_h
#define FiberSensor_h 1

#include "G4VSensitiveDetector.hh"
#include "FiberSensorHit.hh"
#include "Fiber.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class FiberSensor : public G4VSensitiveDetector
{

public:
    FiberSensor(G4String name, G4String collection);
    
    void setFiber(Fiber *);
    virtual ~FiberSensor();
    Fiber *getFiber();
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
    virtual void EndOfEvent(G4HCofThisEvent*HCE);

private:
    FiberSensorHitsCollection * hitsCollection;
    G4ThreeVector theLocalPosError;
    Fiber *fiber;
    G4int HCID;
};




#endif

