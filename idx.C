// Parse and index a binary output from a Struck ADC
void idx(const char* input_file = "input.bin",
		const char* index_folder = "COHERENT/MARS")
{
	cout<<"Open "<<input_file<<endl;
	ifstream input(input_file, ios::binary);
	if (input.is_open()==false) { cout<<"Failed! Quit"<<endl; return; }

	cout<<"Check input file size... ";
	input.seekg(0, ios::end); // move getter to the end of file
	int fsize = input.tellg(); // get input file size
	input.seekg(0, ios::beg); // move getter back to the beginning
	cout<<fsize<<" B / "<<fsize/1024<<" k / "<<fsize/1024/1024<<" M\n"<<endl;
	if (fsize<400) { cout<<"Too small! Quit"<<endl; return; }

	cout<<"Load "<<index_folder<<"/daq.cfg..."<<endl;
	ifstream cfg(Form("%s/daq.cfg",index_folder));
	if (cfg.fail()) { cout<<strerror(errno)<<"! Quit"<<endl; return; }
	string key, value, line; cfg>>key>>value; // get first line in cfg file
	string path(index_folder);
	if (path.find("rosa/")<string::npos) path=path.substr(path.find("rosa/")+5);
	if (path.back()=='/') path.erase(path.end()-1); // remove last '/'
	if (value==path) {
		cout<<"DAQ setup for "<<value<<":"<<endl;
	} else {
		cout<<"Wrong experiment "<<value<<"! Quit"<<endl; return;
	}
	float sampling_rate; string unit;
	cfg>>key>>value>>key>>value>>key>>key>>sampling_rate>>unit;
	cout<<"- sampling rate: "<<sampling_rate<<" "<<unit<<endl;
	int nm, nmused=3; // number of modules used
	int nc; // number of channels per module
	cfg>>key>>key>>key>>key>>key>>key>>nc; // get number of channels per module
	cfg>>key>>key>>key>>key>>nm; // get number of modules used
	cout<<"- number of channles per module: "<<nc<<endl;
	cout<<"- number of modules used: "<<nm<<endl<<endl;
	for (int i=0; i<5+nm*3; i++) getline(cfg, line); // skip ch id & status
	bool sync[3][16] = {0}; // sync requirement for each channel
	for (int m=0; m<nm; m++) 
		for (int c=0; c<nc; c++) cfg>>sync[m][c];
	cfg.close();

	cout<<"Create index file in "<<index_folder<<"..."<<endl;
	TString base_name(gSystem->BaseName(input_file));
	base_name.ReplaceAll("bin","csv");
	ofstream output(Form("%s/%s",index_folder,base_name.Data()));
	if (output.is_open()==false) { cout<<"Failed! Quit."<<endl; return; }
	cout<<index_folder<<"/"<<base_name.Data()<<" created\n"<<endl;
	output<<"# index file of "<<input_file<<endl;

	cout<<"Get input file header"<<endl;
	int word[100]; // read not more than 100 words at a time
	char* byte = (char*) word; // index in unit of byte instead of word
	input.read(byte,12); // get 1st 3 words of file header
	output<<"# file header word 0: 0x"<<hex<<word[0]<<dec<<endl;
	input.seekg(400-12, ios::cur); // skip the rest header(400 bytes in total)

	cout<<"Index data blocks (spills) "<<endl;
	int nspill=0; // total number of spills in file
	int pos[20000] = {0}; // positions of spills in file (max 20000 spills in a file)
	int size[20000][3][16] = {0}; // size of spill in each channel
	int min_size[20000][3]; // min size of spill among all channels in a module
	bitset<4> format[3][16]; // format bits of each channel
	bool used[3][16] = {0}; // channels are not connected by default
	short ch_id[3][16] = {0}; // global channel id

	while (input.good() && input.tellg()<fsize-40) { // 40: spill header size
		input.read(byte,4); // get 1st word of spill header
		if (nspill==0) cout<<"Spill header word 0: 0x"<<hex<<word[0]<<dec<<endl;
		input.seekg(40-4, ios::cur); // skip the rest header (40 bytes in total)
		if (nspill%10==0) cout<<"Index spill "<<nspill<<endl;
		pos[nspill]=input.tellg(); // record spill position

		for (int m=0; m<nmused; m++) { // loop over module
			input.read(byte,8); // get module header (2 words)
			if (nspill==0) { // update nmused when we are in the 1st spill
				if (word[0]==0xabbaabba) { // have reached next spill header
					input.seekg(-8,ios::cur); // go back to the beginning of the spill
					nmused=m; // update nmused
					break; // quit from the module loop
				}
				printf("module %d header word 0: 0x%08x\n",m,word[0]);
			}

			min_size[nspill][m]=INT_MAX;;
			for (int c=0; c<nc; c++) { // loop over channels
				input.read(byte,32); // get channel header (8 words)
				size[nspill][m][c] = word[7];
				if (word[7]==0) continue; else used[m][c]=1; 
				if (word[7]<min_size[nspill][m]) min_size[nspill][m]=word[7];

				if (format[m][c]==0) { // not set yet
					input.read(byte,8); // get event header
					ch_id[m][c]=(*word&0xfff0)>>4; // get channel id
					format[m][c]=byte[0]; // get format bits
					input.seekg(-8,ios::cur); // go back to the beginning of the event
				}
				input.seekg(word[7]*4, ios::cur); // skip the rest
			}
			if (min_size[nspill][m]==INT_MAX) min_size[nspill][m]=0; // no ch has data
		}
		nspill++;
	}
	cout<<nspill<<" spills indexed in total"<<endl;

	// save indices to output
	output<<"# sampling rate: "<<sampling_rate<<" "<<unit<<endl;
	output<<"# number of modules used: "<<nmused<<endl;
	output<<"# number of channels per module: "<<nc<<endl;

	output<<"# global channel id embedded in event headers:\n#";
	for (int m=0; m<nm; m++) {
		for (int c=0; c<nc; c++) output<<setw(5)<<ch_id[m][c];
		output<<endl<<"#";
	}
	output<<" channel status embedded in data (1: connected, 0: empty):\n#";
	for (int m=0; m<nm; m++) {
		for (int c=0; c<nc; c++) output<<setw(5)<<used[m][c];
		output<<endl<<"#";
	}
	output<<" channel sync requirement (1: sync, 0: no need to sync):\n#";
	for (int m=0; m<nm; m++) {
		for (int c=0; c<nc; c++) output<<setw(5)<<sync[m][c];
		output<<endl<<"#";
	}
	output<<" format bits embedded in event headers:\n#";;
	for (int m=0; m<nm; m++) {
		for (int c=0; c<nc; c++) output<<setw(5)<<format[m][c];
		output<<endl<<"#";
	}

	output<<"\n# spill positions and sizes:"<<endl;
	output<<"# numbers under title \"ch N\" are spill sizes for channel N"<<endl;
	output<<"# numbers under title \"card N\" "
		"are the smallest spill size among all channels in that module"<<endl;

	output<<"# spill,   position";
	for (int m=0; m<nm; m++) {
		output<<",  card "<<m;
		for (int c=0; c<nc; c++) 
			if (used[m][c]) output<<",   ch"<<setw(2)<<m*nc+c;
	}
	output<<endl;

	for (int spill=0; spill<nspill; spill++) {
		output<<setw(7)<<spill<<", "<<setw(10)<<pos[spill];
		for (int m=0; m<nm; m++) {
			output<<", "<<setw(8)<<min_size[spill][m];
			for (int c=0; c<nc; c++) {
				if (!used[m][c]) continue;
				output<<","<<setw(8)<<size[spill][m][c];
			}
		}
		output<<endl;
	}

	input.close(); output.close();
}

