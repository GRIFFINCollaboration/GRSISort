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
   fIndexCorrelation = nullptr; // used the 1D histograms for a specific energy ranged used for FitSlices
   fIndexMapSize     = 0;       // number of indexes, never called in making the histograms
   fFolded           = kFALSE;  // not set yet
   fGrouped          = kFALSE;  // not set yet
}

////////////////////////////////////////////////////////////////////////////////
/// Angular correlation default destructor
///
TAngularCorrelation::~TAngularCorrelation()
{
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
/// Y-axis of returned histogram is angular index (or the group number if group = kTRUE)

TH2D* TAngularCorrelation::Create2DSlice(THnSparse* hst, Double_t min, Double_t max, Bool_t fold = kFALSE,
                                         Bool_t group = kFALSE)
{
   // identify the axes (angular index, energy, energy)
   // we assume that the two axes with identical limits are the energy axes
   Int_t    indexaxis, energy1axis, energy2axis;
   Double_t xmin[3];
   Double_t xmax[3];
   for(int i = 0; i < 3; i++) { // goes through all three dimensions of THnSparse and finds the min and max values
      xmin[i] = hst->GetAxis(i)->GetXmin();
      xmax[i] = hst->GetAxis(i)->GetXmax();
   }                                              // then look for matching axis since energy axis should be the same
   if(xmin[0] == xmin[1] && xmax[0] == xmax[1]) { // are 0 and 1 the same? then axis 2 is the index axis
      indexaxis   = 2;
      energy1axis = 0;
      energy2axis = 1;
   } else if(xmin[1] == xmin[2] && xmax[1] == xmax[2]) { // are 1 and 2 the same? then axis 0 is the index axis
      indexaxis   = 0;
      energy1axis = 1;
      energy2axis = 2;
   } else if(xmin[2] == xmin[0] && xmax[2] == xmax[0]) { // are 0 and 2 the same? then axis 1 is the index axis
      indexaxis   = 1;
      energy1axis = 0;
      energy2axis = 2;
   } else {
      printf("Can't identify energy axes. Assuming index axis is axis 0.\n"); // no two axis are the same
      indexaxis   = 0;
      energy1axis = 1;
      energy2axis = 2;
   }

   // project the THnSparse
   hst->GetAxis(energy1axis)->SetRangeUser(min, max);
   TH2D* tempslice = hst->Projection(indexaxis, energy2axis, "eo"); // the "e" option pushes appropriate errors
   tempslice->SetName(Form("%s_proj_%i", hst->GetName(), static_cast<Int_t>((max + min) / 2)));
   tempslice->SetTitle(Form("%s: %i keV", hst->GetTitle(), static_cast<Int_t>((max + min) / 2)));

   TH2D* slice2d = Modify2DSlice(tempslice, fold, group);

   return slice2d;
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

TH2D* TAngularCorrelation::Create2DSlice(TObjArray* hstarray, Double_t min, Double_t max, Bool_t fold = kFALSE,
                                         Bool_t group = kFALSE)
{
   // identify the type of histograms included in the array
   // a TH2D will need to be projected differently than a THnSparse array

   Bool_t   sparse = kFALSE; // true if the array has THnSparse histograms
   Bool_t   hst2d  = kFALSE; // true if the array has some kind of TH2 histogram
   TIter    next(hstarray);
   TObject* obj;
   while((obj = next()) != nullptr) {
      // TH2 loop
      if(obj->InheritsFrom("TH2")) {
         // if there have been no sparse histograms found
         if(!sparse) {
            hst2d = kTRUE;
         } else {
            printf("found both THnSparse and TH2 in array.\n");
            printf("currently, Create2DSlice only deals with one.\n");
            printf("Bailing out.\n");
            return nullptr;
         }
      } else if(obj->InheritsFrom("THnSparse")) {
         // if there have been no sparse histograms found
         if(!hst2d) {
            sparse = kTRUE;
         } else {
            printf("found both THnSparse and TH2 in array.\n");
            printf("currently, Create2DSlice only deals with one.\n");
            printf("Bailing out.\n");
            return nullptr;
         }
      } else {
         printf("Element is neither THnSparse or TH2.\n");
         printf("Bailing.\n");
      }
   }

   // if the array is neither, bail out
   if(!sparse && !hst2d) {
      printf("Can't identify the type of object in the array.\n");
      printf("Returning without slicing.\n");
      return nullptr;
   }

   // get axis properties
   Int_t         elements = hstarray->GetEntries();
   Int_t         bins     = 0;
   Int_t         xmin     = 0;
   Int_t         xmax     = 0;
   const Char_t* name     = nullptr;
   const Char_t* title    = nullptr;
   if(sparse) {
      THnSparse* firsthst = static_cast<THnSparse*>(hstarray->At(0));
      bins                = firsthst->GetAxis(0)->GetNbins();
      xmin                = firsthst->GetAxis(0)->GetBinLowEdge(1);
      xmax                = firsthst->GetAxis(0)->GetBinUpEdge(bins);
      name                = firsthst->GetName();
      title               = firsthst->GetTitle();
   } else if(hst2d) {
      TH2* firsthst = static_cast<TH2*>(hstarray->At(0));
      bins          = firsthst->GetXaxis()->GetNbins();
      xmin          = firsthst->GetXaxis()->GetBinLowEdge(1);
      xmax          = firsthst->GetXaxis()->GetBinUpEdge(bins);
      name          = firsthst->GetName();
      title         = firsthst->GetTitle();
   }
   Int_t ybins = elements;

   Int_t iteration = 0;
   TH2D* newslice;
   if(gFile->Get(Form("%s_%i_%i", name, Int_t(min), Int_t(max))) == nullptr) {
      newslice = new TH2D(Form("%s_%i_%i", name, Int_t(min), Int_t(max)),
                          Form("%s, E_{#gamma 1}=[%.1f,%.1f)", title, min, max), bins, xmin, xmax, ybins, 0, ybins);
   } else {
      while(iteration < 10) {
         if(gFile->Get(Form("%s_%i_%i_%i", name, Int_t(min), Int_t(max), iteration)) == nullptr) {
            break;
         } else {
            ++iteration;
         }
      }
      newslice = new TH2D(Form("%s_%i_%i_%i", name, Int_t(min), Int_t(max), iteration),
                          Form("%s, E_{#gamma 1}=[%.1f,%.1f)", title, min, max), bins, xmin, xmax, ybins, 0, ybins);
   }

   // iterate over the array of 2D gamma-gamma matrices
   for(Int_t i = 0; i < elements; i++) { // takes slice and itterates through all indexes
      // slice this particular matrix
      TH1D* tempslice = nullptr;
      // sparse option
      if(sparse) {
         THnSparse* thishst = static_cast<THnSparse*>(hstarray->At(i));
         thishst->GetAxis(0)->SetRangeUser(min, max);
         tempslice = thishst->Projection(
            1, "oe"); // the "e" option pushes appropriate errors, the "o" makes the projection correct
      }
      // TH2 option
      else if(hst2d) {
         TH2* thishst = static_cast<TH2*>(hstarray->At(i)); // itterating through all the indexes
         thishst->GetXaxis()->SetRangeUser(min, max);
         tempslice = thishst->ProjectionY(); // projecting result of gate into temporary slice
      }

      // iterate through the appropriate bins, transfer values, and take care of error appropriately
      Double_t x, y, oldcontent, newcontent, olderror, newerror;
      Int_t    bin;
      y = i;
      for(Int_t j = 1; j <= bins; j++) {
         x          = tempslice->GetBinCenter(j); // energy
         bin        = newslice->FindBin(x, y);
         newcontent = tempslice->GetBinContent(j); // number of counts
         oldcontent = newslice->GetBinContent(bin);
         newerror   = tempslice->GetBinError(j);
         olderror   = newslice->GetBinError(bin);
         newslice->SetBinContent(bin, oldcontent + newcontent);
         newslice->SetBinError(bin, sqrt(pow(newerror, 2) + pow(olderror, 2)));
      }

      // cleanup
      delete tempslice;
   }

   TH2D* finalslice = Modify2DSlice(newslice, fold, group);
   if(fold || group) {
      delete newslice;
   }

   return finalslice;
}

TH2D* TAngularCorrelation::Modify2DSlice(TH2* hst, Bool_t fold, Bool_t group)
{
   if(!fold && !group) {
      printf("No folding or grouping selected.\n");
      printf("Returning unmodified 2D slice.\n");
      return static_cast<TH2D*>(hst);
   }

   // if desired settings are the same as current settings, then continue on
   // else, change the settings and make new maps.
   if(fold == fFolded && group == fGrouped) {
      // do nothing
   } else {
      GenerateModifiedMaps(fold, group);
   }

   ////consistency check group assignments
   if(group) {
      Int_t group_numbers = fGroups.size();
      Int_t angle_numbers = fAngleMap.size();

      // check for index map
      if(angle_numbers == 0) {
         printf("Angle Map has not been defined yet.\n");
         printf("Need Angle Map to assign groups. \n");
         printf("Bailing out. \n");
         return nullptr;
      }
      // check for group assignments
      if(group_numbers == 0) {
         printf("Groups have not been assigned yet.\n");
         printf("Cannot group Angular Indexes. \n");
         printf("Bailing out. \n");
         return nullptr;
      }
      // consistency check
      if(group_numbers != angle_numbers) {
         if(group_numbers < angle_numbers) {
            printf("Not all angular indexes have been assigned a group. \n");
            printf("Bailing out.\n");
            return nullptr;
         }
         if(group_numbers > angle_numbers) {
            printf("Too many groups have been assigned, not enough angular indexes. \n");
            printf("Bailing out. \n");
            return nullptr;
         }
      }
   }

   // get x-axis parameters
   Int_t bins = hst->GetNbinsX();
   Int_t xmin = hst->GetXaxis()->GetBinLowEdge(1);
   Int_t xmax = hst->GetXaxis()->GetBinUpEdge(bins);

   // get histogram name and title
   const Char_t* hst2dname  = hst->GetName();
   const Char_t* hst2dtitle = hst->GetTitle();

   // get number of y-bins
   Int_t ybins    = hst->GetNbinsY();
   Int_t newybins = GetNumModIndices();

   // initialize the histogram
   TH2D* modified_slice;
   if(static_cast<int>(static_cast<int>((group)) & static_cast<int>(!fold)) != 0) {
      modified_slice = new TH2D(Form("%s_grouped", hst2dname), Form("%s_grouped", hst2dtitle), bins, xmin, xmax,
                                newybins, 0, newybins); // defines slice
   } else if(static_cast<int>(static_cast<int>((fold)) & static_cast<int>(!group)) != 0) {
      modified_slice = new TH2D(Form("%s_folded", hst2dname), Form("%s_folded", hst2dtitle), bins, xmin, xmax, newybins,
                                0, newybins); // defines slice
   } else if(fold && group) {
      modified_slice = new TH2D(Form("%s_grouped_folded", hst2dname), Form("%s_grouped_folded", hst2dtitle), bins, xmin,
                                xmax, newybins, 0, newybins); // defines slice
   } else {
      modified_slice = new TH2D(Form("%s_unknown", hst2dname), Form("%s_unknown", hst2dtitle), bins, xmin, xmax,
                                newybins, 0, newybins); // defines slice
   }

   // loop through the original 2D histograms y-axis...
   for(Int_t i = 0; i < ybins; i++) { // y-axis bin loop
      TH1D* tempslice = nullptr;
      tempslice       = hst->ProjectionX("", i + 1, i + 1);

      Double_t x, y, oldcontent, newcontent, olderror, newerror;
      Int_t    bin;
      // figure out the new index
      y = GetModifiedIndex(i);
      // now loop through the energy axis...
      for(Int_t j = 1; j <= bins; j++) { // x-axis bin loop
         x   = tempslice->GetBinCenter(j);
         bin = modified_slice->FindBin(x, y);
         // pull out the new content / error
         newcontent = tempslice->GetBinContent(j); // number of counts
         newerror   = tempslice->GetBinError(j);
         // pull out the old content / error
         oldcontent = modified_slice->GetBinContent(bin);
         olderror   = modified_slice->GetBinError(bin);
         // re-set new 2D histogram bin with combined value
         modified_slice->SetBinContent(bin, oldcontent + newcontent);
         modified_slice->SetBinError(bin, sqrt(pow(newerror, 2) + pow(olderror, 2)));
      } // end x-axis bin loop

      // cleanup
      delete tempslice;

   } // end y-axis bin loop

   return modified_slice;
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
   hst->GetXaxis()->SetRangeUser(min, max);

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
/// \param[in] visualization Boolean to select whether to draw on a canvas
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
   Int_t       indexmin  = static_cast<Int_t>(hst->GetYaxis()->GetXmin());
   Int_t       indexmax  = static_cast<Int_t>(hst->GetYaxis()->GetXmax());
   const Int_t indexbins = indexmax - indexmin;

   // pull name from hst, modify for 1D hst
   const Char_t* hst2dname  = hst->GetName();
   const Char_t* hst2dtitle = hst->GetTitle();
   const Char_t* hst1dname  = Form("%s_%ikeV", hst2dname, static_cast<Int_t>(peak->GetCentroid()));
   const Char_t* hst1dtitle = Form("%s-%ikeV", hst2dtitle, static_cast<Int_t>(peak->GetCentroid()));

   // initialize histogram to return
   auto* newhst = new TH1D(hst1dname, Form("%s;Angular index;Counts", hst1dtitle), indexbins, indexmin, indexmax);

   // calculate errors on hst (if not already calculated)
   hst->Sumw2();

   // set the range on the energy axis (x)
   // this isn't strictly necessary, but it will make the histograms smaller
   // and visually, easier to see in the diagnostic.
   Double_t minenergy, maxenergy;
   peak->GetRange(minenergy, maxenergy);
   Double_t difference = maxenergy - minenergy;
   minenergy           = minenergy - 0.5 * difference;
   maxenergy           = maxenergy + 0.5 * difference;
   hst->GetXaxis()->SetRangeUser(minenergy, maxenergy);
   Double_t AvgFWHM = 0;
   ///////////////////////////////////////////////////////
   // Fitting and visualization
   ///////////////////////////////////////////////////////

   std::vector<TCanvas*> c;

   TH1D* totalProjection =
      hst->ProjectionX(Form("total_%sproj", hst2dname), hst->GetYaxis()->GetFirst(), hst->GetYaxis()->GetLast());

   // Set initial parameters for peak
   TPeak::SetLogLikelihoodFlag(true);
   peak->SetParameter("centroid", peak->GetCentroid());
   peak->InitParams(totalProjection);
   std::cout<<"initial parameters:"<<std::endl;
   for(int i = 0; i < peak->GetNpar(); i++) {
      double min, max;
      peak->GetParLimits(i, min, max);
      std::cout<<i<<": "<<peak->GetParameter(i)<<"; "<<min<<" - "<<max<<std::endl;
   }
   std::cout<<"==========================="<<std::endl;
   peak->SetParameter(0, totalProjection->GetMaximum() / 2.);
   peak->SetParLimits(0, 0., 2. * totalProjection->GetMaximum());
   peak->SetParameter(1, peak->GetCentroid());
   peak->SetParLimits(1, peak->GetCentroid() - 2., peak->GetCentroid() + 2.);
   peak->SetParameter(2, 0.02 * peak->GetCentroid());
   peak->SetParLimits(2, 0.001 * peak->GetCentroid(), 0.1 * peak->GetCentroid());
   peak->SetParLimits(7, -0.1, 0.1);
   peak->FixParameter(8, 0.);
   peak->SetParameter(9, peak->GetCentroid());
   peak->SetParLimits(9, peak->GetCentroid() - 2, peak->GetCentroid() + 2);
   std::cout<<"Our parameters:"<<std::endl;
   for(int i = 0; i < peak->GetNpar(); i++) {
      double min, max;
      peak->GetParLimits(i, min, max);
      std::cout<<i<<": "<<peak->GetParameter(i)<<"; "<<min<<" - "<<max<<std::endl;
   }
   std::cout<<"=========================="<<std::endl;
   // bool fit = peak->Fit(totalProjection, "LL");
   // if (!fit) continue;
   std::cout<<"Final parameters:"<<std::endl;
   for(int i = 0; i < peak->GetNpar(); i++) {
      double min, max;
      peak->GetParLimits(i, min, max);
      std::cout<<i<<": "<<peak->GetParameter(i)<<"; "<<min<<" - "<<max<<std::endl;
   }
   std::cout<<"=========================="<<std::endl;
   std::cout<<"Peak area = "<<peak->GetArea()<<" +/- "<<peak->GetAreaErr()<<std::endl;
   double peakCentroid = peak->GetParameter(1);
   double sigma        = peak->GetParameter(2);
   std::cout<<"Fixing the centroid to "<<peakCentroid<<", and sigma to "<<sigma<<" (FWHM = "<<2.355 * sigma
            <<")"<<std::endl;
   // loop over the indices
   auto* file = new TFile("Fit_singles.root", "recreate");
   for(Int_t i = 1; i <= indexmax - indexmin; i++) {
      Int_t index = hst->GetYaxis()->GetBinLowEdge(i);
      if(visualization) {
         // find the correct pad
         Int_t canvas = (i - 1) / 16;
         Int_t pad    = (i - 1) % 16;
         if(pad == 0) {
            auto* temp = new TCanvas(Form("c%i", canvas), Form("c%i", canvas), 800, 800);
            temp->Divide(4, 4);
            c.push_back(temp);
         }
         // go to canvas pad
         c[canvas]->cd(pad + 1);
      }

      // pull individual slice
      TH1D* temphst = hst->ProjectionX(Form("%s_proj%i", hst2dname, index), i, i);
      temphst->SetStats(false);
      temphst->SetTitle(Form("%s: angular index %i", hst->GetTitle(), index));
      Set1DSlice(index, temphst);

      // draw the slice to canvas
      if(visualization) {
         Get1DSlice(index)->Draw("pe1");
      }

      // if there are too few counts, continue on (you can fit it manually later)
      if(temphst->Integral() < 50) {
         continue;
      }

      // rename TPeak
      peak->SetName(Form("%s_proj%i_peak", hst2dname, index));

      // Fix FWHM and centroid and set other parameters for single projections rather than total
      peak->InitParams(temphst);
      peak->SetParameter(0, temphst->GetMaximum());
      peak->FixParameter(1, peakCentroid);
      peak->FixParameter(2, sigma);
      peak->SetParLimits(7, -0.1, 0.1);
      peak->FixParameter(8, 0.);
      peak->SetParameter(9, peakCentroid);
      peak->SetParLimits(9, peakCentroid - 2., peakCentroid + 2.);
      peak->SetNpx(1000);
      // fit peak
      peak->Fit(temphst, "LL");
      temphst->Write();

      // fit TPeak
      // Bool_t fitresult = peak->Fit(temphst,"Q");
      // Bool_t fitresult = kFALSE;
      bool fitresult;
      if(visualization) {
         fitresult = peak->Fit(temphst, "Q");
      } else {
         fitresult = peak->Fit(temphst, "Q0");
      }
      if(!fitresult) {
         continue; // if fit failed, continue on to next index
      }
      if(visualization) {
         static_cast<TPeak*>(temphst->GetListOfFunctions()->Last())->Background()->Draw("same");
      }

      Double_t FullWidthHM = peak->GetParameter(2);
      AvgFWHM              = AvgFWHM + FullWidthHM;
      // assign TPeak to fPeaks array
      SetPeak(index, static_cast<TPeak*>(temphst->GetFunction(Form("%s_proj%i_peak", hst2dname, index))));

      // extract area
      Double_t area     = static_cast<TPeak*>(GetPeak(index))->GetArea();
      Double_t area_err = static_cast<TPeak*>(GetPeak(index))->GetAreaErr();
      Double_t chi2     = peak->GetChisquare();
      Double_t ndf      = peak->GetNDF();
      Double_t scale    = TMath::Max(1.0, TMath::Sqrt(chi2 / ndf));
      if(area_err < scale * TMath::Sqrt(area)) {
         printf("Area error was less than the scaled square root of the area.\n");
         printf("This means something is wrong; using scaled square root of area for area error.\n");
         area_err = TMath::Sqrt(area) * scale;
      }

      // fill histogram with area
      newhst->SetBinContent(i, area);
      newhst->SetBinError(i, area_err);
   }

   Double_t TrueFWHM = AvgFWHM / (indexmax - indexmin);
   std::cout<<"True sigma = "<<TrueFWHM<<std::endl;
   ///////////////////////////////////////////////////////
   // Create diagnostic window
   ///////////////////////////////////////////////////////

   // initialize canvas
   auto* c_diag =
      new TCanvas(Form("c_diag_%i", static_cast<Int_t>(peak->GetCentroid())),
                  Form("Diagnostics for fitting %i keV peak", static_cast<Int_t>(peak->GetCentroid())), 800, 800);

   // create plots for chi^2, centroid, and fwhm
   hst2dname = hst->GetName();
   hst1dname = Form("%s_%ikeV", hst2dname, static_cast<Int_t>(peak->GetCentroid()));
   auto* chi2hst =
      new TH1D(Form("%s_chi2", hst1dname), Form("%s: #chi^{2};Angular index;#chi^{2}/NDF value", newhst->GetTitle()),
               indexbins, indexmin, indexmax);
   auto* centroidhst = new TH1D(Form("%s_centroid", hst1dname),
                                Form("%s: Peak centroid;Angular index;Peak centroid (keV)", newhst->GetTitle()),
                                indexbins, indexmin, indexmax);
   auto* fwhmhst = new TH1D(Form("%s_fwhm", hst1dname), Form("%s: FWHM;Angular index;FWHM (keV)", newhst->GetTitle()),
                            indexbins, indexmin, indexmax);

   // assign histogram to fIndexCorrelation
   fIndexCorrelation = newhst;
   fChi2             = chi2hst;
   fCentroid         = centroidhst;
   fFWHM             = fwhmhst;

   UpdateDiagnostics();
   DisplayDiagnostics(c_diag);

   ///////////////////////////////////////////////////////
   // Clean-up
   ///////////////////////////////////////////////////////

   file->Close();
   delete file;
   return fIndexCorrelation;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates graph of counts vs. cos(theta) from histogram of counts vs. angular index
///
/// \param[in] hst One-dimensional histogram of angular index vs. counts
/// \param[in] fold boolean to select whether angles are folded at 90 degree
/// \param[in] group boolean to select whether angles are grouped
///

TGraphAsymmErrors* TAngularCorrelation::CreateGraphFromHst(TH1* hst, Bool_t fold, Bool_t group)
{
   if(!fold && !group) {
      Int_t check = fAngleMap.size();
      if(check == 0) {
         printf("Angles have not been assigned. \n");
         printf("Therefore cannot create graph. \n");
         return nullptr;
      }
   } else {
      // compare fold and group
      if(fold == fFolded && group == fGrouped) {
         // do nothing
      } else {
         GenerateModifiedMaps(fold, group);
      }

      Int_t check = fModifiedAngles.size();
      if(check == 0) {
         printf("Modified angles have not been assigned. \n");
         printf("Therefore cannot create graph. \n");
         return nullptr;
      }

      if(CheckModifiedHistogram(hst)) {
         // do nothing
      } else {
         return nullptr;
      }
   }

   auto* graph = new TGraphAsymmErrors();
   Int_t n     = hst->GetNbinsX();

   for(Int_t i = 1; i <= n; i++) { // bin number loop

      // get index number
      Int_t index = hst->GetXaxis()->GetBinLowEdge(i);

      // get associated angle
      Double_t angle = 0;
      if(!fold && !group) {
         angle = fAngleMap[index];
      } else {
         angle = fModifiedAngles[index];
      }

      // get counts and error
      Double_t y = hst->GetBinContent(i);
      if(y == 0) {
         continue;
      }
      Double_t yerr = hst->GetBinError(i);

      // fill graph with point
      Int_t graphn = graph->GetN();
      graph->SetPoint(graphn, TMath::Cos(angle), y);
      graph->SetPointError(graphn, 0, 0, yerr, yerr);
   } // bin number loop end

   // set title on graph
   graph->SetTitle(Form("%s;cos(#theta);Counts", hst->GetTitle()));

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
   if(arraynum1 == 0 || arraynum2 == 0) {
      printf("Array numbers usually begin at 1 - unless you have programmed\n");
      printf("it differently explicitly, don't trust this output.\n");
   }
   if(fIndexMap.count(arraynum1) == 0) {
      printf("Error: array number %i is not present in the index map.\n", arraynum1);
      return -1;
   }
   if(fIndexMap[arraynum1].count(arraynum2) == 0) {
      printf("Error: element [%i][%i] is not present in the index map.\n", arraynum1, arraynum2);
      return -1;
   }
   // Array numbers start counting at 1
   // Indices of this array start at 0
   return fIndexMap[arraynum1][arraynum2];
}

////////////////////////////////////////////////////////////////////////////////
/// Checks that maps are consistent with each other
/// need to input whether the angles are folded, grouped, or folded and grouped

Bool_t TAngularCorrelation::CheckMaps(Bool_t fold, Bool_t group)
{
   Bool_t result = kTRUE; // result to return
   if(!fold && !group) {
      if(fAngleMap.size() != fWeights.size()) {
         printf("fAngleMap and fWeights do not have the same size.\n");
         printf("fAngleMap size is: %i\n", static_cast<Int_t>(fAngleMap.size()));
         printf("fWeights size is: %i\n", static_cast<Int_t>(fWeights.size()));
         result = kFALSE;
      }
   } else {
      if(fModifiedAngles.size() != fModifiedWeights.size()) {
         printf("fModifiedAngles and fModifiedWeights do not have the same size.\n");
         printf("fModifiedAngles size is: %i\n", static_cast<Int_t>(fModifiedAngles.size()));
         printf("fModifiedWeights size is: %i\n", static_cast<Int_t>(fModifiedWeights.size()));
         result = kFALSE;
      }
   }
   return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map used to construct angular indices
///

void TAngularCorrelation::PrintIndexMap()
{
   Int_t size = fIndexMapSize;
   if(size == 0) {
      printf("Indexes have not been assigned yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }

   for(Int_t i = 1; i <= size; i++) {
      printf("-----------------------------------------------------\n");
      printf("|| Array number 1 | Array number 2 | Angular index ||\n");
      printf("-----------------------------------------------------\n");
      for(Int_t j = 1; j <= size; j++) {
         if(GetAngularIndex(i, j) == -1) {
            continue;
         }
         printf("|| %-14i | %-14i | %-13i ||\n", i, j, GetAngularIndex(i, j));
      }
   }
   printf("-----------------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle vs. weight
///

void TAngularCorrelation::PrintWeights()
{
   Int_t size        = fAngleMap.size();
   Int_t weight_size = fWeights.size();
   if(size == 0) {
      printf("The angle map hasn't been created yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }

   if(weight_size == 0) {
      printf("The weights haven't been calculated yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }

   printf("--------------------------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  |  Weight  ||\n");
   printf("--------------------------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %-13i  |  %-19.4f  |  %-6.0f  ||\n", i, GetAngleFromIndex(i), GetWeightFromIndex(i));
   }
   printf("--------------------------------------------------------\n");
}
////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle vs. group
///

void TAngularCorrelation::PrintGroupIndexMap()
{
   Int_t size       = fAngleMap.size();
   Int_t group_size = fGroups.size();
   if(size == 0) {
      printf("The angle map hasn't been created yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }
   if(group_size == 0) {
      printf("The groups haven't been assigned yet.\n");
      printf("Therefore, can't print.\n");
      return;
   }

   printf("-------------------------------------\n");
   printf("||  Angular index  |  Group index  ||\n");
   printf("-------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %13i  |  %11i  ||\n", i, GetGroupFromIndex(i));
   }
   printf("-------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of group vs. group weight vs. average group angle
///

void TAngularCorrelation::PrintGroupAngleMap()
{
   Int_t size       = GetNumGroups();
   Int_t angle_size = fGroupAngles.size();
   if(size == 0) {
      printf("The number of groups haven't been determined yet. \n");
      printf("Therefore, can't print.\n");
      return;
   }
   if(angle_size == 0) {
      printf("The angles haven't been assigned yet. \n");
      printf("Therefore, can't print.\n");
      return;
   }
   printf("-----------------------------------\n");
   printf("||  Group index  |  Angle (rad)  ||\n");
   printf("-----------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %11i  |  %12.4f  ||\n", i, GetGroupAngleFromIndex(i));
   }
   printf("-----------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. opening angle
///

void TAngularCorrelation::PrintAngleMap()
{
   Int_t size = fAngleMap.size();
   if(size == 0) {
      printf("Folded angles have not been assigned yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }

   printf("---------------------------------------------\n");
   printf("||  Angular index  |  Opening angle (rad)  ||\n");
   printf("---------------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %-13i  |  %-19.4f  ||\n", i, GetAngleFromIndex(i));
   }
   printf("---------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of Folded index vs. opening angle
///

void TAngularCorrelation::PrintModifiedAngleMap()
{
   Int_t size = fModifiedAngles.size();

   if(size == 0) {
      printf("Folded angles have not been assigned yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }

   printf("--------------------------------------\n");
   printf("||  Modified index  |  Angle (rad)  ||\n");
   printf("--------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %14i  |  %11.4f  ||\n", i, GetModifiedAngleFromIndex(i));
   }
   printf("--------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of Folded index vs. opening angle
///

void TAngularCorrelation::PrintModifiedWeights()
{
   Int_t size        = fModifiedAngles.size();
   Int_t weight_size = fModifiedWeights.size();

   if(size == 0) {
      printf("Modified angles have not been assigned yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }
   if(weight_size == 0) {
      printf("Modified weights have not been generated yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }

   printf("-------------------------------------------------\n");
   printf("||  Modified index  |  Angle (rad)  |  Weight  ||\n");
   printf("-------------------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %14i  |  %11.4f  |  %6i  ||\n", i, GetModifiedAngleFromIndex(i), GetModifiedWeight(i));
   }
   printf("-------------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Prints map of angular index vs. Folded Index

void TAngularCorrelation::PrintModifiedIndexMap()
{
   Int_t size = fModifiedIndices.size();
   if(size == 0) {
      printf("Modified indices have not been assigned yet.\n");
      printf("Therefore cannot print map.\n");
      return;
   }

   printf("----------------------------------------\n");
   printf("||  Angular index  |  Modified index  ||\n");
   printf("----------------------------------------\n");
   for(Int_t i = 0; i < size; i++) {
      printf("||  %13i  |  %14i  ||\n", i, GetModifiedIndex(i));
   }
   printf("----------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of angular index vs. opening angle.
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
///

std::vector<Double_t> TAngularCorrelation::GenerateAngleMap(std::vector<Int_t>& arraynumbers,
                                                            std::vector<Int_t>& distances)
{

   std::vector<Double_t> map; // vector to return

   // basic consistency check
   const Int_t size = arraynumbers.size();
   if(size != static_cast<Int_t>(distances.size())) {
      printf("Lengths of array number and distance vectors are inconsistent.\n");
      printf("Array number vector size: %i\n", size);
      printf("Distance vector size: %i\n", static_cast<Int_t>(distances.size()));
   }

   // loop through array numbers (a list of crystals in the array)
   for(Int_t i = 0; i < size; i++) {
      // identify detector and crystal numbers
      Int_t detector1 = (arraynumbers[i] - 1) / 4 + 1;
      Int_t crystal1  = (arraynumbers[i] - 1) % 4;
      // now we will loop through all *unique* combinations
      // we can start from j=i here, because j<i will only produce duplicates
      for(Int_t j = i; j < size; j++) {
         // identify detector and crystal numbers
         Int_t    detector2 = (arraynumbers[j] - 1) / 4 + 1;
         Int_t    crystal2  = (arraynumbers[j] - 1) % 4;
         TVector3 positionone =
            TGriffin::GetPosition(detector1, crystal1, distances[i]); // distance is in mm, usually 110, 145, or 160
         TVector3 positiontwo =
            TGriffin::GetPosition(detector2, crystal2, distances[j]); // distance is in mm, usually 110, 145, or 160
         Double_t angle          = positionone.Angle(positiontwo);    // in radians
         Bool_t   alreadyclaimed = kFALSE;
         for(double m : map) {
            if(TMath::Abs(angle - m) < 0.00005) {
               alreadyclaimed = kTRUE;
               break;
            }
         }
         if(!alreadyclaimed) {
            map.push_back(angle);
         }
      }
   }

   // sort the map
   std::sort(map.begin(), map.end());

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
///
/// This function is called by GenerateMaps()

std::vector<Int_t> TAngularCorrelation::GenerateWeights(std::vector<Int_t>& arraynumbers, std::vector<Int_t>&,
                                                        std::map<Int_t, std::map<Int_t, Int_t>>& indexmap)
{
   std::vector<Int_t> weights; // vector to return

   // get array number size
   Int_t size = arraynumbers.size();

   // find maximum angular index
   Int_t max = 0;
   for(Int_t i = 0; i < size; i++) {
      for(Int_t j = 0; j < size; j++) {
         if(indexmap[i][j] > max) {
            max = indexmap[i][j];
         }
      }
   }

   // initialize vector
   for(Int_t i = 0; i <= max; i++) {
      weights.push_back(0);
   }

   // loop through array numbers (a list of crystals in the array)
   for(Int_t i = 0; i < size; i++) {
      if(arraynumbers[i] < 1 || arraynumbers[i] > 64) {
         printf("%i is not a good array number.\n", arraynumbers[i]);
         printf("Skipping... you'll probably get some errors.\n");
         continue;
      }
      // here, we want all combinations for the indices, so we start from j=0
      for(Int_t j = 0; j < size; j++) {
         Int_t index      = indexmap[arraynumbers[i]][arraynumbers[j]];
         Int_t old_weight = weights[index];
         Int_t new_weight = old_weight + 1;
         weights[index]   = new_weight;
      }
   }

   return weights;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of modified weights vs. modified index
///
/// \param[in] modindices vector that converts angular index to modified index
/// \param[in] weights vector of weights for angular index
///
/// This function is called by GenerateMaps() and GenerateGroupMaps()

std::vector<Int_t> TAngularCorrelation::GenerateModifiedWeights(std::vector<Int_t>& modindices,
                                                                std::vector<Int_t>& weights)
{
   std::vector<Int_t> modifiedweights; // vector to return

   // get weights size
   Int_t size = weights.size();

   // find total number of modified indices
   Int_t max = 0;
   for(Int_t i = 0; i < size; i++) {
      if(modindices[i] > max) {
         max = modindices[i];
      }
   }

   // fill modifiedweights with zeros
   for(Int_t i = 0; i <= max; i++) {
      modifiedweights.push_back(0);
   }

   // iterate over angular modindices to look for same groups
   for(Int_t i = 0; i < size; i++) {
      Int_t thisIndex            = modindices[i];
      modifiedweights[thisIndex] = modifiedweights[thisIndex] + weights[i];
   }

   return modifiedweights;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates map of angle pair vs. angular index
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
/// \param[in] anglemap Angle map (probably created with GenerateAngleMap
///
/// This function is called by GenerateMaps()
///
std::map<Int_t, std::map<Int_t, Int_t>> TAngularCorrelation::GenerateIndexMap(std::vector<Int_t>& arraynumbers,
                                                                              std::vector<Int_t>&    distances,
                                                                              std::vector<Double_t>& anglemap)
{
   // initialize map
   std::map<Int_t, std::map<Int_t, Int_t>> indexmap;

   // get arraynumbers size
   Int_t size = arraynumbers.size();

   // get angle map size
   Int_t mapsize = anglemap.size();

   // loop through array numbers (a list of crystals in the array)
   for(Int_t i = 0; i < size; i++) {
      if(arraynumbers[i] < 1 || arraynumbers[i] > 64) {
         printf("%i is not a good array number.\n", arraynumbers[i]);
         printf("Skipping... you'll probably get some errors.\n");
         continue;
      }

      // identify detector and crystal numbers
      Int_t    detector1 = (arraynumbers[i] - 1) / 4 + 1;
      Int_t    crystal1  = (arraynumbers[i] - 1) % 4;
      TVector3 positionone =
         TGriffin::GetPosition(detector1, crystal1, distances[i]); // distance is in mm, usually 110, 145, or 160

      // here, we want all combinations for the indices, so we start from j=0
      for(Int_t j = 0; j < size; j++) {
         // identify detector and crystal numbers
         Int_t    detector2 = (arraynumbers[j] - 1) / 4 + 1;
         Int_t    crystal2  = (arraynumbers[j] - 1) % 4;
         TVector3 positiontwo =
            TGriffin::GetPosition(detector2, crystal2, distances[j]); // distance is in mm, usually 110, 145, or 160
         Double_t angle = positionone.Angle(positiontwo);             // in radians
         for(Int_t m = 0; m < mapsize; m++) {
            if(TMath::Abs(angle - anglemap[m]) < 0.00005) {
               Int_t index1 = arraynumbers[i];
               Int_t index2 = arraynumbers[j];
               if(index1 == 0 || index2 == 0) {
                  printf("found array number of zero?\n");
               }
               indexmap[index1][index2] = m;
               break;
            }
         }
      }
   }

   return indexmap;
}

////////////////////////////////////////
/// Check Groups for Angular Indexes
///
/// \param[in] group vector of assigned groups for angular indexes (this is user input)
///
/// This function is called by GenerateGroupMaps()
///

Bool_t TAngularCorrelation::CheckGroups(std::vector<Int_t>& group)
{

   // get number of group entries
   Int_t size = group.size();

   // basic consistency check
   if(size != fNumIndices) {
      printf("The group list is inconsistent with the number of angular indices.\n");
      printf("Number of groups: %i\n", size);
      printf("Number of angular indices: %i\n", fNumIndices);
      return kFALSE;
   }

   return kTRUE;
}

////////////////////////////////////////
/// Returns the number of groups
///

Int_t TAngularCorrelation::GetNumGroups()
{
   Int_t max = 0;
   for(int fGroup : fGroups) {
      if(fGroup > max) {
         max = fGroup;
      }
   }
   return max + 1;
}

////////////////////////////////////////
/// Returns the number of groups
///

Int_t TAngularCorrelation::GetNumModIndices()
{
   Int_t max = 0;
   for(int fModifiedIndice : fModifiedIndices) {
      if(fModifiedIndice > max) {
         max = fModifiedIndice;
      }
   }
   return max + 1;
}

////////////////////////////////////////
/// Check angles for groups
///
/// \param[in] groupangles vector (user input)
///
/// This function is called by GenerateGroupMaps()
///

Bool_t TAngularCorrelation::CheckGroupAngles(std::vector<Double_t>& groupangles)
{
   std::vector<Double_t> groupangle; // vector to return

   // get number of group entries
   Int_t size      = groupangles.size();
   Int_t numgroups = GetNumGroups();

   // basic consistency check
   if(size != numgroups) {
      printf("Not all groups have been assigned an angle.\n");
      return kFALSE;
   }

   return kTRUE;
}

////////////////////////////////////////
/// Generate Folded Angles
/// gives angles out for each folded index in radians
///
/// \param[in] anglemap (can be for grouped or ungrouped anglular indexes)
///
/// This function is called by GenerateMaps() and GenerateGroupMaps()
///

std::vector<Double_t> TAngularCorrelation::GenerateFoldedAngles(std::vector<Double_t>& anglemap)
{
   std::vector<Double_t> folds; // vector to return

   // get size
   Int_t size = anglemap.size();

   // consistancy check
   if(size == 0) {
      printf("Angles have not been assigned yet. \n");
      printf("Therefore cannot fold indexes. \n");
      return folds;
   }

   // declare fold array and fill with zeros
   std::vector<Double_t> foldArray;
   for(Int_t i = 0; i < size; i++) {
      foldArray.push_back(0);
   }

   // fill fold array with angle values
   for(Int_t i = 0; i < size; i++) {
      foldArray[i] = anglemap[i];
   }

   // Iterate through fold array
   for(Int_t i = 0; i < size; i++) {
      Double_t fold_angle     = foldArray[i];
      Bool_t   alreadyclaimed = kFALSE;

      for(double fold : folds) {
         if(TMath::Abs(TMath::Sin(fold_angle) - TMath::Sin(fold)) <
            0.000005) { // look for duplicated angles in the fold array
            alreadyclaimed = kTRUE;
            break;
         }
      }
      if(!alreadyclaimed) {
         folds.push_back(fold_angle);
      }
   }

   std::sort(folds.begin(), folds.end());

   return folds;
}

////////////////////////////////////////
/// Generated Folded Indexes
///
/// \param[in] folds vector of the folded angles
/// \param[in] anglemap vector of the unfolded angles
///
/// This function is called by GenerateMaps() and by GenerateGroupMaps()
///

std::vector<Int_t> TAngularCorrelation::GenerateFoldedIndices(std::vector<Double_t>& folds,
                                                              std::vector<Double_t>& anglemap)
{
   std::vector<Int_t> fold_indexes; // vector to return

   // get sizes of fold and angle vectors
   Int_t fold_size  = folds.size();
   Int_t angle_size = anglemap.size();

   // itterate through angle map to find abs(cos(angle)
   for(Int_t i = 0; i < angle_size; i++) {
      Double_t angle     = anglemap[i];
      Double_t sin_angle = TMath::Sin(angle);

      // itterate through folds
      for(Int_t j = 0; j < fold_size; j++) {
         Double_t rad_angle  = folds[j];
         Double_t fold_angle = TMath::Sin(rad_angle);
         if(TMath::Abs(fold_angle - sin_angle) < 0.000005) { // compare abs(cos(angle)) to fold_Angles to find matches
            fold_indexes.push_back(j);                       // assign folds
            break;
         }
      }
   }

   return fold_indexes;
}
////////////////////////////////////////////////////////////////////////////////
/// Creates maps of angle pair vs. angular index and angular index vs. opening angle.
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
///

Int_t TAngularCorrelation::GenerateMaps(std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances)
{
   // basic consistency check
   const Int_t size = arraynumbers.size();
   if(size != static_cast<Int_t>(distances.size())) {
      printf("Lengths of array number and distance vectors are inconsistent.\n");
      printf("Array number vector size: %i\n", size);
      printf("Distance vector size: %i\n", static_cast<Int_t>(distances.size()));
   }

   // clear vector map
   fAngleMap.clear();

   // find maximum array number (which will be the index map size)
   fIndexMapSize = 0;
   for(Int_t i = 0; i < size; i++) {
      if(arraynumbers[i] > fIndexMapSize) {
         fIndexMapSize = arraynumbers[i];
      }
   }

   // generate maps
   fAngleMap   = GenerateAngleMap(arraynumbers, distances);
   fNumIndices = fAngleMap.size();
   fIndexMap   = GenerateIndexMap(arraynumbers, distances, fAngleMap);
   fWeights    = GenerateWeights(arraynumbers, distances, fIndexMap);

   return fNumIndices;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates maps for modified indices with some combination of folding or grouping
///
/// \param[in] fold
/// \param[in] group
///
Int_t TAngularCorrelation::GenerateModifiedMaps(Bool_t fold, Bool_t group)
{
   if(group) {
      if(static_cast<Int_t>(fGroups.size()) != fNumIndices) {
         printf("The groups are not set up properly.\n");
         printf("Cannot create grouped maps.\n");
         printf("Please use AssignGroupMaps first.\n");
         return 0;
      }
   }
   fModifiedAngles  = GenerateModifiedAngles(fold, group);
   Int_t size       = fModifiedAngles.size();
   fModifiedIndices = GenerateModifiedIndices(fold, group);
   fModifiedWeights = GenerateModifiedWeights(fModifiedIndices, fWeights);

   return size;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates maps for Grouped Angular Indexes
/// Including group angle versus group index and angular index versus group index
///
/// \param[in] arraynumbers Vector of array numbers used in this experiment
/// \param[in] distances Vector of detector distances for those array numbers
/// \param[in] group Vector (user input)
/// \param[in] groupangles Vector (user input)
///
Int_t TAngularCorrelation::GenerateGroupMaps(std::vector<Int_t>& arraynumbers, std::vector<Int_t>& distances,
                                             std::vector<Int_t>& group, std::vector<Double_t>& groupangles)
{
   // basic consistency check
   const Int_t size = arraynumbers.size();
   if(size != static_cast<Int_t>(distances.size())) {
      printf("Lengths of array number and distance vectors are inconsistent.\n");
      printf("Array number vector size: %i\n", size);
      printf("Distance vector size: %i\n", static_cast<Int_t>(distances.size()));
   }

   // clear vector map
   fAngleMap.clear();

   // find maximum array number (which will be the index map size)
   fIndexMapSize = 0;
   for(Int_t i = 0; i < size; i++) {
      if(arraynumbers[i] > fIndexMapSize) {
         fIndexMapSize = arraynumbers[i];
      }
   }

   // generate maps
   fAngleMap   = GenerateAngleMap(arraynumbers, distances);
   fNumIndices = fAngleMap.size();
   fIndexMap   = GenerateIndexMap(arraynumbers, distances, fAngleMap);
   fWeights    = GenerateWeights(arraynumbers, distances, fIndexMap);
   AssignGroupMaps(group, groupangles);
   //   fGroupWeights = GenerateModifiedWeights(group, fWeights);
   //   fFoldedGroupAngles = GenerateFoldedAngles(fGroupAngles);
   //   fFoldedGroupIndexes = GenerateFoldedIndices(fFoldedGroupAngles, fGroupAngles);
   //   fFoldedGroupWeights = GenerateModifiedWeights(fFoldedGroupIndexes, fGroupWeights);
   return fNumIndices;
}

////////////////////////////////////////////////////////////////////////////////
/// Assigns maps for Grouped Angular Indexes
/// Including group angle versus group index and angular index versus group index
///
/// \param[in] group Vector (user input)
/// \param[in] groupangles Vector (user input)
///

Int_t TAngularCorrelation::AssignGroupMaps(std::vector<Int_t>& group, std::vector<Double_t>& groupangles)
{
   // generate maps
   if(CheckGroups(group)) {
      fGroups = group;
   }
   if(CheckGroupAngles(groupangles)) {
      fGroupAngles = groupangles;
   }
   return GetNumGroups();
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
/// For more detailed configurations, please use GenerateMaps(std::vector<Int_t> &arraynumbers, std::vector<Int_t>
/// &distances)
///

Int_t TAngularCorrelation::GenerateMaps(Int_t detectors, Int_t distance)
{
   std::vector<Int_t> array_numbers;
   std::vector<Int_t> distances;

   if(detectors == 16) {
      printf("Generating maps for full array setup.\n");
      for(Int_t i = 1; i <= 64; i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if(detectors == 15) {
      printf("Generating maps for full array setup, less detector 13.\n");
      for(Int_t i = 1; i <= 64; i++) {
         if(i >= 49 && i <= 52) {
            continue; // no detector 13
         }
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if(detectors == 12) {
      printf("Generating maps for detectors 5-16.\n");
      for(Int_t i = 17; i <= 64; i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if(detectors == 11) {
      printf("Generating maps for detectors 5-16, less detector 13.\n");
      for(Int_t i = 17; i <= 64; i++) {
         if(i >= 49 && i <= 52) {
            continue; // no detector 13
         }
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else if(detectors == 8) {
      printf("Generating maps for the corona only, detectors 5-12.\n");
      for(Int_t i = 17; i <= 48; i++) {
         array_numbers.push_back(i);
         distances.push_back(distance);
      }
   } else {
      printf("This option isn't coded. Please use the more general GenerateMap(std::vector<Int_t> &arraynumbers, "
             "std::vector<Int_t> &distances)function.\n");
      return 0;
   }

   Int_t val = GenerateMaps(array_numbers, distances);

   return val;
}

////////////////////////////////////////////////////////////////////
/// Generates modified indices
///
/// \param[in] fold boolean indicating whether or not to fold the indices
/// \param[in] group boolean indicating whether or not to group the indices

std::vector<Int_t> TAngularCorrelation::GenerateModifiedIndices(Bool_t fold, Bool_t group)
{
   std::vector<Int_t> modindices;

   if(fNumIndices == 0) {
      printf("You haven't set any angular indices yet.\n");
      printf("Returning empty vector from GenerateModifiedIndices.\n");
      return modindices;
   }
   // make sure the modified angles are set appropriately
   if(fold != fFolded || group != fGrouped) {
      printf("Please call GenerateModifiedAngles before calling GenerateModifiedIndices.\n");
      printf("Returning empty vector from GenerateModifiedIndices.\n");
      return modindices;
   }

   if(group) {
      if(static_cast<Int_t>(fGroups.size()) != fNumIndices) {
         printf("The groups are not set up properly.\n");
         printf("Returning empty vector from GenerateModifiedIndices.\n");
         return modindices;
      }
   }

   if(!group && (!fold)) {
      // well then why are you doing this in the first place?
      // do nothing
   } else if(!fold && group) {
      modindices = fGroups;
   } else if(fold && group) {
      // group angles won't be directly comparable to the normal angle map
      // so we have to make another, temporary one.
      std::vector<Double_t> groupedangles;
      for(Int_t i = 0; i < fNumIndices; i++) {
         Int_t    thisgroup = fGroups[i];
         Double_t thisangle = fGroupAngles[thisgroup];
         groupedangles.push_back(thisangle);
      }
      modindices = GenerateFoldedIndices(fModifiedAngles, groupedangles);
   } else if(fold) {
      modindices = GenerateFoldedIndices(fModifiedAngles, fAngleMap);
   }

   return modindices;
}

////////////////////////////////////////////////////////////////////
/// Clears the modified arrays
///

void TAngularCorrelation::ClearModifiedMaps()
{
   fModifiedIndices.clear();
   fModifiedAngles.clear();
   fModifiedWeights.clear();
}

////////////////////////////////////////////////////////////////////
/// Generates modified angles
///
/// \param[in] fold boolean indicating whether or not to fold the indices
/// \param[in] group boolean indicating whether or not to group the indices

std::vector<Double_t> TAngularCorrelation::GenerateModifiedAngles(Bool_t fold, Bool_t group)
{
   std::vector<Double_t> modangles;

   // checking to see that we have regular indices
   if(fNumIndices == 0) {
      printf("You haven't set any angular indices yet.\n");
      printf("Aborting.\n");
      return modangles;
   }

   // checking for groups set up
   if(static_cast<Int_t>(fGroups.size()) != fNumIndices) {
      printf("The groups are not set up properly.\n");
      printf("Returning empty vector from GenerateModifiedAngles.\n");
      return modangles;
   }

   // checking for group angle setup
   if(group && static_cast<Int_t>(fGroupAngles.size()) != GetNumGroups()) {
      printf("Group angles aren't set up properly.\n");
      printf("Returning empty vector from GenerateModifiedAngles.\n");
      return modangles;
   }

   ClearModifiedMaps();
   printf("Changing modification conditions to:\n");
   if(fold) {
      printf("Folded: yes\n");
   } else {
      printf("Folded: no\n");
   }
   if(group) {
      printf("Grouped: yes\n");
   } else {
      printf("Grouped: no\n");
   }
   fFolded  = fold;
   fGrouped = group;

   if(!group && (!fold)) {
      // well then why are you doing this in the first place?
      // do nothing
   } else if(!fold && group) {
      modangles = fGroupAngles;
   } else if(static_cast<int>(static_cast<int>((fold)) & static_cast<int>(!group)) != 0) {
      modangles = GenerateFoldedAngles(fAngleMap);
   } else if(fold && group) {
      modangles = GenerateFoldedAngles(fGroupAngles);
   }

   return modangles;
}

////////////////////////////////////////////////////////////////////
/// Updates index correlation based on peak array
///

void TAngularCorrelation::UpdateIndexCorrelation()
{
   // loop over quantities in map
   for(auto& fPeak : fPeaks) {
      Int_t index = fPeak.first;
      Int_t bin   = (GetIndexCorrelation())->FindBin(index);

      // extract area
      TPeak* peak = static_cast<TPeak*>(GetPeak(index));
      if(peak == nullptr) {
         return;
      }
      Double_t area     = peak->GetArea();
      Double_t area_err = peak->GetAreaErr();
      Double_t chi2     = peak->GetChisquare();
      Double_t ndf      = peak->GetNDF();
      Double_t scale    = TMath::Max(1.0, TMath::Sqrt(chi2 / ndf));
      if(area_err < scale * TMath::Sqrt(area)) {
         printf("Area error was less than the scaled square root of the area.\n");
         printf("This means something is wrong; using scaled square root of area for area error.\n");
         area_err = TMath::Sqrt(area) * scale;
      }

      // fill histogram with area
      static_cast<TH1D*>(GetIndexCorrelation())->SetBinContent(bin, area);
      static_cast<TH1D*>(GetIndexCorrelation())->SetBinError(bin, area_err);
   }

   return;
}

////////////////////////////////////////////////////////////////////
/// Updates index correlation based on peak array
///

void TAngularCorrelation::ScaleSingleIndex(TH1* hst, Int_t index, Double_t factor)
{

   // get old values
   Double_t old_value = hst->GetBinContent(index + 1);
   Double_t old_error = hst->GetBinError(index + 1);

   // set new values
   Double_t new_value = old_value * factor;
   printf("old value is %f multiplied by %f is %f\n", old_value, factor, new_value);
   Double_t new_area_err = old_error * factor;

   // fill histogram with new values
   hst->SetBinContent(index + 1, new_value);
   hst->SetBinError(index + 1, new_area_err);
}
////////////////////////////////////////////////////////////////////////////////
/// Updates diagnostics based on peak array
///

void TAngularCorrelation::UpdateDiagnostics()
{
   // loop over quantities in map
   for(auto& fPeak : fPeaks) {
      Int_t index = fPeak.first;
      Int_t bin   = (GetIndexCorrelation())->FindBin(index);

      // extract pertinent values from TPeaks
      TPeak* peak = static_cast<TPeak*>(GetPeak(index));
      if(peak == nullptr) {
         return;
      }
      Double_t chi2         = peak->GetChisquare();
      Double_t NDF          = static_cast<Double_t>(peak->GetNDF());
      Double_t centroid     = peak->GetCentroid();
      Double_t centroid_err = peak->GetCentroidErr();
      Double_t fwhm         = peak->GetFWHM();
      Double_t fwhm_err     = peak->GetFWHMErr();

      // fill histogram with values
      static_cast<TH1D*>(GetChi2Hst())->SetBinContent(bin, chi2 / NDF);
      static_cast<TH1D*>(GetCentroidHst())->SetBinContent(bin, centroid);
      static_cast<TH1D*>(GetCentroidHst())->SetBinError(bin, centroid_err);
      static_cast<TH1D*>(GetFWHMHst())->SetBinContent(bin, fwhm);
      static_cast<TH1D*>(GetFWHMHst())->SetBinError(bin, fwhm_err);
   }

   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Fits slice with new peak and updates the index correlation
///
/// \param[in] index angular index
/// \param[in] peak Tpeak to be used for fitting

void TAngularCorrelation::UpdatePeak(Int_t index, TPeak* peak) // sometimes this function will cause grsisort to crash
                                                               // when I try to re-fit a bad peak, we should add in a
                                                               // safe-gaurd to make the function return if the peak
                                                               // cannot be re-fit
{
   // create canvas
   new TCanvas(Form("peak%iupdate", index), Form("Peak %i update", index), 400, 400);

   // get histogram
   TH1D* temphst = Get1DSlice(index);

   // adjust range
   Double_t minenergy, maxenergy;
   peak->GetRange(minenergy, maxenergy);
   Double_t difference = maxenergy - minenergy;
   minenergy           = minenergy - 0.5 * difference;
   maxenergy           = maxenergy + 0.5 * difference;
   temphst->GetXaxis()->SetRangeUser(minenergy, maxenergy);

   // fit peak
   // peak->SetName(name);
   peak->Fit(Get1DSlice(index), "");
   TPeak* hstpeak = static_cast<TPeak*>(temphst->GetListOfFunctions()->Last());

   // push new peak
   SetPeak(index, hstpeak);
   UpdateIndexCorrelation();
   UpdateDiagnostics();
}

////////////////////////////////////////////////////////////////////////////////
/// Returns peak, if it exists
///
/// \param[in] index angular index
///

TPeak* TAngularCorrelation::GetPeak(Int_t index)
{
   if(fPeaks[index] == nullptr) {
      printf("No peak exists for index %i. Returning 0x0.\n", index);
      return nullptr;
   }
   return fPeaks[index];
}

////////////////////////////////////////////////////////////////////////////////
/// Compares 1D histogram to current AC modified settings
///
/// \param[in] hst histogram
///
Bool_t TAngularCorrelation::CheckModifiedHistogram(TH1* hst)
{
   Int_t hstbins    = hst->GetNbinsX();
   Int_t modindices = GetNumModIndices();

   if(hstbins != modindices) {
      printf("The histogram %s does not have the same number of bins as the current settings.\n", hst->GetName());
      printf("Bins in the histogram: %i\n", hstbins);
      printf("Number of modified indices: %i\n\n", modindices);
      PrintModifiedConditions();
      return kFALSE;
   }

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Prints current folding and grouping settings
///
void TAngularCorrelation::PrintModifiedConditions()
{
   printf("Current modification conditions:\n");
   if(fFolded) {
      printf("Folded: yes\n");
   } else {
      printf("Folded: no\n");
   }
   if(fGrouped) {
      printf("Grouped: yes\n");
   } else {
      printf("Grouped: no\n");
   }
   return;
}

////////////////////////////////////////////////////////////////////////////////
/// Divides histogram by weights listed in weight array
///
/// \param[in] hst histogram
/// \param[in] fold boolean indicating whether or not to fold the indices
/// \param[in] group boolean indicating whether or not to group the indices
///

TH1D* TAngularCorrelation::DivideByWeights(TH1* hst, Bool_t fold, Bool_t group)
{

   if(!fold && !group) {
      Int_t size = GetWeightsSize();

      // consistency/stability checks
      if(size == 0) {
         printf("You haven't created the weights yet. Please use the GenerateMaps function to do so.\n");
         return nullptr;
      }
      if(size != hst->GetNbinsX()) {
         printf("Warning: size of weights array is different than number of bins in %s\n", hst->GetName());
      }
      // this loop is just for checking to make sure all indices are in the weight vector
      for(Int_t i = 1; i <= hst->GetNbinsX(); i++) {
         Int_t index = hst->GetBinLowEdge(i);
         if(index >= size) {
            printf("Indices in histogram %s go beyond size of weights array. Aborting.\n", hst->GetName());
            return nullptr;
         }
      }
   } else {
      // compare fold and group
      if(fold == fFolded && group == fGrouped) {
         // do nothing
      } else {
         GenerateModifiedMaps(fold, group);
      }

      if(CheckModifiedHistogram(hst)) {
         // do nothing
      } else {
         return nullptr;
      }

      // check that the weights array is the same size
      if(static_cast<Int_t>(fModifiedWeights.size()) == hst->GetNbinsX()) {
         // do nothing
      } else {
         printf("Weights array size is not the same as the number of bins.\n");
         printf("Returning from DivideByWeights without dividing.");
         return nullptr;
      }
   }

   // now that we're satisified everything is kosher, divide the bins.
   for(Int_t i = 1; i <= hst->GetNbinsX(); i++) {
      printf("\t%i\n", i);
      Int_t index        = hst->GetBinLowEdge(i);
      Int_t found_weight = 0;
      if(!fold && !group) {
         found_weight = GetWeightFromIndex(index);
      } else {
         found_weight = GetModifiedWeight(index);
      }
      Double_t content    = hst->GetBinContent(i);
      Double_t error      = hst->GetBinError(i);
      Double_t weight     = found_weight;
      Double_t newcontent = content / weight;
      Double_t newerror   = error / weight;
      hst->SetBinContent(i, newcontent);
      hst->SetBinError(i, newerror);
   }

   return static_cast<TH1D*>(hst);
}
/*
   Double_t GetPeakInfo(THnSparse *Slice, Double_t centroid, Double_t min, Double_t max){

   Double_t FWHM;
   Int_t indexaxis, energy1axis, energy2axis;
   Double_t xmin[3];
   Double_t xmax[3];
   for (Int_t i=0;i<3;i++) {//goes through all three dimensions of THnSparse and finds the min and max values
   xmin[i] = Slice->GetAxis(i)->GetXmin();
   xmax[i] = Slice->GetAxis(i)->GetXmax();
   }//then look for matching axis since energy axis should be the same
   if (xmin[0] == xmin[1] && xmax[0] == xmax[1]) {//are 0 and 1 the same? then axis 2 is the index axis
   indexaxis = 2;
   energy1axis = 0;
   energy2axis = 1;
   } else if (xmin[1] == xmin[2] && xmax[1] == xmax[2]) {//are 1 and 2 the same? then axis 0 is the index axis
   indexaxis = 0;
   energy1axis = 1;
   energy2axis = 2;
   } else if (xmin[2] == xmin[0] && xmax[2] == xmax[0]) {//are 0 and 2 the same? then axis 1 is the index axis
   indexaxis = 1;
   energy1axis = 0;
   energy2axis = 2;
   } else {
   printf("Can't identify energy axes. Assuming index axis is axis 0.\n");//no two axis are the same
   indexaxis = 0;
   energy1axis = 1;
   energy2axis = 2;
   }

   Slice->GetAxis(energy1axis)->SetRangeUser(min,max);
   TH1D* tempslice = (TH1D*) Slice->Projection(energy2axis,"e"); // the "e" option pushes appropriate errors
   tempslice->SetName(Form("%s_proj_%i",Slice->GetName(),(Int_t)((max+min)/2)));
   tempslice->SetTitle(Form("%s: %i keV",Slice->GetTitle(),(Int_t)((max+min)/2)));

//Make a TPeak to find sigma for the peak
TPeak *p = new TPeak(centroid, centroid-40, centroid+40);
p->Fit(tempslice);
FWHM = p->GetParameter(2);
//Get Slice
return FWHM;
}
*/
void TAngularCorrelation::DisplayDiagnostics(TCanvas* c_diag)
{
   if(c_diag == nullptr) {
      c_diag = new TCanvas(Form("c_diag_%s", GetName()), Form("Diagnostics from %s", GetName()), 800, 800);
   }

   // divide canvas
   c_diag->Divide(2, 2);

   // pull plots for index correlation, chi^2, centroid, and fwhm
   TH1D* indexhst    = GetIndexCorrelation();
   TH1D* chi2hst     = GetChi2Hst();
   TH1D* centroidhst = GetCentroidHst();
   TH1D* fwhmhst     = GetFWHMHst();

   for(int i = 0; i < 52; i++) {
      std::cout<<chi2hst->GetBinContent(i)<<std::endl;
   }

   // format the diagnostic plots
   indexhst->SetStats(false);
   chi2hst->SetStats(false);
   centroidhst->SetStats(false);
   fwhmhst->SetStats(false);
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
}
