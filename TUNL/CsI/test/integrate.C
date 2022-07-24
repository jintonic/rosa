// integrate a waveform after its baseline aligned to zero
void integrate(const char* run="SIS3316Raw_20220616234302_1.root")
{
	int i, n, m, np, bd, row, entry; bool is;
	float a, ah, b, db, f, h, tt, p, dp, dt;
	float s[1024], t[1024], v[4096], ns[4096], sb[4096], tb[4096];
	float pa[100], pt[100], ph[100], bgn[100], end[100];

	TFile *input = new TFile(run);
	const int nc=14; // number of channels enabled
	TTree *ti[nc]={0}; // input trees
	for (int i=0; i<12; i++) { // BDs
		ti[i] = (TTree*) input->Get(Form("t%d",i));
		ti[i]->SetBranchAddress("s",s); // waveform samples
		ti[i]->SetBranchAddress("t",t); // waveform sample time
		ti[i]->SetBranchAddress("n",&n); // number of samples
	}
	ti[12] = (TTree*) input->Get("t12");
	ti[12]->SetBranchAddress("s",v); // waveform sample values
	ti[12]->SetBranchAddress("t",ns); // sample time in ns
	ti[12]->SetBranchAddress("n",&m); // number of samples

	ti[13] = (TTree*) input->Get("t13");
	ti[13]->SetBranchAddress("s",sb); // waveform sample values
	ti[13]->SetBranchAddress("t",tb); // sample time in ns
	ti[13]->SetBranchAddress("n",&m); // number of samples

	TString file(run);
	file.ReplaceAll("SIS3316Raw", "Integrated");
	TFile *output = new TFile(file.Data(),"recreate");
	TTree *to = new TTree("t","CsI tree"); // output tree

	to->Branch("m",&m,"m/I"); // number of samples in CsI waveform
	to->Branch("sb",sb,"sb[m]/F"); // BPM sample value in unit of ADC
	to->Branch("tb",tb,"tb[m]/F"); // BPM waveform sample time
	to->Branch("dt",&dt,"dt/F"); // time of flight of neutron to a BD
	to->Branch("v",v,"v[m]/F"); // CsI sample value in unit of ADC
	to->Branch("ns",ns,"ns[m]/F"); // CsI waveform index in unit of ns
	to->Branch("p",&p,"p/F"); // pedestal of CsI averaged over 100 samples
	to->Branch("dp",&dp,"dp/F"); // RMS of pedestal of CsI waveform
	to->Branch("np",&np,"np/I"); // number of pulses in a CsI waveform
	to->Branch("pa",pa,"pa[np]/F"); // pulse area
	to->Branch("ph",ph,"ph[np]/F"); // pulse height
	to->Branch("pt",pt,"pt[np]/F"); // pulse time (above threshold)
	to->Branch("bgn",bgn,"bgn[np]/F"); // beginning of a pulse
	to->Branch("end",end,"end[np]/F"); // end of a pulse

	to->Branch("a",&a,"a/F"); // area of a BD waveform
	to->Branch("n",&n,"n/I"); // number of samples in CsI waveform
	to->Branch("s",s,"s[n]/F"); // BD sample in unit of ADC
	to->Branch("t",t,"t[n]/F"); // time of a BD waveform sample
	to->Branch("b",&b,"b/F"); // baseline of BD averaged over 100 samples
	to->Branch("f",&f,"f/F"); // (a-ah)/a
	to->Branch("h",&h,"h/F"); // height of a BD waveform
	to->Branch("ah",&ah,"ah/F"); // area of head of a BD waveform
	to->Branch("bd",&bd,"bd/I"); // channel id of a BD
	to->Branch("db",&db,"db/F"); // RMS of a BD waveform baseline
	to->Branch("tt",&tt,"tt/F"); // trigger position of a BD waveform
	to->Branch("is",&is,"is/B"); // whether a BD waveform is saturated

	TString BD(run);
	BD.ReplaceAll("SIS3316Raw", "BDchannels");
	BD.ReplaceAll("root", "txt");
	ifstream BDchannels(BD.Data());

	while (BDchannels>>i>>bd>>row>>entry) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		ti[12]->GetEntry(i);
		p=0; dp=0;
		for (int k=0; k<100; k++) p+=v[k]; p/=100; // calculate pedestal
		for (int k=0; k<m; k++) {
			if (k<100) dp+=(v[k]-p)*(v[k]-p); // calculate pedestal RMS
			v[k]-=p; // remove pedestal
		}
		dp=sqrt(dp)/100; // RMS of pedestal

		// search for pulses
		np=0; bool aboveThreshold, outOfPrevPls, prevSmplBelowThr=true;
		for (int j=0; j<100; j++) { pa[j]=0; ph[j]=0; pt[j]=0; bgn[j]=0; end[j]=0; }
		for (int k=0; k<m; k++) {
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
		for (int k=0; k<100; k++) b+=s[k]; b/=100; // calculate baseline
		for (int k=0; k<n; k++) {
			if (s[k]>16382) is=kTRUE; // saturated (reached 2^14-1)
			if (k<100) db+=(s[k]-b)*(s[k]-b); // calculate baseline RMS
			s[k]-=b; // remove baseline
			if (s[k]>100 && tt<0) tt=k; // software trigger time
		}
		db=sqrt(db)/100; // RMS of baseline
		if (db>0.32 || is==true) continue;

		if (tt<0) tt=5;
		for (int k=tt-5; k<275; k++) { // stops at 275*4=1100ns to avoid after pulse
			if (s[k]>h) h=s[k];
			if (k<tt-5+10) ah+=s[k];
			a+=s[k];
		}
		f=(a-ah)/a; // PSD parameter: tail/total
		tt*=4; // convert to ns
		if (tt<698 || f<0.26) continue;

		// process BPM waveforms
		ti[13]->GetEntry(entry);
		for (int k=tt/4; k<m; k++)
			if (sb[k]>3000 && sb[k-1]<3000) { dt=k*4-tt; break; }
		to->Fill();
	}
	to->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(file.Data(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
