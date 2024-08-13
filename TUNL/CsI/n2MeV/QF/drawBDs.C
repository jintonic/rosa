{
	TChain *t = new TChain("t");
	t->Add("Integrated_20240730182220_1.root");

	TCanvas *c1 = new TCanvas;
	c1->Divide(4,3,0.001,0.001);

	const int dn=12; //detector numbers
	TH2F *h[dn];
	for(int i=0; i<dn; i++)
	{
		c1->cd(i+1);
		gPad->SetLogz();
		h[i] = new TH2F(Form("h%d",i),"",100,0,7e4, 100,0.3,0.5);
		h[i]->SetStats(0);
		t->Draw(Form("f:a>>h%d",i),
				Form("bd==%d && db<0.55 && f>0.35 && a<7e4",i),
				"colz");
	}
}

