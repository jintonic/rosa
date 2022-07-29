// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220729145135_1.root")
{
	int n; bool pu, is; float a, b, db, h, tt; float s[4000], t[4000]={0};

	TFile *input = new TFile(run);
	TTree *ti = (TTree*) input->Get("t12");
	ti->SetBranchAddress("n",&n); // number of samples
	ti->SetBranchAddress("s",s); // waveform samples
	ti->SetBranchAddress("t",t); // waveform sample time
	ti->SetBranchAddress("pu",&pu); // pile-up flag

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","tree"); // output tree

	to->Branch("a",&a,"a/F"); // area of a waveform
	to->Branch("n",&n,"n/I"); // number of samples in CsI waveform
	to->Branch("s",s,"s[n]/F"); // waveform sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of a waveform sample
	to->Branch("b",&b,"b/F"); // baseline averaged over 20 samples
	to->Branch("h",&h,"h/F"); // height of a waveform
	to->Branch("db",&db,"db/F"); // RMS of baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a pulse
	to->Branch("is",&is,"is/O"); // whether a waveform is saturated
	to->Branch("pu",&pu,"pu/O"); // pile-up flag

	int nevt = ti->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;
		ti->GetEntry(i);

		a=0; b=0; db=0; h=0; tt=-1; is=kFALSE;
		for (int k=0; k<50; k++) b+=s[k]; b/=50; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<50) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>100 && tt<0) tt=k; // software trigger time
			if (s[k]>h) { h=s[k]; }
		}
		db=sqrt(db)/50; // RMS of baseline

		if (tt==-1) tt=5;
		for (int k=tt-5; k<700; k++) { a+=s[k]; }
		tt*=4; // convert to ns

		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	input->Close();
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
