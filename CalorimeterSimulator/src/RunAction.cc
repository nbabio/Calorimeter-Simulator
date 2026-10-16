#include "RunAction.hh"
#include "G4AnalysisManager.hh"

#include "Analysis.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
RunAction::RunAction(std::string name) {
    nameOfOutputFile = name;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
RunAction::~RunAction() {}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// We set up the columns when the run starts                            //
//----------------------------------------------------------------------//
void RunAction::BeginOfRunAction(const G4Run* run) {

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile(nameOfOutputFile.c_str());
    // Create ntuple
    // Ntuple 0: energy deposited per layer and event
    man->CreateNtuple("layers", "Energy deposited per layer");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("layer");              // 0 = front layer (first hit by the beam)
    man->CreateNtupleDColumn("energyScintillator"); // MeV
    man->CreateNtupleDColumn("energyAbsorber");     // MeV
    man->CreateNtupleDColumn("x");                  // energy-weighted mean position in the scintillator (cm)
    man->CreateNtupleDColumn("y");
    man->CreateNtupleDColumn("z");
    man->FinishNtuple();

    // Ntuple 1: optical photons born in the scintillators (only if storePhotons is true)
    man->CreateNtuple("photons", "Optical photons at creation");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("layer");
    man->CreateNtupleIColumn("process");
    man->CreateNtupleIColumn("trackID");
    man->CreateNtupleIColumn("parentID");
    man->CreateNtupleDColumn("x");
    man->CreateNtupleDColumn("y");
    man->CreateNtupleDColumn("z");
    man->CreateNtupleDColumn("dirx");
    man->CreateNtupleDColumn("diry");
    man->CreateNtupleDColumn("dirz");
    man->CreateNtupleDColumn("polx");
    man->CreateNtupleDColumn("poly");
    man->CreateNtupleDColumn("polz");
    man->CreateNtupleDColumn("energy");
    man->CreateNtupleDColumn("wavelength");
    man->CreateNtupleDColumn("time");
    man->FinishNtuple();

    // Ntuple 2: one row per event
    man->CreateNtuple("events", "Primary particle and total deposited energy");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("pdgID");              // 2212 = p, 211 = pi+, 321 = K+
    man->CreateNtupleDColumn("primaryEnergy");      // kinetic energy (GeV)
    man->CreateNtupleDColumn("energyScintillator"); // total in all the layers (GeV)
    man->CreateNtupleDColumn("energyAbsorber");     // total in all the layers (GeV)
    man->FinishNtuple();

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// At the end we store                                                  //
//----------------------------------------------------------------------//
void RunAction::EndOfRunAction(const G4Run* aRun)
{
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->Write();
    analysisManager->CloseFile();
    //delete G4AnalysisManager::Instance();
    
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

