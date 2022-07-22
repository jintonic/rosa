## Experimental setup

neutron energy: 11.4 MeV
beam current: 1.1 uA
channel 0-11: backing detectors
channel 12: CsI
channel 13: BPM

## Data processing

```sh
cd /data7/coherent/rosa
# submit indexing jobs to PBS
./idx.sh /home/charlesprior/CsI-sample-data /data7/coherent/rosa/TUNL/CsI/test
# submit b2r jobs to PBS
./b2r.sh /data7/coherent/rosa/TUNL/CsI/test
cd TUNL/CsI/test
# submit jobs to PBS to match timestamps in CsI, BD and BPM events
./matchTimeStamps.sh
# submit jobs to PBS to integrate pulses and apply basic cuts
./integrate.sh
# combine individual ROOT files into Integrated_yyyymmdd.root
root -b -q combine.C
```
