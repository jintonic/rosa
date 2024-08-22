{ // shrink file size with stronger cuts
	TChain c("t");
	c.Add("Integrated_20240802154030.root");
	c.Add("Integrated_20240802164141.root");
	c.Add("Integrated_20240802184149.root");
	c.Add("Integrated_20240802205121.root");

	int m, bd; float f, dp, dt, e, e3, e4, e5, rms; float v[5000], ns[5000];

	c.SetBranchStatus("*",0);
	c.SetBranchAddress("m",&m);
	c.SetBranchAddress("f",&f);  
	c.SetBranchAddress("e",&e);
	c.SetBranchAddress("v",v);
	c.SetBranchAddress("ns",ns);
	c.SetBranchAddress("dp",&dp);
	c.SetBranchAddress("bd",&bd);
	c.SetBranchAddress("dt",&dt);

	TFile output("shrinked.root","recreate");
	TTree *t = c.CloneTree(0);
	t->Branch("rms",&rms,"rms/F");   // RMS of CsI waveform in [1600,3000] ns
	t->Branch("e3",&e3,"e3/F"); // integral of CsI waveform in [1600,3000] ns
	t->Branch("e4",&e4,"e4/F"); // integral of CsI waveform in [1600,4000] ns
	t->Branch("e5",&e5,"e5/F"); // integral of CsI waveform in [1600,5000] ns

	int nevt = c.GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%10000==0) cout<<"Processing event "<<i<<endl;

		c.GetEntry(i);
		if (dp>0.9||f<0.35||f>0.5||dt<80||dt>100||e>1e5) continue;

		e3=e4=e5=rms=0; // calculate new parameters
		for (int k=1600/4; k<5000/4; k++) {
			if (k<3000/4) { e3+=v[k]; rms+=v[k]*v[k]; }
			if (k<4000/4) { e4+=v[k]; }
			e5+=v[k];
		}
		rms=sqrt(rms)/350;

		t->Fill();
	}
	t->Write("",TObject::kOverwrite);
	output.Close();
	gSystem->Chmod(output.GetName(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output.GetName()<<" saved"<<endl;
}
