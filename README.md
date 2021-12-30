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
- [idx.C](idx.C): a [ROOT][] [script][] to create an index file for a [Struck][] [ADC][] binary output, where basic information of the [ADC][] is saved
- [b2r.C](b2r.C): a [ROOT][] [script][] to convert a [Struck][] [ADC][] binary output to [ROOT][] [TTree][] [ntuple][]s
- [b2r.py](b2r.py): a [Python][] [GUI][] to run [b2r.C](b2r.C)

### Similar projects
- <https://github.com/gcrich/binaryToROOT-NGM3316>, original project
- <https://github.com/schedges/binaryToROOT-NGM3316>, a newer fork of the original one, which made the parsing of the 48-bit timestamp more robust across various compilers.

### DAQ software
This project is for the decoding of existing [Struck][] [ADC][]'s output. To run the DAQ and generate binary output, one needs a DAQ program, such as the ones listed below.

- <https://github.com/jrellin/sis3316-jrellin>, python based, seems newer
- <https://github.com/sergey-inform/SIS3316>, python based, first appears on Google
- <https://code.ornl.gov/nmdcimaging/ngmdaq.git> and its fork <https://code.ornl.gov/CASA/ngmdaq>, not publicly accessible.

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
