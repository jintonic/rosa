{
	TChain *t = new TChain("t");
	t->Add("Integrated_20220727184747_1.root");

	const int N=99999; int n, tt; bool is, pu; float a, db, b, h, s[N];
	t->SetBranchAddress("n",&n);  // number of samples
	t->SetBranchAddress("a",&a);  // integrated area
	t->SetBranchAddress("b",&b);  // baseline position
	t->SetBranchAddress("h",&h);  // height of a pulse
	t->SetBranchAddress("s",&s);  // waveform samples
	t->SetBranchAddress("db",&db); // baseline RMS
	t->SetBranchAddress("tt",&tt); // trigger time
	t->SetBranchAddress("is",&is); // whether a waveform is saturated
	t->SetBranchAddress("pu",&pu); // whether more than two waveforms

	int Am1=0;
	TGraph *gAm1 = new TGraph; gAm1->SetName("gAm1");//59.5 keV

	float xa[N]={0}, ya[N]={0};

	int nevt = t->GetEntries(); cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		t->GetEntry(i);
		if ((i+1)%10000==0) cout<<"now event "<<i+1<<endl;

		if (db<0.55 && b>1230 && is<1 && pu<1 && h/a<0.01 && a>124 && a<126e3)
		{//quality cuts for selecting 60 keV events
			Am1++;
			for (int j=0; j<n; j++) { xa[j]=j; ya[j]+=s[j]; }
		}
	} 

	for (int j=0; j<n; j++)  { gAm1->SetPoint(j, xa[j], ya[j]/Am1);}

	TF1  *expo = new TF1("expo","-[2]*TMath::Exp((-x-[1])/[0])",2000,4000); //find the exponiential decay constant from the tail of the pulse
	expo->SetParameter(0,4500);
	expo->SetParLimits(1,50,150);
	expo->SetParLimits(2,15,35);
	gAm1->Fit("expo","R");

	double v1[99999], s1[99999];
	float tau_RC = expo->GetParameter(0)*4; int dt = 4;

	for (int i=1; i<n; i++) {
		s1[i]=ya[i]/Am1;
		v1[i]=(tau_RC+dt)/tau_RC*s1[i]-s1[i-1]+v1[i-1]; 
	}

	TGraph *nAm1 = new TGraph; nAm1->SetName("nAm1");//59.5 keV

	for (int j=0; j<n; j++) {	nAm1->SetPoint(j, j, v1[j]); }

	TFile *output = new TFile("Deconvolution1.root","recreate");
	gAm1->Write();
	nAm1->Write();
	output->Close();
}
