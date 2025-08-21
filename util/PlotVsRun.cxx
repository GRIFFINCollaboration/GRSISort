#include <iostream>
#include <vector>
#include <string>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include "TRunInfo.h"

int ReadArgs(int index, int argc, char** argv, std::vector<std::string>& output);

int main(int argc, char** argv)
{
   if(argc < 5) {
      std::cerr << "Usage: " << argv[0] << " -if <list of input files> -hn <list of histogram names> (optional: -x or -y for projection on x- or y-axis, otherwise 2D-histograms will get split along x-axis)" << std::endl;
      return 1;
   }

   std::vector<std::string> inputFiles;
   std::vector<std::string> histogramNames;
   bool                     projectX = false;
   bool                     projectY = false;

   // read all command line arguments
   for(int i = 0; i < argc; ++i) {
      if(argv[i][0] == '-') {
         if(strcmp(argv[i], "-if") == 0) {
            i = ReadArgs(i + 1, argc, argv, inputFiles);
         } else if(strcmp(argv[i], "-hn") == 0) {
            i = ReadArgs(i + 1, argc, argv, histogramNames);
         } else if(strcmp(argv[i], "-x") == 0) {
            projectX = true;
         } else if(strcmp(argv[i], "-y") == 0) {
            projectY = true;
         }
      }
   }

   // verify input variables are set
   if(inputFiles.empty()) {
      std::cerr << "Missing input files, please provide a list of them using the \"-if\" flag!" << std::endl;
      return 1;
   }

   if(histogramNames.empty()) {
      std::cerr << "Missing names of histograms, please provide a list of them using the \"-hn\" flag!" << std::endl;
      return 1;
   }

   if(projectX && projectY) {
      std::cerr << "Both projection on x-axis and projection on y-axis are enabled, please select only one!" << std::endl;
      return 1;
   }

   // open first root file and check which histograms are in it and whether they are 1D or 2D
   auto* input = new TFile(inputFiles[0].c_str());

   // determine output file name and open output file
   std::stringstream outputFileName;
   outputFileName << "plotVsRun";
   if(histogramNames.size() == 1) {
      outputFileName << "_" << histogramNames[0];
   } else {
      //TODO: find a way to determine the run numbers (maybe add all files to TRunInfo?)
   }
   outputFileName << ".root";
   auto* output = new TFile(outputFileName.str().c_str(), "recreate");

   TObject*           obj = nullptr;
   std::vector<TH2F*> outputHistograms;
   // since we might split one 2D histogram into multiple histograms, we need to keep track of the index
   // so that we can use outputIndex[histogram name index] to find the right output histogram
   std::vector<int> outputIndex;
   int              index         = 0;
   int              nofInputFiles = static_cast<int>(inputFiles.size());

   for(const auto& histogramName : histogramNames) {
      obj = input->Get(histogramName.c_str());
      if(obj == nullptr) {
         std::cerr << "Failed to find \"" << histogramName << "\" in file \"" << input->GetName() << "\"" << std::endl;
         return 1;
      }
      // first check if this is a 2D-histogram because every 2D-histogram inherits from TH1 as well!
      if(obj->InheritsFrom(TH2::Class())) {
         auto* hist = static_cast<TH2*>(obj);
         if(projectX) {
            auto* axis = hist->GetXaxis();
            outputHistograms.emplace_back(new TH2F(Form("%sPxVsFile", histogramName.c_str()), Form("Projection of %s on the x-axis vs. file #", histogramName.c_str()), nofInputFiles, 0.5, nofInputFiles + 0.5, axis->GetNbins(), axis->GetBinLowEdge(1), axis->GetBinLowEdge(axis->GetNbins() + 1)));
            outputIndex.push_back(index++);
         } else if(projectY) {
            auto* axis = hist->GetYaxis();
            outputHistograms.emplace_back(new TH2F(Form("%sPyVsFile", histogramName.c_str()), Form("Projection of %s on the y-axis vs. file #", histogramName.c_str()), nofInputFiles, 0.5, nofInputFiles + 0.5, axis->GetNbins(), axis->GetBinLowEdge(1), axis->GetBinLowEdge(axis->GetNbins() + 1)));
            outputIndex.push_back(index++);
         } else {
            auto* xAxis = hist->GetXaxis();
            auto* yAxis = hist->GetYaxis();
            outputIndex.push_back(index);
            for(int bin = 1; bin <= xAxis->GetNbins(); ++bin) {
               outputHistograms.emplace_back(new TH2F(Form("%s_%dVsFile", histogramName.c_str(), bin), Form("Bin %d, center %.0f of %s vs. file #", bin, xAxis->GetBinCenter(bin), histogramName.c_str()), nofInputFiles, 0.5, nofInputFiles + 0.5, yAxis->GetNbins(), yAxis->GetBinLowEdge(1), yAxis->GetBinLowEdge(yAxis->GetNbins() + 1)));
               index++;
            }
         }
      } else if(obj->InheritsFrom(TH1::Class())) {
         auto* hist = static_cast<TH1*>(obj);
         auto* axis = hist->GetXaxis();   // not strictly needed, we could just use hist in its place
         outputHistograms.emplace_back(new TH2F(Form("%sVsFile", histogramName.c_str()), Form("%s vs. file #", histogramName.c_str()), nofInputFiles, 0.5, nofInputFiles + 0.5, axis->GetNbins(), axis->GetBinLowEdge(1), axis->GetBinLowEdge(axis->GetNbins() + 1)));
         outputIndex.push_back(index++);
      } else {
         std::cerr << "Found object \"" << histogramName << "\" in file \"" << input->GetName() << "\", but it's neither a TH1 nor a TH2, it's a " << obj->ClassName() << std::endl;
         return 1;
      }
   }

   // since we might have created multiple histograms from a single 2D histogram we can have more output histograms than input ones
   if(outputHistograms.size() < histogramNames.size()) {
      std::cerr << "Something went wrong, only found " << outputHistograms.size() << " histograms from " << histogramNames.size() << " histograms?" << std::endl;
      return 1;
   }

   TH1* hist = nullptr;   // could move this higher up and change the "hist" variable for the 2D case?

   // loop over all files and all histograms and copy the data to the output histograms
   // we need to know the file index, so no range for loop
   for(size_t i = 0; i < inputFiles.size(); ++i) {
      if(input == nullptr) {   // no need to re-open the first file, it's already open!
         input = new TFile(inputFiles[i].c_str());
      } else {   // if this is the first file change into it (we opened the output file last so we are currently in that file) so that we can read the right run info
         input->cd();
      }
      // get run info from this file and create a label for the bins from it
      auto label = TRunInfo::CreateLabel(true);
      std::cout << "working on " << i << "/" << inputFiles.size() - 1 << " = " << label << "\r" << std::flush;
      // we need to loop through the histogramNames and the outputHistograms at the same time
      for(size_t j = 0; j < histogramNames.size(); ++j) {
         obj = input->Get(histogramNames[j].c_str());
         if(obj == nullptr) {
            std::cerr << "Failed to find \"" << histogramNames[j] << "\" in file \"" << input->GetName() << "\"" << std::endl;
            continue;
         }
         if(obj->InheritsFrom(TH2::Class())) {
            if(projectX) {
               hist = static_cast<TH1*>(static_cast<TH2*>(obj)->ProjectionX());
            } else if(projectY) {
               hist = static_cast<TH1*>(static_cast<TH2*>(obj)->ProjectionY());
            } else {
               auto* xAxis = static_cast<TH2*>(obj)->GetXaxis();
               for(int xBin = 1; xBin <= xAxis->GetNbins(); ++xBin) {
                  hist = static_cast<TH1*>(static_cast<TH2*>(obj)->ProjectionY(Form("%s_py%d", histogramNames[j].c_str(), xBin), xBin, xBin));
                  for(int yBin = 0; yBin <= hist->GetNbinsX() + 1; ++yBin) {
                     outputHistograms[outputIndex[j] + xBin - 1]->SetBinContent(i + 1, yBin, hist->GetBinContent(yBin));
                  }
                  outputHistograms[outputIndex[j] + xBin - 1]->GetXaxis()->SetBinLabel(i + 1, label.c_str());
               }
               continue;
            }
         } else if(obj->InheritsFrom(TH1::Class())) {
            hist = static_cast<TH1*>(obj);
         } else {
            std::cerr << "Found object \"" << histogramNames[i] << "\" in file \"" << input->GetName() << "\", but it's neither a TH1 nor a TH2, it's a " << obj->ClassName() << std::endl;
            continue;
         }
         // at this point hist is either set or we continued to the next file
         for(int bin = 0; bin <= hist->GetNbinsX() + 1; ++bin) {
            outputHistograms[outputIndex[j]]->SetBinContent(i + 1, bin, hist->GetBinContent(bin));
         }
         outputHistograms[outputIndex[j]]->GetXaxis()->SetBinLabel(i + 1, label.c_str());
      }
      input->Close();
      input = nullptr;
   }
   std::cout << "writing generated histograms to file " << output->GetName() << std::endl;

   output->cd();
   for(auto* outputHistogram : outputHistograms) {
      outputHistogram->Write("", TObject::kOverwrite);
   }

   output->Close();

   return 0;
}

int ReadArgs(int index, int argc, char** argv, std::vector<std::string>& output)
{
   /// This function reads arguments starting from <index> up to argc until it encounters one with leading '-'.
   /// Returns index of last argument pushed back onto the vector (which is argc-1 if no '-' is encountered).
   for(int i = index; i < argc; ++i) {
      if(argv[i][0] == '-') { return i - 1; }
      output.emplace_back(argv[i]);
   }
   return argc - 1;
}
