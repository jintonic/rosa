// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220723210340_1.root")
{
	int n, offset=0; bool is, pu; unsigned long long ts0deg, tsBPM;
	float a, ah, b, db, f, h, tt, th, dt;
	float s[1024], t[1024], sb[1024], tb[1024];

	TFile *input = new TFile(run);

	TTree *t0deg = (TTree*) input->Get("t12");
	t0deg->SetBranchAddress("s",s); // waveform samples
	t0deg->SetBranchAddress("t",t); // waveform sample time
	t0deg->SetBranchAddress("n",&n); // number of samples
	t0deg->SetBranchAddress("pu",&pu); // pile-up flag
	t0deg->SetBranchAddress("ts",&ts0deg); // timestamp in 0-deg BD

	TTree *tBPM = (TTree*) input->Get("t13");
	tBPM->SetBranchAddress("ts",&tsBPM); // timestamp of BMP
	tBPM->SetBranchAddress("n",&n); // number of samples
	tBPM->SetBranchAddress("s",sb); // waveform samples

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","ToF tree"); // output tree

	to->Branch("a",&a,"a/F"); // area of 0-deg BD waveform
	to->Branch("n",&n,"n/I"); // number of samples in 0-deg BD waveform
	to->Branch("s",s,"s[n]/F"); // WF sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of waveform sample
	to->Branch("sb",sb,"sb[n]/F"); // WF sample in unit of ADC
	to->Branch("b",&b,"b/F"); // baseline of BD averaged over 50 samples
	to->Branch("f",&f,"f/F"); // (a-ah)/a
	to->Branch("h",&h,"h/F"); // height of a BD waveform
	to->Branch("ah",&ah,"ah/F"); // area of head of a waveform
	to->Branch("db",&db,"db/F"); // RMS of baseline
	to->Branch("dt",&dt,"dt/F"); // time of flight of n/g to a BPM
	to->Branch("tt",&tt,"tt/F"); // trigger position of a pulse
	to->Branch("th",&th,"th/F"); // position of highest point
	to->Branch("is",&is,"is/O"); // whether a waveform is saturated
	to->Branch("pu",&pu,"pu/O"); // pile-up flag
	to->Branch("ts0",&ts0deg,"ts0/l");
	to->Branch("tsB",&tsBPM,"tsB/l");
	
	int nevt = tBPM->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%10000==0) cout<<"Processing event "<<i<<endl;
		tBPM->GetEntry(i); t0deg->GetEntry(i+offset);
		while (ts0deg<tsBPM-40) { offset++; t0deg->GetEntry(i+offset); }
		if (ts0deg!=tsBPM-40) continue;

		if (pu>0) continue; // reject pile-up events

		a=0; ah=0; b=0; db=0; f=0; h=0; tt=-1; is=kFALSE;
		for (int k=0; k<50; k++) b+=s[k]; b/=50; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<50) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>20 && tt<0) tt=k; // software trigger time
		}
		db=sqrt(db)/50; // RMS of baseline

		if (b<1100 || db>0.53 || is>0 || tt<72) continue;

		if (tt<0) tt=5;
		for (int k=tt-5; k<(tt+175<n?tt+175:n); k++) {
			if (s[k]>h) { h=s[k]; th=k*4; }
			if (k<tt-5+10) ah+=s[k];
			a+=s[k];
		}
		f=(a-ah)/a; // PSD parameter: tail/total

		// dt
		for (int k=tt-25; k<n; k++)
			if (sb[k]>3000 && sb[k+1]>sb[k]) { dt=(k-tt)*4; break; }

		tt*=4; // convert to ns
		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;

	input->Close();
}
