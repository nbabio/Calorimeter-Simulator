#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "PhinderCell.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include <cmath>

//Same values as phinder_stage1.py
static const G4double cellX = 5.0*cm, cellY = 5.0*cm, cellZ = 1.0*cm;
static const G4double guideXMin = 2.0*cm, guideXMax = 3.0*cm;
static const G4double guideYMin = 0.0*cm, guideYMax = 5.2*cm;
static const G4double guideThickness = 0.1*cm, cladThickness = 0.001*cm;
static const G4double readoutThickness = 1.0*um;

//Crossed straight guides (forward simulation default geometry). Centrelines in the xy plane: from
//the far end to the readout. Channel 0 is below the crystal, channel 1 above it
static const G4double crossedWidth = 0.6*cm, crossedThickness = 0.1*cm, crossedClad = 10.0*um;
static const G4ThreeVector ch0Start(1.0*cm, 5.0*cm, 0), ch0End(4.0*cm, 0.5*cm, 0);
static const G4ThreeVector ch1Start(4.0*cm, 5.0*cm, 0), ch1End(1.0*cm, 0.5*cm, 0);


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
PhinderCell::PhinderCell(G4String material, G4String geometry_) {
    scintillatorMaterial = material;
    geometry = geometry_;
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
//----------------------------------------------------------------------//
// createG4Objects: the crystal is the same in both geometries          //
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

    if(geometry == "stage1") createStage1Guide(mother, materials);
    else if(geometry == "crossedGuides") createCrossedGuides(mother, materials);
    else G4Exception("PhinderCell::createG4Objects", "PhinderCell002", FatalException,
                     ("Unknown PhinderCell geometry: " + geometry + " (use stage1 or crossedGuides)").c_str());
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Stage 1: one straight guide along y on the +z face                   //
//----------------------------------------------------------------------//
void PhinderCell::createStage1Guide(G4LogicalVolume *mother, std::map<G4String, G4Material*> &materials) {

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
// Axis of a straight guide in the xy plane: local y runs along the     //
// guide (from start to readout), local x across it                     //
//----------------------------------------------------------------------//
struct GuideAxis {
    G4ThreeVector start, dir, perp;
    G4double length;
    G4RotationMatrix rot;
};

static GuideAxis makeAxis(G4ThreeVector start, G4ThreeVector end) {
    GuideAxis a;
    a.start = start;
    a.length = (end - start).mag();
    a.dir = (end - start).unit();
    G4double phi = std::atan2(-a.dir.x(), a.dir.y());
    a.rot.rotateZ(phi);
    a.perp = G4ThreeVector(std::cos(phi), std::sin(phi), 0);
    return a;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Places a box of a guide given its extent along the axis (u), across  //
// it (v) and in z. The copy number is the channel                      //
//----------------------------------------------------------------------//
static void placeGuideBox(G4String name, G4int copy, const GuideAxis &axis,
                          G4double u0, G4double u1, G4double v0, G4double v1, G4double z0, G4double z1,
                          G4Material *mat, G4LogicalVolume *mother, G4Colour colour) {
    G4Box *solid = new G4Box(name, (v1 - v0)/2.0, (u1 - u0)/2.0, (z1 - z0)/2.0);
    G4LogicalVolume *logical = new G4LogicalVolume(solid, mat, name + "Logical");
    logical->SetVisAttributes(new G4VisAttributes(colour));
    G4ThreeVector centre = axis.start + axis.dir*((u0 + u1)/2.0) + axis.perp*((v0 + v1)/2.0)
                         + G4ThreeVector(0, 0, (z0 + z1)/2.0);
    new G4PVPlacement(G4Transform3D(axis.rot, centre), logical, name + "Phys", mother, false, copy, true);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Crossed guides: two straight guides on opposite faces that form an X //
// in the xy projection. Core and cladding have the same length (no end //
// caps), so each guide is five sibling boxes: core, cladding bottom,   //
// top and two sides. The readout is behind the terminal core face      //
//----------------------------------------------------------------------//
void PhinderCell::createCrossedGuides(G4LogicalVolume *mother, std::map<G4String, G4Material*> &materials) {

    struct Guide {
        G4ThreeVector start, end;
        G4double zMin;
        G4Colour colour;
    };
    std::vector<Guide> guides = {
        {ch0Start, ch0End, -crossedThickness, G4Colour(0.2, 0.5, 0.9, 0.6)},   // channel 0, below the crystal
        {ch1Start, ch1End, cellZ,             G4Colour(0.9, 0.6, 0.2, 0.6)}    // channel 1, above the crystal
    };

    G4double w = crossedWidth/2.0, t = crossedClad;
    for(G4int ch = 0; ch < (G4int) guides.size(); ch++) {
        GuideAxis axis = makeAxis(guides[ch].start, guides[ch].end);
        G4double L = axis.length;
        G4double z0 = guides[ch].zMin, z1 = z0 + crossedThickness;
        G4String id = std::to_string(ch);
        G4Colour cladColour(0.8, 0.8, 0.2, 0.5);

        placeGuideBox("core" + id, ch, axis, 0, L, -w + t, w - t, z0 + t, z1 - t, materials["core"], mother, guides[ch].colour);
        placeGuideBox("cladBottom" + id, ch, axis, 0, L, -w, w, z0, z0 + t, materials["clad"], mother, cladColour);
        placeGuideBox("cladTop" + id, ch, axis, 0, L, -w, w, z1 - t, z1, materials["clad"], mother, cladColour);
        placeGuideBox("cladSideA" + id, ch, axis, 0, L, -w, -w + t, z0 + t, z1 - t, materials["clad"], mother, cladColour);
        placeGuideBox("cladSideB" + id, ch, axis, 0, L, w - t, w, z0 + t, z1 - t, materials["clad"], mother, cladColour);

        //Readout: same name for both channels, the copy number is the channel
        G4Box *solid = new G4Box("readout" + id, w - t, readoutThickness/2.0, (z1 - z0)/2.0 - t);
        G4LogicalVolume *logical = new G4LogicalVolume(solid, materials["readout"], "readout" + id + "Logical");
        logical->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 0.1, 0.1, 1.0)));
        G4ThreeVector centre = axis.start + axis.dir*(L + readoutThickness/2.0) + G4ThreeVector(0, 0, (z0 + z1)/2.0);
        new G4PVPlacement(G4Transform3D(axis.rot, centre), logical, "readoutPhys", mother, false, ch, true);
    }
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Print                                                                //
//----------------------------------------------------------------------//
void PhinderCell::Print() {
    G4cout << "\033[1;34m" << "PHINDER cell (" << geometry << "): " << scintillatorMaterial << ", "
           << cellX/cm << " x " << cellY/cm << " x " << cellZ/cm << " cm, in [0,"
           << cellX/cm << "]x[0," << cellY/cm << "]x[0," << cellZ/cm << "]" << G4endl;
    if(geometry == "crossedGuides") {
        G4cout << "Channel 0 (z in [" << -crossedThickness/cm << ", 0]): (" << ch0Start.x()/cm << ", " << ch0Start.y()/cm
               << ") -> readout (" << ch0End.x()/cm << ", " << ch0End.y()/cm << ") cm" << G4endl;
        G4cout << "Channel 1 (z in [" << cellZ/cm << ", " << (cellZ + crossedThickness)/cm << "]): (" << ch1Start.x()/cm
               << ", " << ch1Start.y()/cm << ") -> readout (" << ch1End.x()/cm << ", " << ch1End.y()/cm << ") cm" << G4endl;
        G4cout << "Section " << crossedWidth/cm << " x " << crossedThickness/cm << " cm, cladding "
               << crossedClad/um << " um, length " << (ch0End - ch0Start).mag()/cm << " cm" << "\033[0m" << G4endl;
    } else {
        G4cout << "Guide: x in [" << guideXMin/cm << ", " << guideXMax/cm << "], y in [" << guideYMin/cm << ", "
               << guideYMax/cm << "], z in [" << cellZ/cm << ", " << (cellZ + guideThickness)/cm
               << "] cm; readout on the +y face of the core" << "\033[0m" << G4endl;
    }
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//