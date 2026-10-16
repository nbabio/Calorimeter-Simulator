#ifndef Calorimeter_h
#define Calorimeter_h 1

#include "GeomObject.hh"
#include <vector>


//----------------------------------------------------------------------//
// Sampling calorimeter: a rectangular prism made of two scintillators  //
// with an absorber between them: S0 W S1 (the number of layers is      //
// fixed in ConfigurationGeometry). The beam enters through the +z face //
// (layer 0 is the front one) and the photodetector will be placed      //
// behind the -z face.                                                  //
//----------------------------------------------------------------------//
class Calorimeter : GeomObject {

public:

    Calorimeter(G4double, G4double, G4double,
                G4double, G4double, G4double,
                G4double, G4double,
                G4double, G4String,
                std::vector<G4double>, G4String,
                G4String);

    G4String getName();

    G4int getNLayers();

    G4double getDepth();

    G4ThreeVector getPosition();

    G4ThreeVector getSize();

    void createG4Objects(G4LogicalVolume *,
                        std::map<G4String, G4Material*> &, G4SDManager*);

    void Print();

private:

    G4String name;
    G4double absorberThickness;
    G4String absorberMaterial;
    std::vector<G4double> scintillatorThickness;
    G4String scintillatorMaterial;

};



#endif
