/*
This script accompanies LeanComptonMatrices.c which is a derivative of the ComptonMatrices.c file.
It is called lean because I have taken out many of the portions that were simply for comparing different
methods.  As I have essentially settled on one method, these additional steps are just bloating.
Part of this fact means that I no longer do any weighting other that event mixing, and I also no longer
include the metzger Nparallel/Nperpendicular method of Compton polarimetry.  This script only includes
the portions that I believe would be useful for actual implementation of this code.  To use this you
should:
Set RUNNUMBER appropriately, as I do not have this information automatically pulled from input files.
Set the correct parameters within the PolarizationCalculation() function, either use an appropriate template one or create the missing one for your cascade.
Set things in Parameter Setup to what you want.
For appropriate theory plot set AGATA_Q to a known quality factor (preferrably one you recently measured)
Whatever scattering event was used to obtain AGATA_Q should be set as E_Q_Measure.  Q will then be scaled
appropriately to predict a scatter for an event of energy E_Q_Use, which must also be set.

Compile:
g++ LeanAnalyzeComptonMatrices.c -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries `grsi-config --cflags --all-libs` `root-config --cflags --libs` -lTreePlayer -lMathMore -lSpectrum -o MakeComptonPlots
Run:
Run with selected filename and no max entries:
./MakeComptonPlots /pathtomatrixfile/CompFile.root

This script is designed to create just the histograms required for producing the Compton polarimetry
plots.  These histograms can the be combined for multiple subruns using the hadd function.  The summed
file can be analysed using the AnalyzeComptonMatrices.c.

Because input files may have odd names, no dynamic output file names are given.  The output file
will be put in the directory specified when the outfile object is declared.  This names the output
based on the runnumber set in the definition section just below includes.

______________________________________________________________________________________________
*/

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <vector>
#include <algorithm>
#include <fstream>
#include <Globals.h>
#include <string>

#include "TMath.h"

#define RUNNUMBER 10577

#define AGATA_Q 0.2509     // The quality factor for creating a predicted Asymmetry plot of 0.5*Q*P*Cos(2*Xi), where                 \
                           // P is determined by PolarizationCalculation()  <- Parameters must be set internally for each transition \
                           // Dan measured 0.24492 with 12 clovers in Summer 2016.                                                   \
                           // Adam measured 0.2509 with 16 clovers in Dec 2017.
#define E_Q_Measure 1332   // The energy of the scattered gamma used to calculate AGATA_Q
#define E_Q_Use 1332       // The energy of the scattered gamma currently being examined.  Used for scaling Q
#define MaxE 3000          // Max energy for plot.
#define PI 3.14159

// Functions

double ScaleQ(double En1, double En2);

// Main
int main(int argc, char** argv)
{
   if(argc != 2) {
      printf("try again (usage: %s <matrix file>).\n", argv[0]);
      return 0;
   }

   for(int i = 100; i <= MaxE; i += 100) {
      std::cout << i << " " << AGATA_Q * ScaleQ(i, E_Q_Use) << std::endl;
   }

   return 0;
}

// ****************************************************************** //
// ********************* FUNCTION DEFINITIONS *********************** //
// ****************************************************************** //

double ScaleQ(double En1, double En2)
{
   // En1 is the energy of the scattering gamma used to determine the Q you wish to scale.  En2 is the
   // energy  of the gamma you are trying to examine now.
   int    nDiv = 1000;
   double m_eC2 = 510.9989;   // keV
   double Q1    = 0.;
   double Q2    = 0.;
   for(int loop = 0; loop < nDiv; loop++) {
      double theta = loop * (180. / nDiv);

      double E1p = En1 / (1. + En1 / m_eC2 * (1. - cos(TMath::DegToRad() * theta)));
      double E2p = En2 / (1. + En2 / m_eC2 * (1. - cos(TMath::DegToRad() * theta)));

      double sigma1 = TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2) / (E1p / En1 + En1 / E1p - TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2));
      double sigma2 = TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2) / (E2p / En2 + En2 / E2p - TMath::Power(TMath::Sin(TMath::DegToRad() * theta), 2));
      Q1 += (180. / nDiv) * (sigma1);
      Q2 += (180. / nDiv) * (sigma2);
   }
   Q1 /= 180.;
   Q2 /= 180.;

   return Q2 / Q1;
}
