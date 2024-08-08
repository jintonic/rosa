// integrate a waveform after its baseline aligned to zero
// run after matchTimeStamps.C
void integrate(const char* run="SIS3316Raw_20240726175828_1.root")
{
	int i, j, k, m, n, bd, row, entry;
	float a, ah, b, db, f, tt, p, dp, dt, h, l, th, tl, e;
	float s[5000], t[5000], v[5000], ns[5000], sb[300], tb[300];

	TFile *input = new TFile(run);
	const int nc=16; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	for (i=1; i<15; i++) { // BDs
		ti[i] = (TTree*) input->Get(Form("t%d",i));
		ti[i]->SetBranchAddress("s",s); // BD waveform samples
		ti[i]->SetBranchAddress("t",t); // BD waveform sample time
		ti[i]->SetBranchAddress("n",&n); // number of samples
	}
	ti[0] = (TTree*) input->Get("t0");
	ti[0]->SetBranchAddress("s",v); // CsI waveform sample values
	ti[0]->SetBranchAddress("t",ns); // sample time in ns
	ti[0]->SetBranchAddress("n",&m); // number of samples

	ti[15] = (TTree*) input->Get("t15");
	ti[15]->SetBranchAddress("s",sb); // BPM waveform sample values
	ti[15]->SetBranchAddress("t",tb); // BPM waveform sample time
	ti[15]->SetBranchAddress("n",&j); // number of samples

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","all channel data"); // output tree

	to->Branch("p",&p,"p/F");     // pedestal of CsI averaged in [0,1200) ns
	to->Branch("e",&e,"e/F");     // integral in [1200,6000] ns in CsI waveform
	to->Branch("m",&m,"m/I");     // number of samples in CsI waveform
	to->Branch("v",v,"v[m]/F");   // CsI waveform sample value in unit of ADC
	to->Branch("ns",ns,"ns[m]/F");// CsI waveform index in unit of ns
	to->Branch("dp",&dp,"dp/F");  // RMS of pedestal of CsI waveform
	to->Branch("h",&h,"h/F");     // max height in CsI integral range
	to->Branch("l",&l,"l/F");     // lowest point in CsI integral range
	to->Branch("th",&th,"th/F");  // time of max height in CsI integral range
	to->Branch("tl",&tl,"tl/F");  // time of lowest point in CsI integral range

	to->Branch("n",&n,"n/I");     // number of samples in BD waveform
	to->Branch("s",s,"s[n]/F");   // BD sample in unit of ADC
	to->Branch("t",t,"t[n]/F");   // time of a BD waveform sample
	to->Branch("a",&a,"a/F");     // area of a BD waveform
	to->Branch("b",&b,"b/F");     // BD baseline averaged over 50 samples
	to->Branch("f",&f,"f/F");     // (a-ah)/a
	to->Branch("ah",&ah,"ah/F");  // area of head of a BD waveform
	to->Branch("bd",&bd,"bd/I");  // channel id of a BD
	to->Branch("db",&db,"db/F");  // RMS of a BD waveform baseline
	to->Branch("tt",&tt,"tt/F");  // trigger position of a BD waveform

	to->Branch("j",&j,"j/I");     // number of samples in BPM waveform
	to->Branch("sb",sb,"sb[j]/F");// BPM sample value in unit of ADC
	to->Branch("tb",tb,"tb[j]/F");// BPM waveform sample time
	to->Branch("dt",&dt,"dt/F");  // time difference between BD and BPM

	TString BD(run);
	BD.ReplaceAll("SIS3316Raw", "BDchannels");
	BD.ReplaceAll("root", "txt");
	ifstream BDchannels(BD.Data());

	cout<<ti[0]->GetEntries()<<" events to be processed"<<endl;
	while (BDchannels>>i>>bd>>row>>entry) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		ti[0]->GetEntry(i);
		p=0; dp=0; m=2500; // only save 2500 samples in recorded CsI waveform
		for (k=0; k<300; k++) p+=v[k]; p/=300; // calculate pedestal
		for (k=0; k<m; k++) {
			if (k<300) dp+=(v[k]-p)*(v[k]-p); // calculate pedestal RMS
			v[k]-=p; // remove pedestal
		}
		dp=sqrt(dp)/300; // RMS of pedestal
		if (dp>10 || p>2000) continue; // remove events with noisy or pile-up CsI WFs

		h=0; l=9999; th=-10; tl=-10; e=0;
		for (k=300; k<1500; k++) {
			e+=v[k];
			if (h<v[k]) { th=k; h=v[k]; }
			if (l>v[k]) { tl=k; l=v[k]; }
		}
		if (e>1e6) continue; // remove events with huge CsI pulses

		// process BD waveform
		ti[bd]->GetEntry(row);
		a=0; ah=0; b=0; db=0; f=0; tt=-1;
		for (k=0; k<60; k++) b+=s[k]; b/=60; // calculate baseline
		for (k=0; k<n; k++) {
			if (k<60) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>60 && tt<0) tt=k; // software trigger time
		}
		db=sqrt(db)/60; // RMS of baseline
		if (db>0.5) continue; // remove events with noisy BD waveforms

		if (tt<0) tt=5; // for empty BD waveforms
		for (k=tt-5; k<(tt+112<n?tt+112:n); k++) {
			if (k<tt-5+10) ah+=s[k];
			a+=s[k];
		}
		f=(a-ah)/a; // PSD parameter: tail/total

		if (bd<4) tt-=400; // for BD 1, 2, 3 with long waveforms
		tt*=4; // convert to ns
		if (tt<290 || f>0.6 || f<0.3 || a>5e4) continue; // loose PSD cuts

		// process BPM waveforms
		ti[15]->GetEntry(entry); dt=-1;
		for (k=tt/4; k<j; k++) {
			if (sb[k]>3000 && sb[k-1]<sb[k] && dt<0) { dt=k*4-tt; break; }
		}
		if (dt<1) continue; // no BPM pulse or BPM & BD pulses overlap

		to->Fill(); // save events that pass all loose cuts
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
