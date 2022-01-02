void view(const char* fi="input.root")
{
	TTreeViewer *viewer = new TTreeViewer;
	viewer->SetScanRedirect(kTRUE); // avoid involving terminal output

	TFile *f = new TFile(fi);
 	TTree *t;
	for (int m=0; m<21; m++) {
		for (int c=0; c<16; c++) {
			t = (TTree*) f->Get(Form("t%d",m*16+c));
			if (t) viewer->AppendTree(t);
		}
	}

	// items (0:x, 1:y, 2:z, 3:cut, 4:scan, users must start at 5)
	viewer->ExpressionItem(5)->SetExpression("h", "~height", kFALSE);
	viewer->ExpressionItem(6)->SetExpression("ih", "~peak position", kFALSE);
	viewer->ExpressionItem(7)->SetExpression("n", "~number of samples", kFALSE);

	// records (for better control)
	TTVSession* session = new TTVSession(viewer);
	viewer->SetSession(session);
	// 1st record
	TTVRecord* record = session->AddRecord(kTRUE);
	record = session->AddRecord(kTRUE);
	session->SetRecordName("Waveforms");
	record->fX        = "t";
	record->fY        = "s";
	record->fZ        = "";
	record->fCut      = "Entry$<10";
	record->fXAlias   = "~Time [ns]";
	record->fYAlias   = "~ADC unit";
	record->fZAlias   = "-empty-";
	record->fCutAlias = "~10evts";
	record->fOption   = "l";
	record->fScanRedirected = kFALSE;
	record->fCutEnabled = kTRUE;

	// 2nd record
	record = session->AddRecord(kTRUE);
	session->SetRecordName("Accumulated Sums");
	record->fX        = "Iteration$";
	record->fY        = "sum";
	record->fZ        = "";
	record->fCut      = "Entry$<50";
	record->fXAlias   = "~Accumulator";
	record->fYAlias   = "~ADC unit";
	record->fZAlias   = "-empty-";
	record->fCutAlias = "~50evts";
	record->fOption   = "l";
	record->fScanRedirected = kFALSE;
	record->fCutEnabled = kTRUE;

	session->First(); // switch to first record

	viewer->Resize(630,330); // otherwise column width would be updated
}
