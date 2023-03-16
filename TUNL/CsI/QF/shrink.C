{ // shrink root file size by applying stronger cuts
	TChain *told = new TChain("t");
	for (int i=1; i<75; i++) told->Add(Form("Integrated_20220729182748_%d.root",i));
	for (int i=1; i<47; i++) told->Add(Form("Integrated_20220729130801_%d.root",i));
	
	told->SetBranchStatus("*",0);
	told->SetBranchStatus("m",1);
	told->SetBranchStatus("p",1);
	told->SetBranchStatus("pb",1);
	told->SetBranchStatus("dp",1);
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

	TFile *output = new TFile("shrinked.root","recreate");
	TTree *t = told->CloneTree(0);

	int m;
	told->SetBranchAddress("m",&m); // number of samples
	float p, dp, ht, dt; 
	told->SetBranchAddress("p",&p); 
	told->SetBranchAddress("dp",&dp);  
	told->SetBranchAddress("ht",&ht);  
	told->SetBranchAddress("dt",&dt); 

	cout<<told->GetEntries()<<" events to be processed"<<endl;
	for (int i=0; i<told->GetEntries(); i++) {
		if (i%5000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		told->GetEntry(i);
		m=1250;
		if (p<4.1||p>4.18) continue; 
		if (ht<325||ht>500) continue;
		if (dp>0.55) continue;
		t->Fill();
	}
	t->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(output->GetName(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}
