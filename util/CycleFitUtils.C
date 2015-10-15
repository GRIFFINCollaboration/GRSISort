TH2F *DeadTimeCorrect(TH2* cycleMat, TH2* triggerMat, Double_t dt_usec, Double_t dwellTime_sec){
   
   Int_t lowCycBin = cycleMat->GetYaxis()->GetFirst();
   Int_t highCycBin = cycleMat->GetYaxis()->GetLast();
   Int_t nCycBins = cycleMat->GetYaxis()->GetNbins();
   Int_t lowTimeBin = cycleMat->GetXaxis()->GetFirst();
   Int_t highTimeBin = cycleMat->GetXaxis()->GetLast();
   Int_t nTimeBins = cycleMat->GetXaxis()->GetNbins();
   const char* finalMatrixName = Form("%s_dtcorr",cycleMat->GetName());
   const char* finalMatrixTitle = Form("%s with Rejected Cycles",cycleMat->GetTitle());
   TH2F* finalMatrix = new TH2F(finalMatrixName, finalMatrixTitle, nTimeBins, lowTimeBin,highTimeBin, nCycBins,lowCycBin,highCycBin);

   for(int i = lowCycBin; i<=highCycBin;++i){
      tmpTrig = triggerMat->ProjectionX(Form("trigger_%d",i),i,i);
      tmpSlice = cycleMat->ProjectionX(Form("slice_%d",i),i,i);
      if(tmpTrig->Integral() < 1) continue;
         
      for(int bin=lowTimeBin; bin<highTimeBin; ++bin){
         Double_t dt_corr = 1./(1.-tmpTrig->GetBinContent(bin)*(dt_usec*1e-6 /dwellTime_sec));
         finalMatrix->Fill(bin,i,tmpSlice->GetBinContent(bin)*dt_corr);
      }
   }
   return finalMatrix;
}

TH2F *CycleRejector(TH2* cycleMat, Int_t counts, bool reject_last = true){
   Int_t nCycRejected = 0;
   Double_t totalData = 0;
   Double_t rejectedData = 0;
   Int_t nCyc = 0;


   printf(" ***********************\n");
   printf(" *    Cycle Rejector   *\n");
   printf(" ***********************\n\n");
   printf("          Threshold: %d Counts\n",counts);
   printf("  Reject Last Cycle: %d\n",reject_last);
   printf("\n");


   Int_t lowCycBin = cycleMat->GetYaxis()->GetFirst();
   Int_t highCycBin = cycleMat->GetYaxis()->GetLast();
   Int_t nCycBins = cycleMat->GetYaxis()->GetNbins();
   Int_t lowTimeBin = cycleMat->GetXaxis()->GetFirst();
   Int_t highTimeBin = cycleMat->GetXaxis()->GetLast();
   Int_t nTimeBins = cycleMat->GetXaxis()->GetNbins();
   const char* finalMatrixName = Form("%s_clean",cycleMat->GetName());
   const char* finalMatrixTitle = Form("%s with Rejected Cycles",cycleMat->GetTitle());
   TH2F* finalMatrix = new TH2F(finalMatrixName, finalMatrixTitle, nTimeBins, lowTimeBin,highTimeBin, nCycBins,lowCycBin,highCycBin);

   TH1D* tmpSlice;
   bool found_last = false;
   //If I do this in reverse order I can easily throw out the last cycle
   for(int i=highCycBin; i>=lowCycBin;--i){
      tmpSlice = cycleMat->ProjectionX(Form("slice_%d",i),i,i);
      if(!found_last && (tmpSlice->Integral() < 1) ){
         continue;
      }
      else if(!found_last && (tmpSlice->Integral() > 0)){
         printf("the last cycle found: %d\n",i);
         found_last = true;
         if(reject_last){
            printf("Rejecting Cycle %d\n",i);
            ++nCyc;
            totalData += tmpSlice->Integral();
            ++nCycRejected;
            rejectedData += tmpSlice->Integral();
            continue;
         }
      }
      
      ++nCyc;
      totalData += tmpSlice->Integral();

      if(tmpSlice->Integral() < counts){
         //If we are under our threshold, we dont want to add this spectrum to our new matrix
         ++nCycRejected;
         rejectedData += tmpSlice->Integral();
         continue;
      }

      for(int bin=lowTimeBin; bin<highTimeBin; ++bin){
         finalMatrix->Fill(bin,i,tmpSlice->GetBinContent(bin));
      }


   }

   tmpSlice->Draw();
   printf("Average Number of Counts: %lf\n",totalData/nCyc);
   printf("Average Counts in Rejected Cycles: %lf\n",rejectedData/nCycRejected);
   printf("Rejecting %d of %d Cycles\n",nCycRejected,nCyc);
   printf("Rejection Rate: %lf %%\n",rejectedData/totalData*100.);
   return finalMatrix;
}
