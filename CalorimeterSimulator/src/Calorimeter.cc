#include "Calorimeter.hh"
#include "CalorimeterSensor.hh"
#include "G4VisAttributes.hh"
#include <numeric>


//----------------------------------------------------------------------//
// Total depth of the calorimeter                                       //
//----------------------------------------------------------------------//
static G4double totalDepth(G4double absorberThickness, const std::vector<G4double> &scintillatorThickness) {
    return scintillatorThickness.size() * absorberThickness +
           std::accumulate(scintillatorThickness.begin(), scintillatorThickness.end(), 0.0);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
Calorimeter::Calorimeter(G4double xPos, G4double yPos, G4double zPos,
                         G4double xRot, G4double yRot, G4double zRot,
                         G4double xSize, G4double ySize,
                         G4double absorberThickness_, G4String absorberMaterial_,
                         std::vector<G4double> scintillatorThickness_, G4String scintillatorMaterial_,
                         G4String name_) :
                         GeomObject(xPos, yPos, zPos, xRot, yRot, zRot, xSize, ySize,
                                    totalDepth(absorberThickness_, scintillatorThickness_)) {
                            name = name_;
                            absorberThickness = absorberThickness_;
                            absorberMaterial = absorberMaterial_;
                            scintillatorThickness = scintillatorThickness_;
                            scintillatorMaterial = scintillatorMaterial_;
                         };
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return name                                                          //
//----------------------------------------------------------------------//
G4String Calorimeter::getName() {
	return name;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Return number of layers                                              //
//----------------------------------------------------------------------//
G4int Calorimeter::getNLayers() {
	return scintillatorThickness.size();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Return total depth (along z)                                         //
//----------------------------------------------------------------------//
G4double Calorimeter::getDepth() {
	return sizes[2];
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Return position                                                      //
//----------------------------------------------------------------------//
G4ThreeVector Calorimeter::getPosition() {
	return getPos();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Return full sizes                                                    //
//----------------------------------------------------------------------//
G4ThreeVector Calorimeter::getSize() {
	return getSizes();
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// createG4Objects                                                      //
//----------------------------------------------------------------------//
void Calorimeter::createG4Objects(G4LogicalVolume *mother,
                                  std::map<G4String, G4Material*> & materials,
                                  G4SDManager *SDman) {

    for(auto mat : {absorberMaterial, scintillatorMaterial}) {
        if(materials.find(mat) == materials.end()) {
            G4Exception("Calorimeter::createG4Objects", "Calorimeter001", FatalException,
                        ("Unknown calorimeter material: " + mat).c_str());
        }
    }

    //Envelope of the calorimeter: a rectangular prism filled with air
    G4String calorimeterName = G4String("calorimeter_") + name;
    solidVolume = new G4Box(calorimeterName, sizes[0]/2.0, sizes[1]/2.0, sizes[2]/2.0);
    logicalVolume = new G4LogicalVolume(solidVolume, materials["air"], calorimeterName);
    G4String calorimeterPhysicalName = G4String("calorimeterPhys_") + name;
    physicalVolume = new G4PVPlacement(getRot(), getPos(), logicalVolume, calorimeterPhysicalName,
                                       mother, false, 0, true);
    G4VisAttributes *envelopeAtt = new G4VisAttributes();
    envelopeAtt->SetForceWireframe(true);
    logicalVolume->SetVisAttributes(envelopeAtt);

    //Both absorber and scintillator are sensitive: we store the deposited energy in each of them
    CalorimeterSensor *sensor = new CalorimeterSensor(G4String("CalorimeterSensor_") + name, "CalorimeterHits");
    SDman->AddNewDetector(sensor);

    //One absorber solid is shared by all the layers (fixed thickness)
    G4Box *absorberSolid = new G4Box("absorber", sizes[0]/2.0, sizes[1]/2.0, absorberThickness/2.0);
    G4LogicalVolume *absorberLogical = new G4LogicalVolume(absorberSolid, materials[absorberMaterial], "absorberLogical");
    absorberLogical->SetSensitiveDetector(sensor);
    absorberLogical->SetVisAttributes(new G4VisAttributes(G4Colour(0.4, 0.4, 0.4, 0.6)));
    G4VisAttributes *scintillatorAtt = new G4VisAttributes(G4Colour(0.2, 0.5, 1.0, 0.3));

    //Layers are stacked from the front face (+z) to the back face (-z). The copy number is the layer index
    G4double zFront = sizes[2]/2.0;
    for(G4int i = 0; i < getNLayers(); i++) {
        G4double zAbsorber = zFront - absorberThickness/2.0;
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, zAbsorber), absorberLogical,
                          "absorberPhys", logicalVolume, false, i, true);

        //Each scintillator gets its own solid so that its thickness can change with z
        G4double dz = scintillatorThickness[i];
        G4String scintName = G4String("scintillator_") + std::to_string(i);
        G4Box *scintSolid = new G4Box(scintName, sizes[0]/2.0, sizes[1]/2.0, dz/2.0);
        G4LogicalVolume *scintLogical = new G4LogicalVolume(scintSolid, materials[scintillatorMaterial], scintName);
        scintLogical->SetSensitiveDetector(sensor);
        scintLogical->SetVisAttributes(scintillatorAtt);
        G4double zScint = zFront - absorberThickness - dz/2.0;
        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, zScint), scintLogical,
                          "scintillatorPhys", logicalVolume, false, i, true);

        zFront -= absorberThickness + dz;
    }
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Print                                                                //
//----------------------------------------------------------------------//
void Calorimeter::Print() {

    G4cout << "\033[1;34m" << "Calorimeter " << name << "\033[0m" << G4endl;
    G4cout << "\033[1;34m" << "Location x: " << pos.x()/CLHEP::cm << ", y: " << pos.y()/CLHEP::cm << ", z: " << pos.z()/CLHEP::cm << G4endl;
    G4cout << "\033[1;34m" << "Rotation x: " << rots.x() << ", y: " << rots.y() << ", z: " << rots.z() << G4endl;
    G4cout << "\033[1;34m" << "Sizes x: " << sizes.x()/CLHEP::cm << ", y: " << sizes.y()/CLHEP::cm << ", z: " << sizes.z()/CLHEP::cm << G4endl;
    G4cout << "\033[1;34m" << "Number of layers: " << getNLayers() << G4endl;
    G4cout << "\033[1;34m" << "Absorber: " << absorberMaterial << ", " << absorberThickness/CLHEP::mm << " mm" << G4endl;
    G4cout << "\033[1;34m" << "Scintillator: " << scintillatorMaterial << ", thicknesses (mm):";
    for(auto dz : scintillatorThickness) G4cout << " " << dz/CLHEP::mm;
    G4cout << "\033[0m" << G4endl << G4endl;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//
