void CreateFit1PEdistr(int fixedMin, int fixedMax)
{
	TChain *t = new TChain("t");
	t->Add(Form("./Integrated_20220722161356_1.root"));
	int n;
	float s[400], h, b, db;
	t->SetBranchAddress("n",&n);
	t->SetBranchAddress("db",&db);
	t->SetBranchAddress("s",s);
	t->SetBranchAddress("b",&b);
	t->SetBranchAddress("h",&h);

	TFile *output = new TFile(Form("SPE.root"),"recreate");
	TH1D *hpe = new TH1D("hpe","",500,-100,400);

	int min=99999, max=0; // integration range (will be updated automatically)
	int nevt = t->GetEntries();
	if (nevt>50000) nevt=50000; // no need to load more than this
	for(int i=0; i<nevt; i++) {
		if (i%5000==0) cout<<"now event "<<i<<endl;
		t->GetEntry(i);
		min=fixedMin/4; max=fixedMax/4;
		if (db>1) continue;
		if (b<1105) continue;
		if (h>200) continue;
		double total=0;
		for (int j=min; j<max; j++) total+=s[j];
		hpe->Fill(total);
	}
	
	hpe->SetTitle(Form(";ADC counts #bullet ns;Entries"));

	TCanvas *can = new TCanvas;
	can->SetLogy();
	gStyle->SetOptFit(1);
	gStyle->SetOptStat(10);

	TF1 *f = new TF1("f", "gaus + gaus(3)"
			"+ [6]*exp(-0.5*((x-2*[4])/[5])**2)"
			" + [7]*exp(-0.5*((x-3*[4])/[5])**2)", -50,200);
	f->SetParNames("n0", "m0", "s0", "norm", "mean", "sigma", "n2", "n3");
	f->SetParameter(1,3);
	f->SetParameter(2,15);
	f->SetParameter(4,72);
	f->SetParameter(5,25);
	f->SetParLimits(4, 50, 100);
	f->SetParLimits(5, 15, 40);
	f->SetParLimits(6, 0, 500);
	f->SetParLimits(7, 0, 500);
	hpe->Fit(f);
	
	TF1 *baseline = new TF1("baseline","gaus",-50,80);
	baseline->SetParameters(f->GetParameters());
	baseline->SetLineColor(kRed);
	baseline->Draw("same");

	TF1 *first = new TF1("first","gaus",-50,190);
	first->SetParameters(f->GetParameter(3),
			f->GetParameter(4), f->GetParameter(5));
	first->SetLineColor(kBlue);
	first->Draw("same");

	TF1 *second = new TF1("second","gaus",-50,260);
	second->SetParameters(f->GetParameter(6),
			2*f->GetParameter(4), f->GetParameter(5));
	second->SetLineColor(kGreen);
	second->Draw("same");

	TF1 *third = new TF1("third","gaus",-50,290);
	third->SetParameters(f->GetParameter(7),
			3*f->GetParameter(4), f->GetParameter(5));
	third->SetLineColor(kMagenta);
	third->Draw("same");

	can->Print("SPE.png");
	gSystem->Chmod("SPE.png",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);

	output->Write();
	output->Close();
	gSystem->Chmod("SPE.root",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
}

//overlap many WFs in one plot to check location of 1 PE pulses by eyes

void DrawWFs()
{
	TChain *t = new TChain("t");
	t->Add(Form("./Integrated_20220722161356_1.root"));
	t->Draw("s:t", "b>1105 && db<1 && h<200","l",200,0);
	TText *text = new TText(.8,.8,Form("./")); text->SetNDC(); text->Draw();
	gPad->Print("1pe.png");
	gSystem->Chmod("1pe.png",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
}

// integration range is not fixed by default
void fit1pe(int fixedMin=0, int fixedMax=0)
{
	if (fixedMin!=0 && fixedMax!=0) DrawWFs();
	CreateFit1PEdistr(fixedMin, fixedMax);
}

