void combine(const char *id="Integrated_20240730182220", int n=33)
{
	TChain t("t");
	for (int i=1; i<=n; i++) {
		cout<<"Add "<<Form("%s_%d.root",id,i)<<endl;
		t.Add(Form("%s_%d.root",id,i));
	}
	cout<<t.GetEntries()<<" events to be combined"<<endl;
	t.Merge(Form("%s.root",id));
}
