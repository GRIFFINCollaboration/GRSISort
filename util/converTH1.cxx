// Author: Benjamin Fenker -- 2014 --

// C++ libraries
#include <iostream>
#include <fstream>
// C libraries
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "TApplication.h"
#include "TFile.h"
#include "TH1.h"

// ****** Compilation command *************************************************
// g++ -o convertTH1 `root-config --cflags` convertTH1.cpp `root-config --glibs`
// ****************************************************************************

// Usage  : ./convertTH1 rootFile.root histName

// Output : ASCII file with header and data.  Header contains ROOT
// file, histogram name, number of bins
// Following the header is the data in two columns for bin center and
// bin content.  All bins are included (including bins with zero
// counts).  Overflow and underflow bins are not included.

// If histogram is in directory, include the entire directory tree in
// the hist name.  For example "QDC/QDC_LowerPMT"

void help()
{
   std::cout << " ***** convertTH1 *****" << std::endl;
   std::cout << " You must provide two input parameters: " << std::endl;
   std::cout << "     1) Name of file containing histogram to convert" << std::endl;
   std::cout << "     2) Name of histogram to convert" << std::endl;
   std::cout << "  Output is a text file containing two columns: " << std::endl;
   std::cout << "     1) Center of X-bin" << std::endl;
   std::cout << "     2) Value of bin" << std::endl;
   std::cout << " **********************" << std::endl;

   exit(0);   // normal
}

int main(int argc, char* argv[])
{
   if(argc < 3) {
      help();
   }

   char* file_name = argv[1];
   char* hist_name = argv[2];

   TApplication app("App", &argc, argv);

   TFile file(file_name, "READ");
   if(file.IsZombie()) {
      std::cout << "Error opening file.  " << file_name << std::endl;
      std::cout << "Perhaps file not there or not closed correctly" << std::endl;
      return 1;
   }
   TH1* hist = nullptr;
   file.GetObject(hist_name, hist);
   if(hist == nullptr) {
      std::cout << "Sorry, histogram " << hist_name << " not found in file" << std::endl;
      return 1;
   }
   if(hist->GetDimension() != 1) {
      std::cout << "Sorry, histogram " << hist_name << " is not a 1D histogram but a " << hist->GetDimension() << "D histogram!" << std::endl;
      return 1;
   }

   std::stringstream str;
   str << hist->GetName() << ".hist";
   std::ofstream out_file(str.str().c_str());
   std::cout << "Making output file: " << str.str() << std::endl;

   out_file << file_name << "\t" << hist_name << "\t" << hist->GetNbinsX() << "\n";
   for(int i = 1; i <= hist->GetNbinsX(); i++) {
      out_file << hist->GetBinCenter(i) << "\t" << hist->GetBinContent(i) << "\n";
   }
   out_file.close();

   std::cout << "Output complete" << std::endl;

   return 0;
}
