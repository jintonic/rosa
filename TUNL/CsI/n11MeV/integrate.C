// integrate a waveform after its baseline aligned to zero
void integrate(const char* fin="SIS3316Raw_20220729182748_9.root")
{
	int i, n, m, np, bd, row, entry, date, time, run, evt; bool is;
	float a, ah, b, db, f, h, tt, p, dp, pb, dt, hm, lm, ht, lt, e16, e18, e20;
	float s[1024], t[1024], v[4096], ns[4096], sb[4096], tb[4096];
	float pa[100], pt[100], ph[100], bgn[100], end[100];
	float v16[4096]={0}, v18[4096]={0}, v20[4096]={0};

	TFile *input = new TFile(fin);
	const int nc=14; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	for (int i=0; i<12; i++) { // BDs
		ti[i] = (TTree*) input->Get(Form("t%d",i));
		ti[i]->SetBranchAddress("s",s); // BD waveform samples
		ti[i]->SetBranchAddress("t",t); // BD waveform sample time
		ti[i]->SetBranchAddress("n",&n); // number of samples
	}
	ti[12] = (TTree*) input->Get("t12");
	ti[12]->SetBranchAddress("s",v); // CsI waveform sample values
	ti[12]->SetBranchAddress("t",ns); // sample time in ns
	ti[12]->SetBranchAddress("n",&m); // number of samples

	ti[13] = (TTree*) input->Get("t13");
	ti[13]->SetBranchAddress("s",sb); // BPM waveform sample values
	ti[13]->SetBranchAddress("t",tb); // BPM waveform sample time
	ti[13]->SetBranchAddress("n",&m); // number of samples

	TString file(fin);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","CsI tree"); // output tree

	to->Branch("date",&date,"date/I"); // The date of the data
	to->Branch("time",&time,"time/I"); // The time of the data
	to->Branch("run",&run,"run/I");    // run number
	to->Branch("evt",&evt,"evt/I"); // event number

	to->Branch("m",&m,"m/I"); // number of samples in CsI waveform
	to->Branch("v",v,"v[m]/F"); // CsI waveform sample value in unit of ADC
	to->Branch("v16",v16,"v16[m]/F"); // CsI waveform corrected with tau=16000
	to->Branch("v18",v18,"v18[m]/F"); // CsI waveform corrected with tau=18000
	to->Branch("v20",v20,"v20[m]/F"); // CsI waveform corrected with tau=20000
	to->Branch("ns",ns,"ns[m]/F"); // CsI waveform index in unit of ns
	to->Branch("p",&p,"p/F"); // pedestal of CsI averaged over 300 samples before integral
	to->Branch("pb",&pb,"pb/F"); // pedestal of CsI averaged over 300 samples after integral
	to->Branch("dp",&dp,"dp/F"); // RMS of pedestal of CsI waveform
	to->Branch("np",&np,"np/I"); // number of pulses in a CsI waveform
	to->Branch("e16",&e16,"e16/F"); // integral of v16 in [1200,5000] ns
	to->Branch("e18",&e18,"e18/F"); // integral of v18 in [1200,5000] ns
	to->Branch("e20",&e20,"e20/F"); // integral of v20 in [1200,5000] ns
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

	to->Branch("sb",sb,"sb[n]/F"); // BPM sample value in unit of ADC
	to->Branch("tb",tb,"tb[n]/F"); // BPM waveform sample time
	to->Branch("dt",&dt,"dt/F"); // time difference between BD and BPM

	TString BD(fin);
	BD.ReplaceAll("SIS3316Raw", "BDchannels");
	BD.ReplaceAll("root", "txt");
	ifstream BDchannels(BD.Data());

	cout<<ti[12]->GetEntries()<<" events to be processed"<<endl;
	while (BDchannels>>evt>>bd>>row>>entry) {
		if (evt%5000==0) cout<<"Processing event "<<evt<<endl;

		// process CsI waveform
		ti[12]->GetEntry(evt);
		p=0; dp=0;
		for (int k=0; k<300; k++) p+=v[k]; p/=300; // calculate pedestal
		for (int k=0; k<m; k++) {
			if (k<300) dp+=(v[k]-p)*(v[k]-p); // calculate pedestal RMS
			v[k]-=p; // remove pedestal
		}
		dp=sqrt(dp)/300; // RMS of pedestal
		if (dp>0.6 || p<1230) continue;

		// get date, time and sub run index from file name
		TString dates(BD(BD.First('_')+3,6)); date = dates.Atoi();
		TString times(BD(BD.First('_')+9,6)); time = times.Atoi();
		TString runs(BD(BD.Last('.')-1,1)); run = runs.Atoi();

		// search for pulses in [300,1000)
		np=0; bool aboveThreshold, outOfPrevPls, prevSmplBelowThr=true;
		for (int j=0; j<100; j++) { pa[j]=0; ph[j]=0; pt[j]=0; bgn[j]=0; end[j]=0; }
		for (int k=300; k<1000; k++) {
			if (v[k]>15) aboveThreshold=true; else aboveThreshold=false;
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
		for (int k=0; k<50; k++) b+=s[k]; b/=50; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<50) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>50 && tt<0) tt=k; // software trigger time
		}
		db=sqrt(db)/50; // RMS of baseline
		if (db>0.55 || is==true) continue;

		if (tt<0) tt=5;
		for (int k=tt-5; k<(tt+112<n?tt+112:n); k++) {
			if (s[k]>h) h=s[k];
			if (k<tt-5+10) ah+=s[k];
			a+=s[k];
		}
		f=(a-ah)/a; // PSD parameter: tail/total
		tt*=4; // convert to ns
		if (tt<290 || f>0.6 || f<0.25 || h/a>0.4) continue;

		// correct overshoot
		hm=0; lm=9999; ht=-10; lt=-10; e16=0; e18=0; e20=0;
		for (int k=1; k<m; k++) {
		 	v16[k]=(1.6e4+4)/1.6e4*v[k]-v[k-1]+v16[k-1];
		 	v18[k]=(1.8e4+4)/1.8e4*v[k]-v[k-1]+v18[k-1];
		 	v20[k]=(2.0e4+4)/2.0e4*v[k]-v[k-1]+v20[k-1];
		}
		// calculate baseline in the tail of the main CsI pulse
		for (int k=1250; k<1250+300; k++)	pb+=20[k]; pb/=300;

		for (int k=300; k<1250; k++) {
			e16+=v16[k]; e18+=v18[k]; e20+=v20[k];
			if (hm<v18[k]) { ht=k; hm=v18[k]; }
			if (lm>v18[k]) { lt=k; lm=v18[k]; }
		}

		// process BPM waveforms
		ti[13]->GetEntry(entry);
		for (int k=tt/4; k<m; k++)
			if (sb[k]>3000 && sb[k-1]<sb[k]) { dt=k*4-tt; break; }
		if (dt<150) continue;
		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
