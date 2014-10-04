




TList *MakeMatrices(TChain* tree, int gammaMult = 2, int betaMult = 0, int coincLow = 0, int coincHigh = 10, int bg = 100, int nofBins = 4000, double low = 0., double high = 4000.) {
  list = new TList;

   TH1F* time = new TH1F("time","time difference;time[10 ns]",200,0.,200.); list->Add(time);
   TH1F* timeEGate = new TH1F("timeEGate","time difference energy gate on 1808 and 1130;time[10 ns]",200,0.,200.);  list->Add(timeEGate);
   TH1F* timeModuleOne = new TH1F("timeModuleOne","time difference for 1. grif-16;time[10 ns]",200,0.,200.); list->Add(timeModuleOne);
   TH2F* matrix = new TH2F("matrix",Form("#gamma-#gamma matrix with #gamma multiplicity >= %d and #beta multiplicity >= %d",gammaMult,betaMult),nofBins, low, high,nofBins, low, high); list->Add(matrix);
   TH2F* matrix_coinc = new TH2F("matrix_coinc",Form("#gamma-#gamma matrix with #gamma multiplicity >= %d and #beta multiplicity >= %d, coincident within %d - %d",gammaMult, betaMult, coincLow, coincHigh),nofBins, low, high,nofBins, low, high);  list->Add(matrix_coinc);
   TH2F* matrix_bg = new TH2F("matrix_bg",Form("#gamma-#gamma matrix with #gamma multiplicity >= %d and #beta multiplicity >= %d, background within %d - %d",gammaMult, betaMult, bg+coincLow, bg+coincHigh),nofBins, low, high,nofBins, low, high); list->Add(matrix_bg);

   TH1F* cfdDiff = new TH1F("cfdDiff","cfd difference",2000,-1000.,1000.); list->Add(cfdDiff);
   TH1F* cfdDiffEGate = new TH1F("cfdDiffEGate","cfd difference energy gate on 1808 and 1130",2000,-1000.,1000.); list->Add(cfdDiffEGate);
   TH1F* cfdDiffModuleOne = new TH1F("cfdDiffModuleOne","cfd difference for 1. grif-16",2000,-1000.,1000.); list->Add(cfdDiffModuleOne);
   
   //TTree* tree = (TTree*) file->Get("AnalysisTree");
 
   TGriffin* grif = 0;
   TSceptar* scep = 0;
   tree->SetBranchAddress("TGriffin", &grif);

   bool gotSceptar;
   if(tree->FindBranch("TSceptar") == 0) {
      gotSceptar = false;
   } else {
      tree->SetBranchAddress("TSceptar", &scep);
      gotSceptar = true;
   }

   //for gamma-gamma we need a gamma multiplicity of at least 2
   if(gammaMult < 2) {
      gammaMult = 2;
   }

   int skipped = 0;
   int notSkipped = 0;
   int coincident = 0;
   int background = 0;

   for(int entry = 0; entry < tree->GetEntries(); ++entry) {
      tree->GetEntry(entry);
      if(grif->GetMultiplicity() < gammaMult || (gotSceptar && scep->GetMultiplicity() < betaMult)) {
         //if(entry < 80) {
            //cout<<entry<<": skipping  gamma mult. = "<<grif->GetMultiplicity();
            //if(gotSceptar) {
               //cout<<", and beta mult. = "<<scep->GetMultiplicity();
            //}
            //cout<<endl;
         //}
         ++skipped;
         continue;
      }
      ++notSkipped;
      //std::cout<<"got multiplicity "<<grif->GetMultiplicity()<<", time 0 = "<<grif->GetGriffinHit(0)->GetTime()<<", time 1 = "<<grif->GetGriffinHit(1)->GetTime()<<std::endl;
      //loop over all gamma's in two loops
      for(int one = 0; one < (int) grif->GetMultiplicity(); ++one) {
         for(int two = 0; two < (int) grif->GetMultiplicity(); ++two) {
            if(two == one) {
               continue;
            }
            if(two > one) {
               time->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
               cfdDiff->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               if((1125<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1815) ||
                  (1125<grif->GetGriffinHit(two)->GetEnergyLow() && grif->GetGriffinHit(two)->GetEnergyLow()<1135 && 1805<grif->GetGriffinHit(one)->GetEnergyLow() && grif->GetGriffinHit(one)->GetEnergyLow()<1815)) {
                  timeEGate->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                  cfdDiffEGate->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               }
               if(grif->GetGriffinHit(one)->GetAddress()<16 && grif->GetGriffinHit(two)->GetAddress()<16) {
                  timeModuleOne->Fill(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime());
                  cfdDiffModuleOne->Fill(grif->GetGriffinHit(two)->GetCfd()-grif->GetGriffinHit(one)->GetCfd());
               }
            }
            if(!gotSceptar && (grif->GetGriffinHit(one)->GetAddress()%16 == 15 || grif->GetGriffinHit(two)->GetAddress()%16 == 15)) {
               continue;
            }
            matrix->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
            if(coincLow <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < coincHigh) {
               matrix_coinc->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
               ++coincident;
            } else if((bg+coincLow) <= TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) && TMath::Abs(grif->GetGriffinHit(two)->GetTime()-grif->GetGriffinHit(one)->GetTime()) < (bg+coincHigh)) {
               matrix_bg->Fill(grif->GetGriffinHit(one)->GetEnergyLow(), grif->GetGriffinHit(two)->GetEnergyLow());
               ++background;
            }
         }
      }
   }

   std::cout<<"skipped "<<skipped<<" entries, didn't skip "<<notSkipped<<" entries: got "<<coincident<<" coincident and "<<background<<" background events"<<std::endl;

   TH2F* matrix_bgcorr = matrix_coinc->Clone("matrix_bgcorr"); list->Add(matrix_bgcorr);
   matrix_bgcorr->SetTitle(Form("#gamma-#gamma matrix with #gamma multiplicity >= %d and #beta multiplicity >= %d, background corrected",gammaMult,betaMult));
   matrix_bgcorr->Add(matrix_bg,-1.);

   TCanvas* c = new TCanvas;
   c->Divide(2,2);
   c->cd(1);
   matrix->Draw("colz");
   c->cd(2);
   matrix_bgcorr->Draw("colz");
   c->cd(3);
   matrix_coinc->Draw("colz");
   c->cd(4);
   matrix_bg->Draw("colz");
   TCanvas* c2 = new TCanvas;
   c2->Divide(2);
   c2->cd(1);
   time->Draw();
   timeEGate->SetLineColor(2);
   timeEGate->Draw("same");
   timeModuleOne->SetLineColor(4);
   timeModuleOne->Draw("same");
   c2->cd(2);
   cfdDiff->Draw();
   cfdDiffEGate->SetLineColor(2);
   cfdDiffEGate->Draw("same");
   cfdDiffModuleOne->SetLineColor(4);
   cfdDiffModuleOne->Draw("same");

   list->Sort();
   return list;
}
