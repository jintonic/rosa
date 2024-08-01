// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20240726175828_1.root")
{
	int i, n, nb, m, np, bd, row, entry; bool is;
	float a, ah, b, db, f, h, tt, p, dp, pb, dt, hm, lm, ht, lt, e2, e3, e4, e5;
	float s[5000], t[5000], v[5000], ns[5000], sb[300], tb[300];
	float pa[100], pt[100], ph[100], bgn[100], end[100];

	TFile *input = new TFile(run);
	const int nc=16; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	for (int i=1; i<15; i++) { // BDs
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
	ti[15]->SetBranchAddress("n",&nb); // number of samples

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","CsI tree"); // output tree

	to->Branch("m",&m,"m/I"); // number of samples in CsI waveform
	to->Branch("v",v,"v[m]/F"); // CsI waveform sample value in unit of ADC
	to->Branch("ns",ns,"ns[m]/F"); // CsI waveform index in unit of ns
	to->Branch("p",&p,"p/F"); // pedestal of CsI averaged over 300 samples before integral
	to->Branch("pb",&pb,"pb/F"); // pedestal of CsI averaged over 300 samples after integral
	to->Branch("dp",&dp,"dp/F"); // RMS of pedestal of CsI waveform
	to->Branch("np",&np,"np/I"); // number of pulses in a CsI waveform
	to->Branch("e2",&e2,"e2/F"); // integral in [1200,2000] ns in CsI waveform
	to->Branch("e3",&e3,"e3/F"); // integral in [1200,3000] ns in CsI waveform
	to->Branch("e4",&e4,"e4/F"); // integral in [1200,4000] ns in CsI waveform
	to->Branch("e5",&e5,"e5/F"); // integral in [1200,5000] ns in CsI waveform
	to->Branch("hm",&hm,"hm/F"); // max height in [300,1000)
	to->Branch("lm",&lm,"lm/F"); // lowest point in [300,1000)
	to->Branch("ht",&ht,"ht/F"); // time of max height in [300,1000)
	to->Branch("lt",&lt,"lt/F"); // time of lowest point in [300,1000)
	to->Branch("pa",pa,"pa[np]/F"); // pulse area
	to->Branch("ph",ph,"ph[np]/F"); // pulse height
	to->Branch("pt",pt,"pt[np]/F"); // pulse time (above threshold)
	to->Branch("bgn",bgn,"bgn[np]/F"); // beginning of a pulse
	to->Branch("end",end,"end[np]/F"); // end of a pulse

	to->Branch("n",&n,"n/I"); // number of samples in BD waveform
	to->Branch("s",s,"s[n]/F"); // BD sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of a BD waveform sample
	to->Branch("a",&a,"a/F"); // area of a BD waveform
	to->Branch("b",&b,"b/F"); // baseline of BD averaged over 50 samples
	to->Branch("h",&h,"h/F"); // height of a BD waveform
	to->Branch("f",&f,"f/F"); // (a-ah)/a
	to->Branch("ah",&ah,"ah/F"); // area of head of a BD waveform
	to->Branch("bd",&bd,"bd/I"); // channel id of a BD
	to->Branch("db",&db,"db/F"); // RMS of a BD waveform baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a BD waveform
	to->Branch("is",&is,"is/B"); // whether a BD waveform is saturated

	to->Branch("nb",&nb,"nb/I"); // number of samples in BD waveform
	to->Branch("sb",sb,"sb[nb]/F"); // BPM sample value in unit of ADC
	to->Branch("tb",tb,"tb[nb]/F"); // BPM waveform sample time
	to->Branch("dt",&dt,"dt/F"); // time difference between BD and BPM

	TString BD(run);
	BD.ReplaceAll("SIS3316Raw", "BDchannels");
	BD.ReplaceAll("root", "txt");
	ifstream BDchannels(BD.Data());

	cout<<ti[0]->GetEntries()<<" events to be processed"<<endl;
	while (BDchannels>>i>>bd>>row>>entry) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		ti[0]->GetEntry(i);
		p=0; dp=0;
		for (int k=0; k<300; k++) p+=v[k]; p/=300; // calculate pedestal
		for (int k=0; k<m; k++) {
			if (k<300) dp+=(v[k]-p)*(v[k]-p); // calculate pedestal RMS
			v[k]-=p; // remove pedestal
		}
		dp=sqrt(dp)/300; // RMS of pedestal
		//if (dp>0.6 || p<1230) continue;

		// search for pulses in [200,2000)
		np=0; bool aboveThreshold, outOfPrevPls, prevSmplBelowThr=true;
		for (int j=0; j<100; j++) { pa[j]=0; ph[j]=0; pt[j]=0; bgn[j]=0; end[j]=0; }
		for (int k=200; k<1000; k++) {
			if (v[k]>40) aboveThreshold=true; else aboveThreshold=false;
			if (np>0 && k==end[np-1] && aboveThreshold) end[np-1]=k+20<m?k+20:m-1;

			outOfPrevPls=true;
			if (np>0 && k-20<end[np-1]) outOfPrevPls=false;

			if (aboveThreshold && prevSmplBelowThr) {
				if (outOfPrevPls) { // create a new pulse
					pt[np]=k*4;
					bgn[np]=k-10<0?0:k-10;
					end[np]=k+20<m?k+20:m-1;
					np++;
					if (np>=100) break; // only record 100 pulses
				} else { // update previous pulse tentative end
					end[np-1]=k+20<m?k+20:m-1;
				}
			}
			prevSmplBelowThr=!aboveThreshold; // flip flag for next sample after using it
		}

		for (int j=0; j<np; j++) { // update pulses
			for (int k=bgn[j]; k<end[j]; k++) {
				if (v[k]>ph[j]) ph[j]=v[k];
				pa[j]+=v[k];
			}
			bgn[j]*=4; end[j]*=4;
		}

		// process BD waveform
		ti[bd]->GetEntry(row);
		a=0; ah=0; b=0; db=0; f=0; h=0; tt=-1; is=kFALSE;
		for (int k=0; k<60; k++) b+=s[k]; b/=60; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<60) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>60 && tt<0) tt=k; // software trigger time
		}
		db=sqrt(db)/60; // RMS of baseline
		//if (db>0.55 || is==true) continue;

		if (tt<0) tt=5;
		for (int k=tt-5; k<(tt+112<n?tt+112:n); k++) {
			if (s[k]>h) h=s[k];
			if (k<tt-5+10) ah+=s[k];
			a+=s[k];
		}
		f=(a-ah)/a; // PSD parameter: tail/total
		tt*=4; // convert to ns
		//if (tt<290 || f>0.6 || f<0.3 || h/a>0.4) continue;

		e2=0; hm=0; lm=9999; ht=-10; lt=-10; e3=0; e4=0; e5=0;
		// calculate baseline after integration
		for (int k=1250; k<1250+300; k++)	pb+=v[k]; pb/=300;

		for (int k=300; k<500; k++) e2+=v[k];
		for (int k=300; k<750; k++) e3+=v[k];
		for (int k=300; k<1000; k++) e4+=v[k];
		for (int k=300; k<1250; k++) {
			e5+=v[k]; // calculate corrected e5 in CsI
			if (hm<v[k]) { ht=k; hm=v[k]; }
			if (lm>v[k]) { lt=k; lm=v[k]; }
		}

		// process BPM waveforms
		ti[15]->GetEntry(entry);
		for (int k=tt/4; k<m; k++)
			if (sb[k]>3000 && sb[k-1]<sb[k]) { dt=k*4-tt; break; }
		//if (dt<150) continue;
		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
