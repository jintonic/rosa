# This file is used to fit waveforms by expo functions
{
	TChain *t = new TChain("t");
	t->Add("shrinked.root");

	TFile *output = new TFile("fittingResults.root","recreate");
	TTree *tnew =new TTree("t","fitting results");

	float slp, x2, norm;
	tnew->Branch("slp",&slp,"slp/F");
	tnew->Branch("x2",&x2,"x2/F");
	tnew->Branch("norm",&norm,"norm/F");

	TCanvas can;
	can.Print("fittingResults.pdf[");
	int nevt=t->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for(int i=0; i<nevt; i++) {
		if(i%5000==0)cout<<"processing event "<<i<<endl; 	
		t->GetEntry(i);
		t->Draw("vc:ns","","qoff",1,i);
		double *ns = t->GetV2();
		double *vc = t->GetV1();
		TGraph g(t->GetSelectedRows(),ns,vc);
		if (i<100) {
			g.Draw("al");
			g.Fit("expo","","",1300,2500);
		  can.Print("fittingResults.pdf");
		}
		g.Fit("expo","Q","",1300,2500);
		norm=g.GetFunction("expo")->GetParameter(0);
		slp=g.GetFunction("expo")->GetParameter(1);
		x2=g.GetFunction("expo")->GetChisquare();
		tnew->Fill();
	}
	can.Print("fittingResults.pdf]");
	tnew->Write();
	output->Close();
}
