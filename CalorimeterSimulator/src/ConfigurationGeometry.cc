//------------------------------------------------------------//

#include "ConfigurationGeometry.hh"
#include <json/json.h>
#include <json/value.h>
#include "G4VisAttributes.hh"
#include <algorithm>



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
ConfigurationGeometry::ConfigurationGeometry(G4String file) {

    Json::Value root;
    Json::Reader reader;

    //We open the JSON file, read it and put it on a string
    std::stringstream  filecontent;
    std::ifstream infile(file.c_str());

    if(!(infile.good())) {
        G4cerr << "\033[1;31m" << "Error opening geometry file: " + file << "\033[0m" << G4endl;
        goodGeometry = false;
        return;
    }

    std::string currline;
    while(getline(infile,currline)) {
        filecontent<<currline;
    }

    infile.close();

    std::string FileContent=filecontent.str();

    //Parsing of the JSON file
    bool parsingSuccessful = reader.parse( FileContent, root );
    if ( !parsingSuccessful ) {
        G4cerr << "\033[1;31m" << "Error parsing file: " + file << "\033[0m" << G4endl;
        goodGeometry = false;
        return;
    }

    if( root.size() > 0 ) {
        //Definition of the Universe ----------------------------------------------
        G4double xSize = atof(root["theWorld"]["xSizeWorld"].asString().c_str());
        G4double ySize = atof(root["theWorld"]["ySizeWorld"].asString().c_str());
        G4double zSize = atof(root["theWorld"]["zSizeWorld"].asString().c_str());

        if(xSize <= 0 || ySize <= 0|| zSize <= 0) {
            G4cerr << "\033[1;31m" << "The size of the Universe has been greater than 0" << "\033[0m" << G4endl;
            goodGeometry = false;
            return;
        }

        uniSizeX = xSize * CLHEP::cm;
        uniSizeY = ySize * CLHEP::cm;
        uniSizeZ = zSize * CLHEP::cm;

        //Definition of the beam---------------------------------------
        xBeamPosition = atof(root["theBeam"]["xBeamPosition"].asString().c_str()) * CLHEP::cm;
        xBeamSigma = atof(root["theBeam"]["xBeamSigma"].asString().c_str()) * CLHEP::cm;
        yBeamPosition = atof(root["theBeam"]["yBeamPosition"].asString().c_str()) * CLHEP::cm;
        yBeamSigma = atof(root["theBeam"]["yBeamSigma"].asString().c_str()) * CLHEP::cm;    
        zBeamPosition = atof(root["theBeam"]["zBeamPosition"].asString().c_str()) * CLHEP::cm;
        xBeamDir = atof(root["theBeam"]["xDir"].asString().c_str()) * CLHEP::degree;
        yBeamDir = atof(root["theBeam"]["yDir"].asString().c_str()) * CLHEP::degree;
        zBeamDir = atof(root["theBeam"]["zDir"].asString().c_str()) * CLHEP::degree;
        energy = atof(root["theBeam"]["energy"].asString().c_str()) * CLHEP::MeV;
        energySigma = atof(root["theBeam"]["energySigma"].asString().c_str()) * CLHEP::MeV;
        tBeamSigma = atof(root["theBeam"]["tBeamSigma"].asString().c_str()) * CLHEP::ns;
        maxOpenAngle = atof(root["theBeam"]["maxOpenAngle"].asString().c_str()) * CLHEP::degree;
        nParticles = atoi(root["theBeam"]["nParticles"].asString().c_str());
        nStep = atoi(root["theBeam"]["nStep"].asString().c_str());
        if(root["theBeam"]["particleDistribution"].asString().find("Poisson") != -1) {
            particleDistribution = 1;
        } else {
            particleDistribution = 0;
        }
        if(root["theBeam"]["energyDistribution"].asString().find("Gauss") != -1) {
            energyDistribution = 2;
        } else {
            energyDistribution = 0;
        }

        particleName = root["theBeam"].get("particle", "proton").asString();

        killPhotons = root["Photons"].get("killPhotons", true).asBool();
        storePhotons = root["Photons"].get("storePhotons", false).asBool();
        opticalPhysics = root["Photons"].get("opticalPhysics", true).asBool();

        //Definition of the Calorimeter ----------------------------------------------
        const Json::Value jCal = root["Calorimeter"];
        if(jCal.isNull()) {
            G4cerr << "\033[1;31m" << "The configuration file must contain a \"Calorimeter\" block" << "\033[0m" << G4endl;
            goodGeometry = false;
            return;
        }
        G4String name = jCal.get("name", "calorimeter").asString().c_str();
        G4double xPos = atof(jCal["xPos"].asString().c_str()) * CLHEP::cm;
        G4double yPos = atof(jCal["yPos"].asString().c_str()) * CLHEP::cm;
        G4double zPos = atof(jCal["zPos"].asString().c_str()) * CLHEP::cm;
        G4double xDir = atof(jCal["xDir"].asString().c_str()) * CLHEP::degree;
        G4double yDir = atof(jCal["yDir"].asString().c_str()) * CLHEP::degree;
        G4double zDir = atof(jCal["zDir"].asString().c_str()) * CLHEP::degree;
        G4double xSizeCal = atof(jCal["xSize"].asString().c_str()) * CLHEP::cm;
        G4double ySizeCal = atof(jCal["ySize"].asString().c_str()) * CLHEP::cm;
        G4String absorberMaterial = jCal["absorberMaterial"].asString().c_str();
        G4double absorberThickness = atof(jCal["absorberThickness"].asString().c_str()) * CLHEP::cm;
        G4String scintillatorMaterial = jCal["scintillatorMaterial"].asString().c_str();
        G4int nLayers = jCal.get("nLayers", 0).asInt();

        //The scintillator thickness is either a single number (same dz for all the layers)
        //or an array with one dz per layer (front to back)
        std::vector<G4double> scintillatorThickness;
        const Json::Value jdz = jCal["scintillatorThickness"];
        if(jdz.isArray()) {
            for(G4int i = 0; i < jdz.size(); i++) scintillatorThickness.push_back(jdz[i].asDouble() * CLHEP::cm);
            if(nLayers != 0 && nLayers != (G4int) scintillatorThickness.size()) {
                G4cerr << "\033[1;31m" << "nLayers does not match the size of scintillatorThickness" << "\033[0m" << G4endl;
                goodGeometry = false;
                return;
            }
        } else {
            scintillatorThickness.assign(nLayers, atof(jdz.asString().c_str()) * CLHEP::cm);
        }

        if(xSizeCal <= 0 || ySizeCal <= 0 || absorberThickness <= 0 || scintillatorThickness.empty() ||
           *std::min_element(scintillatorThickness.begin(), scintillatorThickness.end()) <= 0) {
            G4cerr << "\033[1;31m" << "The sizes, thicknesses and number of layers of the calorimeter have to be greater than 0" << "\033[0m" << G4endl;
            goodGeometry = false;
            return;
        }
        calorimeter = new Calorimeter(xPos, yPos, zPos, xDir, yDir, zDir, xSizeCal, ySizeCal,
                                      absorberThickness, absorberMaterial,
                                      scintillatorThickness, scintillatorMaterial, name);

        G4ThreeVector halfSize = calorimeter->getSize() / 2.0;
        if(std::abs(xPos) + halfSize.x() > uniSizeX/2.0 || std::abs(yPos) + halfSize.y() > uniSizeY/2.0 ||
           std::abs(zPos) + halfSize.z() > uniSizeZ/2.0) {
            G4cerr << "\033[1;31m" << "The calorimeter is not contained in the world" << "\033[0m" << G4endl;
            goodGeometry = false;
            return;
        }
        registerCollection("CalorimeterHits");
    }    
    goodGeometry = true;
    Print();
    return;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
bool ConfigurationGeometry::isGood() {
    return goodGeometry;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::getSizeX() {
    return uniSizeX;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::getSizeY() {
    return uniSizeY;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::getSizeZ() {
    return uniSizeZ;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
Calorimeter * ConfigurationGeometry::getCalorimeter() {
    return calorimeter;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//




//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetXBeamPosition() {
    return xBeamPosition;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetXBeamSigma() {
    return xBeamSigma;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetYBeamPosition() {
    return yBeamPosition;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetYBeamSigma() {
    return yBeamSigma;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetZBeamPosition() {
    return zBeamPosition;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetXDirBeam() {
    return xBeamDir;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetYDirBeam() {
    return yBeamDir;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetZDirBeam() {
    return zBeamDir;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetEnergy() {
    return energy;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetEnergySigma() {
    return energySigma;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetTBeamSigma() {
    return tBeamSigma;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4double ConfigurationGeometry::GetMaxOpenAngle() {
    return maxOpenAngle;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4String ConfigurationGeometry::GetParticleName() {
    return particleName;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4int ConfigurationGeometry::GetNParticles() {
    return nParticles;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4int ConfigurationGeometry::GetNStep() {
    return nStep;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4int ConfigurationGeometry::GetEnergyDistribution() {
    return energyDistribution;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
G4int ConfigurationGeometry::GetParticleDistribution() {
    return particleDistribution;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//



//----------------------------------------------------------------------//
// createG4Objects                                                      //
//----------------------------------------------------------------------//
void ConfigurationGeometry::createG4objects(G4LogicalVolume *mother, 
                                            std::map<G4String, G4Material *> &materials, 
                                            G4SDManager *SDman) {

    //Gantry: a cone at the position of the beam. It is made of air so that it is only a visual element
    G4ThreeVector pos;
    pos.setX(GetXBeamPosition());
    pos.setY(GetYBeamPosition());
    pos.setZ(GetZBeamPosition());
    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateY(yBeamDir);
    rot->rotateX(xBeamDir);
    G4ThreeVector newpos = *rot * pos;

    coneVolume = new G4Cons("gantry", 0.5*CLHEP::cm, 1.0*CLHEP::cm, 3.0*CLHEP::cm, 3.5*CLHEP::cm, 5.0*CLHEP::cm, 0.0, 2.0*3.14159287);
    conelogicalVolume = new G4LogicalVolume(coneVolume, materials["air"], "gantrylogical");
    conePhysical = new G4PVPlacement(rot, newpos, conelogicalVolume, "gantryphysical", mother, false, 0, true);

    G4Colour silicon(238.0/256.0, 162.0/256.0, 11.0/256.0);
    G4VisAttributes *attlog = new G4VisAttributes(silicon);
    conelogicalVolume->SetVisAttributes(attlog);

    calorimeter->createG4Objects(mother, materials, SDman);

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Accesor to class information                                         //
//----------------------------------------------------------------------//
void ConfigurationGeometry::registerCollection(G4String a) {
    collections.push_back(a);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Print the class information                                          //
//----------------------------------------------------------------------//
void ConfigurationGeometry::Print() {

    G4cout << "\033[1;34m" << "---------------------------------------Geometry information-----------------------------------" << "\033[0m" << G4endl;
    G4cout << "\033[1;34m" << "The loaded geometry is as follows: " << G4endl;
    G4cout << "\033[1;34m" << "The world is contained in [" << -uniSizeX/2.0/CLHEP::cm << ", " << uniSizeX/2.0/CLHEP::cm << "]x[" << -uniSizeY/2.0/CLHEP::cm << ", " << uniSizeY/2.0/CLHEP::cm << "]x[" << -uniSizeZ/2.0/CLHEP::cm << ", " << uniSizeZ/2.0/CLHEP::cm << "]" << G4endl;
    calorimeter->Print();
    G4cout << "\033[0m" << G4endl;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//




