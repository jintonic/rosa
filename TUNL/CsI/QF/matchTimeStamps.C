// match timestamps of CsI with those in BDs and BPM
void matchTimeStamps(const char* run="SIS3316Raw_20220728020808_1.root")
{
	TString file(run);
	file.ReplaceAll("SIS3316Raw", "BDchannels");
	file.ReplaceAll("root", "txt");
	ofstream output(file.Data());

	TFile *input = new TFile(run);
	const int nc=14; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	unsigned long long ts[nc]={0}; int entries[nc], row[nc]={0};
	for (int i=0; i<nc; i++) {
		ti[i] = (TTree*) input->Get(Form("t%d",i));
		ti[i]->SetBranchAddress("ts",&ts[i]); // time stamp
		entries[i]=ti[i]->GetEntries();
		printf("%6d entris in ch %2d\n",entries[i],i);
	}

	// loop over all CsI events
	for (int evt=0; evt<entries[12]; evt++) {
		if (evt%10000==0) cout<<evt<<" events processed"<<endl;
		unsigned long long previousTS=ts[12];
		ti[12]->GetEntry(evt); // get ts[12], a CsI timestamp
		if (ts[12]<previousTS) cout<<"timestamp jumped from "<<previousTS
			<<" to "<<ts[12]<<" at event "<<evt<<endl;

		// check all BDs' timestamps
		int nmatch=0; // number of BDs that have matching timestamp
		int cm=-1; // channel that has matched timestamp
		for (int ch=0; ch<12; ch++) {
			if (row[ch]>=entries[ch]) continue; // skip finished channels
			ti[ch]->GetEntry(row[ch]); // get ts[ch]
			if (ts[ch]==ts[12]-40) { cm=ch; nmatch++; row[ch]++; } // a match is found
			if (ts[ch]<ts[12]-40) // If CsI was too busy to process a BD trigger,
				row[ch]++;          // this BD row must be skipped
		}
		if (nmatch!=1) continue; // no match or more than 1 match

		// check BPM timestamps
		for	(int i=0; i<20; i++) {
		 	ti[13]->GetEntry(row[13]+i);
			if (ts[13]==ts[12]) {
				row[13]+=i;
				output<<evt<<" \t "<<cm<<" \t "<<row[cm]-1<<" \t "<<row[13]<<endl;
				row[13]++;
				break;
			}
		}
	}
	output.close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	input->Close();
}
