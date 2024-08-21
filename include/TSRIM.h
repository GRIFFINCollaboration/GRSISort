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
   TSRIM() = default;
   // looks like the emax-min inputs don't work right now for 94Sr and maybe other high mass nuclei
   explicit TSRIM(const char* infilename, double emax = -1.0, double emin = 0.0, bool printfile = true);
   TSRIM(const TSRIM&)                = default;
   TSRIM(TSRIM&&) noexcept            = default;
   TSRIM& operator=(const TSRIM&)     = default;
   TSRIM& operator=(TSRIM&&) noexcept = default;

   virtual ~TSRIM() = default;

private:
   std::vector<double> fIonEnergy;
   std::vector<double> fdEdX;
   TGraph*             fEnergyLoss{nullptr};
   TGraph*             fEgetX{nullptr};
   TGraph*             fXgetE{nullptr};
   TSpline3*           fsEnergyLoss{nullptr};
   TSpline3*           fsEgetX{nullptr};
   TSpline3*           fsXgetE{nullptr};
   std::vector<double> fE;   // units of keV
   std::vector<double> fX;   // units of um
   double              fEmin{0.};
   double              fEmax{0.};
   double              fXmin{0.};
   double              fXmax{0.};
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

   double GetEmax() const { return fEmax; };
   double GetEmin() const { return fEmin; };
   double GetXmax() const { return fXmax; };
   double GetXmin() const { return fXmin; };

   TGraph*   GetEnergyLossGraph() const { return fEnergyLoss; };
   TGraph*   GetEvsXGraph() const { return fXgetE; };
   TGraph*   GetXvsEGraph() const { return fEgetX; };
   TSpline3* GetEvsXSpline() const { return fsXgetE; };
   TSpline3* GetXvsESpline() const { return fsEgetX; };

   /// \cond CLASSIMP
   ClassDef(TSRIM, 0)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
