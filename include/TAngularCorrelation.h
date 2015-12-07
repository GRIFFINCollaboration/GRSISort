#ifndef TANGULARCORRELATION_H
#define TANGULARCORRELATION_H

/** \addtogroup Fitting & Analysis
 *  @{
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"
#include "TGraphAsymmErrors.h"
#include "TPeak.h"

class TAngularCorrelation : public TObject {
   private:
      TH2D* f2DSlice; // a 2D histogram of angular index vs. gamma energy
      TH1D* fIndexCorrelations; // a 1D plot of counts vs. angular index
      std::vector<TPeak> fPeaks; // array of TPeaks used to create fIndexCorrelations
      static Int_t** fIndexMap; // a map correlating array number pairs with angular index
      static std::vector<Double_t> fAngleMap; // a array correlating angular index with opening angle
      static std::vector<Int_t> fWeights; // an array correlating angular index with weight (number of detector pairs at that index)
      //TODO: Figure out how to do the group map
      //static map fGroupMap; // a map specifying how grouping will occur

   public:
      virtual ~TAngularCorrelation();
      TAngularCorrelation();

      // creates a 2D index vs. energy spectrum by slicing a THnSparse, with options for folding and grouping
      TH2D* Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t folded, Bool_t grouping);
      // this will fit a TH2F from CreateSlice, similar to FitSlicesY (probably utilizing that). It returns a histogram of area vs. angular index. It will program the fPeaks array as well.
      TH1D* FitSlices(TH2* hst,TPeak* peak);
      // this will take the Int_tegral of the area in-between min and max for each angular index bin
      TH1D* IntegralSlices(TH2* hst, Double_t min, Double_t max);
      // this will convert a histogram with x-axis of angular index to a graph with an x-axis of angle
      TGraphAsymmErrors* CreateGraphFromHst(TH1* hst);

      // map functions
      Bool_t CheckMaps(); // checks to make sure fIndexMap, fAngleMap, and fWeights are consistent 
      void PrintIndexMap(); // print the map
      void PrintAngleMap(); // print the map
      Int_t GetAngularIndex(Int_t arraynum1, Int_t arraynum2); // returns the angular index for a pair of detectors
      Double_t GetAngleFromIndex(Int_t index) { return fAngleMap[index]; } // returns the opening angle for a specific angular index
      Double_t GetWeightFromIndex(Int_t index) { return fWeights[index]; } // returns the weight for a specific angular index
      Int_t SetAngleMap(Double_t* angles); // sets the angles in the map, with an array of angles, where the angular index is determined by the index of the array element
      Int_t GenerateWeights(Int_t* detectors); // with input of array number array (crystals that were present in data collection), generates the weights for each angular index (no input generates weights for 16 detectors)
      Int_t SetWeights(Int_t* weights); // input is weight array itself
      Int_t GenerateIndexMaps(Int_t distance); // not sure what kind of input we need for something other than the default
      Int_t GenerateGroupingMap(); // not sure what this function would actually do
   
/// \cond CLASSIMP
   ClassDef(TAngularCorrelation,0)
/// \endcond
};
/*! @} */
#endif 
