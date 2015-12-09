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
      std::map<int,TPeak*> fPeaks; /// array of TPeaks used to create fIndexCorrelations
      Int_t** fIndexMap; /// 2D square array correlating array number pairs with angular index
      Int_t fNumIndices; // number of angular indices
      Int_t fIndexMapSize; /// size of fIndexMap
      std::vector<Double_t> fAngleMap; /// array correlating angular index with opening angle
      std::vector<Int_t> fWeights; /// array correlating angular index with weight (number of detector pairs at that index)

   public:
      virtual ~TAngularCorrelation();
      TAngularCorrelation();

      // getters
      TH2D* Get2DSlice() { return f2DSlice; }
      TH1D* GetIndexCorrelation() { return fIndexCorrelation; }
      TPeak* GetPeak(Int_t index) { return fPeaks[index]; }
      Int_t GetAngularIndex(Int_t arraynum1, Int_t arraynum2); // returns the angular index for a pair of detectors
      //TODO: move the next function to implementation file and check if in range
      Double_t GetAngleFromIndex(Int_t index) { return fAngleMap[index]; } // returns the opening angle for a specific angular index
      //TODO: move the next function to implementation file and check if in range
      Double_t GetWeightFromIndex(Int_t index) { return fWeights[index]; } // returns the weight for a specific angular index

      // simple setters
      void Set2DSlice(TH2D* hst) { f2DSlice = hst; }
      void SetIndexCorrelation(TH1D* hst) { fIndexCorrelation = fIndexCorrelation; }
      //TODO: move the next function to implementation file and update fIndexCorrelation
      void SetPeak(Int_t index, TPeak* peak) { fPeaks[index] = peak; }

      TH2D* Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t folded, Bool_t grouping);
      TH1D* IntegralSlices(TH2* hst, Double_t min, Double_t max);
      TH1D* FitSlices(TH2* hst,TPeak* peak,Bool_t visualization);
      TGraphAsymmErrors* CreateGraphFromHst(TH1* hst);
      TGraphAsymmErrors* CreateGraphFromHst() { return CreateGraphFromHst(fIndexCorrelation); }
      //void UpdateIndexCorrelation();

      // map functions
      Bool_t CheckMaps(); // checks to make sure fIndexMap, fAngleMap, and fWeights are consistent 
      void PrintIndexMap(); // print the map
      void PrintAngleMap(); // print the map
      void PrintWeights(); // print the map
      //Int_t SetAngleMap(Double_t* angles); // sets the angles in the map, with an array of angles, where the angular index is determined by the index of the array element
      //Int_t SetWeights(Int_t* weights); // input is weight array itself
      static std::vector<Double_t> GenerateAngleMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances);
      static Int_t** GenerateIndexMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::vector<Double_t> &anglemap);
      static std::vector<Int_t> GenerateWeights(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, Int_t** &indexmap); // with input of array number array (crystals that were present in data collection), generates the weights for each angular index (no input generates weights for 16 detectors)
      Int_t GenerateMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances);
      Int_t GenerateMaps(Int_t detectors, Int_t distance);
   
/// \cond CLASSIMP
   ClassDef(TAngularCorrelation,0)
/// \endcond
};
/*! @} */
#endif 
