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
      TH2D* f2DSlice; /// 2D histogram of angular index vs. gamma energy
      TH1D* fIndexCorrelation; /// 1D plot of counts vs. angular index
      std::vector<TPeak*> fPeaks; /// array of TPeaks used to create fIndexCorrelations
      Int_t** fIndexMap; /// 2D array correlating array number pairs with angular index
      std::vector<Double_t> fAngleMap; /// array correlating angular index with opening angle
      std::vector<Int_t> fWeights; /// array correlating angular index with weight (number of detector pairs at that index)
      //TODO: Figure out how to do the group map
      //static map fGroupMap; // a map specifying how grouping will occur

   public:
      virtual ~TAngularCorrelation();
      TAngularCorrelation();

      // getters
      TH2D* Get2DSlice() { return f2DSlice; }
      TH1D* GetIndexCorrelation() { return fIndexCorrelation; }
      TPeak* GetPeak(Int_t index) { return fPeaks[index]; }
      Int_t GetAngularIndex(Int_t arraynum1, Int_t arraynum2) { return fIndexMap[arraynum1][arraynum2]; } // returns the angular index for a pair of detectors
      Double_t GetAngleFromIndex(Int_t index) { return fAngleMap[index]; } // returns the opening angle for a specific angular index
      Double_t GetWeightFromIndex(Int_t index) { return fWeights[index]; } // returns the weight for a specific angular index

      // simple setters
      void Set2DSlice(TH2D* hst) { f2DSlice = hst; }
      void SetIndexCorrelation(TH1D* hst) { fIndexCorrelation = fIndexCorrelation; }
      void SetPeak(Int_t index,TPeak* peak) { fPeaks[index] = peak; }

      TH2D* Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t folded, Bool_t grouping);
      TH1D* IntegralSlices(TH2* hst, Double_t min, Double_t max);
      TH1D* FitSlices(TH2* hst,TPeak* peak);
      TGraphAsymmErrors* CreateGraphFromHst(TH1* hst);
      TGraphAsymmErrors* CreateGraphFromHst() { return CreateGraphFromHst(fIndexCorrelation); }

      // map functions
      Bool_t CheckMaps(); // checks to make sure fIndexMap, fAngleMap, and fWeights are consistent 
      void PrintIndexMap(); // print the map
      void PrintAngleMap(); // print the map
      //Int_t SetAngleMap(Double_t* angles); // sets the angles in the map, with an array of angles, where the angular index is determined by the index of the array element
      //Int_t GenerateWeights(Int_t* detectors); // with input of array number array (crystals that were present in data collection), generates the weights for each angular index (no input generates weights for 16 detectors)
      //Int_t SetWeights(Int_t* weights); // input is weight array itself
      Int_t GenerateIndexMaps(Int_t distance); // not sure what kind of input we need for something other than the default
      //Int_t GenerateGroupingMap(); // not sure how this function is going to work
   
/// \cond CLASSIMP
   ClassDef(TAngularCorrelation,0)
/// \endcond
};
/*! @} */
#endif 
