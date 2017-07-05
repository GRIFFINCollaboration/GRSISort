// g++ Root2Rad.cxx `root-config --cflags --libs` -oRoot2Rad
//
//    Author:  P.C. Bender, <pcbend@gmail.com>
//    *
//    *  * Please indicate changes with your initials.
//    *
//
//    Short and sweet program. Finds all the 1d (and 2d) histograms in a tfile by
//    itterating over the keys, than turn each of them into a fortran formatted
//    .spei/.mat file used by radware.
//
//    If histograms are larger than 16384 bins, it trys to be smart to decide
//    whether to truncate or compress the hist to fit the 16384 radwar limit.
//
//    Matrices are all truncated to 4096 or padded to 4096 if there are to few
//    bins.
//
//
// to compile:
//   g++ Root2Rad.cxx -oRoot2Rad `root-config --cflags --libs`
//
// to run:
//   ./Root2Rad rootfile.root
//

#include <cstdint>
#include <fstream>
#include <iostream>

#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TSystem.h"
#include "TList.h"
#include "TClass.h"
#include "TKey.h"
#include "TTimeStamp.h"
#include "THnSparse.h"

#include "Globals.h"
#include "GHSym.h"

struct SpeHeader {
   int32_t buffsize; /*fortran file, each record starts with record size */ // 14
   char    label[8];
   int32_t size;
   int32_t junk1;
   int32_t junk2;
   int32_t junk3;
   int32_t buffcheck; /*fortran file, record ends with record size :) */ // 14
} __attribute__((packed));

// the above would be followed by an integer of bin size * 4
// the value of all bins in int sizes
// an integer os bin size * 4           ------- number of char in the histogram.

void AddToList(TList*, TH2*, bool, bool);
void WriteHist(TH1*, std::fstream*);
void WriteMat(TH2*, std::fstream*);
void WriteM4b(TH2*, std::fstream*);

int main(int argc, char** argv)
{
   TFile* infile = nullptr;
   if(argc < 2 || (infile = TFile::Open(argv[1], "read")) == nullptr) {
      std::cout<<"problem opening file."<<std::endl
               <<"Usage: "<<argv[0]
               <<" file.root (optional: -s to split large matrices, -c to compress large matrices)"<<std::endl;
      return 1;
   }

   bool split    = false;
   bool compress = false;
   for(int i = 2; i < argc; ++i) {
      if(strcmp(argv[i], "-s") == 0) {
         split = true;
      } else if(strcmp(argv[i], "-c") == 0) {
         compress = true;
      } else {
         std::cout<<"Unrecognized flag "<<argv[i]<<std::endl;
      }
   }

   std::string path = infile->GetName();
   path.erase(path.find_last_of('.'));

#if defined(OS_DARWIN)
   struct stat st = {0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0, {0}};
#elif defined(__clang__)
   struct stat st = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
#else
   struct stat st = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

   if(stat(path.c_str(), &st) == -1) {
      mkdir(path.c_str(), 0755);
   }

   // std::string outfilename = infile->GetName();
   // outfilename.erase(outfilename.find_last_of('.'));
   // outfilename.append(".spe");
   // std::fstream outfile;
   // outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);

   TList* keys = infile->GetListOfKeys();
   keys->Sort();
   TIter next(keys);
   auto* histsToWrite = new TList();
   auto* matsToWrite  = new TList();
   auto* m4bsToWrite  = new TList();
   // int counter = 1;
   while(TKey* currentkey = dynamic_cast<TKey*>(next())) {
      std::string keytype = currentkey->ReadObj()->IsA()->GetName();
      if(keytype.compare(0, 3, "TH1") == 0) {
         // printf("%i currentkey->GetName() = %s\n",counter++, currentkey->GetName());
         // if((counter-1)%4==0)
         //	printf("*****************************\n");
         histsToWrite->Add(currentkey->ReadObj());
      } else if(keytype.compare(0, 4, "TH2C") == 0 || keytype.compare(0, 4, "TH2S") == 0) {
         AddToList(matsToWrite, dynamic_cast<TH2*>(currentkey->ReadObj()), split, compress);
      } else if(keytype.compare(0, 3, "TH2") == 0) {
         AddToList(m4bsToWrite, dynamic_cast<TH2*>(currentkey->ReadObj()), split, compress);
      } else if(keytype.compare(0, 3, "THn") == 0) {
         THnSparse* hist = (dynamic_cast<THnSparse*>(currentkey->ReadObj()));
         if(hist->GetNdimensions() == 1) {
            histsToWrite->Add(hist->Projection(0));
         } else if(hist->GetNdimensions() == 2) {
            if(keytype.compare(17, 1, "C") == 0 || keytype.compare(17, 1, "S") == 0) {
               AddToList(matsToWrite, hist->Projection(0, 1), split, compress);
            } else {
               AddToList(m4bsToWrite, hist->Projection(0, 1), split, compress);
            }
         }
      } else if(keytype.compare(0, 5, "GHSym") == 0) {
         if(keytype.compare(5, 1, "F") == 0) {
            AddToList(m4bsToWrite, dynamic_cast<GHSymF*>(currentkey->ReadObj())->GetMatrix(), split, compress);
         } else if(keytype.compare(5, 1, "D") == 0) {
            AddToList(m4bsToWrite, dynamic_cast<GHSymF*>(currentkey->ReadObj())->GetMatrix(), split, compress);
         } else {
            std::cout<<"unknown GHSym type "<<keytype<<std::endl;
         }
      } else {
         std::cout<<"skipping "<<keytype<<std::endl;
      }
   }

   // printf("histsToWrite->GetSize() = %i\n", histsToWrite->GetSize());

   TIter nexthist(histsToWrite);
   while(TH1* currenthist = dynamic_cast<TH1*>(nexthist())) {
      std::string outfilename = path + "/";
      outfilename.append(currenthist->GetName());
      outfilename.append(".spe");
      std::fstream outfile;
      outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
      WriteHist(currenthist, &outfile);
      printf("\t%s written to file %s.\n", currenthist->GetName(), outfilename.c_str());
      outfile.close();
   }

   TIter nextmat(matsToWrite);
   while(TH2* currentmat = dynamic_cast<TH2*>(nextmat())) {
      std::string outfilename = path + "/";
      outfilename.append(currentmat->GetName());
      outfilename.append(".mat");
      std::fstream outfile;
      outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
      WriteMat(currentmat, &outfile);
      printf("\t%s written to file %s.\n", currentmat->GetName(), outfilename.c_str());
      outfile.close();
   }

   TIter nextm4b(m4bsToWrite);
   while(TH2* currentm4b = dynamic_cast<TH2*>(nextm4b())) {
      std::string outfilename = path + "/";
      outfilename.append(currentm4b->GetName());
      outfilename.append(".m4b");
      std::fstream outfile;
      outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
      WriteM4b(currentm4b, &outfile);
      printf("\t%s written to file %s.\n", currentm4b->GetName(), outfilename.c_str());
      outfile.close();
   }

   // printf("closing file %s.\n",outfilename.c_str());
   // outfile.close();

   return 0;
}

void AddToList(TList* list, TH2* hist, bool split, bool compress)
{
   if((!split && !compress) || hist->GetXaxis()->GetNbins() <= 4096) {
      list->Add(hist);
      return;
   }
   if(split && compress) {
      TH2* splitHist = static_cast<TH2*>(hist->IsA()->New());
      splitHist->SetBins(4096, 0., 4096., 4096, 0., 4096.);
      splitHist->SetName(Form("%s_low", hist->GetName()));
      for(int binx = 1; binx <= 4096; ++binx) {
         for(int biny = 1; biny <= 4096; ++biny) {
            // ignore overflow cells
            if(binx <= hist->GetXaxis()->GetNbins() && biny <= hist->GetYaxis()->GetNbins()) {
               splitHist->SetBinContent(binx, biny, hist->GetBinContent(binx, biny));
            }
         }
      }
      list->Add(splitHist);
      int rebin = (hist->GetXaxis()->GetNbins() + 4095) / 4096;
      std::cout<<"rebinning "<<hist->GetName()<<" by "<<rebin<<std::endl;
      list->Add(hist->Rebin2D(rebin, rebin));
      return;
   } else if(split) {
      int  nofSplits = (hist->GetXaxis()->GetNbins() + 4095) / 4096;
      TH2* splitHist = static_cast<TH2*>(hist->IsA()->New());
      splitHist->SetBins(4096, 0., 4096., 4096, 0., 4096.);
      for(int i = 0; i < nofSplits; ++i) {
         for(int j = 0; j <= i; ++j) {
            std::cout<<hist->GetName()<<": x = "<<i * 4096<<" - "<<(i + 1) * 4096<<", y = "<<j * 4096
                     <<" - "<<(j + 1) * 4096<<std::endl;
            splitHist->Reset();
            splitHist->SetName(Form("%s_%d_%d", hist->GetName(), i, j));
            for(int binx = 1; binx <= 4096; ++binx) {
               for(int biny = 1; biny <= 4096; ++biny) {
                  // ignore overflow cells
                  if(i * 4096 + binx <= hist->GetXaxis()->GetNbins() &&
                     j * 4096 + biny <= hist->GetYaxis()->GetNbins()) {
                     splitHist->SetBinContent(binx, biny, hist->GetBinContent(i * 4096 + binx, j * 4096 + biny));
                  }
               }
            }
            list->Add(splitHist->Clone(Form("%s_%d_%d", hist->GetName(), i, j)));
         }
      }
      return;
   }
   // only option left now is compress
   int rebin = (hist->GetXaxis()->GetNbins() + 4095) / 4096;
   std::cout<<"rebinning "<<hist->GetName()<<" by "<<rebin<<std::endl;
   hist->SetName(Form("%s_rebin%d", hist->GetName(), rebin));
   list->Add(hist->Rebin2D(rebin, rebin));
}

void WriteMat(TH2* mat, std::fstream* outfile)
{
   int xbins = mat->GetXaxis()->GetNbins();
   int ybins = mat->GetYaxis()->GetNbins();

   auto* empty = new TH1D("empty", "empty", 4096, 0., 4096.);

   for(int y = 1; y <= 4096; ++y) {
      uint16_t buffer[4096] = {0};
      TH1D*    proj;
      if(y <= ybins) {
         proj = mat->ProjectionX("proj", y, y);
      } else {
         proj = empty;
      }
      for(int x = 1; x <= 4096; ++x) {
         if(x <= xbins) {
            buffer[x - 1] = static_cast<uint16_t>(proj->GetBinContent(x)); //    mat->GetBinContent(x,y));
         } else {
            buffer[x - 1] = 0;
         }
      }
      outfile->write(reinterpret_cast<char*>(&buffer), sizeof(buffer));
   }
   delete empty;
}

void WriteM4b(TH2* mat, std::fstream* outfile)
{
   int xbins = mat->GetXaxis()->GetNbins();
   int ybins = mat->GetYaxis()->GetNbins();

   auto* empty = new TH1D("empty", "empty", 4096, 0., 4096.);

   for(int y = 1; y <= 4096; ++y) {
      uint32_t buffer[4096] = {0};
      TH1D*    proj;
      if(y <= ybins) {
         proj = mat->ProjectionX("proj", y, y);
      } else {
         proj = empty;
      }
      for(int x = 1; x <= 4096; ++x) {
         if(x <= xbins) {
            buffer[x - 1] = static_cast<uint32_t>(proj->GetBinContent(x)); //    mat->GetBinContent(x,y));
         } else {
            buffer[x - 1] = 0;
         }
      }
      outfile->write(reinterpret_cast<char*>(&buffer), sizeof(buffer));
   }
   delete empty;
}

void WriteHist(TH1* hist, std::fstream* outfile)
{
   SpeHeader spehead{};
   spehead.buffsize = 24;
   strncpy(spehead.label, hist->GetName(), 8);

   if(hist->GetRMS() > 16384 / 2) {
      while(hist->GetNbinsX() > 16384) {
         hist = hist->Rebin(2);
         printf(DBLUE "\t!!  %s has been compressed by 2." RESET_COLOR "\n", hist->GetName());
      }
      spehead.size = hist->GetNbinsX();
   } else if(hist->GetNbinsX() > 16384) {
      spehead.size = 16384;
   } else {
      spehead.size = hist->GetNbinsX();
   }

   spehead.junk1     = 1;
   spehead.junk2     = 1;
   spehead.junk3     = 1;
   spehead.buffcheck = 24; /*fortran file, record ends with record size :) */ // 14

   outfile->write(reinterpret_cast<char*>(&spehead), sizeof(SpeHeader));

   int32_t histsizeinbytes = spehead.size * 4;

   outfile->write(reinterpret_cast<char*>(&histsizeinbytes), sizeof(int32_t));
   float bin = 0.0;
   for(int x = 1; x <= spehead.size; ++x) {
      if(x <= hist->GetNbinsX()) {
         bin = static_cast<float>(hist->GetBinContent(x));
         outfile->write(reinterpret_cast<char*>(&bin), sizeof(int32_t));
      } else {
         bin = 0.0;
         outfile->write(reinterpret_cast<char*>(&bin), sizeof(int32_t));
      }
   }

   outfile->write(reinterpret_cast<char*>(&histsizeinbytes), sizeof(int32_t));
}
