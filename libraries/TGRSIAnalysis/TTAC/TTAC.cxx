#include <iostream>
#include "TTAC.h"
#include <TRandom.h>
#include <TMath.h>

/// \cond CLASSIMP
ClassImp(TTAC)
/// \endcond

TTAC::TTAC() {
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TTAC::~TTAC()	{
   //Default Destructor
}

TTAC::TTAC(const TTAC& rhs) : TGRSIDetector() {
   //Copy Contructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TTAC::Clear(Option_t *opt)	{
   //Clears all of the hits
   //The Option "all" clears the base class.
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
      TGRSIDetector::Clear(opt);
   }
   fTACHits.clear();
}

void TTAC::Copy(TObject &rhs) const {
   //Copies a TTAC
   TGRSIDetector::Copy(rhs);
   
   static_cast<TTAC&>(rhs).fTACHits    = fTACHits;
}

TTAC& TTAC::operator=(const TTAC& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TTAC::Print(Option_t *opt) const	{
   //Prints out TTAC Multiplicity, currently does little.
   printf("%lu fTACHits\n",fTACHits.size());
}

void TTAC::AddFragment(std::shared_ptr<const TFragment> frag, TChannel* chan) {
   TTACHit hit(*frag);
   fTACHits.push_back(std::move(hit));
}

TGRSIDetectorHit* TTAC::GetHit(const Int_t& idx){
   //Gets the TTACHit at index idx.
   return GetTACHit(idx);
}

TTACHit* TTAC::GetTACHit(const int& i) {
   try{
      return &fTACHits.at(i);
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return NULL;
}
