
#include <cstdio>
#include <string>

#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include "TChannel.h"

void Make1DChargeHistograms(TChain *chain, TFile *outfile=0) {


   TH2F *hist = new TH2F("hist","hist",3000,0,3000,16000,0,64000);
   printf("Making charge histograms..."); fflush(stdout);
   chain->Project("hist","Charge/125.0:ChannelNumber");
   printf(" done!\n");  fflush(stdout);

   for(int x=0;x<3000;x++) {
      TH1D *h = hist->ProjectionY("py",x,x);
      if(h->Integral()>0) {
         TChannel *channel = TChannel::GetChannelByNumber(x);
         if(!channel)
            h->SetNameTitle(Form("hist_%04i",x),Form("hist_%04i",x));
         else
            h->SetNameTitle(Form("hist_%04i",x),channel->GetChannelName());
         if(outfile) {
            outfile->cd();
            h->Write();
            printf("Wrote hist %s to file %s\n",h->GetName(),outfile->GetName());
         }   
      }
   }
   return;
}


#ifndef __CINT__

void PrintUsage() {
   printf("How to use: \n");
   return;
}

int main(int argc, char **argv) {
   
   if(argc <2) {
        PrintUsage();
        return 1;
   }
   TChain *frag_chain = new TChain("FragmentTree");
   std::string outfilename; 

   

   for(int x=1;x<argc;x++) {
      if(strncmp(argv[x],"fragment",7)==0) {
         frag_chain->Add(argv[x]);
      }
      else if(strncmp(argv[x],"analysis",8)==0) {
         // do nothing...
      } else  {
         if(outfilename.length()==0) {
             outfilename.assign(argv[x]);
          } else {
             PrintUsage();
             return 1;
          } 
      }  
   }

   if(frag_chain->GetNtrees()<1) {
      PrintUsage();
      return 1;
   }

   frag_chain->GetEntry(0);
   TChannel::ReadCalFromTree(frag_chain->GetTree());

   TFile *rootout = 0;

   if(outfilename.length()==0)
      rootout = new TFile("chargehist_output.root","recreate");
   else    
      rootout = new TFile(outfilename.c_str(),"recreate");

   Make1DChargeHistograms(frag_chain,rootout); 

   return 0;
}

#endif


