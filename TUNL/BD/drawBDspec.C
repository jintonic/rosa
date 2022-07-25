{
	TChain *t = new TChain("t");
	t -> Add("Integrated_20220720153101_1.root");

	TCanvas *can = new TCanvas;
	can->Divide(4,4,0.001,0.001);
	
	//TH1F *h2 = new TH1F("h2","",100,0,80e3);
	int dn=16; //detector numbers

	for(int i=0; i<dn; i++)
	{
		can->cd(i+1);
		gPad->SetLogy();
		t->Draw("a",Form("bd==%d",i));
	}
}

