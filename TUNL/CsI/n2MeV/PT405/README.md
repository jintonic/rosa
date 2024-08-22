## Data

- 24/7/26: 40 K, 50 Ohm, ~ 20 PE/keV
- 24/7/30: 40 K, 1k Ohm, ~ 20 PE/keV
- 24/8/02: 60 K, 1k Ohm, ~ 10 PE/keV

Detailed log: <https://docs.google.com/spreadsheets/d/11mwPQ_ER7XrjiXmSumxyJo9ouqV7LPQCh-fYRRpllSM>

## Data processing

idx.C takes about 3 min
b2r.sh takes about 37 min.

1. run [matchTimeStamps.sh](matchTimeStamps.sh). It takes about 20 min.
2. run [integrate.sh](integrate.sh), which takes about 31 min
3. run `root -b -q combine.sh 60K/`, which takes about 3:15 min.
4. run `cd 60K; root -b -q shrink.C`, which takes about 1:15 min.

- SIS3316Raw_20240726185848_14 failed in b2r
- SIS3316Raw_20240726205239_24 failed in b2r
- SIS3316Raw_20240726175828_1 failed in matchTimeStamp

## rsync data from TUNL

```sh
ssh third.party
rsync -av bbq:/Volumes/csi_daq/production/SIS3316Raw_2024073018*.bin .
rsync -av *.bin p:/data7/coherent/data/CryoCsI/QF/n2MeV/40K/1kOhm/
```

`-a` is used to preserve all meta data of a file. `-v` turns on verbose output. `bbq` is defined in `~/.ssh/config` as

```sshconfig
Host bbq
  HostName barbeaudaq
  User daq
  ProxyJump hades
Host hades
  HostName hades.internal.tunl.duke.edu
  User jl1189
  ProxyJump physics
Host physics
  HostName login2.phy.duke.edu
  User jl1189
```
