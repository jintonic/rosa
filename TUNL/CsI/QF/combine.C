void combine(const char *id="Integrated_20220616234302", int n=1)
{
	TChain t("t");
	for (int i=1; i<=n; i++) t.Add(Form("%s_%d.root",id,i));
	t.Merge(Form("%s.root",id));
}
