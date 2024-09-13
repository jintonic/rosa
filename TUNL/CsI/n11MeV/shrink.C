{ // shrink root file size by applying stronger cuts
	TChain *told = new TChain("t");
	for (int i=1; i<75; i++) told->Add(Form("Integrated_20220729182748_%d.root",i));
	
	told->SetBranchStatus("*",0);
	told->SetBranchStatus("m",1);
	told->SetBranchStatus("pb",1);
	told->SetBranchStatus("e16",1);
	told->SetBranchStatus("e18",1);
	told->SetBranchStatus("e20",1);
	told->SetBranchStatus("hm",1);
	told->SetBranchStatus("ht",1);
	told->SetBranchStatus("bd",1);
	told->SetBranchStatus("ns",1);
	told->SetBranchStatus("v18",1);
	told->SetBranchStatus("v",1);
	told->SetBranchStatus("dt",1);
	told->SetBranchStatus("date",1);
	told->SetBranchStatus("time",1);
	told->SetBranchStatus("run",1);
	told->SetBranchStatus("evt",1);

	TFile *output = new TFile("shrinked.root","recreate");
	TTree *t = told->CloneTree(0);

	int m;
	float ht, hm, e16, e18, e20; 
	float v18[4096];	
	told->SetBranchAddress("m",&m); // number of samples
	told->SetBranchAddress("ht",&ht);  
	told->SetBranchAddress("hm",&hm); // value of the highest point
	told->SetBranchAddress("e16",&e16);
	told->SetBranchAddress("e18",&e18);
	told->SetBranchAddress("e20",&e20);
	told->SetBranchAddress("v18",v18);

	float pc, pc1, sd1, rms, pc5, pc6, sd6, sd5, lm, e3;
	int np1, np5, np6;

	t->Branch("pc",&pc,"pc/F"); // pedestal of CsI in [0, 300] samples after overshoot correction
	t->Branch("np1",&np1,"np1/I"); // number of pulses in [0, 300]
	t->Branch("pc1",&pc1,"pc1/F"); // pc without pulses in [0, 300]
	t->Branch("sd1",&sd1,"sd1/F"); // RMS of pedestal of CsI waveform after correcting the overshoot
	t->Branch("sd5",&sd5,"sd5/F"); // SD of CsI waveform in (4000,5000)ns without spikes above 10
	t->Branch("sd6",&sd6,"sd6/F"); // SD of CsI waveform in (5000,6000)ns without spikes above 10
	t->Branch("rms",&rms,"rms/F"); // RMS of integral range of CsI waveform after correcting the overshoot
	t->Branch("pc5",&pc5,"pc5/F"); // pedestal of CsI waveform in range (4000,5000)ns without spikes above 10
	t->Branch("pc6",&pc6,"pc6/F"); // pedestal of CsI waveform in range (5000,6000)ns without spikes above 10
	t->Branch("np5",&np5,"np5/I"); // number of pulses in [1000, 1250]
	t->Branch("np6",&np6,"np6/I"); // number of pulses in [1250, 1500]
	t->Branch("lm",&lm,"lm/F");    // value of the lowest point
	t->Branch("e3",&e3,"e3/F");    // integral in [1200, 3000] ns

	int nevt = told->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%10000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		told->GetEntry(i);
		m=1500; lm=0;
		if (ht<325||ht>600) continue;
		if (hm>500) continue; // remove very large pulses
		pc=0; sd1=0; rms=0; pc5=0; pc6=0;sd5=0; sd6=0;
		for (int k=0; k<300; k++) pc+=v18[k];
		pc/=300;

		// calculate pedestal excluding pulses in the range
		np1=0; pc1=0;np5=0; np6=0;
		for (int k=0; k<300; k++){
			if(v18[k]<10) { pc1+=v18[k]; np1++; } 
		}
		pc1/=np1;
		for (int k=0; k<m; k++) {
			v18[k]-=pc1; // remove pedestal
			if (lm>v18[k]) lm=v18[k];
		}
	 	for (int k=0; k<300; k++)
			if(v18[k]<10) sd1+=(v18[k]-pc1)*(v18[k]-pc1); // calculate pedestal RMS
		sd1=sqrt(sd1/np1);

		for(int k=300;k<750;k++) rms+=(v18[k]-pc1)*(v18[k]-pc1);
		rms=sqrt(rms/(750-300));

		e3=0; for (int k=300; k<750; k++) e3+=v18[k];

		//calculate pedestal and RMS of in (4000,5000) and (5000,6000) 	
		for (int k=1000; k<1250; k++)	{
		 	if (v18[k]<10) { pc5+=v18[k]; np5++; }
	 	}
		pc5/=np5;

		for (int k=1000; k<1250; k++) {
			if(v18[k]<10) sd5+=(v18[k]-pc5)*(v18[k]-pc5);
		}
		sd5=sqrt(sd5/np5);

		for (int k=1250; k<1500; k++) {
			if(v18[k]<10) {pc6+=v18[k]; np6++; }
		}
		pc6/=np6;

		for (int k=1250; k<1500; k++) {
			if(v18[k]<10) sd6+=(v18[k]-pc6)*(v18[k]-pc6);
		}
		sd6=sqrt(sd6/np6);
		
		if (abs(pc)>10) continue;
		if (pc5<-3||pc5>4) continue;
		if (pc6<-3||pc6>4) continue;
		if (sd1<2.7||sd1>3.7) continue;
		if (sd5<2.6||sd5>3.6) continue;
		if (sd6<2.6||sd6>3.6) continue;
		if (lm<-12) continue;

		t->Fill();
	}
	t->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(output->GetName(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}

