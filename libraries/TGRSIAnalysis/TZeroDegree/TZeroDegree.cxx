#include <iostream>
#include "TZeroDegree.h"
#include <TRandom.h>
#include <TMath.h>

/// \cond CLASSIMP
ClassImp(TZeroDegree)
/// \endcond


bool TZeroDegree::fSetWave = false;

TZeroDegree::TZeroDegree() {
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TZeroDegree::~TZeroDegree()	{
   //Default Destructor
}

TZeroDegree::TZeroDegree(const TZeroDegree& rhs) : TGRSIDetector() {
   //Copy Contructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TZeroDegree::Clear(Option_t *opt)	{
   //Clears all of the hits
   //The Option "all" clears the base class.
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
   }
   fZeroDegreeHits.clear();
}

void TZeroDegree::Copy(TObject &rhs) const {
   //Copies a TZeroDegree
   TGRSIDetector::Copy(rhs);
   
   static_cast<TZeroDegree&>(rhs).fZeroDegreeHits    = fZeroDegreeHits;
}

TZeroDegree& TZeroDegree::operator=(const TZeroDegree& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TZeroDegree::Print(Option_t *opt) const	{
   ///Prints out TZeroDegree multiplicity, currently does little.
   printf("%lu fZeroDegreeHits\n",fZeroDegreeHits.size());
}

void TZeroDegree::PushBackHit(TGRSIDetectorHit *scHit) {
   ///Adds a Hit to the list of TZeroDegree Hits
   fZeroDegreeHits.push_back(*(static_cast<TZeroDegreeHit*>(scHit)));
}

void TZeroDegree::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
   ///Builds the ZDS Hits directly from the TFragment. Basically, loops through the data for an event and sets observables.
   if(frag == NULL || mnemonic == NULL) {
      return;
   }
   
   for(size_t i = 0; i < frag->Charge.size(); ++i) {
      TZeroDegreeHit hit;
      hit.SetAddress(frag->ChannelAddress);
      hit.SetTimeStamp(frag->GetTimeStamp());
      hit.SetCfd(frag->GetCfd(i));
      hit.SetCharge(frag->GetCharge(i));
      
      if(TZeroDegree::SetWave()){
         if(frag->wavebuffer.size() == 0) {
            printf("Warning, TZeroDegree::SetWave() set, but data waveform size is zero!\n");
         } else {
            hit.SetWaveform(frag->wavebuffer);
         }
         if(hit.GetWaveform()->size() > 0) {
            hit.AnalyzeWaveform();
         }
      }
      
      AddHit(&hit);
   }
}

TGRSIDetectorHit* TZeroDegree::GetHit(const Int_t& idx){
   //Gets the TZeroDegreeHit at index idx.
   return GetZeroDegreeHit(idx);
}

TZeroDegreeHit* TZeroDegree::GetZeroDegreeHit(const int& i) {
   try{
      return &fZeroDegreeHits.at(i);
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return NULL;
}
