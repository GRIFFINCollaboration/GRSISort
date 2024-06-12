#ifndef TSRIMMANAGER_H
#define TSRIMMANAGER_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <map>
#include <utility>

#include "TGraph.h"
#include "TSpline.h"
#include "TH1.h"
#include "TF1.h"

class TSRIM {
public:
   virtual ~TSRIM() = default;

   TSRIM();
   // looks like the emax-min inputs don't work right now for 94Sr and maybe other high mass nuclei
   TSRIM(const char* infilename, double emax = -1.0, double emin = 0.0, bool printfile = true);

private:
   std::vector<double> IonEnergy;
   std::vector<double> dEdX;
   TGraph*             fEnergyLoss;
   TGraph*             fEgetX{nullptr};
   TGraph*             fXgetE{nullptr};
   TSpline3*           sEnergyLoss{nullptr};
   TSpline3*           sEgetX{nullptr};
   TSpline3*           sXgetE{nullptr};
   std::vector<double> E;   // units of keV
   std::vector<double> X;   // units of um
   double              Emin{0.};
   double              Emax{0.};
   double              Xmin{0.};
   double              Xmax{0.};
   static const double dx;   // um [sets accuracy of energy loss E vs X functions]

   std::map<std::pair<double, double>, double> AdjustedEnergyMap;

public:
   void ReadEnergyLossFile(const char* filename, double emax = -1.0, double emin = 0.0, bool printfile = true);

   double GetAdjustedEnergy(double energy, double thickness, double stepsize = dx);
   double GetEnergyLost(double energy, double distance, double stepsize = dx)
   {
      return energy - GetAdjustedEnergy(energy, distance, stepsize);
   };

   double GetEnergy(double energy, double dist);
   double GetEnergyChange(double energy, double dist) { return GetEnergy(energy, dist) - energy; };

   double GetEmax() { return Emax; };
   double GetEmin() { return Emin; };
   double GetXmax() { return Xmax; };
   double GetXmin() { return Xmin; };

   TGraph*   GetEnergyLossGraph() { return fEnergyLoss; };
   TGraph*   GetEvsXGraph() { return fXgetE; };
   TGraph*   GetXvsEGraph() { return fEgetX; };
   TSpline3* GetEvsXSpline() { return sXgetE; };
   TSpline3* GetXvsESpline() { return sEgetX; };
   /// \cond CLASSIMP
   ClassDef(TSRIM, 0)
   /// \endcond
};
/*! @} */
#endif
