#include "TSRIM.h"

#include "Globals.h"
#include "TGRSIUtilities.h"

#include <TVector3.h>
#include <TMath.h>

#include <cmath>
#include <sstream>
#include <TStopwatch.h>

#include <cstdio>

const double TSRIM::dx = 1.0;   // um [sets accuracy of energy loss E vs X functions]

TSRIM::TSRIM(const char* infilename, double emax, double emin, bool printfile)
{
   ReadEnergyLossFile(infilename, emax, emin, printfile);
}

void TSRIM::ReadEnergyLossFile(const char* filename, double emax, double emin, bool printfile)
{
   // if Steffen makes a TSRIM file that this thing can't read it's beacuse of mac encoding.
   //		use dos2unix -c mac d_in_Si.txt in terminal
   std::ifstream infile;

   std::string fname = filename;
   if(fname.find(".txt") == std::string::npos) {
      fname.append(".txt");
   }

   std::string        grsipath = getenv("GRSISYS");
   std::ostringstream ostr;
   ostr << grsipath << "/libraries/TAnalysis/SRIMData/";
   if(!DirectoryExists(ostr.str().c_str())) {
      ostr.str(std::string());
      ostr << grsipath << "/SRIMData/";
   }
   if(!DirectoryExists(ostr.str().c_str())) {
      std::cout << std::endl
                << "Failed to find directory with SRIM data, tried \"" << grsipath << "/libraries/TAnalysis/SRIMData/\" and \"" << grsipath << "/SRIMData/\"!" << std::endl;
   }
   ostr << fname;
   if(printfile) {
      std::cout << std::endl
                << "Searching for " << ostr.str() << "..." << std::endl;
   }

   infile.open(ostr.str().c_str());
   if(!infile.good()) {
      std::cout << "{TSRIM} Warning : Couldn't find the file '" << filename << "' ..." << std::endl;
      return;
   }
   std::string line;
   std::string word;
   double      density_scale = 0.;
   double      temp          = 0.;

   std::vector<double>      number_input;
   std::vector<double>      dEdX_temp;
   std::vector<std::string> string_input;

   while(!std::getline(infile, line).fail()) {
      if(line.length() == 0u) {
         continue;
      }
      std::istringstream linestream(line);
      number_input.clear();
      string_input.clear();
      while(!(linestream >> word).fail()) {
         std::istringstream str(word);
         if(!(str >> temp).fail()) {   // if it's a number
            number_input.push_back(temp);
         } else {
            string_input.push_back(str.str());
         }
      }

      if((string_input[0].compare(0, 3, "keV") == 0) && (string_input[1].compare(0, 1, "/") == 0) &&
         (string_input[2].compare(0, 6, "micron") == 0)) {
         density_scale = number_input[0];
         //			cout<<"dEdX will be scaled by "<<density_scale<<" so that stopping power is in keV/um \n";
      } else if(number_input.size() != 6) {
         continue;
      }

      if(string_input[0].compare(0, 3, "eV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         fIonEnergy.push_back(number_input[0] * 1e-3);   // convert eV to keV.
      } else if(string_input[0].compare(0, 3, "keV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         fIonEnergy.push_back(number_input[0]);   // already in keV.
      } else if(string_input[0].compare(0, 3, "MeV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         fIonEnergy.push_back(number_input[0] * 1e3);   // convert MeV to keV.
      } else if(string_input[0].compare(0, 3, "GeV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         fIonEnergy.push_back(number_input[0] * 1e6);   // convert GeV to keV.
      } else {
         continue;
      }

      dEdX_temp.push_back((number_input[1] + number_input[2]));
   }

   if(!dEdX_temp.empty()) {
      if(density_scale == 0.) {
         std::cout << "WARNING: stopping power remains in original units, unable to find scale factor." << std::endl;
         density_scale = 1.;
      }

      for(double index : dEdX_temp) {
         fdEdX.push_back(index * density_scale);
      }

      fEnergyLoss = new TGraph(static_cast<Int_t>(fIonEnergy.size()), fIonEnergy.data(), fdEdX.data());
      fEnergyLoss->GetXaxis()->SetTitle("Energy (keV)");
      fEnergyLoss->GetYaxis()->SetTitle("dE/dx (keV/um)");
      fsEnergyLoss = new TSpline3("dEdX_vs_E", fEnergyLoss);

      double dataEmax = TMath::MaxElement(static_cast<Int_t>(fIonEnergy.size()), fIonEnergy.data());
      double dataEmin = TMath::MinElement(static_cast<Int_t>(fIonEnergy.size()), fIonEnergy.data());

      if(emax == -1.0) {
         emax = dataEmax;   // default to highest available energy in data table
      } else if(emax > dataEmax || emax < dataEmin) {
         std::cout << std::endl
                   << "{TSRIM} WARNING: specified emax is out of range. Setting emax to default value (" << dataEmax << ")" << std::endl;
         emax = dataEmax;   // default to highest available energy in data table
      }
      if(emin == 0.0) {
         emin = dataEmin;   // default to lowest available energy in data table
      } else if(emin < dataEmin || emin > dataEmax) {
         std::cout << std::endl
                   << "{TSRIM} WARNING: specified emin is out of range. Setting emin to default value (" << dataEmin << ")" << std::endl;
         emin = dataEmin;   // default to lowest available energy in data table
      }
      if(emax < emin) {
         double emaxtemp = emax;
         emax            = emin;
         emin            = emaxtemp;
      }

      // Use linear multistep method (order 3) - Adams-Bashford method (explicit).
      double                xtemp = 0;
      double                xstep = dx;
      double                etemp = emax;
      std::array<double, 4> k     = {0, 0, 0, 0};

      for(int i = 0; i < 3; i++) {   // start by using euler step on first few points
         fX.push_back(xtemp);
         fE.push_back(etemp);
         k[i] = fsEnergyLoss->Eval(etemp);   // contains gradient at previous steps

         xtemp += xstep;
         etemp -= xstep * fsEnergyLoss->Eval(etemp);
      }

      while(fE.back() > 0) {   // keep going until E goes negative
         fX.push_back(xtemp);
         fE.push_back(etemp);

         xtemp += xstep;
         k[3] = k[2];
         k[2] = k[1];
         k[1] = k[0];
         k[0] = fsEnergyLoss->Eval(etemp);
         // extrapolate to new energy using weighted average of gradients at previous points
         etemp -= xstep * (55 / 24 * k[0] - 59 / 24 * k[1] + 37 / 24 * k[2] - 8 / 24 * k[3]);
      }
      // force the last element to be emin (linear interpolation, small error)
      fX.back() = fX[fX.size() - 2] + (fX.back() - fX[fX.size() - 2]) * (emin - fE[fE.size() - 2]) / (fE.back() - fE[fE.size() - 2]);
      fE.back() = emin;

      fEmin = TMath::MinElement(static_cast<Long64_t>(fE.size()), fE.data());
      fEmax = TMath::MaxElement(static_cast<Long64_t>(fE.size()), fE.data());
      fXmin = TMath::MinElement(static_cast<Long64_t>(fX.size()), fX.data());
      fXmax = TMath::MaxElement(static_cast<Long64_t>(fX.size()), fX.data());

      fXgetE = new TGraph(static_cast<Int_t>(fX.size()), fX.data(), fE.data());
      fXgetE->SetName("XgetE");
      fXgetE->SetTitle(filename);
      fXgetE->GetXaxis()->SetTitle("Distance (um)");
      fXgetE->GetYaxis()->SetTitle("Energy (keV)");
      fsXgetE = new TSpline3(Form("%s_Xspline", filename), fXgetE);
      fsXgetE->SetName(Form("%s_Xspline", filename));

      fEgetX = new TGraph(static_cast<Int_t>(fE.size()));
      for(int x = fE.size() - 1; x >= 0; x--) {   // make sure x data is increasing
         fEgetX->SetPoint(fE.size() - x - 1, fE.at(x), fX.at(x));
      }
      fEgetX->SetName("EgetX");
      fEgetX->SetTitle(filename);
      fEgetX->GetYaxis()->SetTitle("Distance (um)");
      fEgetX->GetXaxis()->SetTitle("Energy (keV)");
      fsEgetX = new TSpline3(Form("%s_Espline", filename), fEgetX);
      fsEgetX->SetName(Form("%s_Espline", filename));
   }

   if(printfile) {
      std::cout << std::endl
                << "\t" << fname << " file read in, " << fdEdX.size() << " entries found." << std::endl;
      std::cout << "[Energy loss range = " << fEmax << " - " << fEmin << " keV & total range = " << fXmin << " - " << fXmax << " um ]" << std::endl;
   }
}

double TSRIM::GetEnergy(double energy, double dist)
{
   double xbegin = fsEgetX->Eval(energy);

   if(energy > fEmax || xbegin + dist < fXmin) {
      std::cout << std::endl
                << " {TSRIM} WARNING: data is out of range. Results may be unpredictable." << std::endl
                << DRED "\t\tenergy = " << energy << " keV \txbegin = " << xbegin << " um\t dist = " << dist << " um\t xend = " << xbegin + dist << " um" << std::endl
                << DYELLOW << "\t\tErange = [" << fEmin << ", " << fEmax << "] keV \t\t Xrange = [0 , " << fXmax << " um" << RESET_COLOR << std::endl;
   } else if(xbegin > fXmax || xbegin + dist > fXmax) {
      return 0.0;
   }

   return fsXgetE->Eval(xbegin + dist);
}

// THIS FUNCTION DOES A MORE ACCURATE ENERGY LOSS CALCULATION BASED ON SMALL EXTRAPOLATIONS
double TSRIM::GetAdjustedEnergy(double energy, double thickness, double stepsize)
{
   if(fEnergyLoss == nullptr) {
      std::cout << "energy loss file has not yet been read in." << std::endl;
      return 0.0;
   }

   double energy_temp = energy;
   double xstep       = stepsize;
   double xtot        = 0.0;                                                  // MAKE XSTEP SMALLER FOR BETTER RESULTS. 1UM SHOULD BE FINE ... UNLESS YOU ARE AT THE BRAGG PEAK ??
   energy_temp -= fmod(thickness, xstep) * fsEnergyLoss->Eval(energy_temp);   // get rid of fractional distance
   xtot += fmod(thickness, xstep);

   if(thickness >= xstep) {
      while(xtot < thickness) {
         energy_temp -= xstep * fsEnergyLoss->Eval(energy_temp);   // update energy recursively so that it decreases each step
         xtot += xstep;
         if(energy_temp <= 0.0) {
            return 0.0;   // if no energy is remaining then final energy is zero
         }
      }
   }

   return energy_temp;
}
