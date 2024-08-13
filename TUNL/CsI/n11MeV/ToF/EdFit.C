void EdFit(const char* dir = "./")
{
  	gROOT->SetStyle("Plain");
    gStyle->SetPadTopMargin(0.01);
    gStyle->SetPadLeftMargin(0.11);
    gStyle->SetPadRightMargin(0.07);
    gStyle->SetPadBottomMargin(0.1);
    gStyle->SetOptStat(1111);
		gStyle->SetOptFit(1);
		gStyle->SetFitFormat(".2f");
		gStyle->SetStatFont(22);
		gStyle->SetTextFont(22);
		gStyle->SetLabelFont(22,"XYZ");
		gStyle->SetTitleFont(22,"XYZ");
		gStyle->SetLegendFont(22);
		gStyle->SetLabelSize(0.05,"XYZ");
		gStyle->SetTitleSize(0.05,"XYZ");
		gStyle->SetTitleOffset(0.85,"X");
		gStyle->SetTitleOffset(1.05,"Y");

		TChain *t = new TChain("t");
		t->Add(Form("./Integrated_20220725165627_1.root"));
		int dt; float f;
		t->SetBranchAddress("dt",&dt);
		t->SetBranchAddress("f",&f);
		
		TCanvas* c3 = new TCanvas("c3","dt data and simulation");
		TH1F *T = new TH1F("T","",50,0,150);
		t->Draw("220-dt>>T", "dt>110 && dt<150 && f<0.6 && f>0.3");
		TF1 *f1 = new TF1("f1","expo",88,105);
		T->Fit(f1,"R");
		T->GetFunction("f1")->SetLineColor(kRed);
		T->GetYaxis()->SetRangeUser(0,100000);

		// model,smear gaussian + exponential decay (convolution)
		TH1F *random = new TH1F("random", ";Neutron energy [keV]" ";Number of events", 20,9000,11000);
		TH1F *time = new TH1F("time", ";dt in TOF [ns]" ";Number of events", 67,0,200);
		TH1F *DDT = new TH1F("DDT", ";ddt in TOF [ns]" ";Number of events", 200,0,50);
		TF1 *f2 = new TF1("f2", "0.5*[2]*exp(0.5*[2]*(-(2*[1]-2*x)+[0]*[0]*[2]))*(erfc((-([1]-x)+[0]*[0]*[2])/(sqrt(2)*[0])))",9000,11000);
		f2->SetParNames("width","shift", "decay");
		f2->SetParameters(50,10600.1,0.001);
		//TCanvas* c1 = new TCanvas("c1","fitting");
		//f->Draw();
		//generate 1000 random data		
		int i; float c = 299792458.00; double En, v, deltaT, ddt;
		for (i=0; i<=204808;i++)
		{
			En = f2->GetRandom();
			random->Fill(En);

			v = sqrt(2*En/1000./939.56542052)*c;
			deltaT = 4.66*(1/v-1/c)*1000000000.;//4.66m is the distance from BD surface to the neutron beam window
			time->Fill(deltaT);

			ddt = (1/v-1/c)*0.8*1000000000;//0.8 is the distance difference in twomeasurements
			DDT->Fill(ddt);		
		}
		//random->Draw("same");
		//TCanvas* c2 = new TCanvas("c2","dt");
		time->SetLineStyle(kDashed);
		time->Fit(f1,"","same",88,95);
		time->GetFunction("f1")->SetLineColor(kBlue);
		time->Draw("same");
		//DDT->Draw();
		
		TLatex latex;
		latex.SetTextSize(0.045);
		latex.DrawLatex(10,75000,
						Form("Mean: \t %.2f #pm %.2f", time->GetMean(), time->GetMeanError()));
		latex.DrawLatex(10,70000,
						Form("constant: \t %.2f #pm %.2f", time->GetFunction("f1")->GetParameter(0), time->GetFunction("f1")->GetParError(0)));
		latex.DrawLatex(10,65000,
						Form("slope: \t %.2f #pm %.2f", time->GetFunction("f1")->GetParameter(1), time->GetFunction("f1")->GetParError(1)));
		//canvas.Print("E_d.png");
		TCanvas* c1 = new TCanvas("c1","fitting");
		f2->Draw();
}
