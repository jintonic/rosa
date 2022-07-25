// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220720153101_1.root")
{
	int n, bd; bool is, pu;
	float a, ah, b, db, f, h, tt, th;
	float s[1024], t[1024];

	TFile *input = new TFile(run);
	const int nc=13; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	for (int i=0; i<nc; i++) { // BDs
		ti[i] = (TTree*) input->Get(Form("t%d",i));
		ti[i]->SetBranchAddress("s",s); // waveform samples
		ti[i]->SetBranchAddress("t",t); // waveform sample time
		ti[i]->SetBranchAddress("n",&n); // number of samples
		ti[i]->SetBranchAddress("pu",&pu); // pile-up flag
	}

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	to = new TTree("t","BD tree"); // output tree

	to->Branch("a",&a,"a/F"); // area of a BD waveform
	to->Branch("n",&n,"n/I"); // number of samples in CsI waveform
	to->Branch("s",s,"s[n]/F"); // BD sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of a BD waveform sample
	to->Branch("b",&b,"b/F"); // baseline of BD averaged over 100 samples
	to->Branch("f",&f,"f/F"); // (a-ah)/a
	to->Branch("h",&h,"h/F"); // height of a BD waveform
	to->Branch("ah",&ah,"ah/F"); // area of head of a waveform
	to->Branch("bd",&bd,"bd/I"); // channel id
	to->Branch("db",&db,"db/F"); // RMS of baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a pulse
	to->Branch("th",&th,"th/F"); // position of highest point
	to->Branch("is",&is,"is/B"); // whether a waveform is saturated
	to->Branch("pu",&pu,"pu/O"); // pile-up flag

	int nevt = ti[0]->GetEntries();// nevt=186032;
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;
		for (int j=0; j<nc; j++) {
			ti[j]->GetEntry(i);

			a=0; ah=0; b=0; db=0; f=0; h=0; tt=-1; is=kFALSE; bd=j;
			for (int k=0; k<50; k++) b+=s[k]; b/=50; // calculate baseline
			for (int k=0; k<n; k++) {
				if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
				if (k<50) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
				s[k]-=b; // remove baseline
				if (s[k]>20 && tt<0) tt=k; // software trigger time
			}
			db=sqrt(db)/50; // RMS of baseline

			if (tt<0) tt=5;
			for (int k=tt-5; k<150; k++) {
				if (s[k]>h) { h=s[k]; th=k*4; }
				if (k<tt-5+10) ah+=s[k];
				a+=s[k];
			}
			f=(a-ah)/a; // PSD parameter: tail/total
			tt*=4; // convert to ns

			if (db>0.54 || is>0 || tt<295) continue;
			to->Fill();
		}
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	input->Close();
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
