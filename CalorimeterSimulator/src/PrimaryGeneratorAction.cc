#include <iomanip>
#include "PrimaryGeneratorAction.hh"
#include <sstream>
#include <sys/time.h>
#include <iostream>
#include <list>

#include "G4Event.hh"
#include "G4OpticalPhoton.hh"
#include "G4RandomDirection.hh"

using namespace std;



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
PrimaryGeneratorAction::PrimaryGeneratorAction(ConfigurationGeometry *myGeom_, G4long randomSeed_, G4double pt_) {

    //Configuration information
    myGeom = myGeom_;
    randomSeed = randomSeed_;  
    
    //Particle gun
    G4String particleName;
    particleGun = new G4ParticleGun();
    MyRndEngine = CLHEP::HepRandom::getTheEngine();
    MyRndEngine->setSeed(randomSeed, 1);
    // Create the table containing all particle names
    particleTable = G4ParticleTable::GetParticleTable();
    fProton = particleTable->FindParticle(particleName=myGeom->GetParticleName());
    if(fProton == nullptr) {
        G4Exception("PrimaryGeneratorAction", "Beam001", FatalException,
                    ("Unknown particle: " + myGeom->GetParticleName()).c_str());
    }
    particleGun->SetParticlePosition(G4ThreeVector(0.,0.,0));
    particleGun->SetParticleDefinition(fProton);

    //Beam
    beam = new Beam(myGeom, MyRndEngine, fProton->GetPDGMass());

    // Create the messenger file
    gunMessenger = new PrimaryGeneratorMessenger(this);

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructuor                                                          //
//----------------------------------------------------------------------//
PrimaryGeneratorAction::~PrimaryGeneratorAction() {
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Real generation of the primaries                                     //
//----------------------------------------------------------------------//
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {

    if(myGeom->pointSource) {
        generatePointSource(anEvent);
        return;
    }

    G4int maxParticles = beam->GetNParticles();
       
    for ( unsigned j=0; j< maxParticles; j++) {
        
	    particleGun->SetParticleDefinition(fProton);
	    std::vector<G4double> vect = beam->fireParticle();
        //Momentum
        particleGun->SetParticleMomentum(vect[0]);
        particleGun->SetParticlePosition(G4ThreeVector(vect[1], vect[2], vect[3]));
        particleGun->SetParticleTime(vect[4]);
        particleGun->SetParticleMomentumDirection(G4ThreeVector(vect[5], vect[6], vect[7]));
        particleGun->GeneratePrimaryVertex(anEvent);

        
    }

    beam->updateBeam();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Point source of optical photons: isotropic, monochromatic, t = 0.    //
// Each event is one source position, cycling through the list         //
//----------------------------------------------------------------------//
void PrimaryGeneratorAction::generatePointSource(G4Event* anEvent) {

    G4int iSource = anEvent->GetEventID() % myGeom->sourcePositions.size();
    particleGun->SetParticleDefinition(G4OpticalPhoton::Definition());
    particleGun->SetParticleEnergy(myGeom->photonEnergy);
    particleGun->SetParticlePosition(myGeom->sourcePositions[iSource]);
    particleGun->SetParticleTime(0);

    for(G4int i = 0; i < myGeom->photonsPerEvent; i++) {
        //Random linear polarization perpendicular to the direction
        G4ThreeVector dir = G4RandomDirection();
        G4ThreeVector e1 = dir.orthogonal().unit();
        G4ThreeVector e2 = dir.cross(e1);
        G4double phi = CLHEP::twopi * G4UniformRand();
        particleGun->SetParticleMomentumDirection(dir);
        particleGun->SetParticlePolarization(std::cos(phi)*e1 + std::sin(phi)*e2);
        particleGun->GeneratePrimaryVertex(anEvent);
    }
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

