## Data

- 24/7/26: 40 K, 50 Ohm, ~ 20 PE/keV
- 24/7/30: 40 K, 1k Ohm, ~ 20 PE/keV
- 24/8/02: 60 K, 1k Ohm, ~ 10 PE/keV

## Data processing

idx.C takes about 3 min

1. run [matchTimeStamps.sh](matchTimeStamps.sh)
2. run [integrate.sh](integrate.sh), which takes about 6:24 min
3. run `root -b -q combine.C'("Integrate_20240730182220")'`

## rsync data from TUNL

```sh
rsync -av bbq:/Volumes/csi_daq/production/SIS3316Raw_2024073018*.bin .
rsync -av *.bin p:/data7/coherent/rosa/TUNL/CsI24/QF/
```

`-a` is used to preserve all meta data of a file. `-v` turns on verbose output. `bbq` is defined in `~/.ssh/config` as

```sshconfig
Host physics
  HostName login2.phy.duke.edu
  User jl1189
Host hades
  HostName hades.internal.tunl.duke.edu
  User jl1189
  ProxyJump physics
Host bbq
  HostName barbeaudaq
  User daq
  ProxyJump hades
```
