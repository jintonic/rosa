// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220722161356_1.root")
{
	int n; bool is; float a, b, db, h, tt; float s[1024], t[1024];

	TFile *input = new TFile(run);
	TTree *ti = (TTree*) input->Get("t14");
	ti->SetBranchAddress("n",&n); // number of samples
	ti->SetBranchAddress("s",s); // waveform samples
	ti->SetBranchAddress("t",t); // waveform sample time

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","SPE tree"); // output tree

	to->Branch("a",&a,"a/F"); // area of a waveform
	to->Branch("n",&n,"n/I"); // number of samples in CsI waveform
	to->Branch("s",s,"s[n]/F"); // waveform sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of a waveform sample
	to->Branch("b",&b,"b/F"); // baseline averaged over 20 samples
	to->Branch("h",&h,"h/F"); // height of a waveform
	to->Branch("db",&db,"db/F"); // RMS of baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a pulse
	to->Branch("is",&is,"is/B"); // whether a waveform is saturated

	int nevt = ti->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;
		ti->GetEntry(i);

		a=0; b=0; db=0; h=0; tt=-1; is=kFALSE;
		for (int k=0; k<20; k++) b+=s[k]; b/=20; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<20) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>15 && tt<0) tt=k; // software trigger time
			if (s[k]>h) { h=s[k]; }
		}
		db=sqrt(db)/20; // RMS of baseline
		tt*=4; // convert to ns

		for (int k=25; k<40; k++) { a+=s[k]; }

		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	input->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
