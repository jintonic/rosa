{
	TChain *ts = new TChain("t");
	ts->Add("Integrated_20220720153101_1.root");

	TChain *tb = new TChain("t");
	tb->Add("Integrated_20220719233159_1.root");

	TH1F *hs = new TH1F("hs","",80,0,80e3);
	TH1F *hb = new TH1F("hb","",80,0,80e3);
	ts->Draw("a>>hs", "bd==12");
	tb->Draw("a>>hb", "bd==12");

	gPad->SetLogy();
	hs->Draw();
	hb->SetLineColor(kRed);
	hb->Sumw2();
	hb->Scale(0.25);
	hb->Draw("histesame");
}
