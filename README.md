## ROSA - Rootifying Output from Struck ADCs
[Python][] and [ROOT][] scripts to convert binary output from [Struck][] [ADC][]s to [ROOT][] [TTree][] [ntuple][]s. A similar project, [TOWARD][], is developed for [CAEN][] [digitizers][].

[![Getting started](https://img.shields.io/badge/Get-Started-red?style=flat)](#getting-started)

### Features
- cross-platform, tested in Windows, Mac & Linux
- scripts only, no compilation
- simple trees with only basic data types, no library is needed to define complicated data structures

### Prerequisites
- CERN [ROOT][], any version
- [Python][] 3, [tkinter][] is used to create a simple [GUI][]. It should be included by default in most [Python][] installations.

### Scripts
- [idx.C](idx.C): a [ROOT][] [script][] to create an index file for a [Struck][] [ADC][] binary output, where basic information of the [ADC][] is saved
- [b2r.C](b2r.C): a [ROOT][] [script][] to convert a [Struck][] [ADC][] binary output to [ROOT][] [TTree][] [ntuple][]s
- [view.C](view.C): a [ROOT][] [script][] to run a [ROOT][] [TTreeViewer][] to examine the contents of [ROOT][] trees
- [rosa.py](rosa.py): a [Python][] [GUI][] to run all the scripts above

### Getting started
1. [Download][] and unzip this project to your local machine (or use [git][] [clone][] if you know how).
2. Get into the project folder and create a subdirectory there for your experiment.
3. Create a `daq.cfg` file inside your newly created folder, and specify your [Struck][] [ADC][] setup therein. The syntax of such a configuration file is demonstrated in [COHERENT/MARS/daq.cfg](COHERENT/MARS/daq.cfg).
4. Run [rosa.py](rosa.py) (Double click the file if you are in a file browser, or `python3 rosa.py` if you are in a terminal) to launch a simple [GUI][]. Your folder should appear in the top left list box called "Select experiment:".
5. Select your folder in the list, the content of `daq.cfg` in your folder should be shown in the [GUI][]. Double check if everything is OK.
6. Click the button "Run idx.C", a window will pop up for you to select a [Struck][] [ADC][] binary output file to parse. After the parsing process, a [CSV][] file will be created in your folder. It contains all the information that is needed to run [b2r.C](b2r.C), for example, an index of positions and sizes of data blocks in the selected binary output file. NOTE: If the output file is compressed, it will be unzipped first. The unzipped file will be placed in your folder and then be indexed.
7. Select the [CSV][] file in the list box called "Select index file:". Its content will be displayed in the [GUI][]. Double check if the binary output is correctly indexed.
8. Click the button "Run b2r.C" to convert your binary file to a [ROOT][] file, which will be shown in the list box "Select ROOT file:" in the [GUI][].
9. Select the newly created [ROOT][] file, click the button "Run view.C" to run a [ROOT][] [TTreeViewer][] to check your data.

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
[Download]: https://github.com/jintonic/rosa/archive/refs/heads/main.zip
[CSV]: https://en.wikipedia.org/wiki/Comma-separated_values
[git]: https://git-scm.com
[clone]: https://git-scm.com/book/en/v2/Git-Basics-Getting-a-Git-Repository
[TTreeViewer]: https://root.cern.ch/doc/master/classTTreeViewer.html
