import json, os, optparse
import copy
import numpy as np


# Datasets: PID [p, pi+, K+] x energy [5-1000 GeV, 20 log(E)-spaced values] = 60 datasets
# Each dataset is split in several jobs (one Generator process each, with its own seed).
# This script writes one configuration file per dataset and a list of jobs (one command per line)
# that can be run in parallel with runDatasets.sh


PARTICLES = {"proton": "p", "pi+": "pip", "kaon+": "kp"}


if __name__=='__main__':

    parser = optparse.OptionParser(usage='usage: %prog [options]', version='%prog 1.0')
    parser.add_option('-i', '--input', action='store', type='string', dest='inputFile', default='myConf.json', help='Base configuration file (world, beam and calorimeter geometry)')
    parser.add_option('-o', '--outdir', action='store', type='string', dest='outdir', default='datasets', help='Output directory for configurations, jobs and ROOT files')
    parser.add_option('-g', '--generator', action='store', type='string', dest='generator', default='../../CalorimeterSimulator-build/Generator', help='Path to the Generator executable')
    parser.add_option('-n', '--events', action='store', type='int', dest='events', default=200000, help='Events per dataset')
    parser.add_option('-j', '--jobs', action='store', type='int', dest='jobs', default=20, help='Jobs per dataset')
    parser.add_option('--emin', action='store', type='float', dest='emin', default=5.0, help='Minimum energy (GeV)')
    parser.add_option('--emax', action='store', type='float', dest='emax', default=1000.0, help='Maximum energy (GeV)')
    parser.add_option('--nE', action='store', type='int', dest='nE', default=20, help='Number of log(E)-spaced energies')
    parser.add_option('--nLayers', action='store', type='int', dest='nLayers', default=0, help='Override the number of layers of the base configuration (0 = keep it)')
    parser.add_option('--seed', action='store', type='int', dest='seed', default=1, help='First random seed (each job gets a different one)')

    (opts, args) = parser.parse_args()

    with open(opts.inputFile, 'r') as file:
        base = json.load(file)

    if opts.nLayers > 0:
        base['Calorimeter']['nLayers'] = opts.nLayers

    #High-energy showers: no optical photons (they would be tens of millions per event)
    base['Photons']['opticalPhysics'] = False
    base['Photons']['killPhotons'] = True
    base['Photons']['storePhotons'] = False

    energies = np.logspace(np.log10(opts.emin), np.log10(opts.emax), opts.nE) # GeV
    eventsPerJob = int(np.ceil(opts.events / opts.jobs))

    confDir = os.path.join(opts.outdir, 'conf')
    rootDir = os.path.join(opts.outdir, 'root')
    logDir = os.path.join(opts.outdir, 'logs')
    for d in [confDir, rootDir, logDir]:
        os.makedirs(d, exist_ok=True)
    generator = os.path.abspath(opts.generator)

    jobs = []
    summary = []
    seed = opts.seed
    for particle, tag in PARTICLES.items():
        for iE, E in enumerate(energies):
            name = '%s_E%02d_%.1fGeV' % (tag, iE, E)
            conf = copy.deepcopy(base)
            conf['theBeam']['particle'] = particle
            conf['theBeam']['energy'] = E * 1000.0 # MeV
            conf['theBeam']['energyDistribution'] = "Constant"
            conf['theBeam']['energySigma'] = 0.0
            confFile = os.path.abspath(os.path.join(confDir, name + '.json'))
            with open(confFile, 'w') as f:
                f.write(json.dumps(conf, indent=4))
            remaining = opts.events
            offset = 0
            for job in range(opts.jobs):
                n = min(eventsPerJob, remaining)
                if n <= 0:
                    break
                remaining -= n
                out = os.path.abspath(os.path.join(rootDir, '%s_%03d.root' % (name, job)))
                log = os.path.abspath(os.path.join(logDir, '%s_%03d.log' % (name, job)))
                #The output is renamed only if the job finishes, so that interrupted jobs are run again
                tmp = out.replace('.root', '.tmp.root')
                jobs.append('%s --input %s --output %s --number %d --seed %d --eventOffset %d > %s 2>&1 && mv %s %s' % (generator, confFile, tmp, n, seed, offset, log, tmp, out))
                offset += n
                seed += 1
            summary.append({'name': name, 'particle': particle, 'energyGeV': E, 'events': opts.events, 'config': confFile})

    with open(os.path.join(opts.outdir, 'jobs.txt'), 'w') as f:
        f.write('\n'.join(jobs) + '\n')
    with open(os.path.join(opts.outdir, 'datasets.json'), 'w') as f:
        f.write(json.dumps(summary, indent=4))

    print('Energies (GeV):', ' '.join('%.1f' % e for e in energies))
    print('%d datasets, %d jobs written to %s' % (len(summary), len(jobs), os.path.join(opts.outdir, 'jobs.txt')))
