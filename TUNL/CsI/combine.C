{
	TChain t("t");
	for (int i=1; i<68; i++) t.Add(Form("Integrated_20220616234302_%d.root",i));
	t.Merge("Integrated_20220616.root");
}
