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
    man->CreateNtupleDColumn("energyAbsorber");     // MeV, absorber behind the scintillator of this layer (0 for the last layer)
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

    // Ntuple 3: point source, one row per event (only with PointSource)
    man->CreateNtuple("sources", "Point source of optical photons");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("sourceIndex");
    man->CreateNtupleDColumn("x");                  // cm
    man->CreateNtupleDColumn("y");
    man->CreateNtupleDColumn("z");
    man->CreateNtupleIColumn("nPhotons");           // photons fired (denominator of the efficiency)
    man->FinishNtuple();

    // Ntuple 4: optical photons that reach the readout of the PHINDER cell
    man->CreateNtuple("arrivals", "Optical photons at the readout");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("trackID");
    man->CreateNtupleDColumn("x");                  // cm
    man->CreateNtupleDColumn("y");
    man->CreateNtupleDColumn("z");
    man->CreateNtupleDColumn("dirx");
    man->CreateNtupleDColumn("diry");
    man->CreateNtupleDColumn("dirz");
    man->CreateNtupleDColumn("time");               // ns; with the point source (t=0) it is the transport time
    man->CreateNtupleIColumn("channel");            // readout copy number: 0 (always 0 in stage 1) or 1
    man->FinishNtuple();

    // Ntuple 5: final fate of every optical photon in the PHINDER cell. Same codes as PhotonStatus of
    // the Python simulator: 2 CAUGHT, 7/8 ABSORBED_LAMBDA_AT_Scintillator/Guide, 9/10 LOST_AT_Scintillator/Guide
    man->CreateNtuple("fates", "Final fate of the optical photons");
    man->CreateNtupleIColumn("eventNumber");
    man->CreateNtupleIColumn("fate");
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

