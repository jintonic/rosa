{ // shrink root file size by applying stronger cuts
	TChain *told = new TChain("t");
	for (int i=1; i<75; i++) told->Add(Form("Integrated_20220729182748_%d.root",i));
	
	told->SetBranchStatus("*",0);
	told->SetBranchStatus("m",1);
	told->SetBranchStatus("pb",1);
	told->SetBranchStatus("e2",1);
	told->SetBranchStatus("e3",1);
	told->SetBranchStatus("e4",1);
	told->SetBranchStatus("e5",1);
	told->SetBranchStatus("hm",1);
	told->SetBranchStatus("ht",1);
	told->SetBranchStatus("bd",1);
	told->SetBranchStatus("ns",1);
	told->SetBranchStatus("vc",1);
	told->SetBranchStatus("dt",1);
	told->SetBranchStatus("pt",1);

	TFile *output = new TFile("shrinked.root","recreate");
	TTree *t = told->CloneTree(0);

	int m;
	float  ht, hm; 
	float vc[4096];	
	told->SetBranchAddress("m",&m); // number of samples
	told->SetBranchAddress("ht",&ht);  
	told->SetBranchAddress("hm",&hm);
	told->SetBranchAddress("vc",vc);

	float pc, dpc, dwi, pbc;

	t->Branch("pc",&pc,"pc/F"); // pedestal of CsI averaged over 300 samples before integral after correcting the overshoot
	t->Branch("pbc",&pbc,"pbc/F"); // pedestal of CsI averaged over 300 samples after integral after correcting the overshoot 
	t->Branch("dpc",&dpc,"dpc/F"); // RMS of pedestal of CsI waveform after correcting the overshoot
	t->Branch("dwi",&dwi,"dwi/F"); // RMS of integral range of CsI waveform after correcting the overshoot

	cout<<told->GetEntries()<<" events to be processed"<<endl;
	for (int i=0; i<told->GetEntries(); i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		told->GetEntry(i);
		m=1250;
		if (ht<325||ht>600) continue;
		if (hm>500) continue;
		pc=0; pbc=0; dpc=0; dwi=0;
		for (int k=200; k<200+100; k++)	pc+=vc[k]; pc/=100;
			for (int k=750; k<1000; k++)	pbc+=vc[k]; pbc/=250;
			for (int k=0; k<m; k++) {
				if (k<300) dpc+=(vc[k]-pc)*(vc[k]-pc); // calculate pedestal RMS
				if (k>300&&k<1000) dwi+=(vc[k]-pc)*(vc[k]-pc);//trigger range RMS
				vc[k]-=pc; // remove pedestal
			}
		dwi=sqrt(dwi)/700;
		dpc=sqrt(dpc)/300; // RMS of pedestal

		if (pc<-10||pc>10) continue;
		if (dpc>0.6) continue;
		if ((pbc-pc)>5||(pbc-pc)<-5) continue;
		t->Fill();
	}
	t->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(output->GetName(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
