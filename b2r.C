// Save binary output from Struck ADCs to ROOT trees.
// Arguments:
// - fi: input binary file path and name
// - fo: output ROOT file path and name
// - nm: number of modules (cards) per VME crate
// - nc: number of channels per module (card)
// - sr: sampling rate in MHz
// - bit: precision of ADC
void b2r(const char* fi = "input.bin", const char* fo = "output.root",
	 	const int nm = 2, const int nc = 16,
	 	const float sr = 250, const int bit=14)
{
	cout<<"\nOpen "<<fi<<endl;
	ifstream *input = new ifstream(fi, ios::binary);
	input->seekg(0, ios::end); // move getter to the end of file
	long int fsize = input->tellg();// get input file size
	input->seekg(0, ios::beg); // move getter back to the beginning
	if (fsize>0) cout<<"File size: "<<fsize/1024<<" kB"<<endl;
	else { cout<<"Failed opening input! Quit..."<<endl; return; }

	cout<<"\nParse binary input file"<<endl;
	int word[100]; // read not more than 100 words at a time
	char* byte = (char*) word; // index in unit of byte instead of word
	input->read(byte,12); // get 1st 3 words of file header
	for (int i=0; i<3; i++) printf(" file header word %d: 0x%08x\n",i,word[i]);
  input->seekg(400-12, ios::cur); // skip the rest header	(400 bytes in total)

	int nspill=0; // total number of spills in file
	int spill[999] = {0}; // position of each spill in file (max 999 spills)
	bitset<4> format[99]; // format bits per channel (max 99 channels)
	bool empty[99]={0}; // whether a channel is empty
	while (input->good() && input->tellg()<fsize-40) { // 40: spill header size
		input->read(byte,4); // get 1st word of spill header
		if (nspill==0) printf("spill header word 0: 0x%x\n",word[0]);
		input->seekg(40-4, ios::cur); // skip the rest header (40 bytes in total)
		spill[nspill]=input->tellg(); // record spill position

		for (int m=0; m<nm; m++) { // loop over each module (card)
			input->read(byte,8); // get card header (8 bytes), including card number
			if (nspill==0) printf("card%d header word 0: 0x%08x\n",m,word[0]);

			for (int c=0; c<nc; c++) { // loop over each channel
				input->read(byte,32); // get channel header (8 words)
				if (word[7]==0) { // word[7]: size of a data spill for a channel
					empty[m*nc+c]=1; continue;
			 	}
				input->read(byte,8); // get event header
				short ch=(*word&0xfff0)>>4; // get channel number imbedded in header
				if (ch!=m*nc+c) { cout<<m<<"(module)*"<<nc<<"(# of ch/module)+"
					<<c<<"(channel)!="<<ch<<" Quit!"<<endl; return; }
				format[ch]=byte[0]; // get format bits imbedded in header
				input->seekg((word[7]-2)*4, ios::cur); // skip the rest
			}
		}
	 	nspill++;
	}
	cout<<"Total number of spills: "<<nspill<<endl;

	cout<<"\nCreate "<<fo<<endl;
	TFile *output = new TFile(fo, "recreate");
	int	sum[8]={0}; // accumulator sums
  int	ih, max, vbt, vat; float es, em, h;
	int n, ns; long long ts; float s[99999]={0}, t[99999]; 
	for (int i=0; i<99999; i++) t[i]=i*(1000/sr);

	cout<<"Create a tree per channel"<<endl;
	TTree* tree[99]={0}; // assume there are maximally 99 channels
	for (int i=0; i<nm*nc; i++) {
		if (empty[i]) continue; // don't create tree for empty channel
		tree[i] = new TTree(Form("t%d",i), Form("SIS3316 channel %d",i));
		tree[i]->Branch("n", &n, "n/I"); // number of waveform samples
		tree[i]->Branch("s", s, "s[n]/F"); // waveform samples
		tree[i]->Branch("t", t, "t[n]/F"); // time [ns] of waveform samples
		tree[i]->Branch("ts", &ts, "ts/l"); // 48-bit event timestamp
		if (format[i][0]==1) {
			tree[i]->Branch("ns", &ns, "ns/I"); // number of accumulator sums
			tree[i]->Branch("sum", sum, "sum[ns]/I"); // accumulator sums
			tree[i]->Branch("h", &h, "h/F"); // height of a waveform
			tree[i]->Branch("ih", &ih, "ih/I"); // sample index of the highest point
		}
		if (format[i][2]==1) {
			tree[i]->Branch("max", &max, "max/I"); // MAW maximum value
			tree[i]->Branch("vbt", &vbt, "vbt/I"); // MAW value before trigger
			tree[i]->Branch("vat", &vat, "vat/I"); // MAW value after trigger
		}
		if (format[i][3]==1) {
			tree[i]->Branch("es", &es, "es/F"); // start energy value
			tree[i]->Branch("em", &em, "em/F"); // max energy value
		}
	}

	cout<<"\nConvert binary input to ROOT trees..."<<endl;
	for (int ispill=0; ispill<nspill; ispill++) { // loop over spills
		input->seekg(spill[ispill], ios::beg); // jump to the start of a spill

		for (int module=0; module<nm; module++) { // loop over modules (cards)
			input->seekg(8, ios::cur); // skip module header (8 bytes)

			for (int channel=0; channel<nc; channel++) { // loop over channels
				input->read(byte,32); // get channel header (8 words)
				int nwords=word[7]; // size of a spill of data for a channel (words)

				while (nwords>0) { // loop over events
					input->read(byte,8); nwords-=2; // get event header
					ts = *word&0xffff0000; // upper timestamp (started at 16th bit)
					ts = ts<<16; // move it up by 16 bit again
					ts = ts + word[1]; // combine upper (16) and lower (32) bits

					int ch = module*nc+channel; // global channel number
					if (format[ch][0]==1) { // peak + accumulator sums (gate 1~6)
						input->read(byte,28); nwords-=7;
						h = float(*word&0xffff); ih = int((*word&0xffff0000)>>16);
						ns=6; sum[0]=word[1]&0xffffff;
						for (int i=1; i<6; i++) sum[i]=word[i+1];
					}
					if (format[ch][1]==1) { // accumulator sums from gate 7 & 8
						input->read(byte,8); nwords-=2;
						ns=8; sum[6]=word[0]; sum[7]=word[1];
					}
					if (format[ch][2]==1) { // moving average window (MAW) info.
						input->read(byte,12); nwords-=3;
						max=word[0]; vbt=word[1]; vat=word[2];
					}
					if (format[ch][3]==1) { // energy values
						input->read(byte,8); nwords-=2;
						es=word[0]; em=word[1];
					}

					input->read(byte,4); nwords-=1; // number of waveform samples
					n=2*((*word)&0x3ffffff); bitset<4> flag(byte[3]);
					for (int i=0; i<n; i+=2) {
						input->read(byte,4); nwords-=1; // read two samples
						s[i]=*word&0xffff; s[i+1]=(*word&0xffff0000)>>16;
					}

					if (!empty[ch]) tree[ch]->Fill();
				}
			}
		}
		cout<<ispill+1<<"/"<<nspill<<" spills processed"<<endl;
	}
	input->close();

	cout<<"\nWrite trees to "<<output->GetName()<<endl;
	for (int i=0; i<nm*nc; i++) {
		if (empty[i]) continue;
		tree[i]->Write("", TObject::kOverwrite);
		printf("# of events in channel %2d: %8llu\n",i,tree[i]->GetEntries());
	}
	output->Close();
}

