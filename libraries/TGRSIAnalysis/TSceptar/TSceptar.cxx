#include <iostream>
#include "TSceptar.h"
#include <TRandom.h>
#include <TMath.h>

/// \cond CLASSIMP
ClassImp(TSceptar)
   /// \endcond

   bool TSceptar::fSetWave = false;

TVector3 TSceptar::gPaddlePosition[21] = {
   // Sceptar positions from Evan; Thanks Evan.
   TVector3(0, 0, 1),
   TVector3(14.3025, 4.6472, 22.8096),
   TVector3(0, 15.0386, 22.8096),
   TVector3(-14.3025, 4.6472, 22.8096),
   TVector3(-8.8395, -12.1665, 22.8096),
   TVector3(8.8395, -12.1665, 22.8096),
   TVector3(19.7051, 6.4026, 6.2123),
   TVector3(0, 20.7192, 6.2123),
   TVector3(-19.7051, 6.4026, 6.2123),
   TVector3(-12.1784, -16.7622, 6.2123),
   TVector3(12.1784, -16.7622, 6.2123),
   TVector3(19.7051, 6.4026, -6.2123),
   TVector3(0, 20.7192, -6.2123),
   TVector3(-19.7051, 6.4026, -6.2123),
   TVector3(-12.1784, -16.7622, -6.2123),
   TVector3(12.1784, -16.7622, -6.2123),
   TVector3(14.3025, 4.6472, -22.8096),
   TVector3(0, 15.0386, -22.8096),
   TVector3(-14.3025, 4.6472, -22.8096),
   TVector3(-8.8395, -12.1665, -22.8096),
   TVector3(8.8395, -12.1665, -22.8096)};

TSceptar::TSceptar()
{
// Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   // Class()->AddRule("TSceptar sceptar_hits attributes=NotOwner");
   // Class()->AddRule("TSceptar sceptardata attributes=NotOwner");
   Clear();
}

TSceptar::~TSceptar()
{
   // Default Destructor
}

TSceptar::TSceptar(const TSceptar& rhs) : TGRSIDetector()
{
// Copy Contructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   rhs.Copy(*this);
}

void TSceptar::Clear(Option_t* opt)
{
   // Clears all of the hits
   // The Option "all" clears the base class.
   // if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
   TGRSIDetector::Clear(opt);
   //}
   fSceptarHits.clear();
}

void TSceptar::Copy(TObject& rhs) const
{
   // Copies a TSceptar
   TGRSIDetector::Copy(rhs);

   static_cast<TSceptar&>(rhs).fSceptarHits = fSceptarHits;
}

TSceptar& TSceptar::operator=(const TSceptar& rhs)
{
   rhs.Copy(*this);
   return *this;
}

void TSceptar::AddFragment(std::shared_ptr<const TFragment> frag, TChannel*)
{
   TSceptarHit scHit(*frag);                 // Construction of TSceptarHit is handled in the constructor
   fSceptarHits.push_back(std::move(scHit)); // Can't use scHit outside of vector after using std::move
}

void TSceptar::Print(Option_t*) const
{
   // Prints out TSceptar Multiplicity, currently does little.
   printf("%lu fSceptarHits\n", fSceptarHits.size());
}

TGRSIDetectorHit* TSceptar::GetHit(const Int_t& idx)
{
   // Gets the TSceptarHit at index idx.
   return GetSceptarHit(idx);
}

TSceptarHit* TSceptar::GetSceptarHit(const int& i)
{
   try {
      return &fSceptarHits.at(i);
   } catch(const std::out_of_range& oor) {
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return nullptr;
}
