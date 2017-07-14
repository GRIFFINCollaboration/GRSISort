
#include "Globals.h"

#include "TSRIM.h"

#include <TVector3.h>
#include <TMath.h>

#include <cmath>
#include <sstream>
#include <TStopwatch.h>

#include <cstdio>

/// \cond CLASSIMP
ClassImp(TSRIM)
   /// \endcond

   const double TSRIM::dx = 1.0; // um [sets accuracy of energy loss E vs X functions]

TSRIM::TSRIM()
{
   fEnergyLoss = nullptr;
}

TSRIM::TSRIM(const char* infilename, double emax, double emin, bool printfile)
{
   fEnergyLoss = nullptr;
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

   char        buf[256];
   std::string grsipath = getenv("GRSISYS");
   sprintf(buf, "%s/libraries/TGRSIAnalysis/SRIMData/%s", grsipath.c_str(), fname.c_str());
   if(printfile) {
      printf("\nSearching for %s..\n", buf);
   }

   infile.open(buf);
   if(!infile.good()) {
      printf("{TSRIM} Warning : Couldn't find the file '%s' ..\n", filename);
      return;
   }
   std::string line;
   std::string word;
   double      density_scale = 0.;
   double      temp;

   std::vector<double>      number_input, dEdX_temp;
   std::vector<std::string> string_input;

   while(std::getline(infile, line).good() ) {
      if(line.length() == 0u) {
         continue;
      }
      std::stringstream linestream(line);
      number_input.clear();
      string_input.clear();
      while((linestream >> word).good()) {
         std::stringstream ss(word);
         if( (ss >> temp).good() ) { // if it's a number
            number_input.push_back(temp);
         } else {
            string_input.push_back(ss.str());
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
         IonEnergy.push_back(number_input[0] * 1e-3); // convert eV to keV.
      } else if(string_input[0].compare(0, 3, "keV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         IonEnergy.push_back(number_input[0]); // already in keV.
      } else if(string_input[0].compare(0, 3, "MeV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         IonEnergy.push_back(number_input[0] * 1e3); // convert MeV to keV.
      } else if(string_input[0].compare(0, 3, "GeV") == 0 && string_input[1].compare(0, 1, "/") != 0) {
         IonEnergy.push_back(number_input[0] * 1e6); // convert GeV to keV.
      } else {
         continue;
      }

      dEdX_temp.push_back((number_input[1] + number_input[2]));
   }

   if(!dEdX_temp.empty()) {
      if(density_scale == 0.) {
         printf("WARNING: stopping power remains in original units, unable to find scale factor.\n");
         density_scale = 1.;
      }

      for(double i : dEdX_temp) {
         dEdX.push_back(i * density_scale);
      }

      fEnergyLoss = new TGraph(IonEnergy.size(), &IonEnergy[0], &dEdX[0]);
      fEnergyLoss->GetXaxis()->SetTitle("Energy (keV)");
      fEnergyLoss->GetYaxis()->SetTitle("dE/dx (keV/um)");
      sEnergyLoss = new TSpline3("dEdX_vs_E", fEnergyLoss);

      double dataEmax = TMath::MaxElement(IonEnergy.size(), &IonEnergy[0]);
      double dataEmin = TMath::MinElement(IonEnergy.size(), &IonEnergy[0]);

      if(emax == -1.0) {
         emax = dataEmax; // default to highest available energy in data table
      } else if(emax > dataEmax || emax < dataEmin) {
         printf("\n{TSRIM} WARNING: specified emax is out of range. Setting emax to default value (%.02f)\n", dataEmax);
         emax = dataEmax; // default to highest available energy in data table
      }
      if(emin == 0.0) {
         emin = dataEmin; // default to lowest available energy in data table
      } else if(emin < dataEmin || emin > dataEmax) {
         printf("\n{TSRIM} WARNING: specified emin is out of range. Setting emin to default value (%.02f)\n", dataEmin);
         emin = dataEmin; // default to lowest available energy in data table
      }
      if(emax < emin) {
         double emaxtemp = emax;
         emax            = emin;
         emin            = emaxtemp;
      }

      // Use linear multistep method (order 3) - Adams-Bashford method (explicit).
      double xtemp = 0, xstep = dx;
      double etemp = emax;
      double k[4]  = {0, 0, 0, 0};

      for(int i = 0; i < 3; i++) { // start by using euler step on first few points
         X.push_back(xtemp);
         E.push_back(etemp);
         k[i] = sEnergyLoss->Eval(etemp); // contains gradient at previous steps

         xtemp += xstep;
         etemp -= xstep * sEnergyLoss->Eval(etemp);
      }

      while(E.back() > 0) { // keep going until E goes negative
         X.push_back(xtemp);
         E.push_back(etemp);

         xtemp += xstep;
         k[3] = k[2];
         k[2] = k[1];
         k[1] = k[0];
         k[0] = sEnergyLoss->Eval(etemp);
         // extrapolate to new energy using weighted average of gradients at previous points
         etemp -= xstep * (55 / 24 * k[0] - 59 / 24 * k[1] + 37 / 24 * k[2] - 8 / 24 * k[3]);
      }
      // force the last element to be emin (linear interpolation, small error)
      X.back() =
         X[X.size() - 2] + (X.back() - X[X.size() - 2]) * (emin - E[E.size() - 2]) / (E.back() - E[E.size() - 2]);
      E.back() = emin;

      Emin = TMath::MinElement(E.size(), &E[0]);
      Emax = TMath::MaxElement(E.size(), &E[0]);
      Xmin = TMath::MinElement(X.size(), &X[0]);
      Xmax = TMath::MaxElement(X.size(), &X[0]);

      fXgetE = new TGraph(X.size(), &X[0], &E[0]);
      fXgetE->SetName("XgetE");
      fXgetE->SetTitle(filename);
      fXgetE->GetXaxis()->SetTitle("Distance (um)");
      fXgetE->GetYaxis()->SetTitle("Energy (keV)");
      sXgetE = new TSpline3(Form("%s_Xspline", filename), fXgetE);
      sXgetE->SetName(Form("%s_Xspline", filename));

      fEgetX = new TGraph(E.size());
      for(int x = E.size() - 1; x >= 0; x--) { // make sure x data is increasing
         fEgetX->SetPoint(E.size() - x - 1, E.at(x), X.at(x));
      }
      fEgetX->SetName("EgetX");
      fEgetX->SetTitle(filename);
      fEgetX->GetYaxis()->SetTitle("Distance (um)");
      fEgetX->GetXaxis()->SetTitle("Energy (keV)");
      sEgetX = new TSpline3(Form("%s_Espline", filename), fEgetX);
      sEgetX->SetName(Form("%s_Espline", filename));
   }

   if(printfile) {
      printf("\n\t%s file read in, %lu entries found.\n", fname.c_str(), dEdX.size());
      printf("[Energy loss range = %.03f - %.03f keV & total range = %.03f - %.03f um ]\n", Emax, Emin, Xmin, Xmax);
   }
}

double TSRIM::GetEnergy(double energy, double dist)
{

   double xbegin = sEgetX->Eval(energy);
   // printf("xbegin = %.02f \t energy = %.02f \t xend = %.02f \t Xmax = %.02f\n",xbegin,energy,xbegin+dist,Xmax);

   if(energy > Emax || xbegin + dist < Xmin) {
      printf("\n {TSRIM} WARNING: data is out of range. Results may be unpredictable.\n" DRED
             "\t\tenergy = %.03f keV \txbegin = %.02f um\t dist = %.02f um\t xend = %.02f um\n" DYELLOW
             "\t\tErange = [%.03f , %.03f] keV \t\t Xrange = [0 , %.1f] um\n" RESET_COLOR,
             energy, xbegin, dist, xbegin + dist, Emin, Emax, Xmax);
   } else if(xbegin > Xmax || xbegin + dist > Xmax) {
      return 0.0;
   }

   return sXgetE->Eval(xbegin + dist);
}

// THIS FUNCTION DOES A MORE ACCURATE ENERGY LOSS CALCULATION BASED ON SMALL EXTRAPOLATIONS
double TSRIM::GetAdjustedEnergy(double energy, double thickness, double stepsize)
{
   if(fEnergyLoss == nullptr) {
      printf("energy loss file has not yet been read in.\n");
      return 0.0;
   }

   double energy_temp = energy;
   double xstep       = stepsize,
          xtot =
             0.0; // MAKE XSTEP SMALLER FOR BETTER RESULTS. 1UM SHOULD BE FINE ... UNLESS YOU ARE AT THE BRAGG PEAK ??
   energy_temp -= fmod(thickness, xstep) * sEnergyLoss->Eval(energy_temp); // get rid of fractional distance
   xtot += fmod(thickness, xstep);

   if(thickness >= xstep) {

      while(xtot < thickness) {
         //				printf("energy_temp = %.02f\t DEDX = %.02f\n",energy_temp,fEnergyLoss->Eval(energy_temp,0,"S"));
         energy_temp -=
            xstep * sEnergyLoss->Eval(energy_temp); // update energy recursively so that it decreases each step
         xtot += xstep;
         if(energy_temp <= 0.0) {
            return 0.0; // if no energy is remaining then final energy is zero
         }
      }
   }

   return energy_temp;
}
