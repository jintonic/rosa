## ROSA - Rootifying Output from Struck ADCs
[Python][] and [ROOT][] scripts to convert binary output from [Struck][] [ADCs][] to [ROOT][] [TTree][] [ntuples][]. A similar project, [TOWARD][], is developed for [CAEN][] [digitizers][].

### Features
- cross-platform, tested in Windows, Mac & Linux
- scripts only, no compilation
- simple trees with only basic data types, no library is needed to define complicated data structures

### Prerequisites
- CERN [ROOT][], any version
- [Python][] [tkinter][] is used to create a simple [GUI][]. It should be included by default in most [Python][] installations.

[Python]: https://www.python.org
[ROOT]: https://root.cern.ch
[Struck]: https://www.struck.de
[ADCs]: https://www.struck.de/vme.htm
[TTree]: https://root.cern.ch/root/htmldoc/guides/users-guide/Trees.html
[ntuples]: https://en.wikipedia.org/wiki/Tuple
[TOWARD]: https://github.com/jintonic/toward
[CAEN]: https://www.caen.it
[digitizers]: https://www.caen.it/sections/digitizer-families
[tkinter]: https://docs.python.org/3/library/tkinter.html
[GUI]: https://en.wikipedia.org/wiki/Graphical_user_interface
