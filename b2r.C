// Convert binary output from Struck ADCs to ROOT format
// Ref. https://github.com/gcrich/binaryToROOT-NGM3316
// 
// Arguments:
// - nm: number of modules (cards)
// - fi: input binary file
// - fo: output ROOT file
void b2r(const int nm = 2, const char* fi =
		"/data3/coherent/data/mars/gammaScan_20180305/SIS3316Raw_20180305194100_1.bin",
		//"/data3/coherent/data/mars/prodRun1_waveforms/SIS3316Raw_20171127180918_3.bin",
		const char* fo = "output.root")
{
	ifstream *input = new ifstream(fi, ios::binary);
	input->seekg(0, ios::end); // move getter to the end of file
	long int fsize = input->tellg();// get input file size
	input->seekg(0, ios::beg); // move getter back to the beginning

	TFile *output = new TFile(fo, "recreate");
	const int nc = 16; // number of channels in one module (card)
	short adc[99999]= {0}; float s[99999]={0}; // waveform samples
	int n, len, cha, evt, ttt, tt, th, tl;
	float b, db, h, l; bool is;
	TTree* t[128]={0}; // assume there are maximally 128 channels
	for (int i=0; i<nm*nc; i++) {
		t[i] = new TTree(Form("t%d",i), Form("SIS3316 channel %d",i));
		t[i]->Branch("n", &n, "n/I"); // number of waveform samples
		t[i]->Branch("adc", adc, "adc[n]/S"); // waveform samples in ADC unit
		t[i]->Branch("h", &h, "h/F"); // highest point of a waveform
		t[i]->Branch("th", &th, "th/I"); // location of the highest sample
	}

	int word[65536]; // read not more than 65536 words at a time
	char* byte = (char*) word; // index to count in byte instead of word
	input->read(byte,400); // get file header (400 bytes)
	for (int i=0; i<3; i++) printf("  file hdr word %d: 0x%08x\n",i,word[i]);
	while (input->good() && input->tellg()<fsize-80) { // 80: packet + mo + ch hdr
		input->read(byte,40); // get packet header (40 bytes), filled with 0xabba
		for (int i=0;i<1;i++) printf("packet hdr word %d: 0x%x\n",i,word[i]);
		for (int module=0; module<nm; module++) { // loop over each module (card)
			input->read(byte,8); // get module header (8 bytes), including card number
			for (int i=0;i<1;i++) printf("module hdr word %d: 0x%08x\n",i,word[i]);
			for (int channel=0; channel<16; channel++) { // loop over each channel
				input->read(byte,32); // get channel header (8 words)
				int size=word[7]; if (size==0) continue; // skip any empty channel
				printf("size of channel data block: %u words\n", size);
				input->read(byte,8); size-=8/4; // get event header for a channel
				short ch=short((*word&0xfff0)>>4); printf("channel: %u, ", ch);
				bitset<4> format(*byte); cout<<"format bits: "<<format<<endl;
				unsigned long timestamp = (unsigned long)((*word&0xffff0000)<<16);
				timestamp = timestamp | word[1];
				printf("time stamp: %lu\n", timestamp);
				
				if (format[0]==1) { // peak + accumulator sums (gate 1~6)
					input->read(byte,28); size-=28/4;
					h = float(byte[4]); th = int(byte[5]);
				}
				if (format[1]==1) { // accumulator sums from gate 7 & 8
					input->read(byte,8); size-=8/4;
				}
				if (format[2]==1) { // moving average window (MAW) info.
					input->read(byte,12); size-=12/4;
				}
				if (format[3]==1) { // energy values
					input->read(byte,8); size-=8/4;
				}

				input->read(byte,4); size-=4/4; // number of waveform samples
				bitset<4> flag(byte[3]); cout<<"flags: "<<flag<<endl;
				n=2*((*word)&0x3ffffff); cout<<"number of waveform samples: "<<n<<endl;
				if (n==0) { adc[0]=0; t[ch]->Fill(); input->seekg(size*4,ios::cur); continue; }
				for (int i=0; i<n; i+=2) {
					input->read(byte,4); size-=4/4; // read two samples
					adc[i]=short(byte[0]); adc[i+1]=short(byte[2]);
				}
				t[ch]->Fill();
				cout<<"\nsize: "<<size<<endl;
				input->seekg(size*4,ios::cur);
			}
		}
		//break;
	}
	input->close();

	for (int i=0; i<nm*nc; i++) {
		if (t[i]->GetEntries()>0) t[i]->Write("", TObject::kOverwrite);
		cout<<t[i]->GetEntries()<<" events saved in "<<"channel "<<i<<endl;
	}
	cout<<"Close "<<output->GetName()<<endl;
	output->Close();
}
