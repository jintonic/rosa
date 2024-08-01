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
