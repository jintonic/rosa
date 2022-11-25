{
	TChain *ts1 = new TChain("t");
	ts1->Add("Integrated_20220725211810_1.root");
	TChain *ts2 = new TChain("t");
	ts2->Add("Integrated_20220725234812_1.root");
	TChain *ts3 = new TChain("t");
	ts3->Add("Integrated_20220726094820_1.root");

	TChain *tb = new TChain("t");
	tb->Add("Integrated_20220722205019_1.root");

	TH1F *hs1 = new TH1F("hs1","",100,0,1e5);
	TH1F *hs2 = new TH1F("hs2","",100,0,1e5);
	TH1F *hs3 = new TH1F("hs3","",100,0,1e5);
	TH1F *hb = new TH1F("hb","",100,0,2e6);
	ts1->Draw("a>>hs1", "db<0.55 && b>1100 && is<1 && h/a<0.4");
	ts2->Draw("a>>hs2", "db<0.55 && b>1100 && is<1 && h/a<0.4");
	ts3->Draw("a>>hs3", "db<0.55 && b>1100 && is<1 && h/a<0.4");
	tb->Draw("a>>hb", "db<0.55 && b>1100 && is<1 && h/a<0.01");

	gPad->SetGridx();
	gPad->SetGridy();
	hs1->Draw();
	hs2->Draw("same");
	hs3->Draw("same");
	hb->SetLineColor(kRed);
	hb->Sumw2();
	hb->Scale(0.03);
	hb->Draw("histesame");
}
