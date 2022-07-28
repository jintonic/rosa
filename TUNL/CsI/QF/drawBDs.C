{
	TChain *t = new TChain("t");
	t -> Add("Integrated_20220728005806_1.root");

	TCanvas *can = new TCanvas;
	can->Divide(4,3,0.001,0.001);
	
	const int dn=12; //detector numbers
	TH2F *h[dn];
	for(int i=0; i<dn; i++)
	{
		can->cd(i+1);
		gPad->SetLogz();
		h[i] = new TH2F(Form("h%d",i),"",140,0,7e4, 120,0,0.6);
		h[i]->SetStats(0);
		t->Draw(Form("f:a>>h%d",i),
				Form("bd==%d && db<0.55 && is<1 && f<0.6 && a<7e4",i),
			 	"colz");
	}
}

