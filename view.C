void view(const char* fi="output.root")
{
	TTreeViewer *viewer = new TTreeViewer;
	viewer->SetScanRedirect(kTRUE); // avoid involving terminal output

	TFile *f = new TFile(fi);
 	TTree *t;
	for (int ch=0; ch<16; ch++) {
		t = (TTree*) f->Get(Form("t%d",ch));
		viewer->AppendTree(t);
	}

	// items (0:x, 1:y, 2:z, 3:cut, 4:scan, users must start at 5)
	viewer->ExpressionItem(5)->SetExpression("h", "~height", kFALSE);
	viewer->ExpressionItem(6)->SetExpression("th", "~peak position", kFALSE);
	viewer->ExpressionItem(7)->SetExpression("n", "~number of samples", kFALSE);

	// records (for better control)
	TTVSession* session = new TTVSession(viewer);
	viewer->SetSession(session);
	// 1st record
	TTVRecord* record = session->AddRecord(kTRUE);
	record = session->AddRecord(kTRUE);
	session->SetRecordName("Waveforms");
	record->fX        = "Iteration$";
	record->fY        = "adc";
	record->fZ        = "";
	record->fCut      = "Entry$<10";
	record->fXAlias   = "~Time";
	record->fYAlias   = "~ADC unit";
	record->fZAlias   = "-empty-";
	record->fCutAlias = "~10evts";
	record->fOption   = "l";
	record->fScanRedirected = kFALSE;
	record->fCutEnabled = kTRUE;

	session->First(); // switch to first record

	viewer->Resize(630,330); // otherwise column width would be updated
}
