#include "DetectorConstruction.hh"

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4Mag_UsualEqRhs.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Para.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4UserLimits.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4RunManager.hh"

#include "G4ios.hh"
#include "G4VisAttributes.hh"

#include "G4PVReplica.hh"

#include "G4SubtractionSolid.hh"



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
DetectorConstruction::DetectorConstruction(ConfigurationGeometry *w) {
    myConf = w;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
DetectorConstruction::~DetectorConstruction() {

    DestroyMaterials();

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Creates all the geometrical structures                               //
//----------------------------------------------------------------------//
G4VPhysicalVolume* DetectorConstruction::Construct() {

    //Building the materials
    ConstructMaterials();

    //Printing the geometry
    myConf->Print();

    //Manager of objects in memory
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    G4String SDname;

    //Creating the world
    G4VSolid* worldSolidPrim = new G4Box("worldBoxPrim", 1.1 * myConf->getSizeX()/2.0 , 1.1 * myConf->getSizeY() / 2.0 , 1.1 * myConf->getSizeZ()/2.0 );
    G4LogicalVolume* worldLogicalPrim = new G4LogicalVolume(worldSolidPrim, materials["air"], "worldLogicalPrim",0,0,0);
    G4VPhysicalVolume* worldPhysicalPrim = new G4PVPlacement(0, G4ThreeVector(), worldLogicalPrim, "worldPhysicalPrim", 0, 0, 0);

    G4VSolid* worldSolid = new G4Box("worldBox", myConf->getSizeX()/2.0 , myConf->getSizeY()/2.0 , myConf->getSizeZ()/2.0 );
    G4LogicalVolume* worldLogical = new G4LogicalVolume(worldSolid, materials["air"], "worldLogical",0,0,0);
    G4VPhysicalVolume* worldPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), worldLogical, "worldPhysical", worldLogicalPrim, false, 0);

    //Both worlds are drawn as wireframe so the inner volumes are visible
    G4VisAttributes *worldVisAtt = new G4VisAttributes();
    worldVisAtt->SetForceWireframe(true);
    worldLogicalPrim->SetVisAttributes(worldVisAtt);
    worldLogical->SetVisAttributes(worldVisAtt);

    myConf->createG4objects(worldLogical, materials, SDman);
 
    DumpGeometricalTree(worldPhysicalPrim, 3);
    
    return worldPhysicalPrim;

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Construct all the materials                                          //
//----------------------------------------------------------------------//
void DetectorConstruction::ConstructMaterials() {

    G4NistManager* man = G4NistManager::Instance();

    materials.insert(std::pair<G4String, G4Material *>("air", man->FindOrBuildMaterial("G4_AIR")));
    materials.insert(std::pair<G4String, G4Material *>("iron", man->FindOrBuildMaterial("G4_Fe")));
    materials.insert(std::pair<G4String, G4Material *>("uranium", man->FindOrBuildMaterial("G4_U")));
    materials.insert(std::pair<G4String, G4Material *>("aluminium", man->FindOrBuildMaterial("G4_Al")));
    materials.insert(std::pair<G4String, G4Material *>("carbon", man->FindOrBuildMaterial("G4_C")));
    materials.insert(std::pair<G4String, G4Material *>("argon", man->FindOrBuildMaterial("G4_Ar")));
    materials.insert(std::pair<G4String, G4Material *>("lead", man->FindOrBuildMaterial("G4_Pb")));
    materials.insert(std::pair<G4String, G4Material *>("silicon", man->FindOrBuildMaterial("G4_Si")));
    materials.insert(std::pair<G4String, G4Material *>("steel", man->FindOrBuildMaterial("G4_STAINLESS-STEEL")));
    materials.insert(std::pair<G4String, G4Material *>("lung", man->FindOrBuildMaterial("G4_LUNG_ICRP")));
    materials.insert(std::pair<G4String, G4Material *>("bone", man->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU")));
    materials.insert(std::pair<G4String, G4Material *>("fat", man->FindOrBuildMaterial("G4_ADIPOSE_TISSUE_ICRP")));
    materials.insert(std::pair<G4String, G4Material *>("brain", man->FindOrBuildMaterial("G4_BRAIN_ICRP")));

    materials.insert(std::pair<G4String, G4Material *>("tungsten", man->FindOrBuildMaterial("G4_W")));

    // Energy grid for the optical properties: 517 - 354 nm (E = 1239.84/λ). It covers the emission of both scintillators
    std::vector<G4double> E = {2.40*eV, 2.60*eV, 2.80*eV, 3.00*eV, 3.20*eV, 3.50*eV};

    // Plastic scintillators from Eljen Technology (polyvinyltoluene base). Values from the datasheets:
    //                    EJ-200      EJ-204
    // Light yield        10000/MeV   10400/MeV
    // Emission peak      425 nm      408 nm
    // Rise time          0.9 ns      0.7 ns
    // Decay time         2.1 ns      1.8 ns
    // Bulk attenuation   380 cm      160 cm
    // Refractive index   1.58        1.58
    // Density            1.023 g/cm3 1.023 g/cm3
    // H:C atoms/cm3      5.17e22 : 4.69e22  (EJ-204: 5.15e22 : 4.68e22)
    // The emission spectra are approximations of the datasheet curves (normalized to 1)
    G4Element *elH = man->FindOrBuildElement("H");
    G4Element *elC = man->FindOrBuildElement("C");
    struct ScintillatorData {
        G4String name;
        G4double nH, nC;                       // atoms/cm3 (x1e22)
        G4double yield, riseTime, decayTime, attLength;
        std::vector<G4double> emissionE, emission;
    };
    std::vector<ScintillatorData> scintillators = {
        {"EJ200", 5.17, 4.69, 10000./MeV, 0.9*ns, 2.1*ns, 380.*cm,
         // 500, 480, 460, 450, 440, 430, 425, 420, 410, 400, 390 nm
         {2.480*eV, 2.583*eV, 2.695*eV, 2.755*eV, 2.818*eV, 2.883*eV, 2.917*eV, 2.952*eV, 3.024*eV, 3.100*eV, 3.179*eV},
         {0.05, 0.15, 0.35, 0.50, 0.70, 0.95, 1.00, 0.85, 0.45, 0.15, 0.02}},
        {"EJ204", 5.15, 4.68, 10400./MeV, 0.7*ns, 1.8*ns, 160.*cm,
         // 490, 470, 450, 440, 430, 420, 415, 408, 400, 390, 380 nm
         {2.530*eV, 2.638*eV, 2.755*eV, 2.818*eV, 2.883*eV, 2.952*eV, 2.988*eV, 3.039*eV, 3.100*eV, 3.179*eV, 3.263*eV},
         {0.05, 0.15, 0.35, 0.50, 0.70, 0.95, 1.00, 0.95, 0.60, 0.20, 0.03}}
    };
    for(auto &sc : scintillators) {
        G4double massH = sc.nH * elH->GetA();
        G4double massC = sc.nC * elC->GetA();
        G4Material *mat = new G4Material(sc.name, 1.023*g/cm3, 2);
        mat->AddElement(elH, massH/(massH + massC));
        mat->AddElement(elC, massC/(massH + massC));
        auto *mpt = new G4MaterialPropertiesTable();
        mpt->AddProperty("RINDEX",                  E, std::vector<G4double>(E.size(), 1.58));
        mpt->AddProperty("ABSLENGTH",               E, std::vector<G4double>(E.size(), sc.attLength));
        mpt->AddProperty("SCINTILLATIONCOMPONENT1", sc.emissionE, sc.emission);
        mpt->AddConstProperty("SCINTILLATIONYIELD",         sc.yield);
        mpt->AddConstProperty("RESOLUTIONSCALE",            1.0); // 1.0 = Poisson statistics
        mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", sc.decayTime);
        mpt->AddConstProperty("SCINTILLATIONRISETIME1",     sc.riseTime);
        mat->SetMaterialPropertiesTable(mpt);
        mat->GetIonisation()->SetBirksConstant(0.126*mm/MeV); // Typical value for plastic scintillators
        materials.insert(std::pair<G4String, G4Material *>(sc.name, mat));
    }

    // Air with RINDEX=1 so that light can exit the scintillator without dying at the boundary
    auto* airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", E, std::vector<G4double>(E.size(), 1.0));
    materials["air"]->SetMaterialPropertiesTable(airMPT);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destroy all the materials                                            //
//----------------------------------------------------------------------//
void DetectorConstruction::DestroyMaterials() {
    // Destroy all allocated elements and materials
    size_t i;
    G4MaterialTable* matTable = (G4MaterialTable*)G4Material::GetMaterialTable();
    for(i=0; i<matTable->size(); i++) delete (*(matTable))[i];
    matTable->clear();
    G4ElementTable* elemTable = (G4ElementTable*)G4Element::GetElementTable();
    for(i=0; i<elemTable->size(); i++) delete (*(elemTable))[i];
    elemTable->clear();

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


void DetectorConstruction::DumpGeometricalTree(G4VPhysicalVolume* aVolume,G4int depth)
{

    for(int isp=0; isp<depth; isp++)
    {
        G4cout << "  ";
    }
    G4cout << aVolume->GetName() << "[" << aVolume->GetCopyNo() << "] "
           << aVolume->GetLogicalVolume()->GetName() << " "
           << aVolume->GetLogicalVolume()->GetNoDaughters() << " "
           << aVolume->GetLogicalVolume()->GetMaterial()->GetName();
    if(aVolume->GetLogicalVolume()->GetSensitiveDetector())
    {
        G4cout << " " << aVolume->GetLogicalVolume()->GetSensitiveDetector()->GetFullPathName();
    }
    G4cout << G4endl;
    for(int i=0; i<aVolume->GetLogicalVolume()->GetNoDaughters(); i++)
    {
        DumpGeometricalTree(aVolume->GetLogicalVolume()->GetDaughter(i),depth+1);
    }

}
