import json, sys, optparse
import copy



if __name__=='__main__':

    parser = optparse.OptionParser(usage='usage: %prog [options] path', version='%prog 1.0')
    parser.add_option('-i', '--input', action='store', type='string', dest='inputFile', default='confExample.json', help='Configuration file template')
    parser.add_option('-o', '--output', action='store', type='string', dest='outputFile', default='conf.json', help='Final configuration file')

    (opts, args) = parser.parse_args()

    with open(opts.inputFile, 'r') as file:
        data = json.load(file)

    theWorld = copy.copy(data['theWorld'])
    theBeam = copy.copy(data['theBeam'])
    calorimeter = copy.copy(data['Calorimeter'])
    photons = copy.copy(data['Photons'])
    
    #Modify world directly in data
    theWorld['xSizeWorld'] = 300
    theWorld['ySizeWorld'] = 300
    theWorld['zSizeWorld'] = 300
    #Modify beam directly in data (the beam travels towards -z)
    theBeam['particle'] = "proton" # proton (p), pi+ (pip), kaon+ (kp)
    theBeam['xBeamPosition'] = 0
    theBeam['yBeamPosition'] = 0
    theBeam['zBeamPosition'] = 140
    theBeam['xBeamSigma'] = 0.4
    theBeam['yBeamSigma'] = 0.4
    theBeam['xDir'] = 0
    theBeam['yDir'] = 0
    theBeam['zDir'] = 0
    theBeam['maxOpenAngle'] = 0.02
    theBeam['nStep'] = 100
    theBeam['nParticles'] = 1
    theBeam['nParticlesDistribution'] = "Constant"
    theBeam['energy'] = 220.0
    theBeam['energyDistribution'] = "Gaussian"
    theBeam['energySigma'] = 1.0
    theBeam['tBeamSigma'] = 3.0
    #Modify the calorimeter directly in data: a sandwich of nLayers x (absorber + scintillator)
    #The beam enters through the +z face (layer 0) and the photodetector will go behind the -z face
    nLayers = 1
    calorimeter['name'] = "calorimeter"
    calorimeter['xPos'] = 0
    calorimeter['yPos'] = 0
    calorimeter['zPos'] = 0 # Center of the calorimeter
    calorimeter['xDir'] = 0
    calorimeter['yDir'] = 0
    calorimeter['zDir'] = 0
    calorimeter['xSize'] = 20
    calorimeter['ySize'] = 20
    calorimeter['nLayers'] = nLayers
    calorimeter['absorberMaterial'] = "tungsten"
    calorimeter['absorberThickness'] = 0.2 # 2 mm
    calorimeter['scintillatorMaterial'] = "EJ200" # or "EJ204"
    calorimeter['scintillatorThickness'] = 1.0 # dz = 1 cm. It can also be a list with one dz per layer (front to back), e.g. [1.0]*15 + [2.0]*15
    #Modify photons directly in data
    photons['killPhotons'] = False # true = optical photons are killed as soon as they are created (there is no photodetector yet), false = they are tracked until they are absorbed
    photons['storePhotons'] = False # true = every optical photon born in a scintillator is stored in the "photons" ntuple (very large files!)
    photons['opticalPhysics'] = True # false = no optical photons at all (scintillation and Cherenkov are not simulated). Needed for high-energy productions


    newData = dict()
    newData['theWorld'] = theWorld
    newData['theBeam'] = theBeam
    newData['Photons'] = photons
    newData['Calorimeter'] = calorimeter

    print('Making a copy')
    json_str = json.dumps(newData, indent=4)
    with open(opts.outputFile, "w") as f:
        f.write(json_str)
