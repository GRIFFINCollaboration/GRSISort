


void check885() {

   if(!AnalysisTree) {
      printf("no analysis tree found!\n");
      return;
   }
   
   AnalysisTree->Project("hist(8000,0,4000)","griffin_hits.GetEnergy()","@sceptar_hits.size()>0");
   AnalysisTree->Project("histsing(8000,0,4000)","griffin_hits.GetEnergy()");
   TH1F *hist  = (TH1F*)gROOT->FindObjectAny("hist");
   TH1F *hist2 = (TH1F*)hist->Clone("hist2");
   TH1F *histsing = (TH1F*)gROOT->FindObjectAny("histsing");
   
  // double sum = hist2->Integral(883/hist2->GetBinWidth(883),887/hist2->GetBinWidth(887));

   TPeak * peak = new TPeak(883,875,892);
   //double sing_area = 0.0;
  // peak->Fit(histsing);
  // sing_area = peak->GetArea();

 //  peak->Clear();
   peak->Fit(hist);
   double sum = peak->GetArea();

  // double b_effic = sum/sing_area;

   hist2->GetXaxis()->SetRangeUser(870,910);

   double time = 0.0;
   if(FragmentTree) {
      time = FragmentTree->GetMaximum("MidasTimeStamp") - FragmentTree->GetMinimum("MidasTimeStamp");
   }
   
   if(TGRSIRunInfo::Get()) {
     // printf("\n
      printf("\n\tRun %05i_%03i:\t%i counts in %.2f seconds.\t%.4f cnts/min\n",
      TGRSIRunInfo::Get()->RunNumber(),TGRSIRunInfo::Get()->SubRunNumber(),
      (int)sum,time,sum/(time/60.0));
   }

   TCanvas *c1 = new TCanvas;
   c1->Divide(1,2);
   c1->cd(1);
   hist->Draw();
   c1->cd(2);
   hist2->Draw();
   
   return;
}


