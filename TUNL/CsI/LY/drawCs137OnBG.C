{
	TChain *ts = new TChain("t");
	ts->Add("Integrated_20220722182222_1.root");

	TChain *tb = new TChain("t");
	tb->Add("Integrated_20220722205019_1.root");

	TH1F *hs = new TH1F("hs","",100,0,2e6);
	TH1F *hb = new TH1F("hb","",100,0,2e6);
	ts->Draw("a>>hs", "db<0.55 && b>1100 && is<1 && h/a<0.01");
	tb->Draw("a>>hb", "db<0.55 && b>1100 && is<1 && h/a<0.01");

	gPad->SetGridx();
	gPad->SetGridy();
	hs->Draw();
	hb->SetLineColor(kRed);
	hb->Sumw2();
	hb->Scale(0.05);
	hb->Draw("histesame");
}
