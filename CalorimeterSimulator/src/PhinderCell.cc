#include "PhinderCell.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

//Same values as phinder_stage1.py
static const G4double cellX = 5.0*cm, cellY = 5.0*cm, cellZ = 1.0*cm;
static const G4double guideXMin = 2.0*cm, guideXMax = 3.0*cm;
static const G4double guideYMin = 0.0*cm, guideYMax = 5.2*cm;
static const G4double guideThickness = 0.1*cm, cladThickness = 0.001*cm;
static const G4double readoutThickness = 1.0*um;


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
PhinderCell::PhinderCell(G4String material) {
    scintillatorMaterial = material;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Places a box given its minimum and maximum corners. The physical     //
// volume is called name + "Phys"                                       //
//----------------------------------------------------------------------//
static void placeBox(G4String name, G4ThreeVector min, G4ThreeVector max, G4Material *mat,
                     G4LogicalVolume *mother, G4Colour colour) {
    G4ThreeVector half = (max - min)/2.0;
    G4Box *solid = new G4Box(name, half.x(), half.y(), half.z());
    G4LogicalVolume *logical = new G4LogicalVolume(solid, mat, name + "Logical");
    logical->SetVisAttributes(new G4VisAttributes(colour));
    new G4PVPlacement(nullptr, (max + min)/2.0, logical, name + "Phys", mother, false, 0, true);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// createG4Objects                                                      //
//----------------------------------------------------------------------//
void PhinderCell::createG4Objects(G4LogicalVolume *mother, std::map<G4String, G4Material*> &materials) {

    for(auto mat : {scintillatorMaterial, G4String("core"), G4String("clad"), G4String("readout")}) {
        if(materials.find(mat) == materials.end()) {
            G4Exception("PhinderCell::createG4Objects", "PhinderCell001", FatalException,
                        ("Unknown material: " + mat).c_str());
        }
    }

    placeBox("cell", G4ThreeVector(0, 0, 0), G4ThreeVector(cellX, cellY, cellZ),
             materials[scintillatorMaterial], mother, G4Colour(0.2, 0.5, 1.0, 0.3));

    //In Python the core is nested in the cladding box and covers its full x and y extent, so the
    //cladding is only a bottom and a top sheet. Geant4 does not allow a daughter to share faces with
    //its mother, so the three layers are placed as siblings
    G4double z0 = cellZ;
    G4double z1 = cellZ + cladThickness;
    G4double z2 = cellZ + guideThickness - cladThickness;
    G4double z3 = cellZ + guideThickness;
    placeBox("cladBottom", G4ThreeVector(guideXMin, guideYMin, z0), G4ThreeVector(guideXMax, guideYMax, z1),
             materials["clad"], mother, G4Colour(0.8, 0.8, 0.2, 0.5));
    placeBox("core", G4ThreeVector(guideXMin, guideYMin, z1), G4ThreeVector(guideXMax, guideYMax, z2),
             materials["core"], mother, G4Colour(0.2, 0.9, 0.3, 0.5));
    placeBox("cladTop", G4ThreeVector(guideXMin, guideYMin, z2), G4ThreeVector(guideXMax, guideYMax, z3),
             materials["clad"], mother, G4Colour(0.8, 0.8, 0.2, 0.5));

    //Readout: thin layer behind the +y face of the core, with the core refractive index.
    //Python catches the photon when it crosses that face, before any Fresnel reflection
    placeBox("readout", G4ThreeVector(guideXMin, guideYMax, z1), G4ThreeVector(guideXMax, guideYMax + readoutThickness, z2),
             materials["readout"], mother, G4Colour(1.0, 0.1, 0.1, 1.0));
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Print                                                                //
//----------------------------------------------------------------------//
void PhinderCell::Print() {
    G4cout << "\033[1;34m" << "PHINDER cell: " << scintillatorMaterial << ", "
           << cellX/cm << " x " << cellY/cm << " x " << cellZ/cm << " cm, in [0,"
           << cellX/cm << "]x[0," << cellY/cm << "]x[0," << cellZ/cm << "]" << G4endl;
    G4cout << "Guide: x in [" << guideXMin/cm << ", " << guideXMax/cm << "], y in [" << guideYMin/cm << ", "
           << guideYMax/cm << "], z in [" << cellZ/cm << ", " << (cellZ + guideThickness)/cm
           << "] cm; readout on the +y face of the core" << "\033[0m" << G4endl;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//