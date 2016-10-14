// Author: Benjamin Fenker -- 2014 --

// C++ libraries
#include <iostream>
// C libraries
#include <stdio.h>
#include <stdlib.h>

#include <TApplication.h>
#include <TBox.h>
#include <TFile.h>
#include <TH1.h>
#include <TLatex.h>
#include <TPaveStats.h>


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

using namespace std;

void help() {
  cout << " ***** convertTH1 *****" << endl;
  cout << " You must provide two input parameters: " << endl;
  cout << "     1) Name of file containing histogram to convert" << endl;
  cout << "     2) Name of histogram to convert" << endl;
  cout << "  Output is a text file containing two columns: " << endl;
  cout << "     1) Center of X-bin" << endl;
  cout << "     2) Value of bin" << endl;
  cout << " **********************" << endl;

  exit(0);			// normal
}

int main (int argc, char *argv[]) {
  if (argc < 3) help();


  char *file_name = argv[1];
  char *hist_name = argv[2];

  TApplication app("App", &argc, argv);

  // cout << file_name << endl;
  // cout << hist_name << endl;

  TFile file(file_name, "READ");
  if (file.IsZombie()) {
    cout << "Error opening file.  " << file_name << cout;
    cout << "Perhaps file not there or not closed correctly" << endl;
    return (1);
  }
  TH1 *hist;
  //  file.ls();
  file.GetObject(hist_name, hist);
  if (!hist) {
    cout << "Sorry, histogram " << hist_name << " not found in file" << endl;
    return (1);
  }

  char out_file_name[500];
  snprintf(out_file_name, sizeof(out_file_name), "%s.hist", hist->GetName());
  FILE *out_file;
  out_file = fopen(out_file_name, "w");
  cout << "Making output file: " << out_file_name << endl;
  fprintf(out_file, "%s\t%s\t%d\n",
   	  file_name, hist_name, hist -> GetNbinsX());
  for (int i = 1; i <= hist -> GetNbinsX(); i++) {
    fprintf(out_file, "%g\t%g\n",
	    hist -> GetBinCenter(i), hist -> GetBinContent(i));
  }
  fclose(out_file);
  cout << "Output complete" << endl;
  return 0;
}

