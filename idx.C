// Parse and index a binary output from a Struck ADC
void idx(const char* input_file = "input.bin",
	 	const char* output_folder = "COHERENT/MARS",
		const int nch = 16) // max 16 channels per board, FIXME: get it from input
{
	cout<<"Open "<<input_file<<endl;
	ifstream input(input_file, ios::binary);
	if (input.is_open()==false) { cout<<"Failed! Quit"<<endl; return; }

	cout<<"Check input file size ";
	input.seekg(0, ios::end); // move getter to the end of file
	int fsize = input.tellg(); // get input file size
	input.seekg(0, ios::beg); // move getter back to the beginning
	cout<<fsize<<" B / "<<fsize/1024<<" k / "<<fsize/1024/1024<<" M\n"<<endl;
	if (fsize<400) { cout<<"Too small! Quit"<<endl; return; }

	cout<<"Create index file in "<<output_folder<<endl;
	TString base_name(gSystem->BaseName(input_file));
	base_name.ReplaceAll("bin","csv");
	ofstream output(Form("%s/%s",output_folder,base_name.Data()));
	if (output.is_open()==false) { cout<<"Failed! Quit."<<endl; return; }
	cout<<output_folder<<"/"<<base_name.Data()<<" created\n"<<endl;
	output<<"# index file of "<<input_file<<endl;

	cout<<"Get input file header"<<endl;
	int word[100]; // read not more than 100 words at a time
	char* byte = (char*) word; // index in unit of byte instead of word
	input.read(byte,12); // get 1st 3 words of file header
	output<<"# file header word 0: 0x"<<hex<<word[0]<<dec<<endl;
	input.seekg(400-12, ios::cur); // skip the rest header	(400 bytes in total)

	cout<<"Index data blocks (spills) "<<endl;
	int nspill=0; // total number of spills in file
	int pos[999] = {0}; // positions of spills in file
	int size[999][99] = {0}; // spill size of each channel (max 99 channels)
	bitset<4> format[99]; // format bits of eachchannel
	int max_slot_id = 21; // max 21 slots per VME chassis
	bool empty[21][16] = {0}; // whether a channel is empty
	int min_size[999][21]; // min size of spill among all channels

	while (input.good() && input.tellg()<fsize-40) { // 40: spill header size
		input.read(byte,4); // get 1st word of spill header
		if (nspill==0) cout<<"Spill header word 0: 0x"<<hex<<word[0]<<dec<<endl;
		input.seekg(40-4, ios::cur); // skip the rest header (40 bytes in total)
		cout<<"Index spill "<<nspill<<endl;
		pos[nspill]=input.tellg(); // record spill position

		for (int slot=0; slot<max_slot_id; slot++) { // loop over slots
			input.read(byte,8); // get slot header (2 words)
			if (nspill==0) { // update max_slot_id when we are in the 1st spill
				if (word[0]==0xabbaabba) { // have reached next spill header
					input.seekg(-8,ios::cur); // go back to the beginning of the spill
					max_slot_id=slot; // update max_slot_id
					break; // quit from the slot loop
				}
				printf("slot%d header word 0: 0x%08x\n",slot,word[0]);
			}

			min_size[nspill][slot]=INT_MAX;;
			for (int ch=0; ch<nch; ch++) { // loop over channels
				input.read(byte,32); // get channel header (8 words)
				size[nspill][slot*nch+ch] = word[7];
				if (word[7]==0) { empty[slot][ch]=1; continue; }
				if (word[7]<min_size[nspill][slot]) min_size[nspill][slot]=word[7];

				input.read(byte,8); // get event header
				short channel=(*word&0xfff0)>>4; // get channel number imbedded in header
				if (nspill==0 && channel!=slot*nch+ch) cout<<"Warning: "<<slot<<
					"(slot) x "<<nch<<"(# of ch/slot) + " <<ch<<"(ch) != "<<channel<<endl;
				format[channel]=byte[0]; // get format bits imbedded in header
				input.seekg((word[7]-2)*4, ios::cur); // skip the rest
			}
		}
		nspill++;
	}

	output<<"# slot used:";
	for (int slot=0; slot<max_slot_id; slot++) output<<" "<<slot;
	output<<endl;
	
	output<<"# numbers under title \"ch N\" are spill sizes for channel N"<<endl;
	output<<"# numbers under title \"slot N\" "
		"are the smallest spill size among all channels in that slot"<<endl;

	output<<"# format bits:       ";
	for (int slot=0; slot<max_slot_id; slot++) {
		output<<"         ";
		for (int ch=0; ch<nch; ch++) 
			if (empty[slot][ch]==false) output<<setw(6)<<format[slot*nch+ch]<<", ";
	}
	output<<endl;

	output<<"# spill,   position";
	for (int slot=0; slot<max_slot_id; slot++) {
		output<<",  slot "<<slot;
		for (int ch=0; ch<nch; ch++) 
			if (empty[slot][ch]==false) output<<",   ch"<<setw(2)<<slot*nch+ch;
	}
	output<<endl;

	for (int spill=0; spill<nspill; spill++) {
		output<<setw(7)<<spill<<", "<<setw(10)<<pos[spill];
		for (int slot=0; slot<max_slot_id; slot++) {
			output<<", "<<setw(7)<<min_size[spill][slot];
			for (int ch=0; ch<nch; ch++) {
				if (empty[slot][ch]) continue;
				output<<","<<setw(7)<<size[spill][slot*nch+ch];
			}
		}
		output<<endl;
	}

	cout<<"Close files"<<endl; input.close(); output.close();
}
