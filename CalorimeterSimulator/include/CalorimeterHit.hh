#ifndef CalorimeterHit_h
#define CalorimeterHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"


//----------------------------------------------------------------------//
// Energy deposited in one layer (absorber or scintillator) during one  //
// event. The position is the energy-weighted mean of the steps         //
//----------------------------------------------------------------------//
class CalorimeterHit : public G4VHit
{
public:

    CalorimeterHit(G4int, G4bool);
    virtual ~CalorimeterHit();

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);

    void AddStep(G4double, G4ThreeVector);

    virtual void Print();

private:
    G4int layerID;
    G4bool scintillator;
    G4double energy;
    G4ThreeVector weightedPos;

public:
    inline G4int GetLayerID() const {
        return layerID;
    }
    inline G4bool IsScintillator() const {
        return scintillator;
    }
    inline G4double GetEnergy() const {
        return energy;
    }
    inline G4ThreeVector GetMeanPos() const {
        return energy > 0 ? weightedPos / energy : G4ThreeVector();
    }

};


//----------------------------------------------------------------------//
// Collections of this                                                  //
//----------------------------------------------------------------------//
typedef G4THitsCollection<CalorimeterHit> CalorimeterHitsCollection;

extern G4Allocator<CalorimeterHit> CalorimeterHitAllocator;
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Redefinition of new operator                                         //
//----------------------------------------------------------------------//
inline void* CalorimeterHit::operator new(size_t) {
    return (void*)CalorimeterHitAllocator.MallocSingle();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Redefinition of delete operator                                      //
//----------------------------------------------------------------------//
inline void CalorimeterHit::operator delete(void* aHit) {
    CalorimeterHitAllocator.FreeSingle((CalorimeterHit*) aHit);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

#endif
