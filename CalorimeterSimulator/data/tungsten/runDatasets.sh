#!/bin/bash
# Runs the jobs written by makeDatasets.py in parallel and merges the output of each dataset
# Usage: ./runDatasets.sh [datasets directory] [number of parallel jobs]
# Each Generator process needs ~750 MB of RAM: choose the number of parallel jobs accordingly
# Jobs whose ROOT file already exists are skipped, so the script can be relaunched after an interruption

DIR=${1:-datasets}
NPAR=${2:-4}

if [ ! -f "$DIR/jobs.txt" ]; then
    echo "No $DIR/jobs.txt: run makeDatasets.py first"
    exit 1
fi

# Skip the jobs that already finished (the last field of each job is its final output file)
grep -v '^$' "$DIR/jobs.txt" | while read -r job; do
    out=$(echo "$job" | awk "{print \$NF}")
    [ -f "$out" ] || echo "$job"
done > "$DIR/pending.txt"

echo "$(wc -l < "$DIR/pending.txt") jobs pending, running $NPAR in parallel"
xargs -P "$NPAR" -I CMD bash -c CMD < "$DIR/pending.txt"

# Merge the jobs of each dataset into a single file
if command -v hadd > /dev/null; then
    mkdir -p "$DIR/merged"
    for f in $(ls "$DIR"/root/*_000.root 2>/dev/null); do
        name=$(basename "$f" _000.root)
        hadd -f -k "$DIR/merged/$name.root" "$DIR"/root/${name}_[0-9][0-9][0-9].root > /dev/null 2>&1 # finished jobs only (not the .tmp.root of interrupted ones)
    done
    echo "Merged files in $DIR/merged"
else
    echo "hadd not found: source thisroot.sh to merge the outputs"
fi
