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
      TH2D* fModSlice; //2D histogram that is grouped, folded, or grouped and folded
      TH1D* fIndexCorrelation; /// 1D plot of counts vs. angular index
      TH1D* fChi2; /// 1D plot of chi^2 vs. angular index
      TH1D* fCentroid; /// 1D plot of centroid vs. angular index
      TH1D* fFWHM; /// 1D plot of FWHM vs. angular index
      std::map<Int_t,TPeak*> fPeaks; /// array of TPeaks used to create fIndexCorrelations
      std::map<Int_t,TH1D*> f1DSlices; /// array of 1D histograms used to create fIndexCorrelations
      std::map<Int_t,std::map<Int_t,Int_t>> fIndexMap; /// 2D square array correlating array number pairs with angular index
      Int_t fNumIndices; /// number of angular indices
      Int_t fIndexMapSize; /// size of fIndexMap
      Int_t fGroupSize; /// size of Group indexes
      std::vector<Double_t> fAngleMap; /// array correlating angular index with opening angle
      std::vector<Int_t> fWeights; /// array correlating angular index with weight (number of detector pairs at that index)
      std::vector<Double_t> fFoldedAngles; /// array correlating Anglular Index with a Folded Index 
      std::vector<Int_t> fFoldedAngularIndexes; /// array correlating angular index with a folded index 
      std::vector<Int_t> fFoldedAngularWeights; /// array correlating Group Index with a Folded Index 
      std::vector<Int_t> fGroups; /// array correlating angular index with group assignment 
      std::vector<Int_t> fGroupWeights; /// array correlating group assignment with weight 
      std::vector<Double_t> fGroupAngles; /// array correlating group assignment with their average angles
      std::vector<Double_t> fFoldedGroupAngles; /// array correlating Group Index with a Folded angle 
      std::vector<Int_t>fFoldedGroupIndexes; //array correlating group index with a folded index
      std::vector<Int_t> fFoldedGroupWeights; /// array correlating Group Index with a Folded Index 
      Bool_t fFolded; /// switch to indicate a folded correlation
      Bool_t fGrouped; /// switch to indicated a grouped correlation

   public:
      virtual ~TAngularCorrelation();
      TAngularCorrelation();

      // getters
      TH2D* Get2DSlice() { return f2DSlice; }
      TH1D* GetIndexCorrelation() { return fIndexCorrelation; }
      TH1D* GetChi2Hst() { return fChi2; }
      TH1D* GetCentroidHst() { return fCentroid; }
      TH1D* GetFWHMHst() { return fFWHM; }
      TPeak* GetPeak(Int_t index);
      TH1D* Get1DSlice(Int_t index) { return f1DSlices[index]; }
      Int_t GetAngularIndex(Int_t arraynum1, Int_t arraynum2); // returns the angular index for a pair of detectors
      //TODO: move the next function to implementation file and check if in range
      Double_t GetAngleFromIndex(Int_t index) { return fAngleMap[index]; } // returns the opening angle for a specific angular index
      //TODO: move the next function to implementation file and check if in range
      Double_t GetWeightFromIndex(Int_t index) { return fWeights[index]; } // returns the weight for a specific angular index
      Double_t GetGroupFromIndex(Int_t index) { return fGroups[index]; } // returns the assigned group for a specific angular index
      Double_t GetGroupWeightFromIndex(Int_t index) { return fGroupWeights[index]; }//returns the determined weight for each group
      Double_t GetGroupAngleFromIndex(Int_t index) { return fGroupAngles[index]; } // returns the average angle for each group
      Double_t GetFoldedAngleFromIndex(Int_t index) { return fFoldedAngles[index]; } // returns the folded angle value for each angular index
      Double_t GetFoldedAngularIndex(Int_t index) { return fFoldedAngularIndexes[index]; } // returns the folded index for each angular index
      Double_t GetFoldedAngleWeightFromIndex(Int_t index) { return fFoldedAngularWeights[index]; } //returns the weight for each folded angle
      Double_t GetFoldedGroupAngleFromIndex(Int_t index) { return fFoldedGroupAngles[index]; } // returns the average angle for each group 
      Double_t GetFoldedGroupIndex(Int_t index) { return fFoldedGroupIndexes[index]; } // returns the folded index value for each group index
      Double_t GetFoldedGroupWeightFromIndex(Int_t index) { return fFoldedGroupWeights[index]; } // returns the weight for each folded group
      Int_t GetWeightsSize() { return fWeights.size();}
      Int_t GetGroupWeightsSize() { return fGroupWeights.size();}
      Int_t GetFoldedAngularWeightsSize() {return fFoldedAngularWeights.size();}
      Int_t GetFoldedGroupWeightsSize() {return fFoldedGroupWeights.size();}
      // simple setters
      void Set2DSlice(TH2D* hst) { f2DSlice = hst; }
      void SetIndexCorrelation(TH1D* hst) { } //fIndexCorrelation = fIndexCorrelation; }
      //TODO: move the next function to implementation file and update fIndexCorrelation
      void SetPeak(Int_t index, TPeak* peak) { fPeaks[index] = peak; }
      void Set1DSlice(Int_t index, TH1D* slice) { f1DSlices[index] = slice; }

      // functions that do most of the work
      TH2D* Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t fold, Bool_t group);
      TH2D* Create2DSlice(TObjArray* hst, Double_t min, Double_t max, Bool_t fold, Bool_t group);
      TH2D* Modify2DSlice(TH2* hst, Bool_t fold, Bool_t group);
      TH1D* IntegralSlices(TH2* hst, Double_t min, Double_t max);
      TH1D* FitSlices(TH2* hst,TPeak* peak,Bool_t visualization);
      TH1D* DivideByWeights(TH1* hst, Bool_t fold, Bool_t group);
     // void DivideByWeights();
      void UpdatePeak(Int_t index,TPeak* peak);
      void ScaleSingleIndex(TH1* hst, Int_t index, Int_t factor);
      TGraphAsymmErrors* CreateGraphFromHst(TH1* hst, Bool_t fold, Bool_t group);
      TGraphAsymmErrors* CreateGraphFromHst() { return CreateGraphFromHst(fIndexCorrelation, kFALSE, kFALSE); }
      void UpdateIndexCorrelation();
      void UpdateDiagnostics();
      void DisplayDiagnostics(TCanvas* c_diag);

      // map functions
      Bool_t CheckMaps(Bool_t fold, Bool_t group); // checks to make sure fIndexMap, fAngleMap, and fWeights are consistent 
      void PrintIndexMap(); // print the map
      void PrintAngleMap(); // print the map
      void PrintWeights(); // print the map
      void PrintFoldedAngleMap(); // print the map
      void PrintFoldedAngularIndexes(); // print the map
      void PrintGroups(); // print the map
      void PrintGroupIndexes(); // print the map
      void PrintFoldedGroupAngleMap(); //print the map
      void PrintFoldedGroupIndexes(); // print the map
      
      //Int_t SetAngleMap(Double_t* angles); // sets the angles in the map, with an array of angles, where the angular index is determined by the index of the array element
      //Int_t SetWeights(Int_t* weights); // input is weight array itself
      static std::vector<Double_t> GenerateAngleMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances);
      static std::map<Int_t,std::map<Int_t,Int_t>> GenerateIndexMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::vector<Double_t> &anglemap);
      static std::vector<Int_t> GenerateWeights(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::map<Int_t,std::map<Int_t,Int_t>> &indexmap); // with input of array number array (crystals that were present in data collection), generates the weights for each angular index (no input generates weights for 16 detectors)
      static std::vector<Int_t> GenerateModifiedWeights(std::vector<Int_t> &index, std::vector<Int_t> &weights);
      Int_t GenerateGroupMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::vector<Int_t> &group, std::vector<Double_t> &groupangles);
      Int_t GenerateMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances);
      Int_t GenerateMaps(Int_t detectors, Int_t distance);
      static std::vector<Int_t> AssignGroups(std::vector<Int_t> &group, std::vector<Double_t> &anglemap);
      static std::vector<Int_t> GenerateFoldedIndexes(std::vector<Double_t> &folds, std::vector<Double_t> &anglemap); 
      static std::vector<Double_t>AssignGroupAngles(std::vector<Double_t> &groupangles, std::vector<Int_t> &groupweights);
      static std::vector<Double_t>GenerateFoldedAngles(std::vector<Double_t> &anglemap);
     
     
/// \cond CLASSIMP
   ClassDef(TAngularCorrelation,0)
/// \endcond
};
/*! @} */
#endif 
