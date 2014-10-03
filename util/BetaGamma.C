

void BetaGamma(TFile *file, int nofBins = 16000, double lowRange = 0., double highRange = 16000.){
   TGriffin *grif = 0;
   TGriffinHit* grif_hit = 0;
   AnalysisTree->SetBranchAddress("TGriffin",&grif);

   TH1D* gammaOnly = new TH1D("gammaOnly","gamma's w/o any coincidences",nofBins,lowRange,highRange);
   TH1D* gammaMult = new TH1D("gammaMult","gamma's w/o any beta's",nofBins,lowRange,highRange);
   TH1D* gammaSingles = new TH1D("gammaSingles","gamma's",nofBins,lowRange,highRange);
   TH1D* betaGamma = new TH1D("betaGamma","gamma's coincident with beta's",nofBins,lowRange,highRange);
   bool foundBeta = false;
   for(int i = 0; i<AnalysisTree->GetEntries(); i++){
     AnalysisTree->GetEntry(i);
     if(grif->GetMultiplicity() <=1) {
        if(grif->GetGriffinHit(0)->GetAddress()%16 != 15) {
           gammaOnly->Fill(grif->GetGriffinHit(0)->GetEnergyLow());
           gammaSingles->Fill(grif->GetGriffinHit(0)->GetEnergyLow());
        }
        continue;
     }

     foundBeta = false;
     for(int j =0; j<grif->GetMultiplicity();j++){
       grif_hit = grif->GetGriffinHit(j);
       if(grif_hit->GetAddress()%16==15) {
          foundBeta = true;
          break;
       }
     }
     for(int j =0; j<grif->GetMultiplicity();j++){
       grif_hit = grif->GetGriffinHit(j);
       if(grif_hit->GetAddress()%16!=15) {
          gammaSingles->Fill(grif_hit->GetEnergyLow());
       }
     }

     if(!foundBeta) {
       for(int j =0; j<grif->GetMultiplicity();j++){
         grif_hit = grif->GetGriffinHit(j);
         if(grif_hit->GetAddress()%16!=15) {
            gammaMult->Fill(grif_hit->GetEnergyLow());
         }
       }
       continue;
     }

     for(int j =0; j<grif->GetMultiplicity();j++){
       grif_hit = grif->GetGriffinHit(j);
       if(grif_hit->GetAddress()%16!=15) {
          betaGamma->Fill(grif_hit->GetEnergyLow());
       }
     }
   }

  TCanvas *c2 = new TCanvas;
  c2->Divide(2,2);
  c2->cd(1);
  betaGamma->Draw();
  c2->cd(2);
  gammaSingles->Draw();
  c2->cd(3);
  gammaOnly->Draw();
  c2->cd(4);
  gammaMult->Draw();

  return;
}
