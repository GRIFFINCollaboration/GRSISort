//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#ifndef PACESAngularCorrelationEventSelector_h
#define PACESAngularCorrelationEventSelector_h

#include "TChain.h"
#include "TFile.h"

#include "TH1.h"
#include "TH2.h"
#include "THnSparse.h"
#include "TGraph.h"

// Header file for the classes stored in the TTree if any.
#include "TGriffin.h"
#include "TPaces.h"
#include "TGRSISelector.h"
#include "TGRSIRunInfo.h"

// Fixed size dimensions of array or collections stored in the TTree if any.
std::vector<std::pair<double, double>> PACESAngleCombinations(bool group_all);

class PACESAngularCorrelationEventSelector : public TGRSISelector {

 public :
   TGriffin    * fGrif;
   TPaces      * fPaces;

   std::vector<std::pair<double, double>> fPACESAngleCombinations;   
   std::map<double, double> fPACESAngleMap;

   //USER DEFS::
   //PACES numbering system begins at #1 for experiments circa October 2017 (after run 10097) and #0 preceding this time.
   int runRef = 10097;
   int maxanglebin;
   //Correct PACES non-linearities, x = measured energy (keV), y = energy residual (correction, keV), dp = number of data points
   int dp = 10;
   double x[5][10]={{50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.},
		    {50.,100.,200.,300.,400.,500.,600.,700.,800.,900.}
		    };
   double y[5][10]={{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},
		    {0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}
		    };
   //END USER DEFS.

   PACESAngularCorrelationEventSelector(TTree * /*tree*/ =0) : TGRSISelector(), fGrif(0), fPaces(0) {
      SetOutputPrefix("PACESAngularCorrelationEvent");

      fPACESAngleCombinations = PACESAngleCombinations(true);

      //The number of matrices to be made corresponds to the maximum angle bin (not the size of the map), 'maxanglebin'.
      for(int i = 0; i < static_cast<int>(fPACESAngleCombinations.size()); ++i) {
         if(fPACESAngleCombinations[i].second>0) maxanglebin = fPACESAngleCombinations[i].second;
         fPACESAngleMap.insert(std::make_pair(fPACESAngleCombinations[i].first, fPACESAngleCombinations[i].second));
      }
   }

   virtual ~PACESAngularCorrelationEventSelector() { }
   virtual Int_t   Version() const { return 2; }
   void CreateHistograms();
   void FillHistograms();
   void InitializeBranches(TTree *tree);

   ClassDef(PACESAngularCorrelationEventSelector,2);
};

#endif

#ifdef PACESAngularCorrelationEventSelector_cxx
void PACESAngularCorrelationEventSelector::InitializeBranches(TTree* tree) {
   if (!tree) return;
   tree->SetBranchAddress("TGriffin", &fGrif);
   tree->SetBranchAddress("TPaces", &fPaces);
}

#endif // #ifdef PACESAngularCorrelationEventSelector_cxx

//GRIFFIN-PACES angle combinations ::
//Position 13 in the GRIFFIN array is empty to allow space for PACES LN2 dewar.

std::vector<std::pair<double, double>> PACESAngleCombinations(bool group_all)
{
   double dist=110;
   double angleBin = 0.001;
   //ALL PACES detectors.
   const int nga = 31, wa=19;
   double groupA[nga][wa]={
			  {251, 180, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {319, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {183, 248, 157, 230, 18, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {90, 297, 17, 250, 318, 181, 302, 158, 85, 60, -1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {229, 182, 249, 316, 61, 225, 162, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
			  {89, 298, 317, 45, 301, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {156, 231, 86, 22, 19, 118, 16, 296, 91, 303, 313, 159, 119, 84, 228, -1,-1,-1,-1},
			  {312, 153, 226, 63, 224, 163, 161, 234, 62, 88, 299, -1,-1,-1,-1,-1,-1,-1,-1},
			  {245, 44, 300, 186, 87, 23, 46, 94, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {293, 21, 244, 152, 227, 187, 160, 235, 274, 117, -1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {81, 314, 116, 154, 254, 315, 233, 47, 133, 202, 95, -1,-1,-1,-1,-1,-1,-1,-1},
			  {270, 292, 20, 65, 134, 221, 201, 246, 166, 275, 185, 93, -1,-1,-1,-1,-1,-1,-1},
			  {125, 80, 13, 155, 294, 255, 2, 269, 232, 247, 66, 184, 41, 273, -1,-1,-1,-1,-1},
			  {82, 220, 26, 167, 253, 57, 92, 54, 70, 295, -1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {265, 14, 132, 124, 203, 271, 222, 149, 64, 1, 40, 272, 135, 238, 126, 165, 200, 12, 83},
			  {3, 27, 122, 252, 309, 268, 197, 67, 42, 58, 138, -1,-1,-1,-1,-1,-1,-1,-1},
			  {98, 53, 148, 71, 223, 264, 190, 289, 25, 56, 121, 164, 239, 310, 69, 189, 55, 266, -1},
			  {278, 127, 15, 109, 0, 196, 150, 139, 43, 237, 99, -1,-1,-1,-1,-1,-1,-1,-1},
			  {288, 24, 129, 123, 206, 217, 308, 59, 198, 279, 170, 52, 97, 68, 137, 267, -1,-1,-1},
			  {108, 128, 290, 151, 207, 236, 191, 120, 37, 277, 311, -1,-1,-1,-1,-1,-1,-1},
			  {110, 188, 30, 216, 258, 199, 136, 171, 77, 259, 96, -1,-1,-1,-1,-1,-1,-1,-1},
			  {76, 291, 218, 145, 130, 210, 169, 205, 36, 276, -1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {111, 31, 38, 102, 131, 285, 29, 204, 8, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {282, 9, 105, 7, 219, 144, 146, 209, 6, 211, 168, 257, 213, -1,-1,-1,-1,-1,-1},
			  {101, 78, 174, 256, 286, 106, 281, 33, 79, 34, 39, 103, 214, 173, -1,-1,-1,-1,-1},
			  {284, 75, 28, 260, 283, 104, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {147, 11, 74, 208, 4, 261, 192, 10, 143, 212, -1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {5, 100, 175, 287, 140, 263, 72, 195, 32, 280, 107, -1,-1,-1,-1,-1,-1,-1,-1},
			  {35, 215, 172, 193, 142, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {73, 262,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
			  {141, 194,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1} 
			  };
   //SINGLE PACES detector at 90 degrees.
   const int ngb = 20, wb=3;
   double groupB[ngb][wb]={{90,85,-1},{86,118,91},{119,84,88},{87,94,-1},{117,81,116},{95,65,93},{125,80,66},{82,92,70},{124,64,126},{83,122,67},{98,71,121},
                          {69,127,109},{99,123,97},{68,108,120},{110,77,96},{111,102,105},{101,78,106},{79,103,75},{104,74,-1},{100,72,107}};
   std::vector<std::pair<double, double>> paces_hpge_angle_pair;
   std::vector<std::pair<double, double>> result;

   //Index between 0-319 identifies PACES-HPGe crystal combination.
   for (int firstDet = 1; firstDet <= 16; ++firstDet){
      if(firstDet==13) continue;
      for (int firstCry = 0; firstCry < 4; ++firstCry){
	 for (int secondCry = 0; secondCry < 5; ++secondCry){
	 //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	    int index = (64*secondCry)+(((firstDet-1)*4)+firstCry);
	    double ge_angle = TGriffin::GetPosition(firstDet, firstCry, dist).Angle(TPaces::GetPosition(secondCry))*180./TMath::Pi(); 
	    paces_hpge_angle_pair.push_back(std::make_pair(ge_angle, index));
	 }
      }
   }

   // GROUPING :: Two options are provided. Detector combinations are grouped into pre-determined bins.
   // If 'group_all' is false, grouping of GRIFFIN-PACES angles is restricted to a SINGLE PACES detector (@phi=90 degrees). If true, grouping is performed for all PACES detectors. 
   // For new or modified angle combinations (e.g. to adjust relative statistical weighting), run "GRIFFIN_PACES_AngleCalc.C" to find an appropriate grouping.

   if(group_all){
      for(int i=0;i<int(nga);++i){
         for(int j=0;j<int(wa);++j){
	    if(groupA[i][j]==-1) continue;   
	    result.push_back(std::make_pair(groupA[i][j], double(i)));
         }
      }   
   return result;

   } else if(!group_all){
      for(int i=0;i<int(ngb);++i){
         for(int j=0;j<int(wb);++j){
	    if(groupB[i][j]==-1) continue;   
	    result.push_back(std::make_pair(groupB[i][j], double(i)));
         }
      }   
   return result;
   }
}
