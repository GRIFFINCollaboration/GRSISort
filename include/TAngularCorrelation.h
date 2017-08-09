#ifndef TANGULARCORRELATION_H
#define TANGULARCORRELATION_H

/** \addtogroup Fitting Fitting & Analysis
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
   TH1D* fIndexCorrelation; /// 1D plot of counts vs. angular index

   // for diagnostics and re-fitting
   TH1D* fChi2{nullptr};              /// 1D plot of chi^2 vs. angular index
   TH1D* fCentroid{nullptr};          /// 1D plot of centroid vs. angular index
   TH1D* fFWHM{nullptr};              /// 1D plot of FWHM vs. angular index
   std::map<Int_t, TPeak*> fPeaks;    /// array of TPeaks used to create fIndexCorrelations
   std::map<Int_t, TH1D*>  f1DSlices; /// array of 1D histograms used to create fIndexCorrelations

   // mapping information
   std::map<Int_t, std::map<Int_t, Int_t>>
                         fIndexMap;     /// 2D square array correlating array number pairs with angular index
   Int_t                 fNumIndices{0};/// number of angular indices
   Int_t                 fIndexMapSize; /// size of fIndexMap
   std::vector<Double_t> fAngleMap;     /// array correlating angular index with opening angle
   std::vector<Int_t> fWeights; /// array correlating angular index with weight (number of detector pairs at that index)

   // folding and grouping information
   std::vector<Int_t>    fGroups;      /// array correlating angular index with group assignment
   std::vector<Double_t> fGroupAngles; /// array correlating group assignment with their average angles
   Bool_t                fFolded;      /// switch to indicate a folded correlation
   Bool_t                fGrouped;     /// switch to indicate a grouped correlation

   // modified indices information
   std::vector<Int_t>    fModifiedIndices; // array correlating angular index with modified index
   std::vector<Int_t>    fModifiedWeights; // array correlating modified index with weights
   std::vector<Double_t> fModifiedAngles;  // array correlating modified index with angles

public:
   ~TAngularCorrelation() override;
   TAngularCorrelation();

   //----------------- getters -----------------
   TH1D* GetIndexCorrelation() { return fIndexCorrelation; }

   // diagnostics and re-fitting
   TH1D*  GetChi2Hst() { return fChi2; }
   TH1D*  GetCentroidHst() { return fCentroid; }
   TH1D*  GetFWHMHst() { return fFWHM; }
   TPeak* GetPeak(Int_t index);
   TH1D* Get1DSlice(Int_t index) { return f1DSlices[index]; }

   // information
   Int_t GetAngularIndex(Int_t arraynum1, Int_t arraynum2); // returns the angular index for a pair of detectors
   // TODO: move the next function to implementation file and check if in range
   Double_t GetAngleFromIndex(Int_t index)
   {
      return fAngleMap[index];
   } // returns the opening angle for a specific angular index
   // TODO: move the next function to implementation file and check if in range
   Double_t GetWeightFromIndex(Int_t index)
   {
      return fWeights[index];
   } // returns the weight for a specific angular index
   Int_t GetGroupFromIndex(Int_t index)
   {
      return fGroups[index];
   } // returns the assigned group for a specific angular index
   Double_t GetGroupAngleFromIndex(Int_t gindex)
   {
      return fGroupAngles[gindex];
   } // returns the angle for each group index
   Int_t GetModifiedIndex(Int_t index)
   {
      return fModifiedIndices[index];
   } // returns the modified index from the angular index
   Int_t GetModifiedWeight(Int_t modindex)
   {
      return fModifiedWeights[modindex];
   } // returns in the weight from the modified index
   Double_t GetModifiedAngleFromIndex(Int_t modindex)
   {
      return fModifiedAngles[modindex];
   }                                                  // returns the angle from the modified index
   Int_t GetNumGroups();                              // returns the number of groups assigned
   Int_t GetNumModIndices();                          // returns the number of modified indices
   Int_t GetWeightsSize() { return fWeights.size(); } // returns in the size of the fWeights array

   //----------------- setters -----------------
   void SetIndexCorrelation(TH1D* hst) { fIndexCorrelation = hst; }

   // diagnostics and re-fitting
   // TODO: move the next function to implementation file and update fIndexCorrelation
   void SetPeak(Int_t index, TPeak* peak) { fPeaks[index] = peak; }
   void Set1DSlice(Int_t index, TH1D* slice) { f1DSlices[index] = slice; }

   //----------------- functions that do most of the work
   TH2D* Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t fold, Bool_t group);
   TH2D* Create2DSlice(TObjArray* hstarray, Double_t min, Double_t max, Bool_t fold, Bool_t group);
   TH2D* Modify2DSlice(TH2* hst, Bool_t fold, Bool_t group);
   TH1D* IntegralSlices(TH2* hst, Double_t min, Double_t max);
   TH1D* FitSlices(TH2* hst, TPeak* peak, Bool_t visualization);
   TH1D* DivideByWeights(TH1* hst, Bool_t fold, Bool_t group);
   TGraphAsymmErrors* CreateGraphFromHst(TH1* hst, Bool_t fold, Bool_t group);
   TGraphAsymmErrors* CreateGraphFromHst() { return CreateGraphFromHst(fIndexCorrelation, kFALSE, kFALSE); }

   //----------------- functions for diagnostics, re-fitting, modification of ACs
   void UpdatePeak(Int_t index, TPeak* peak);
   void ScaleSingleIndex(TH1* hst, Int_t index, Double_t factor);
   void UpdateIndexCorrelation();
   void UpdateDiagnostics();
   void DisplayDiagnostics(TCanvas* c_diag);

   //----------------- functions for checking and printing the mapping
   void   PrintIndexMap();           // print the map
   void   PrintAngleMap();           // print the map
   void   PrintWeights();            // print the map
   void   PrintGroupIndexMap();      // print the map
   void   PrintGroupAngleMap();      // print the group angle map
   void   PrintModifiedIndexMap();   // prints a map between angular and modified indices
   void   PrintModifiedAngleMap();   // prints a map of angles for the modified indices
   void   PrintModifiedWeights();    // prints a map of modified weights
   void   PrintModifiedConditions(); // prints the current folding and grouping conditions
   Bool_t CheckGroups(std::vector<Int_t>& group);
   Bool_t CheckGroupAngles(std::vector<Double_t>& groupangles);
   Bool_t CheckMaps(Bool_t fold, Bool_t group); // checks to make sure fIndexMap, fAngleMap, and fWeights are consistent
   Bool_t CheckModifiedHistogram(TH1* hst);     // checks to make sure histogram is consistent with current settings

   //----------------- functions for generating the mapping
   // original maps
   static std::vector<Double_t> GenerateAngleMap(std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances);
   static std::map<Int_t, std::map<Int_t, Int_t>> GenerateIndexMap(std::vector<Int_t>& arraynumbers,
                                                                   std::vector<Int_t>&    distances,
                                                                   std::vector<Double_t>& anglemap);
   static std::vector<Int_t> GenerateWeights(
      std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances,
      std::map<Int_t, std::map<Int_t, Int_t>>& indexmap); // with input of array number array (crystals that were
                                                          // present in data collection), generates the weights for each
                                                          // angular index (no input generates weights for 16 detectors)
   Int_t GenerateMaps(std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances);
   Int_t GenerateMaps(Int_t detectors, Int_t distance);

   // modified maps
   Int_t AssignGroupMaps(std::vector<Int_t>& group, std::vector<Double_t>& groupangles);
   Int_t GenerateGroupMaps(std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances, std::vector<Int_t>& group,
                           std::vector<Double_t>& groupangles);
   Int_t GenerateModifiedMaps(Bool_t fold, Bool_t group);
   std::vector<Int_t> GenerateModifiedIndices(Bool_t fold, Bool_t group);
   std::vector<Double_t> GenerateModifiedAngles(Bool_t fold, Bool_t group);
   std::vector<Int_t> GenerateModifiedWeights(std::vector<Int_t>& modindices, std::vector<Int_t>& weights);
   static std::vector<Int_t> GenerateFoldedIndices(std::vector<Double_t>& folds, std::vector<Double_t>& anglemap);
   static std::vector<Double_t> GenerateFoldedAngles(std::vector<Double_t>& anglemap);
   void ClearModifiedMaps();

   /// \cond CLASSIMP
   ClassDefOverride(TAngularCorrelation, 1)
   /// \endcond
};
/*! @} */
#endif
