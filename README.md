# Calorimeter Simulator

A **Geant4** simulation of a **sampling calorimeter** made of alternating layers of absorber (tungsten by default) and plastic scintillator (EJ-200 / EJ-204). A particle beam (protons, pions, kaons, …) hits the calorimeter and, for every event, the program writes to a ROOT file the energy deposited layer by layer, the total energy and, optionally, every optical photon produced in the scintillators.

The geometry, the beam and the physics options are all defined in a **JSON** file, so you don't need to recompile to change the setup.

---

## Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Quick start](#quick-start)
- [Command-line options](#command-line-options)
- [Configuration file (JSON)](#configuration-file-json)
- [Output file](#output-file)
- [Large-scale dataset production](#large-scale-dataset-production)
- [How it works](#how-it-works)
- [Repository layout](#repository-layout)

---

## Requirements

| Dependency | Tested version | Used for |
|---|---|---|
| [Geant4](https://geant4.web.cern.ch/) | 11.1 – 11.3 | Simulation (with datasets installed; Qt/OpenGL optional for visualization) |
| [JsonCpp](https://github.com/open-source-parsers/jsoncpp) | — | Reading the configuration file (linked as the static library `libjsoncpp.a`) |
| CMake | ≥ 3.5 | Build |
| C++ compiler | C++17 | Build |
| [ROOT](https://root.cern/) | 6.28+ | Optional: analysis, and `hadd` to merge files |
| Python 3 + `numpy` | — | Optional: configuration and dataset generation scripts |

## Installation

### 1. Install Geant4

Follow the [official guide](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/). Make sure to build it with the datasets (`-DGEANT4_INSTALL_DATA=ON`) and, if you want visualization, with Qt or OpenGL (`-DGEANT4_USE_QT=ON`, `-DGEANT4_USE_OPENGL_X11=ON`).

### 2. Install JsonCpp

```bash
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/software/jsoncpp -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF
make -j$(nproc) install
```

`CMakeLists.txt` looks for the headers in `$JSONCPPDIR/include/` and for the library in `$JSONCPPDIR/lib/libjsoncpp.a`.

### 3. Set up the environment

Before building or running, load Geant4 and define `JSONCPPDIR`:

```bash
export G4INSTALLDIR=/path/to/geant4-install
export JSONCPPDIR=/path/to/jsoncpp
source $G4INSTALLDIR/bin/geant4.sh
source /path/to/root-install/bin/thisroot.sh   # optional
```

The [setup.sh](setup.sh) script does this automatically based on the machine's hostname; add a block with your own paths and run `source setup.sh`.

### 4. Build

```bash
git clone https://github.com/nbabio/CalorimeterSimulator.git
cd Calorimeter-Simulator
mkdir CalorimeterSimulator-build && cd CalorimeterSimulator-build
cmake ../CalorimeterSimulator
make -j$(nproc)
```

This produces the `Generator` executable in `CalorimeterSimulator-build/`. To build without the graphical interface (e.g. on a cluster) use `cmake -DWITH_GEANT4_UIVIS=OFF ../CalorimeterSimulator`.

## Quick start

From `CalorimeterSimulator-build/`:

```bash
# Batch mode: 1000 events, output in output.root
./Generator --input ../CalorimeterSimulator/data/confExample.json \
            --output output.root --number 1000 --seed 1

# Interactive mode with visualization (OpenGL)
./Generator --input ../CalorimeterSimulator/data/confExample.json \
            --output output.root --number 1 --geant vis
```

In interactive mode [init_vis.mac](CalorimeterSimulator/init_vis.mac) is executed and the Geant4 session opens; shoot events with `/run/beamOn 10`.

## Command-line options

| Option | Required | Description |
|---|---|---|
| `--input <file.json>` | yes | Configuration file |
| `--output <file.root>` | yes | Output file (the `.root` extension sets the format) |
| `--number <N>` | yes | Number of events (> 0) |
| `--seed <seed>` | no | Random number generator seed (default 0) |
| `--geant <any value>` | no | If given, starts the interactive session with visualization instead of batch mode |
| `--killPhotons <0/1>` | no | Overrides `Photons.killPhotons` from the JSON |
| `--eventOffset <N>` | no | Added to the event number in the output. Useful to merge files from several jobs without repeated event numbers |
| `--pt <value>` | no | Accepted for backwards compatibility; currently has no effect |

## Configuration file (JSON)

Full example in [confExample.json](CalorimeterSimulator/data/confExample.json). **Units:** lengths in cm, angles in degrees, energies in MeV, times in ns.

```json
{
    "theWorld":   { "xSizeWorld": 100, "ySizeWorld": 100, "zSizeWorld": 100 },
    "Photons":    { "killPhotons": true, "storePhotons": false, "opticalPhysics": true },
    "theBeam": {
        "particle": "proton",
        "xBeamPosition": 0, "xBeamSigma": 0.1,
        "yBeamPosition": 0, "yBeamSigma": 0.1,
        "zBeamPosition": 45,
        "xDir": 0.0, "yDir": 0.0, "zDir": 0.0,
        "maxOpenAngle": 0.02, "nStep": 100,
        "nParticles": 1,
        "energy": 250, "energySigma": 1, "energyDistribution": "Constant",
        "tBeamSigma": 1
    },
    "Calorimeter": {
        "name": "calorimeter",
        "xPos": 0, "yPos": 0, "zPos": 0,
        "xDir": 0, "yDir": 0, "zDir": 0,
        "xSize": 20, "ySize": 20,
        "nLayers": 30,
        "absorberMaterial": "tungsten", "absorberThickness": 0.2,
        "scintillatorMaterial": "EJ200", "scintillatorThickness": 1.0
    }
}
```

### `theWorld`

Full size (cm) of the air box that holds the simulation, centred at the origin. The calorimeter must be fully contained in it.

### `theBeam`

| Field | Description |
|---|---|
| `particle` | Geant4 particle name: `proton`, `pi+`, `kaon+`, `e-`, `mu-`, … |
| `xBeamPosition`, `yBeamPosition` | Beam centre in the transverse plane (cm) |
| `xBeamSigma`, `yBeamSigma` | Gaussian beam width (cm) |
| `zBeamPosition` | z position the particles are shot from. **The beam travels towards −z** |
| `xDir`, `yDir` | Rotation of the beam (position and direction) around the X and Y axes (degrees) |
| `maxOpenAngle`, `nStep` | The beam direction sweeps a grid of slopes in [−`maxOpenAngle`, +`maxOpenAngle`] with `nStep` steps, moving one step per event. Set it to `0` for a parallel beam |
| `nParticles` | Primary particles per event |
| `energy` | Kinetic energy (MeV) |
| `energyDistribution` | `Constant` or `Gaussian` |
| `energySigma` | Energy spread when `Gaussian` (MeV) |
| `tBeamSigma` | Gaussian spread of the initial time (ns) |

### `Calorimeter`

| Field | Description |
|---|---|
| `name` | Calorimeter name |
| `xPos`, `yPos`, `zPos` | Position of the centre (cm) |
| `xDir`, `yDir`, `zDir` | Rotations (degrees) |
| `xSize`, `ySize` | Transverse size (cm) |
| `nLayers` | Number of layers (absorber + scintillator) |
| `absorberMaterial` | `tungsten`, `lead`, `iron`, `steel`, `uranium`, `aluminium`, `carbon`, `silicon`, … |
| `absorberThickness` | Absorber thickness (cm) |
| `scintillatorMaterial` | `EJ200` or `EJ204` |
| `scintillatorThickness` | Scintillator thickness (cm). Either a number (same for every layer) or a list with one value per layer, front to back, e.g. `[1.0, 1.0, 2.0, 2.0]` |

The total depth is `nLayers × absorberThickness + Σ scintillatorThickness`.

### `Photons`

| Field | Default | Description |
|---|---|---|
| `opticalPhysics` | `true` | Enables scintillation and Cherenkov light. **Turn it off for high-energy showers**: they produce tens of millions of photons per event |
| `killPhotons` | `true` | Kills optical photons as soon as they are created (there is no photodetector yet) |
| `storePhotons` | `false` | Stores every optical photon created in a scintillator in the `photons` ntuple. Produces **very** large files |

### Generating configurations with Python

[makeConfiguration.py](CalorimeterSimulator/data/makeConfiguration.py) takes a template, changes the parameters set in the script and writes a new JSON file:

```bash
cd CalorimeterSimulator/data
python3 makeConfiguration.py -i confExample.json -o myConf.json
```

## Output file

The ROOT file contains three `TTree`s:

**`layers`**: one row per layer with deposited energy, per event

| Branch | Unit | Description |
|---|---|---|
| `eventNumber` | — | Event number (+ `eventOffset`) |
| `layer` | — | Layer index (0 = first layer crossed by the beam) |
| `energyScintillator` | MeV | Energy deposited in the scintillator |
| `energyAbsorber` | MeV | Energy deposited in the absorber |
| `x`, `y`, `z` | cm | Energy-weighted mean position of the deposit in the scintillator |

**`events`**: one row per event

| Branch | Unit | Description |
|---|---|---|
| `eventNumber` | — | Event number |
| `pdgID` | — | PDG code of the primary (2212 = p, 211 = π⁺, 321 = K⁺) |
| `primaryEnergy` | GeV | Kinetic energy of the primary |
| `energyScintillator` | GeV | Sum over all scintillators |
| `energyAbsorber` | GeV | Sum over all absorbers |

**`photons`**: only filled with `storePhotons: true`. One row per optical photon created in a scintillator: `eventNumber`, `layer`, `process` (0 = scintillation, 1 = Cherenkov), `trackID`, `parentID`, position (cm), direction, polarization, `energy` (eV), `wavelength` (nm) and `time` (ns).

A quick look in ROOT:

```bash
root -l output.root
root [1] events->Draw("energyScintillator/primaryEnergy")
root [2] layers->Draw("energyScintillator:layer", "", "prof")
```

## Large-scale dataset production

To produce large samples on several cores there are two scripts in [CalorimeterSimulator/data](CalorimeterSimulator/data):

1. **[makeDatasets.py](CalorimeterSimulator/data/makeDatasets.py)** starts from a base configuration and creates one configuration per particle × energy combination (p, π⁺ and K⁺ with log-spaced energies, by default 20 values between 5 and 1000 GeV). It splits each dataset into several jobs with different seeds and `eventOffset`, and turns optical physics off automatically.

   ```bash
   cd CalorimeterSimulator/data
   python3 makeDatasets.py -i myConf.json -o datasets -n 200000 -j 20 \
           --emin 5 --emax 1000 --nE 20
   ```

   | Option | Default | Description |
   |---|---|---|
   | `-i` | `myConf.json` | Base configuration |
   | `-o` | `datasets` | Output directory |
   | `-g` | `../../CalorimeterSimulator-build/Generator` | Path to the executable |
   | `-n` | 200000 | Events per dataset |
   | `-j` | 20 | Jobs per dataset |
   | `--emin`, `--emax`, `--nE` | 5, 1000, 20 | Energy range and number of energies (GeV) |
   | `--nLayers` | 0 | Overrides the number of layers (0 = keep it) |
   | `--seed` | 1 | First seed |

   It writes `datasets/conf/*.json`, `datasets/jobs.txt` (one command per line) and `datasets/datasets.json` (summary).

2. **[runDatasets.sh](CalorimeterSimulator/data/runDatasets.sh)** runs the jobs in parallel and merges the output of each dataset with `hadd`:

   ```bash
   ./runDatasets.sh datasets 8     # 8 processes in parallel
   ```

   Each process uses ~750 MB of RAM, so pick the number of processes accordingly. Finished jobs are skipped, so the script can be relaunched after an interruption. The merged files end up in `datasets/merged/`.

## How it works

```
JSON ──► ConfigurationGeometry ──► DetectorConstruction ──► world + calorimeter
                 │                                              │
                 ▼                                              ▼
       PrimaryGeneratorAction (Beam)                    CalorimeterSensor
                 │                                     (hits per layer)
                 ▼                                              │
        Geant4 transports the particles ────────────────────────┘
                 │
                 ▼
  StackingAction (optical photons)  ·  EventAction  ──►  RunAction  ──►  output.root
```

- **[Generator.cc](CalorimeterSimulator/Generator.cc)** is the entry point. It parses the options, builds the `G4RunManager` and registers all the user classes. Then it calls `BeamOn(N)` in batch mode or opens the interactive session.
- **[ConfigurationGeometry](CalorimeterSimulator/src/ConfigurationGeometry.cc)** reads and validates the JSON (positive sizes, calorimeter contained in the world, `nLayers` consistent with the thickness list) and converts everything to Geant4 units.
- **[DetectorConstruction](CalorimeterSimulator/src/DetectorConstruction.cc)** defines the materials and builds the world. Materials are NIST ones plus the EJ-200/EJ-204 scintillators with their optical properties: light yield, emission spectrum, rise/decay times, attenuation length, refractive index and Birks constant. It also places an air cone at the beam position as a visual reference (the "gantry").
- **[Calorimeter](CalorimeterSimulator/src/Calorimeter.cc)** is a rectangular air box holding `nLayers` absorber + scintillator pairs, stacked from the +z face (where the beam enters) towards −z. The copy number of each volume is its layer index.
- **[CalorimeterSensor](CalorimeterSimulator/src/CalorimeterSensor.cc) / [CalorimeterHit](CalorimeterSimulator/src/CalorimeterHit.cc)** is the sensitive detector attached to absorbers and scintillators. For each event it accumulates a single hit per layer and volume type, with the total energy and the energy-weighted mean position. Optical photons do not create hits.
- **[Beam](CalorimeterSimulator/src/Beam.cc) / [PrimaryGeneratorAction](CalorimeterSimulator/src/PrimaryGeneratorAction.cc)** generate the primaries. They use a Gaussian profile in x, y and t, a constant or Gaussian energy, and a direction that follows the opening-angle grid described above.
- **[ModularPhysicsList](CalorimeterSimulator/src/ModularPhysicsList.cc)** registers standard + extra EM physics, decays, `FTFP_BERT_HP` hadronic physics with HP elastic scattering, stopping, ions, radioactive decay and, optionally, `G4OpticalPhysics`.
- **[StackingAction](CalorimeterSimulator/src/StackingAction.cc)** intercepts every new optical photon. It stores the photon in the `photons` ntuple if requested, then kills it or lets it propagate depending on `killPhotons`.
- **[EventAction](CalorimeterSimulator/src/EventAction.cc)** reads the hits at the end of each event and fills the `layers` and `events` ntuples.
- **[RunAction](CalorimeterSimulator/src/RunAction.cc)** creates the ntuples at the start of the run, then writes and closes the file at the end.

## Repository layout

```
Calorimeter-Simulator/
├── setup.sh                      # Per-machine environment setup
└── CalorimeterSimulator/
    ├── CMakeLists.txt
    ├── Generator.cc              # main()
    ├── include/  src/            # Simulation classes
    ├── init_vis.mac  vis.mac     # Visualization macros
    ├── data/                     # JSON configurations and production scripts
    └── dataAnalysis/             # Analysis scripts (ROOT/Python)
```

> **Note:** most of the scripts in [dataAnalysis/](CalorimeterSimulator/dataAnalysis) (track reconstruction, POCA, …) come from the previous version of the project (a proton tomography simulator) and have not been adapted to the current output format.
