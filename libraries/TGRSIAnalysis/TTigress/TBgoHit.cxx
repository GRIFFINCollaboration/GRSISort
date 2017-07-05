#include "TBgoHit.h"

#include "TClass.h"

#include "TTigress.h"

/// \cond CLASSIMP
ClassImp(TBgoHit)
   /// \endcond

   TBgoHit::TBgoHit()
{
   Clear();
}

TBgoHit::~TBgoHit() = default;

TBgoHit::TBgoHit(const TBgoHit& rhs) : TGRSIDetectorHit()
{
   rhs.Copy(*this);
}

void TBgoHit::Clear(Option_t* opt)
{
   TGRSIDetectorHit::Clear(opt);
}

void TBgoHit::Copy(TObject& rhs) const
{
   TGRSIDetectorHit::Copy(rhs);
}

void TBgoHit::Print(Option_t* opt) const
{
   TString sopt(opt);
   printf("==== BgoHit @ 0x%p\n ====", (void*)this);
   printf("\t%s\n", GetName());
   printf("\tCharge: %.2f\n", Charge());
   printf("\tTime:   %.2f\n", GetTime());
   std::cout<<"\tTime:   "<<GetTimeStamp()<<"\n";
   printf("============================\n");
}

int TBgoHit::GetCrystal() const
{
   TChannel* chan = GetChannel();
   if(chan == nullptr) {
      return -1;
   }
   return chan->GetCrystalNumber();
}
