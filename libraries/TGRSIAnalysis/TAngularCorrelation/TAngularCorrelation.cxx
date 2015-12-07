/** \class TAngularCorrelation
 * An angular correlation class
 **/
#include <TAngularCorrelation.h>
#include "TVector3.h"
#include <sys/stat.h>
#include "TGriffin.h"

/// \cond CLASSIMP
ClassImp(TAngularCorrelation)
/// \endcond

////////////////////////////////////////////////////////////////////////////////
/// Angular correlation default constructor
///
TAngularCorrelation::TAngularCorrelation() {
   f2DSlice = 0;
   fIndexCorrelation = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Angular correlation default destructor
///
TAngularCorrelation::~TAngularCorrelation() {
   delete f2DSlice;
   delete fIndexCorrelation;
   fPeaks.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// Create energy-gated 2D histogram of energy vs. angular index
///
/// \param[in] hst Three-dimensional histogram of angular index vs. energy vs. energy
/// \param[in] min Minimum of energy gate
/// \param[in] max Maximum of energy gate
/// \param[in] fold Switch for turning folding on
/// \param[in] group Switch for turning grouping on (not yet implemented)
///
/// Projects out the events with one energy between min and max
/// X-axis of returned histogram is second energy
/// Y-axis of returned histogram is angular index

TH2D* TAngularCorrelation::Create2DSlice(THnSparse *hst, Double_t min, Double_t max, Bool_t fold = kFALSE, Bool_t group = kFALSE){
   // identify the axes (angular index, energy, energy)
   // TODO: make this smart
   int indexaxis = 0;
   int energy1axis = 1;
   int energy2axis = 2;

   // project the THnSparse
   hst->GetAxis(energy1axis)->SetRangeUser(min,max);
   f2DSlice = (TH2D*) hst->Projection(indexaxis,energy2axis,"e"); // the "e" option pushes appropriate errors

   // TODO: folding
   if (fold) {
      // do nothing
   }
   // TODO: grouping
   if (group) {
      // do nothing
   }
   
   return f2DSlice;
}

////////////////////////////////////////////////////////////////////////////////
/// Create 1D histogram of counts vs. angular index
///
/// \param[in] hst Two-dimensional histogram of angular index vs. energy
/// \param[in] min Minimum of energy gate
/// \param[in] max Maximum of energy gate
///
/// For each bin (angular index), projects out the total number of events within
/// some energy range (given by min and max).

TH1D* TAngularCorrelation::IntegralSlices(TH2* hst, Double_t min, Double_t max){
//TODO: make IntegralSlicesX and IntegralSlicesY, both derived from this - add an option for slicing axis
   // set the range on the energy axis (x)
   hst->GetXaxis()->SetRangeUser(min,max);

   // calculate errors (if not already calculated)
   hst->Sumw2();

   // project counts to angular index axis
   fIndexCorrelation = hst->ProjectionY();

   return fIndexCorrelation;
}

////////////////////////////////////////////////////////////////////////////////
/// Create 1D histogram of counts vs. angular index
///
/// \param[in] hst Two-dimensional histogram of angular index vs. energy
/// \param[in] peak TPeak template used to fit one dimensional histograms
///
/// For each bin (angular index), fits one-dimensional projection with given TPeak
/// and returns a TH1D with x-axis of angular index and a y-axis of TPeak area for
/// that angular index.

TH1D* TAngularCorrelation::FitSlices(TH2* hst, TPeak* peak){
//TODO: This entire function
   return fIndexCorrelation;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates graph of counts vs. cos(theta) from histogram of counts vs. angular index
///
/// \param[in] hst One-dimensional histogram of angular index vs. counts
///

TGraphAsymmErrors* TAngularCorrelation::CreateGraphFromHst(TH1* hst) {
   TGraphAsymmErrors* graph = new TGraphAsymmErrors();
   
   Int_t n = hst->GetNbinsX();
   for (Int_t i=1;i<=n;i++) { // bin number loop
      // get index number
      Int_t index = hst->GetXaxis()->GetBinLowEdge(i);

      // get associated angle
      Double_t angle = fAngleMap[index];

      // get counts and error
      Double_t y = hst->GetBinContent(i);
      if (y==0) continue;
      Double_t yerr = hst->GetBinError(i);

      // fill graph with point
      Int_t graphn = graph->GetN();
      graph->SetPoint(graphn,TMath::Cos(angle),y);
      graph->SetPointError(graphn,0,0,yerr,yerr);
   } // bin number loop end

   // set title on graph
   graph->SetTitle(Form("%s;cos(#theta);Counts",hst->GetTitle()));

   return graph;
}

////////////////////////////////////////////////////////////////////////////////
/// Checks that maps are consistent with each other
///

Bool_t TAngularCorrelation::CheckMaps(){
   Bool_t result = kTRUE; // result to return

   if (fAngleMap.size()!=fWeights.size()) {
      printf("fAngleMap and fWeights do not have the same size.\n");
      printf("fAngleMap size is: %i\n",(Int_t) fAngleMap.size());
      printf("fWeights size is: %i\n",(Int_t) fWeights.size());
      result = kFALSE;
   }
   return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map used to construct angular indices
void TAngularCorrelation::PrintIndexMap() {
   Int_t size = fAngleMap.size();

   printf("-----------------------------------------------------\n");
   printf("|| Array number 1 | Array number 2 | Angular index ||\n");
   for (Int_t i=0;i<size;i++)
   {
      for (Int_t j=0;j<size;j++)
         printf("|| %i | %i | %i ||\n",i,j,GetAngularIndex(i,j));
   }
   return;
}

void TAngularCorrelation::PrintAngleMap() {
   Int_t size = fAngleMap.size();

   printf("---------------------------------------\n");
   printf("||  Angular index  |  Opening angle  ||\n");
   for (Int_t i=0;i<size;i++) {
      //TODO: fix the formatting in this printf statement
      printf("||  %i  | %f ||\n",i,GetAngleFromIndex(i));
   }
   printf("---------------------------------------\n");

   return;
}

// TODO: Generalize this so input is vector of detectors and vector of distances
Int_t TAngularCorrelation::GenerateIndexMaps(Int_t distance=110) {

   for (Int_t i=1;i<=16;i++) // detector one
   {
      for (Int_t j=0;j<4;j++) // crystal one
      {
         for (Int_t k=1;k<=16;k++) // detector two
         {
            for (Int_t l=0;l<4;l++) // crystal two
            {
               TVector3 positionone = TGriffin::GetPosition(i,j,distance); // distance is in mm, usually 110, 145, or 160
               TVector3 positiontwo = TGriffin::GetPosition(k,l,distance); // distance is in mm, usually 110, 145, or 160
               Double_t angle = positionone.Angle(positiontwo); // in radians
               Bool_t alreadyclaimed = kFALSE;
               for (Int_t m=0;m<(Int_t)fAngleMap.size();m++)
               {
                  if (abs(angle-fAngleMap[m])<0.00005)
                  {
                     alreadyclaimed = kTRUE;
                     break;
                  }
               } // map size loop
               if (!alreadyclaimed) fAngleMap.push_back(angle);
            } // crystal two loop
         } // detector two loop
      } // crystal one loop
   } // detector one loop

   Int_t size = fAngleMap.size();
   printf("Angular index to angle map has %i elements.\n",size);
   std::sort(fAngleMap.begin(),fAngleMap.end());

   fIndexMap = 0;
   fIndexMap = new Int_t*[size];

   for (int i=1;i<=16;i++) // detector one
   {
      for (int j=0;j<4;j++) // crystal one
      {
         fIndexMap[4*(i-1)+j+1] = new int[size];
         for (int k=1;k<=16;k++) // detector two
         {
            for (int l=0;l<4;l++) // crystal two
            {
               TVector3 positionone = TGriffin::GetPosition(i,j,distance);
               TVector3 positiontwo = TGriffin::GetPosition(k,l,distance);
               double angle = positionone.Angle(positiontwo); // in radians
               for (int m=0;m<size;m++)
               {
                  if (abs(angle-fAngleMap[m])<0.00005)
                  {
                     fIndexMap[4*(i-1)+j+1][4*(k-1)+l+1] = m;
                     break;
                  }
               } // map size loop
            } // crystal two loop
         } // detector two loop
      } // crystal one loop
   } // detector one loop

   return size;
}


