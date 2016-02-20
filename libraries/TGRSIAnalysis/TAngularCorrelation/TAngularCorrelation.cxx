/** \class TAngularCorrelation
 * An angular correlation class
 **/
#include <cstdio>
#include <map>
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
   fFolded = kFALSE;
   fGrouped = kFALSE;
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
   // we assume that the two axes with identical limits are the energy axes
   Int_t indexaxis,energy1axis,energy2axis;
   Double_t xmin[3];
   Double_t xmax[3];
   for (Int_t i=0;i<3;i++) {
      xmin[i] = hst->GetAxis(i)->GetXmin();
      xmax[i] = hst->GetAxis(i)->GetXmax();
   }
   if (xmin[0] == xmin[1] && xmax[0] == xmax[1]) {
      indexaxis = 2;
      energy1axis = 0;
      energy2axis = 1;
   } else if (xmin[1] == xmin[2] && xmax[1] == xmax[2]) {
      indexaxis = 0;
      energy1axis = 1;
      energy2axis = 2;
   } else if (xmin[2] == xmin[0] && xmax[2] == xmax[0]) {
      indexaxis = 1;
      energy1axis = 0;
      energy2axis = 2;
   } else {
      printf("Can't identify energy axes. Assuming index axis is axis 0.\n");
      indexaxis = 0;
      energy1axis = 1;
      energy2axis = 2;
   }

   // project the THnSparse
   hst->GetAxis(energy1axis)->SetRangeUser(min,max);
   f2DSlice = (TH2D*) hst->Projection(indexaxis,energy2axis,"e"); // the "e" option pushes appropriate errors
   f2DSlice->SetName(Form("%s_proj_%i",hst->GetName(),(Int_t)((max+min)/2)));
   f2DSlice->SetTitle(Form("%s: %i keV",hst->GetTitle(),(Int_t)((max+min)/2)));

   // TODO: folding
   if (fold) {
      // set kFolded bool
      // check for angle map calculated
      // fold f2DSlice
      // compare cos theta of angular indices
   }
   // TODO: grouping
   if (group) {
      // do nothing
   }
   
   return f2DSlice;
}

////////////////////////////////////////////////////////////////////////////////
/// Create energy-gated 2D histogram of energy vs. angular index
///
/// \param[in] hstarray TObjArray of TH2 energy vs. energy plots for each angular index
/// \param[in] min Minimum of energy gate
/// \param[in] max Maximum of energy gate
/// \param[in] fold Switch for turning folding on
/// \param[in] group Switch for turning grouping on (not yet implemented)
///
/// Assumes that the index of the TObjArray is the angular index
///
/// Projects out the events with one energy between min and max
/// X-axis of returned histogram is second energy
/// Y-axis of returned histogram is angular index

TH2D* TAngularCorrelation::Create2DSlice(TObjArray *hstarray, Double_t min, Double_t max, Bool_t fold = kFALSE, Bool_t group = kFALSE)
{
   // identify the type of histograms included in the array
   // a TH2D will need to be projected differently than a THnSparse array
   Bool_t sparse = kFALSE; // true if the array has THnSparse histograms
   Bool_t hst2d = kFALSE; // true if the array has some kind of TH2 histogram
   TIter next(hstarray);
   TObject* obj;
   while((obj=next())) {
      // TH2 loop
      if (obj->InheritsFrom("TH2")) {
         // if there have been no sparse histograms found
         if (!sparse) {
            hst2d = kTRUE;
         }
         else {
            printf("found both THnSparse and TH2 in array.\n");
            printf("currently, Create2DSlice only deals with one.\n");
            printf("Bailing out.\n");
            return 0;
         }
      } else if (obj->InheritsFrom("THnSparse")) {
         // if there have been no sparse histograms found
         if (!hst2d) {
            sparse = kTRUE;
         }
         else {
            printf("found both THnSparse and TH2 in array.\n");
            printf("currently, Create2DSlice only deals with one.\n");
            printf("Bailing out.\n");
            return 0;
         }
      }
      else {
         printf("Element is neither THnSparse or TH2.\n");
         printf("Bailing.\n");
      }
   }

   // if the array is neither, bail out
   if (!sparse && !hst2d) {
      printf("Can't identify the type of object in the array.\n");
      printf("Returning without slicing.\n");
      return 0;
   }

   // get axis properties
   Int_t elements = hstarray->GetEntries();
   Int_t bins = 0;
   Int_t xmin = 0;
   Int_t xmax = 0;
   const Char_t *name = 0;
   const Char_t *title = 0;
   if (sparse) {
      THnSparse* firsthst = (THnSparse*) hstarray->At(0);
      bins = firsthst->GetAxis(0)->GetNbins();
      xmin = firsthst->GetAxis(0)->GetBinLowEdge(1);
      xmax = firsthst->GetAxis(0)->GetBinUpEdge(bins);
      name = firsthst->GetName();
      title = firsthst->GetTitle();
   }
   else if (hst2d) {
      TH2* firsthst = (TH2*) hstarray->At(0);
      bins = firsthst->GetXaxis()->GetNbins();
      xmin = firsthst->GetXaxis()->GetBinLowEdge(1);
      xmax = firsthst->GetXaxis()->GetBinUpEdge(bins);
      name = firsthst->GetName();
      title = firsthst->GetTitle();
   }

   // create the empty histogram
   TH2D* newslice = new TH2D(Form("%s_%i_%i",name,Int_t(min),Int_t(max)),Form("%s, E_{#gamma 1}=[%.1f,%.1f)",title,min,max),bins,xmin,xmax,elements,0,elements);

   // iterate over the array of 2D gamma-gamma matrices
   for (Int_t i=0;i<elements;i++) {
      
      // slice this particular matrix
      TH1D* tempslice = 0;
      // sparse option
      if (sparse) {
         THnSparse* thishst = (THnSparse*) hstarray->At(i);
         thishst->GetAxis(0)->SetRangeUser(min,max);
         tempslice = (TH1D*) thishst->Projection(1,"e"); // the "e" option pushes appropriate errors
      }
      // TH2 option
      else if (hst2d) {
         TH2* thishst = (TH2*) hstarray->At(i);
         thishst->GetXaxis()->SetRangeUser(min,max);
         tempslice = thishst->ProjectionY();
      }

      // save histogram values
      Double_t *xvalues = new Double_t[bins];
      Double_t *yvalues = new Double_t[bins];
      Double_t *weights = new Double_t[bins];
      for (Int_t j=1;j<=bins;j++) {
         xvalues[j-1] = tempslice->GetBinCenter(j);
         yvalues[j-1] = i;
         weights[j-1] = tempslice->GetBinContent(j);
      }

      // fill the 2D histogram with those values
      newslice->FillN(bins,xvalues,yvalues,weights);

      // cleanup
      delete tempslice;
      delete [] xvalues;
      delete [] yvalues;
      delete [] weights;
   }

   // TODO: folding
   if (fold) {
      // set kFolded bool
      // check for angle map calculated
      // fold f2DSlice
      // compare cos theta of angular indices
   }
   // TODO: grouping
   if (group) {
      // do nothing
   }

   f2DSlice = newslice;
   
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
   TH1D* newhst = new TH1D(hst1dname,Form("%s;Angular index;Counts",hst1dtitle),indexbins,indexmin,indexmax);

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

   // loop over the indices
   for (Int_t i=1;i<=indexmax-indexmin;i++) {
      Int_t index = hst->GetYaxis()->GetBinLowEdge(i);

      if (visualization) {
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
      }

      // pull individual slice
      TH1D* temphst = hst->ProjectionX(Form("%s_proj%i",hst2dname,index),i,i);
      temphst->SetStats(0);
      temphst->SetTitle(Form("%s: angular index %i",hst->GetTitle(),index));
      this->Set1DSlice(index,temphst);

      // draw the slice to canvas
      if (visualization) this->Get1DSlice(index)->Draw("pe1");

      // if there are too few counts, continue on (you can fit it manually later)
      if (temphst->Integral()<100) continue;

      // rename TPeak
      peak->SetName(Form("%s_proj%i_peak",hst2dname,index));

      // fit TPeak
      Bool_t fitresult = peak->Fit(temphst,"Q");
      if (!fitresult) continue; // if fit failed, continue on to next index
      if (visualization) peak->Background()->Draw("same");

      // assign TPeak to fPeaks array
      this->SetPeak(index,static_cast<TPeak*>(temphst->GetFunction(Form("%s_proj%i_peak",hst2dname,index))));

      // extract area
      Double_t area = static_cast<TPeak*>(this->GetPeak(index))->GetArea();
      Double_t area_err = static_cast<TPeak*>(this->GetPeak(index))->GetAreaErr();

      // fill histogram with area
      newhst->SetBinContent(i,area);
      newhst->SetBinError(i,area_err);
   }

   ///////////////////////////////////////////////////////
   // Create diagnostic window
   ///////////////////////////////////////////////////////

   // initialize canvas
   TCanvas* c_diag = new TCanvas(Form("c_diag_%i",(Int_t)peak->GetCentroid()),Form("Diagnostics for fitting %i keV peak",(Int_t)peak->GetCentroid()),800,800);

   // create plots for chi^2, centroid, and fwhm
   TH1D* chi2hst = new TH1D(Form("%s_chi2",hst1dname),Form("%s: #chi^{2};Angular index;#chi^{2}/NDF value",newhst->GetTitle()),indexbins,indexmin,indexmax);
   TH1D* centroidhst = new TH1D(Form("%s_centroid",hst1dname),Form("%s: Peak centroid;Angular index;Peak centroid (keV)",newhst->GetTitle()),indexbins,indexmin,indexmax);
   TH1D* fwhmhst = new TH1D(Form("%s_fwhm",hst1dname),Form("%s: FWHM;Angular index;FWHM (keV)",newhst->GetTitle()),indexbins,indexmin,indexmax);

   // assign histogram to fIndexCorrelation
   fIndexCorrelation = newhst;
   fChi2 = chi2hst;
   fCentroid = centroidhst;
   fFWHM = fwhmhst;

   this->UpdateDiagnostics();
   this->DisplayDiagnostics(c_diag);

   ///////////////////////////////////////////////////////
   // Clean-up
   ///////////////////////////////////////////////////////

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
   if (arraynum1==0 || arraynum2==0) {
      printf("Array numbers usually begin at 1 - unless you have programmed\n");
      printf("it differently explicitly, don't trust this output.\n");
   }
   if (fIndexMap.count(arraynum1)==0) {
      printf("Error: array number %i is not present in the index map.\n",arraynum1);
      return -1;
   }
   if (fIndexMap[arraynum1].count(arraynum2)==0) {
      printf("Error: element [%i][%i] is not present in the index map.\n",arraynum1,arraynum2);
      return -1;
   }
   // Array numbers start counting at 1
   // Indices of this array start at 0
   return fIndexMap[arraynum1][arraynum2];
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
   Int_t size = fIndexMapSize;

   for (Int_t i=1;i<=size;i++)
   {
      printf("-----------------------------------------------------\n");
      printf("|| Array number 1 | Array number 2 | Angular index ||\n");
      printf("-----------------------------------------------------\n");
      for (Int_t j=1;j<=size;j++) {
         if (GetAngularIndex(i,j)==-1) continue;
         printf("|| %-14i | %-14i | %-13i ||\n",i,j,GetAngularIndex(i,j));
      }
   }
   printf("-----------------------------------------------------\n");
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

   printf("--------------------------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  |  Weight  ||\n");
   printf("--------------------------------------------------------\n");
   for (Int_t i=0;i<size;i++) {
      printf("||  %-13i  |  %-19.4f  |  %-6.0f  ||\n",i,GetAngleFromIndex(i),GetWeightFromIndex(i));
   }
   printf("--------------------------------------------------------\n");

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle
///

void TAngularCorrelation::PrintAngleMap()
{
   Int_t size = fAngleMap.size();

   printf("---------------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  ||\n");
   printf("---------------------------------------------\n");
   for (Int_t i=0;i<size;i++) {
      printf("||  %-13i  |  %-19.4f  ||\n",i,GetAngleFromIndex(i));
   }
   printf("---------------------------------------------\n");

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
/// \param[in] indexmap Index map (probably created with GenerateIndexMap)
///
/// The indices for the index map start from zero, so when using array numbers
/// (which start from one) as input for those indices, you need to subtract one.

std::vector<Int_t> TAngularCorrelation::GenerateWeights(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::map<Int_t,std::map<Int_t,Int_t>> &indexmap)
{
   std::vector<Int_t> weights; // vector to return

   // get array number size
   Int_t size = arraynumbers.size();

   // find maximum angular index
   Int_t max = 0;
   for (Int_t i=0;i<size;i++) {
      for (Int_t j=0;j<size;j++) {
         if (indexmap[i][j]>max) max = indexmap[i][j];
      }
   }
   
   // initialize vector
   for (Int_t i=0;i<=max;i++) {
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
         Int_t index = indexmap[arraynumbers[i]][arraynumbers[j]];
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

std::map<Int_t,std::map<Int_t,Int_t>> TAngularCorrelation::GenerateIndexMap(std::vector<Int_t> &arraynumbers, std::vector<Int_t> &distances, std::vector<Double_t> &anglemap)
{
   // initialize map
   std::map<Int_t,std::map<Int_t,Int_t>> indexmap;

   // get arraynumbers size
   Int_t size = arraynumbers.size();

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
               Int_t index1 = arraynumbers[i];
               Int_t index2 = arraynumbers[j];
               if (index1==0 || index2==0) printf("found array number of zero?\n");
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

////////////////////////////////////////////////////////////////////////////////
/// Updates index correlation based on peak array
///

void TAngularCorrelation::UpdateIndexCorrelation()
{
   // loop over quantities in map
   for (std::map<Int_t,TPeak*>::iterator it=fPeaks.begin(); it!=fPeaks.end(); ++it) {
      Int_t index = it->first;
      Int_t bin = ((TH1D*) this->GetIndexCorrelation())->FindBin(index);

      // extract area
      TPeak* peak = static_cast<TPeak*>(this->GetPeak(index));
      if (!peak) return;
      Double_t area = peak->GetArea();
      Double_t area_err = peak->GetAreaErr();

      // fill histogram with area
      static_cast<TH1D*>(this->GetIndexCorrelation())->SetBinContent(bin,area);
      static_cast<TH1D*>(this->GetIndexCorrelation())->SetBinError(bin,area_err);
   }

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Updates diagnostics based on peak array
///

void TAngularCorrelation::UpdateDiagnostics()
{
   // loop over quantities in map
   for (std::map<Int_t,TPeak*>::iterator it=fPeaks.begin(); it!=fPeaks.end(); ++it) {
      Int_t index = it->first;
      Int_t bin = ((TH1D*) this->GetIndexCorrelation())->FindBin(index);

      // extract pertinent values from TPeaks
      TPeak* peak = static_cast<TPeak*>(this->GetPeak(index));
      if (!peak) return;
      Double_t chi2 = peak->GetChisquare();
      Double_t NDF = (Double_t)peak->GetNDF();
      Double_t centroid = peak->GetCentroid();
      Double_t centroid_err = peak->GetCentroidErr();
      Double_t fwhm = peak->GetFWHM();
      Double_t fwhm_err = peak->GetFWHMErr();

      // fill histogram with values
      static_cast<TH1D*>(this->GetChi2Hst())->SetBinContent(bin,chi2/NDF);
      static_cast<TH1D*>(this->GetCentroidHst())->SetBinContent(bin,centroid);
      static_cast<TH1D*>(this->GetCentroidHst())->SetBinError(bin,centroid_err);
      static_cast<TH1D*>(this->GetFWHMHst())->SetBinContent(bin,fwhm);
      static_cast<TH1D*>(this->GetFWHMHst())->SetBinError(bin,fwhm_err);
   }

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Fits slice with new peak and updates the index correlation
///
/// \param[in] index angular index
/// \param[in] peak Tpeak to be used for fitting

void TAngularCorrelation::UpdatePeak(Int_t index,TPeak* peak)
{
   // create canvas
   new TCanvas(Form("peak%iupdate",index),Form("Peak %i update",index),200,200);

   // get histogram
   TH1D* temphst = this->Get1DSlice(index);
   const Char_t *name = temphst->GetListOfFunctions()->At(0)->GetName();

   // adjust range
   Double_t minenergy,maxenergy;
   peak->GetRange(minenergy,maxenergy);
   Double_t difference = maxenergy-minenergy;
   minenergy = minenergy-0.5*difference;
   maxenergy = maxenergy+0.5*difference;
   temphst->GetXaxis()->SetRangeUser(minenergy,maxenergy);

   // fit peak
   peak->SetName(name);
   peak->Fit(this->Get1DSlice(index),"");

   // push new peak
   this->SetPeak(index,static_cast<TPeak*>(temphst->GetFunction(name)));
   UpdateIndexCorrelation();
   UpdateDiagnostics();

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Returns peak, if it exists
///
/// \param[in] index angular index
///

TPeak* TAngularCorrelation::GetPeak(Int_t index)
{
   if (!fPeaks[index]) {
      printf("No peak exists for index %i. Returning 0x0.\n",index);
      return 0x0;
   } else {
      return fPeaks[index];
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Divides histogram by weights listed in weight array
///
/// \param[in] hst histogram
///

TH1D* TAngularCorrelation::DivideByWeights(TH1* hst)
{
   Int_t size = this->GetWeightsSize();

   // consistency/stability checks
   if (size==0) {
      printf("You haven't created the weights yet. Please use the GenerateMaps function to do so.\n");
      return 0x0;
   }
   if (size!=hst->GetNbinsX()) {
      printf("Warning: size of weights array is different than number of bins in %s\n",hst->GetName());
   }

   // this loop is just for checking to make sure all indices are in the weight vector
   for (Int_t i=1;i<=hst->GetNbinsX();i++) {
      Int_t index = hst->GetBinLowEdge(i);
      if (index>=size) {
         printf("Indices in histogram %s go beyond size of weights array. Aborting.\n",hst->GetName());
         return 0x0;
      }
   }  

   // now that we're satisified everything is kosher, divide the bins.
   for (Int_t i=1;i<=hst->GetNbinsX();i++) {
      Int_t index = hst->GetBinLowEdge(i);
      Double_t content = hst->GetBinContent(i);
      Double_t error = hst->GetBinError(i);
      Double_t weight = this->GetWeightFromIndex(index);
      Double_t newcontent = content/weight;
      Double_t newerror = error/weight;
      hst->SetBinContent(i,newcontent);
      hst->SetBinError(i,newerror);
   }

   return (TH1D*) hst;
}

////////////////////////////////////////////////////////////////////////////////
/// Divides index correlation by weights listed in weight array
///

void TAngularCorrelation::DivideByWeights()
{
   TH1D* hst = DivideByWeights(fIndexCorrelation);
   if (hst!=0x0) fIndexCorrelation=hst;
   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Displays diagnostics based on peak array
///

void TAngularCorrelation::DisplayDiagnostics(TCanvas* c_diag)
{
   if (c_diag == 0) {
      c_diag = new TCanvas(Form("c_diag_%s",this->GetName()),Form("Diagnostics from %s",this->GetName()),800,800);
   }

   // divide canvas
   c_diag->Divide(2,2);

   // pull plots for index correlation, chi^2, centroid, and fwhm
   TH1D* indexhst = this->GetIndexCorrelation();
   TH1D* chi2hst = this->GetChi2Hst();
   TH1D* centroidhst = this->GetCentroidHst();
   TH1D* fwhmhst = this->GetFWHMHst();

   // format the diagnostic plots
   indexhst->SetStats(0);
   chi2hst->SetStats(0);
   centroidhst->SetStats(0);
   fwhmhst->SetStats(0);
   chi2hst->SetMarkerStyle(4);

   // plot chi^2, centroid, and FWHM
   c_diag->cd(1);
   indexhst->Draw();
   c_diag->cd(2);
   chi2hst->Draw("p");
   c_diag->cd(3);
   centroidhst->Draw();
   c_diag->cd(4);
   fwhmhst->Draw();

   return;
}
