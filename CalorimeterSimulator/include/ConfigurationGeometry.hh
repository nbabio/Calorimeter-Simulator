#ifndef ConfigurationGeometry_h
#define ConfigurationGeometry_h 1

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "assert.h"

#include "globals.hh"
#include "Calorimeter.hh"


class ConfigurationGeometry {

public:

    ConfigurationGeometry(G4String);
    
    bool isGood();

    // Information about the GEANT4 universe 
    G4double getSizeX();
    G4double getSizeY();
    G4double getSizeZ();
   
    // Informatio about the BEAM
    G4double GetXBeamPosition();
    G4double GetXBeamSigma();
    G4double GetYBeamPosition();
    G4double GetYBeamSigma();
    G4double GetZBeamPosition();
    G4double GetXDirBeam();
    G4double GetYDirBeam();
    G4double GetZDirBeam();
    G4double GetEnergy();
    G4int GetNStep();
    G4double GetEnergySigma();
    G4double GetTBeamSigma();
    G4int GetEnergyDistribution();
    G4int GetNParticles();
    G4int GetParticleDistribution();
    G4double GetMaxOpenAngle();
    G4String GetParticleName();


    // Calorimeter information
    Calorimeter *getCalorimeter();

    //Creating the geometry
    void createG4objects(G4LogicalVolume *, 
                         std::map<G4String, G4Material*> &,
                         G4SDManager *);
    // Detector collections
    void registerCollection(G4String);
    std::vector <G4String> collections;

    G4bool killPhotons;
    G4bool storePhotons;
    G4bool opticalPhysics;
    
    void Print();

private:

    G4double xBeamPosition, xBeamSigma;
    G4double yBeamPosition, yBeamSigma;
    G4double zBeamPosition;
    G4double xBeamDir, yBeamDir, zBeamDir;
    G4double energy, energySigma;
    G4double tBeamSigma;
    G4double maxOpenAngle;
    G4int nParticles, nStep;
    G4int energyDistribution, particleDistribution;
    G4String particleName;

    G4double uniSizeX, uniSizeY, uniSizeZ;
    Calorimeter *calorimeter;
    bool goodGeometry;

    G4VSolid *coneVolume;
    G4LogicalVolume *conelogicalVolume;
    G4VPhysicalVolume *conePhysical;


};



#endif

