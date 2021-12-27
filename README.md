## ROSA - Rootifying Output from Struck ADCs
[Python][] and [ROOT][] scripts to convert binary output from [Struck][] [ADC][]s to [ROOT][] [TTree][] [ntuple][]s. A similar project, [TOWARD][], is developed for [CAEN][] [digitizers][].

### Features
- cross-platform, tested in Windows, Mac & Linux
- scripts only, no compilation
- simple trees with only basic data types, no library is needed to define complicated data structures

### Prerequisites
- CERN [ROOT][], any version
- [Python][] [tkinter][] is used to create a simple [GUI][]. It should be included by default in most [Python][] installations.

### Scripts
- [b2r.C](b2r.C): a [ROOT][] [script][] to convert a [Struck][] [ADC][] binary output to [ROOT][] [TTree][] [ntuple][]s
- [b2r.py](b2r.py): a [Python][] [GUI][] to run [b2r.C](b2r.C)

[Python]: https://www.python.org
[ROOT]: https://root.cern.ch
[script]: https://root.cern.ch/root/htmldoc/guides/users-guide/Cling.html
[Struck]: https://www.struck.de
[ADC]: https://www.struck.de/vme.htm
[TTree]: https://root.cern.ch/root/htmldoc/guides/users-guide/Trees.html
[ntuple]: https://en.wikipedia.org/wiki/Tuple
[TOWARD]: https://github.com/jintonic/toward
[CAEN]: https://www.caen.it
[digitizers]: https://www.caen.it/sections/digitizer-families
[tkinter]: https://docs.python.org/3/library/tkinter.html
[GUI]: https://en.wikipedia.org/wiki/Graphical_user_interface
