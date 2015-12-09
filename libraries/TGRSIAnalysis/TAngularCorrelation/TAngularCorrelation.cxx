/** \class TAngularCorrelation
 * An angular correlation class
 **/
#include <cstdio>
#include <TAngularCorrelation.h>
#include "TVector3.h"
#include <sys/stat.h>
#include "TGriffin.h"
#include "TMath.h"
#include "TCanvas.h"

/// \cond CLASSIMP
ClassImp(TAngularCorrelation)
/// \endcond

////////////////////////////////////////////////////////////////////////////////
/// Angular correlation default constructor
///
TAngularCorrelation::TAngularCorrelation()
{
   f2DSlice = 0;
   fIndexCorrelation = 0;
   fIndexMapSize = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Angular correlation default destructor
///
TAngularCorrelation::~TAngularCorrelation()
{
   delete f2DSlice;
   delete fIndexCorrelation;
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

TH2D* TAngularCorrelation::Create2DSlice(THnSparse *hst, Double_t min, Double_t max, Bool_t fold = kFALSE, Bool_t group = kFALSE)
{
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

TH1D* TAngularCorrelation::IntegralSlices(TH2* hst, Double_t min, Double_t max)
{
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

TH1D* TAngularCorrelation::FitSlices(TH2* hst, TPeak* peak, Bool_t visualization = kTRUE)
{
   ///////////////////////////////////////////////////////
   // Set-up histograms
   // ////////////////////////////////////////////////////

   // pull angular index limits from hst
   // assumes that angular index is y-axis and energy is x-axis
   Int_t indexmin = (Int_t) hst->GetYaxis()->GetXmin();
   Int_t indexmax = (Int_t) hst->GetYaxis()->GetXmax();
   const Int_t indexbins = indexmax-indexmin;

   // pull name from hst, modify for 1D hst
   const Char_t* hst2dname = hst->GetName();
   const Char_t* hst2dtitle = hst->GetTitle();
   const Char_t* hst1dname = Form("%s_%ikeV",hst2dname,(Int_t)peak->GetCentroid());
   const Char_t* hst1dtitle = Form("%s-%ikeV",hst2dtitle,(Int_t)peak->GetCentroid());

   // initialize histogram to return
   TH1D* newhst = new TH1D(hst1dname,hst1dtitle,indexbins,indexmin,indexmax);

   // calculate errors on hst (if not already calculated)
   hst->Sumw2();

   // set the range on the energy axis (x)
   // this isn't strictly necessary, but it will make the histograms smaller
   // and visually, easier to see in the diagnostic.
   Double_t minenergy,maxenergy;
   peak->GetRange(minenergy,maxenergy);
   Double_t difference = maxenergy-minenergy;
   minenergy = minenergy-0.5*difference;
   maxenergy = maxenergy+0.5*difference;
   hst->GetXaxis()->SetRangeUser(minenergy,maxenergy);

   ///////////////////////////////////////////////////////
   // Fitting and visualization
   ///////////////////////////////////////////////////////

   std::vector<TCanvas*> c;
   std::vector<TH1D*> slices;

   // loop over the indices
   for (Int_t i=1;i<=indexmax-indexmin;i++) {
      Int_t index = hst->GetYaxis()->GetBinLowEdge(i);
      // find the correct pad
      Int_t canvas = (i-1)/16;
      Int_t pad = (i-1)%16;
      if (pad==0) {
         TCanvas* temp = new TCanvas(Form("c%i",canvas),Form("c%i",canvas),800,800);
         temp->Divide(4,4);
         c.push_back(temp);
      }
      // go to canvas pad
      c[canvas]->cd(pad+1);
      // pull individual slice
      TH1D* temphst = hst->ProjectionY(Form("%s_proj%i",hst2dname,index),i,i);
      slices.push_back(temphst);
      slices[i-1]->Draw("pe1");

      // rename TPeak
      peak->SetName(Form("%s_proj%i_peak",hst2dname,index));

      // fit TPeak
      peak->Fit(slices[i-1]);

      // assign TPeak to fPeaks array
      this->SetPeak(index,(TPeak*) slices[i-1]->GetFunction(Form("%s_proj%i_peak",hst2dname,index)));

      // extract area
      Double_t area = ((TPeak*) this->GetPeak(index))->GetArea();
      Double_t area_err = ((TPeak*) this->GetPeak(index))->GetAreaErr();

      // fill histogram with area
      newhst->SetBinContent(i,area);
      newhst->SetBinError(i,area_err);
   }

   ///////////////////////////////////////////////////////
   // Clean-up
   ///////////////////////////////////////////////////////

   // assign histogram to fIndexCorrelation
   fIndexCorrelation = newhst;

   return fIndexCorrelation;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates graph of counts vs. cos(theta) from histogram of counts vs. angular index
///
/// \param[in] hst One-dimensional histogram of angular index vs. counts
///

TGraphAsymmErrors* TAngularCorrelation::CreateGraphFromHst(TH1* hst)
{
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
/// Get angular index from two array numbers
/// 
/// \param[in] arraynum1 first array number
/// \param[in] arraynum2 second array number
///

Int_t TAngularCorrelation::GetAngularIndex(Int_t arraynum1, Int_t arraynum2)
{
   if (arraynum1>=fIndexMapSize || arraynum2>=fIndexMapSize) {
      printf("Error: One or more array numbers is larger than the size of fIndexMap.\n");
      return -1;
   }
   // Array numbers start counting at 1
   // Indices of this array start at 0
   Int_t index1 = arraynum1-1;
   Int_t index2 = arraynum2-1;
   return fIndexMap[index1][index2];
}

////////////////////////////////////////////////////////////////////////////////
/// Checks that maps are consistent with each other
///

Bool_t TAngularCorrelation::CheckMaps()
{
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
///

void TAngularCorrelation::PrintIndexMap()
{
   Int_t size = fAngleMap.size();

   printf("-----------------------------------------------------\n");
   printf("|| Array number 1 | Array number 2 | Angular index ||\n");
   for (Int_t i=1;i<size;i++)
   {
      for (Int_t j=1;j<size;j++) {
         if (GetAngularIndex(i,j)==-1) continue;
         //TODO: fix the formatting in this printf statement
         printf("|| %i | %i | %i ||\n",i,j,GetAngularIndex(i,j));
      }
   }
   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle vs. weight
///

void TAngularCorrelation::PrintWeights()
{
   Int_t size = fAngleMap.size();
   Int_t weight_size = fWeights.size();
   if (size==0) {
      printf("The angle map hasn't been created yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }
   if (weight_size==0) {
      printf("The weights haven't been calculated yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }

   printf("---------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  |  Weight  ||\n");
   for (Int_t i=0;i<size;i++) {
      //TODO: fix the formatting in this printf statement
      printf("||  %i  | %f | %f ||\n",i,GetAngleFromIndex(i),GetWeightFromIndex(i));
   }
   printf("---------------------------------------\n");

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle
///

void TAngularCorrelation::PrintAngleMap()
{
   Int_t size = fAngleMap.size();

   printf("---------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  ||\n");
   for (Int_t i=0;i<size;i++) {
      //TODO: fix the formatting in this printf statement
      printf("||  %i  | %f ||\n",i,GetAngleFromIndex(i));
   }
   printf("---------------------------------------\n");

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of angular index vs. opening angle.
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
///

std::vector<Double_t> TAngularCorrelation::GenerateAngleMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances)
{

   std::vector<Double_t> map; // vector to return

   // basic consistency check
   const Int_t size = arraynumbers.size();
   if (size!=(Int_t)distances.size()) {
      printf("Lengths of array number and distance vectors are inconsistent.\n");
      printf("Array number vector size: %i\n",size);
      printf("Distance vector size: %i\n",(Int_t)distances.size());
   }

   // loop through array numbers (a list of crystals in the array)
   for (Int_t i=0;i<size;i++) {
      // identify detector and crystal numbers
      Int_t detector1 = (arraynumbers[i]-1)/4 + 1;
      Int_t crystal1 = (arraynumbers[i]-1)%4;
      // now we will loop through all *unique* combinations
      // we can start from j=i here, because j<i will only produce duplicates
      for (Int_t j=i;j<size;j++) {
         // identify detector and crystal numbers
         Int_t detector2 = (arraynumbers[j]-1)/4 + 1;
         Int_t crystal2 = (arraynumbers[j]-1)%4;
         TVector3 positionone = TGriffin::GetPosition(detector1,crystal1,distances[i]); // distance is in mm, usually 110, 145, or 160
         TVector3 positiontwo = TGriffin::GetPosition(detector2,crystal2,distances[j]); // distance is in mm, usually 110, 145, or 160
         Double_t angle = positionone.Angle(positiontwo); // in radians
         Bool_t alreadyclaimed = kFALSE;
         for (Int_t m=0;m<(Int_t)map.size();m++) {
            if (TMath::Abs(angle-map[m])<0.00005)
            {
               alreadyclaimed = kTRUE;
               break;
            }
         }
         if (!alreadyclaimed) map.push_back(angle);
      }
   }

   // sort the map
   std::sort(map.begin(),map.end());

   return map;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of weights vs. angular index
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
/// \param[in] indexmap Index map (probably created with GenerateIndexMap
///

std::vector<Int_t> TAngularCorrelation::GenerateWeights(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, Int_t** &indexmap)
{
   std::vector<Int_t> weights; // vector to return

   // get array number size
   Int_t size = arraynumbers.size();

   // find maximum array number
   Int_t max = 0;
   for (Int_t i=0;i<size;i++) {
      if (arraynumbers[i]>max) max = arraynumbers[i];
   }
   
   // initialize vector
   for (Int_t i=0;i<max;i++) {
      weights.push_back(0);
   }

   // loop through array numbers (a list of crystals in the array)
   for (Int_t i=0;i<size;i++) {
      if (arraynumbers[i]<1 || arraynumbers[i]>64) {
         printf("%i is not a good array number.\n",arraynumbers[i]);
         printf("Skipping... you'll probably get some errors.\n");
         continue;
      }
      // here, we want all combinations for the indices, so we start from j=0
      for (Int_t j=0;j<size;j++) {
         Int_t index = indexmap[i][j];
         Int_t old_weight = weights[index];
         Int_t new_weight = old_weight+1;
         weights[index] = new_weight;
      }
   }

   return weights;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of angle pair vs. angular index
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
/// \param[in] anglemap Angle map (probably created with GenerateAngleMap
///

Int_t** TAngularCorrelation::GenerateIndexMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::vector<Double_t> &anglemap)
{

   // get array number size
   Int_t size = arraynumbers.size();

   // find maximum array number
   Int_t max = 0;
   for (Int_t i=0;i<size;i++) {
      if (arraynumbers[i]>max) max = arraynumbers[i];
   }
   
   // initialize array
   Int_t** indexmap = 0;
   indexmap = new Int_t*[max];
   for (Int_t i=0;i<max;i++) {
      indexmap[i] = new Int_t[max];
      for (Int_t j=0;j<max;j++) {
         indexmap[i][j] = -1;
      }
   }

   // get angle map size
   Int_t mapsize = anglemap.size();

   // loop through array numbers (a list of crystals in the array)
   for (Int_t i=0;i<size;i++) {
      if (arraynumbers[i]<1 || arraynumbers[i]>64) {
         printf("%i is not a good array number.\n",arraynumbers[i]);
         printf("Skipping... you'll probably get some errors.\n");
         continue;
      }

      // identify detector and crystal numbers
      Int_t detector1 = (arraynumbers[i]-1)/4 + 1;
      Int_t crystal1 = (arraynumbers[i]-1)%4;
      TVector3 positionone = TGriffin::GetPosition(detector1,crystal1,distances[i]); // distance is in mm, usually 110, 145, or 160

      // here, we want all combinations for the indices, so we start from j=0
      for (Int_t j=0;j<size;j++) {
         // identify detector and crystal numbers
         Int_t detector2 = (arraynumbers[j]-1)/4 + 1;
         Int_t crystal2 = (arraynumbers[j]-1)%4;
         TVector3 positiontwo = TGriffin::GetPosition(detector2,crystal2,distances[j]); // distance is in mm, usually 110, 145, or 160
         Double_t angle = positionone.Angle(positiontwo); // in radians
         for (Int_t m=0;m<mapsize;m++) {
            if (TMath::Abs(angle-anglemap[m])<0.00005)
            {
               Int_t index1 = arraynumbers[i]-1;
               Int_t index2 = arraynumbers[j]-1;
               indexmap[index1][index2] = m;
               break;
            }
         }
      }
   }

   return indexmap;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates maps of angle pair vs. angular index and angular index vs. opening angle.
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
///

Int_t TAngularCorrelation::GenerateMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances)
{
   // basic consistency check
   const Int_t size = arraynumbers.size();
   if (size!=(Int_t)distances.size()) {
      printf("Lengths of array number and distance vectors are inconsistent.\n");
      printf("Array number vector size: %i\n",size);
      printf("Distance vector size: %i\n",(Int_t)distances.size());
   }

   // clear vector map
   fAngleMap.clear();

   // find maximum array number (which will be the index map size)
   fIndexMapSize = 0;
   for (Int_t i=0;i<size;i++) {
      if (arraynumbers[i]>fIndexMapSize) fIndexMapSize = arraynumbers[i];
   }

   // generate maps
   fAngleMap = GenerateAngleMap(arraynumbers,distances);
   fNumIndices = fAngleMap.size();
   fIndexMap = GenerateIndexMap(arraynumbers,distances,fAngleMap);
   fWeights = GenerateWeights(arraynumbers,distances,fIndexMap);

   return fNumIndices;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates maps for typical GRIFFIN configurations
///
/// \param[in] detectors number of detectors
/// \param[in] distance distance of detectors (in mm)
///
/// 16 detectors: full array
/// 15 detectors: full array less detector 13
/// 12 detectors: upstream lampshade and corona, detectors 5-16
/// 11 detectors: upstream lampshade and corona, less detector 13
/// 8 detectors: corona only
/// For more detailed configurations, please use GenerateMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances)
///

Int_t TAngularCorrelation::GenerateMaps(Int_t detectors, Int_t distance)
{
   std::vector<Int_t> array_numbers;
   std::vector<Int_t> distances;

   if (detectors==16) {
      printf("Generating maps for full array setup.\n");
      for (Int_t i=1;i<=64;i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if (detectors==15) {
      printf("Generating maps for full array setup, less detector 13.\n");
      for (Int_t i=1;i<=64;i++) {
         if (i>=49 && i<=52) continue; // no detector 13
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if (detectors==12) {
      printf("Generating maps for detectors 5-16.\n");
      for (Int_t i=17;i<=64;i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if (detectors==11) {
      printf("Generating maps for detectors 5-16, less detector 13.\n");
      for (Int_t i=17;i<=64;i++) {
         if (i>=49 && i<=52) continue; // no detector 13
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if (detectors==8) {
      printf("Generating maps for the corona only, detectors 5-12.\n");
      for (Int_t i=17;i<=48;i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else {
      printf("This option isn't coded. Please use the more general GenerateMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances)function.\n");
      return 0;
   }

   Int_t val = GenerateMaps(array_numbers,distances);

   return val;
}
