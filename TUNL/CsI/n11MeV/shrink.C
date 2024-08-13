{ // shrink root file size by applying stronger cuts
	TChain *told = new TChain("t");
	for (int i=1; i<75; i++) told->Add(Form("Integrated18__20220729182748_%d.root",i));
	
	told->SetBranchStatus("*",0);
	told->SetBranchStatus("m",1);
	told->SetBranchStatus("pb",1);
	told->SetBranchStatus("e2",1);
	told->SetBranchStatus("e3",1);
	told->SetBranchStatus("e4",1);
	told->SetBranchStatus("e5",1);
	told->SetBranchStatus("hm",1);
	told->SetBranchStatus("ht",1);
	told->SetBranchStatus("bd",1);
	told->SetBranchStatus("ns",1);
	told->SetBranchStatus("vc",1);
	told->SetBranchStatus("v",1);
	told->SetBranchStatus("dt",1);
	told->SetBranchStatus("date",1);
	told->SetBranchStatus("time",1);
	told->SetBranchStatus("run",1);
	told->SetBranchStatus("evnt",1);

	TFile *output = new TFile("shrinked185k.root","recreate");
	TTree *t = told->CloneTree(0);

	int m;
	float  ht, hm; 
	float vc[4096],v[4096];	
	told->SetBranchAddress("m",&m); // number of samples
	told->SetBranchAddress("ht",&ht);  
	told->SetBranchAddress("hm",&hm);
	told->SetBranchAddress("vc",vc);
	told->SetBranchAddress("v",v);

	float pc, pc1, sd1, dwi, pc5, pc6, sd6, sd5, lm, en, en5;
	int ng20, np1, np5, np6, npp;
	float pa[100], pt[100], ph[100], bgn[100], end[100];

	t->Branch("pc",&pc,"pc/F"); // pedestal of CsI averaged over 300 samples before integral after correcting the overshoot
	t->Branch("pc1",&pc1,"pc1/F"); // pedestal of CsI averaged over 300 samples before integral after correcting the overshoot
	t->Branch("sd1",&sd1,"sd1/F"); // RMS of pedestal of CsI waveform after correcting the overshoot
	t->Branch("dwi",&dwi,"dwi/F"); // RMS of integral range of CsI waveform after correcting the overshoot
	t->Branch("pc5",&pc5,"pc5/F"); // pedestal of corrected CsI waveform averaged in range (4000,5000)ns without spikes above 10
	t->Branch("pc6",&pc6,"pc6/F"); // pedestal of corrected CsI waveform averaged in range (5000,6000)ns without spikes above 10
	t->Branch("sd6",&sd6,"sd6/F");// SD of corrected CsI waveform averaged in range (5000,6000)ns without spikes above 10
	t->Branch("sd5",&sd5,"sd5/F");// SD of corrected CsI waveform averaged in range (4000,5000)ns without spikes above 10
	t->Branch("np1",&np1,"np1/I");
	t->Branch("np5",&np5,"np5/I");
	t->Branch("np6",&np6,"np6/I");
	t->Branch("lm",&lm,"lm/F");
	t->Branch("ng20",&ng20,"ng20/I");
	t->Branch("en",&en,"en/F");
	t->Branch("en5",&en5,"en5/F");

	t->Branch("npp",&npp,"npp/I"); // number of pulses in a CsI waveform
	t->Branch("ph",ph,"ph[npp]/F"); // pulse height
	t->Branch("pa",pa,"pa[npp]/F"); // pulse area
	t->Branch("pt",pt,"pt[npp]/F"); // pulse time (above threshold)
	t->Branch("bgn",bgn,"bgn[npp]/F"); // beginning of a pulse
	t->Branch("end",end,"end[npp]/F"); // end of a pulse



	int nevt = told->GetEntries();
	cout<<nevt<<" events to be processed"<<endl;
	for (int i=0; i<nevt; i++) {
		if (i%10000==0) cout<<"Processing event "<<i<<endl;

		// process CsI waveform
		told->GetEntry(i);
		m=1500;lm=0;
		if (ht<325||ht>600) continue; // consider removing
		if (hm>500) continue;
		pc=0; sd1=0; dwi=0; pc5=0; pc6=0;sd5=0; sd6=0;
		for (int k=0; k<300; k++) pc+=vc[k];
		pc/=300;

		for (int k=0; k<m; k++) {
			vc[k]-=pc; // remove pedestal	calculated in a larger range
			if (lm>vc[k]) lm=vc[k];
		}

		// calculate pedestal excluding pulses in the range
		np1=0; pc1=0;np5=0; np6=0;
		for (int k=0; k<300; k++){
			if(vc[k]<10) {pc1+=vc[k];np1++;} 
		}
		pc1/=np1;
		for (int k=0; k<m; k++) vc[k]-=pc1; // remove pedestal
	 	for (int k=0; k<300; k++){ 
			if(vc[k]<10) sd1+=(vc[k]-pc1)*(vc[k]-pc1); // calculate pedestal RMS
		}
		sd1=sqrt(sd1/np1);

		for(int k=0;k<m;k++){
			if (k>300&&k<750) dwi+=(vc[k]-pc1)*(vc[k]-pc1);//trigger range RMS
			}
		dwi=sqrt(dwi/(750-300));

		en=0; int cont=0; en5=0;

		for (int k=300;k<1250;k++){
			en5+=vc[k];
		}

		ng20=0;
		for (int k=300; k<600; k++) {
			if(vc[k]>15&&vc[k]>vc[k-1]&&vc[k]>vc[k+1]) ng20++; 
		}

		//calculate pedestal and RMS of waveform tail(4000,5000) and (5000,6000) 	
		for (int k=1000; k<1250; k++)	{
		 	if (vc[k]<10){pc5+=vc[k];np5++;}
	 	}
		pc5/=np5;
		for (int k=1000; k<1250; k++){ 
			if(vc[k]<10) sd5+=(vc[k]-pc5)*(vc[k]-pc5);
		}
		sd5=sqrt(sd5/np5);
		for (int k=1250; k<1500; k++) {
			if(vc[k]<10){pc6+=vc[k];np6++;}
		}
		pc6/=np6;
		for (int k=1250; k<1500; k++){
		       if(vc[k]<10) sd6+=(vc[k]-pc6)*(vc[k]-pc6);
		}
		sd6=sqrt(sd6/np6);
		
		// search for pulses in [300,1000)
		npp=0; bool aboveThreshold, outOfPrevPls, prevSmplBelowThr=true;
		for (int j=0; j<100; j++) { pa[j]=0; ph[j]=0; pt[j]=0; bgn[j]=0; end[j]=0; }
		for (int k=300; k<1250; k++) {
			if (vc[k]>12) aboveThreshold=true; else aboveThreshold=false;
			if (npp>0 && k==end[npp-1] && aboveThreshold) end[npp-1]=k+10<m?k+10:m-1;

			outOfPrevPls=true;
			if (npp>0 && k-10<end[npp-1]) outOfPrevPls=false;

			if (aboveThreshold && prevSmplBelowThr) {
				if (outOfPrevPls) { // create a new pulse
					pt[npp]=k*4;
					bgn[npp]=k-5<0?0:k-5;
					end[npp]=k+10<m?k+10:m-1;
					npp++;
					if (npp>=100) break; // only record 100 pulses
				} else { // update previous pulse tentative end
					end[npp-1]=k+10<m?k+10:m-1;
				}
			}
			prevSmplBelowThr=!aboveThreshold; // flip flag for next sample after using it
	       	}

		for (int j=0; j<npp; j++) { // update pulses
			for (int k=bgn[j]; k<end[j]; k++) {
				if (vc[k]>ph[j]) ph[j]=vc[k];
				pa[j]+=vc[k];
			}
			bgn[j]*=4; end[j]*=4;en+=pa[j];
		}


		if (pc<-10||pc>10) continue;
		if (pc5<-3||pc5>4) continue;
		if (pc6<-3||pc6>4) continue;
		if (sd1<2.7||sd1>3.7) continue;
		if (sd5<2.6||sd5>3.6) continue;
		if (sd6<2.6||sd6>3.6) continue;
		if (lm<-12) continue;
		t->Fill();
	}
	t->Write("",TObject::kOverwrite);
	output->Close();
	gSystem->Chmod(output->GetName(),S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	cout<<"File "<<output->GetName()<<" saved"<<endl;
}

