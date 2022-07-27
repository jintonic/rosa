//using three sets of distance versus dt data to get the slope of d/dt, then caculate the speed and energy of neutron beam
{
	gROOT->SetStyle("Plain");
	gStyle->SetPadTopMargin(0.01);
	gStyle->SetPadLeftMargin(0.08);
	gStyle->SetPadRightMargin(0.01);
	gStyle->SetPadBottomMargin(0.1);
	gStyle->SetOptStat(1111);
	gStyle->SetOptFit(1);
	gStyle->SetFitFormat(".2f");
	gStyle->SetStatY(0.5);
	gStyle->SetStatFont(22);
	gStyle->SetTextFont(22);
	gStyle->SetLabelFont(22,"XYZ");
	gStyle->SetTitleFont(22,"XYZ");
	gStyle->SetLegendFont(22);
	gStyle->SetLabelSize(0.05,"XYZ");
	gStyle->SetTitleSize(0.05,"XYZ");
	gStyle->SetTitleOffset(0.85,"X");
	gStyle->SetTitleOffset(0.8,"Y");

	const int n = 3; float x[n]={89,74.3,60.6}, dx[n]={1,1.7,0.6}, y[n]={4.66,3.86,3.17}, dy[n]={0.01,0.03,0.01};
	TGraphErrors *g = new TGraphErrors (n,x,y,dx,dy);
	g->SetMarkerStyle(4);
	g->SetMarkerSize(1);
	g->SetTitle(";dt [ns];distance [m]");
	g->Draw("ap");

	TF1 *f = new TF1("f","pol1",60,89.5);
	g->Fit(f,"R");
	g->GetFunction("f")->SetLineColor(kRed);

	float a = f->GetParameter(1)*1000000000;
	float c = 299792458.00; float v, En;
	v =	a*c/(c+a);
	En = 939.56542052/2*v*v/c/c;
	cout<<En<<endl;
	TLatex latex;
	latex.SetTextSize(0.045);
	latex.DrawLatex(60,4.4,Form("Neutron energy measured from: \t %.2f", En));
}
