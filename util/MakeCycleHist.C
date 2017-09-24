
int MakeCycleHist(TFile *file, long cycleLength = 2950000000, double lowEnergy = 0., double highEnergy = 16000., 
                  int nofXBins = 30000, double lowXRange = 0., double highXRange = 30000., 
                  int nofYBins = 100, double lowYRange = 0., double highYRange = 100.) {
   TChannel::ReadCalFromTree(FragmentTree);
   TH2F *channelVsPPGCycleTime = new TH2F("channelVsPPGCycleTime",Form("channel vs. cycle time with energy = %.1f - %.1f keV, based on ppg timestamps;cycle time [ms];channel",lowEnergy,highEnergy),nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   channelVsPPGCycleTime->SetStats(false);
   channelVsPPGCycleTime->GetXaxis()->CenterTitle();
   channelVsPPGCycleTime->GetYaxis()->CenterTitle();
   TH2F *ppgVsPPGCycleTime = new TH2F("ppgVsPPGCycleTime",Form("ppg vs. cycle time with energy = %.1f - %.1f keV, based on ppg timestamps;cycle time [ms];ppg pattern",lowEnergy,highEnergy),nofXBins,lowXRange,highXRange,5,0,5);
   ppgVsPPGCycleTime->GetYaxis()->SetBinLabel(1,"tape move");
   ppgVsPPGCycleTime->GetYaxis()->SetBinLabel(2,"background");
   ppgVsPPGCycleTime->GetYaxis()->SetBinLabel(3,"beam on");
   ppgVsPPGCycleTime->GetYaxis()->SetBinLabel(4,"decay");
   ppgVsPPGCycleTime->GetYaxis()->SetBinLabel(5,"unknown");
   ppgVsPPGCycleTime->SetStats(false);
   ppgVsPPGCycleTime->GetXaxis()->CenterTitle();
   ppgVsPPGCycleTime->GetYaxis()->CenterTitle();
   ppgVsPPGCycleTime->GetYaxis()->SetTitleOffset(1.25);

   TH2F *channelVsCycleTime = new TH2F("channelVsCycleTime",Form("channel vs. cycle time with energy = %.1f - %.1f keV, based on cycle length;cycle time [ms];channel",lowEnergy,highEnergy),nofXBins,lowXRange,highXRange,nofYBins,lowYRange,highYRange);
   channelVsCycleTime->SetStats(false);
   channelVsCycleTime->GetXaxis()->CenterTitle();
   channelVsCycleTime->GetYaxis()->CenterTitle();
   TH2F *ppgVsCycleTime = new TH2F("ppgVsCycleTime",Form("ppg vs. cycle time with energy = %.1f - %.1f keV, based on cycle length;cycle time [ms];ppg pattern",lowEnergy,highEnergy),nofXBins,lowXRange,highXRange,5,0,5);
   ppgVsCycleTime->GetYaxis()->SetBinLabel(1,"tape move");
   ppgVsCycleTime->GetYaxis()->SetBinLabel(2,"background");
   ppgVsCycleTime->GetYaxis()->SetBinLabel(3,"beam on");
   ppgVsCycleTime->GetYaxis()->SetBinLabel(4,"decay");
   ppgVsCycleTime->GetYaxis()->SetBinLabel(5,"unknown");
   ppgVsCycleTime->SetStats(false);
   ppgVsCycleTime->GetXaxis()->CenterTitle();
   ppgVsCycleTime->GetYaxis()->CenterTitle();
   ppgVsCycleTime->GetYaxis()->SetTitleOffset(1.25);

   long maxTime = FragmentTree->GetMaximum("TimeStampHigh")*((long)pow(2,29));
   TH1F *countsPerCycle = new TH1F("countsPerCycle","counts per cycle;cycle number;counts",maxTime/cycleLength,0,maxTime/cycleLength);
   TH2F *cycleVsCycleTime = new TH2F("cycleVsCycleTime","cycle number vs. cycle time for beam on events;cycle time [ms];cycle number",nofXBins,lowXRange,highXRange,maxTime/cycleLength,0,maxTime/cycleLength);
   TH1F *beamOnCounts = new TH1F("beamOnCounts","beam on counts;time [s];beam on events",nofXBins,lowXRange,highXRange/10.);

   long timestamp;
   long time;
   long fillTime;
   long cycleTime;
   int maxCycleNumber = 0;
   int entries = FragmentTree->GetEntries();
   TFragment *frag = 0;
   FragmentTree->SetBranchAddress("TFragment",&frag);

   for(int x=0;x<entries;x++) {
      FragmentTree->GetEntry(x);
      timestamp = frag->GetTimeStamp();
      if(lowEnergy < frag->GetEnergy() && frag->GetEnergy() < highEnergy) {
         cycleTime = timestamp - cycleLength*(timestamp/cycleLength);
         if(timestamp/cycleLength > maxCycleNumber) { 
            maxCycleNumber = timestamp/cycleLength; 
         }
         countsPerCycle->Fill(timestamp/cycleLength);
         channelVsCycleTime->Fill(cycleTime/100000., frag->ChannelNumber); 
         switch(frag->PPG) {
            case 0x0008:
               ppgVsCycleTime->Fill(cycleTime/100000., "tape move", 1); 
               break;
            case 0xd000:
               ppgVsCycleTime->Fill(cycleTime/100000., "background", 1); 
               break;
            case 0xc001:
               ppgVsCycleTime->Fill(cycleTime/100000., "beam on", 1); 
               cycleVsCycleTime->Fill(cycleTime/100000., timestamp/cycleLength);
               beamOnCounts->Fill(timestamp/1e8);
               break;
            case 0xc000:
               ppgVsCycleTime->Fill(cycleTime/100000., "decay", 1); 
               break;
            default:
               ppgVsCycleTime->Fill(cycleTime/100000., "unknown", 1); 
               break;
         }
      }
      if(frag->PPG == 0xd000) {
         time = timestamp;
         continue;
      }
      if(lowEnergy < frag->GetEnergy() && frag->GetEnergy() < highEnergy) {
         fillTime= timestamp - time;
         channelVsPPGCycleTime->Fill(fillTime/100000., frag->ChannelNumber); 
         switch(frag->PPG) {
            case 0x0008:
               ppgVsPPGCycleTime->Fill(fillTime/100000., "tape move", 1); 
               break;
            case 0xd000:
               ppgVsPPGCycleTime->Fill(fillTime/100000., "background", 1); 
               break;
            case 0xc001:
               ppgVsPPGCycleTime->Fill(fillTime/100000., "beam on", 1); 
               break;
            case 0xc000:
               ppgVsPPGCycleTime->Fill(fillTime/100000., "decay", 1); 
               break;
            default:
               ppgVsPPGCycleTime->Fill(fillTime/100000., "unknown", 1); 
               break;
         }
      }
      if(x%50000==0) 
         cout<<"\t"<<x<<" / "<<entries<<"\r"<<flush;
   }
   cout<<"\t"<<entries<<" / "<<entries<<"\r"<<endl;

   TCanvas *c = new TCanvas;
   c->Divide(2,2);
   c->cd(1);
   channelVsPPGCycleTime->Draw("colz");
   c->cd(2);
   ppgVsPPGCycleTime->Draw("colz");
   c->cd(3);
   channelVsCycleTime->Draw("colz");
   c->cd(4);
   ppgVsCycleTime->Draw("colz");
   TCanvas *s = new TCanvas;
   s->Divide(2);
   s->cd(1);
   beamOnCounts->Draw();
   s->cd(2);
   cycleVsCycleTime->Draw("colz");

   return maxCycleNumber;
}  





