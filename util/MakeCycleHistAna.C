
TList* MakeCycleHist(TFile *file, long cycleLength = 2950000000, double lowEnergy = 0., double highEnergy = 16000., 
                     int nofXBins = 30000, double lowXRange = 0., double highXRange = 30000., 
                     int nofYBins = 4000, double lowYRange = 0., double highYRange = 4000.) {
   TChannel::ReadCalFromTree(AnalysisTree);

   TList* list = new TList;

   TH2F *gammaVsCycleTime = new TH2F("gammaVsCycleTime","E_{#gamma} vs. cycle time;cycle time [ms];E_{#gamma} [keV]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   gammaVsCycleTime->SetStats(false);
   gammaVsCycleTime->GetXaxis()->CenterTitle();
   gammaVsCycleTime->GetYaxis()->CenterTitle();
   list->Add(gammaVsCycleTime);
   TH2F *betaVsCycleTime = new TH2F("betaVsCycleTime","E_{#beta} vs. cycle time;cycle time [ms];E_{#beta} [a.u.]",nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   betaVsCycleTime->SetStats(false);
   betaVsCycleTime->GetXaxis()->CenterTitle();
   betaVsCycleTime->GetYaxis()->CenterTitle();
   list->Add(betaVsCycleTime);

   long timestamp;
   long time;
   long fillTime;
   long cycleTime;
   int maxCycleNumber = 0;
   int entries = AnalysisTree->GetEntries();
   TGriffin *grif = 0;
   AnalysisTree->SetBranchAddress("TGriffin",&grif);
   TSceptar *scep = 0;
   if(AnalysisTree->FindBranch("TSceptar") != 0) {
      AnalysisTree->SetBranchAddress("TSceptar",&scep);
   }

   for(int x=0;x<entries;x++) {
      AnalysisTree->GetEntry(x);
      for(size_t g = 0; g < grif->GetMultiplicity(); ++g) {
         timestamp = grif->GetGriffinHit(g)->GetTime();
         gammaVsCycleTime->Fill(grif->GetCycleTimeInMilliSeconds(timestamp), grif->GetGriffinHit(g)->GetEnergyLow()); 
      }
      if(scep != 0) {
         for(size_t b = 0; b < scep->GetMultiplicity(); ++b) {
            timestamp = scep->GetSceptarHit(b)->GetTime();
            gammaVsCycleTime->Fill(grif->GetCycleTimeInMilliSeconds(timestamp), scep->GetSceptarHit(b)->GetEnergy()); 
         }
      }
      if(x%50000==0) 
         cout << "\t" << x << " / " << entries << "\r" << flush;
   }
   cout << "\t" << entries << " / " << entries << "\r" << endl;

   TCanvas *c = new TCanvas;
   c->Divide(2);
   c->cd(1);
   gammaVsCycleTime->Draw("colz");
   c->cd(2);
   betaVsCycleTime->Draw("colz");

   return list;
}  





