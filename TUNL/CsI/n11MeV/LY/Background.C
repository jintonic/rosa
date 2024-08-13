void Background(const char* run="Integrated_20220722205019_1.root")
{
	TChain *t = new TChain("t");
	t->Add(Form("./Integrated_20220722205019_1.root"));
	int n, tt; bool is; float a, db, b, h, s[99999]= {0};
	t->SetBranchAddress("s",&s); // waveform samples
	t->SetBranchAddress("n",&n); // number of samples
	t->SetBranchAddress("a",&a); // area
	t->SetBranchAddress("db",&db);//deviation
	t->SetBranchAddress("b",&b);//baseline
	t->SetBranchAddress("h",&h);//height
	t->SetBranchAddress("tt",&tt);//triger time
	t->SetBranchAddress("is",&is);// whether a waveform is saturated

	float s1[4000]={0.};
	int nevt=t->GetEntries(), nevt1=0;
	for (int i=0; i<nevt; i++) { // event loop
		t->GetEntry(i); // load data from disk to memory
		if(db<0.55 && b>1100 && is<1 && h/a<0.01 && a>2e5 && a<2.8e5) { //cuts in the 662 keV region
			for (int j=0; j<n; j++) { // loop over waveform index
				s1[j]+=s[j]; 
			}nevt1++;
		}
	}     
	//loop for average waveform
	for (int j=0; j<n; j++) {s1[j]=s1[j]/nevt1;}

	float idx[4000];
	for (int j=0; j<4000; j++) idx[j]=j;    
	TGraph *a1 = new TGraph(n, idx, s1);
	a1->SetName("a1");
	a1->SetTitle(";Time [ns];Height [ADC counts]");
	//a1->GetXaxis()->SetRangeUser(0,1000);
	a1->SetLineColor(kBlue);

	TCanvas *can = new TCanvas;
	can->SetGrid();
	gStyle->SetOptFit(1);
	gStyle->SetOptStat(10);

	// overshoot model
	TF1 *f = new TF1("f",
			"[0]*([1]*exp(-(x-[3])/[2])-[2]*exp(-(x-[3])/[1]))/([1]-[2])",150,3000);
	f->SetParNames("Norm","#tau_{i}", "#tau", "Start");
	f->SetLineColor(kRed);
	f->SetParameter(0,1100);
	f->SetParLimits(0, 1000,1500);
	f->SetParameter(1,300);
	f->SetParameter(2,2000);
	f->SetParameter(3,110);
	f->SetParLimits(3,90,200);
	a1->Fit("f","R");

	//overshoot correction model
	TF1 *f1 = new TF1("f1","[0]*exp(-(x-[1])/[2])",150,3000);
	f1->SetParNames("Norm","start", "decay time");
	f1->SetLineColor(kGreen);
	//parameters from overshoot model, decay time is tau_i
	f1->FixParameter(0,f->GetParameter(0));
	f1->FixParameter(1,f->GetParameter(3));
	f1->FixParameter(2,f->GetParameter(1));

	a1->Fit("f1","R+");
	a1->Draw();

	double B=0;//B is the area before correction function starts
	for (int i=60; i<97; i++) {B+=s1[i];} 
	double A = f1->Integral(97,1500); A =A+B;
	//120 is the point where correction function starts, 1500 is the point correction function goes to zero

	TLatex latex;
	latex.SetTextSize(0.05);
	latex.DrawLatex(500,600, Form("662 keV peak area after correction: %.0f",A));

		can->Print("Background.png");
	gSystem->Chmod("Background.png",S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
}
