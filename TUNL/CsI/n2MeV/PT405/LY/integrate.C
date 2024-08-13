// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220727184747_1.root")
{
	int n, np; bool pu, is; float a, b, db, h, tt; float s[5000], t[5000]={0};
	float pa[100], pt[100], ph[100], bgn[100], end[100];

	TFile *input = new TFile(run);
	TTree *ti = (TTree*) input->Get("t0");
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
	to->Branch("b",&b,"b/F"); // baseline averaged over 400 samples
	to->Branch("h",&h,"h/F"); // height of a waveform
	to->Branch("db",&db,"db/F"); // RMS of baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a pulse
	to->Branch("is",&is,"is/O"); // whether a waveform is saturated
	to->Branch("pu",&pu,"pu/O"); // pile-up flag
	to->Branch("np",&np,"np/I"); // number of pulses in a CsI waveform
	to->Branch("pa",pa,"pa[np]/F"); // pulse area
	to->Branch("ph",ph,"ph[np]/F"); // pulse height
	to->Branch("pt",pt,"pt[np]/F"); // pulse time (above threshold)
	to->Branch("bgn",bgn,"bgn[np]/F"); // beginning of a pulse
	to->Branch("end",end,"end[np]/F"); // end of a pulse

	int nevt = ti->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;
		ti->GetEntry(i);

		a=0; b=0; db=0; h=0; tt=-1; is=kFALSE;
		for (int k=0; k<400; k++) b+=s[k]; b/=400; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<400) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>100 && tt<0) tt=k; // software trigger time
			if (s[k]>h) { h=s[k]; }
		}
		db=sqrt(db)/400; // RMS of baseline

		if (tt==-1) tt=30;
		for (int k=tt-30; k<3000; k++) { a+=s[k]; }

		// search for pulses in [3000,5000)
		np=0; bool aboveThreshold, outOfPrevPls, prevSmplBelowThr=true;
		for (int j=0; j<100; j++) { pa[j]=0; ph[j]=0; pt[j]=0; bgn[j]=0; end[j]=0; }
		for (int k=3000; k<5000; k++) {
			if (s[k]>20) aboveThreshold=true; else aboveThreshold=false;
			if (np>0 && k==end[np-1] && aboveThreshold) end[np-1]=k+40<n?k+40:n-1;

			outOfPrevPls=true;
			if (np>0 && k-40<end[np-1]) outOfPrevPls=false;

			if (aboveThreshold && prevSmplBelowThr) {
				if (outOfPrevPls) { // create a new pulse
					pt[np]=k*4;
					bgn[np]=k-30<0?0:k-30;
					end[np]=k+40<n?k+40:n-1;
					np++;
					if (np>=100) break; // only record 100 pulses
				} else { // update previous pulse tentative end
					end[np-1]=k+40<n?k+40:n-1;
				}
			}
			prevSmplBelowThr=!aboveThreshold; // flip flag for next sample after using it
		}

		for (int j=0; j<np; j++) { // update pulses
			for (int k=bgn[j]; k<end[j]; k++) {
				if (s[k]>ph[j]) ph[j]=s[k];
				pa[j]+=s[k];
			}
			bgn[j]*=4; end[j]*=4;
		}
		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	input->Close();
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
