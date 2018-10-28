#include "TRuntimeObjects.h"
#include "TFragment.h"

extern "C" void MakeFragmentHistograms(TRuntimeObjects& obj)
{
   std::shared_ptr<const TFragment> frag = obj.GetFragment();

   if(frag != nullptr) {
      obj.FillHistogram("Addresses", 0xffff, 0, 0xffff, frag->GetAddress());
      obj.FillHistogram(Form("charge0x%04x", frag->GetAddress()), 2000, 0, 20000, frag->GetCharge());
      obj.FillHistogram(Form("energy0x%04x", frag->GetAddress()), 2000, 0, 2000, frag->GetEnergy());
   }
}
