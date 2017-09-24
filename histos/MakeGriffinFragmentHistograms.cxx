#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"

#include "TGriffin.h"

extern "C" void MakeFragmentHistograms(TRuntimeObjects& obj)
{
   std::shared_ptr<const TFragment> frag = obj.GetFragment();
   TChannel*                        chan = frag->GetChannel();

   if(frag != nullptr && chan != nullptr) {
      obj.FillHistogram("channel", 2000, 0, 2000, frag->GetChannelNumber());

      if(chan->GetClassType() == TSceptar::Class()) {
         obj.FillHistogram("sceptar_charge", 4000, 0, 4000, frag->GetCharge());
      }

      if((chan->GetClassType() == TGriffin::Class()) && (chan->GetMnemonic()->OutputSensor() == TMnemonic::kA)) {
         obj.FillHistogram("griffin_energy", 8000, 0, 8000, frag->GetEnergy());

         obj.FillHistogram("grif_channel_charge", 65, 0, 65, frag->GetArrayNumber(), 10000, 0, 10000,
                           frag->GetCharge());

         obj.FillHistogram("grif_channel_energy", 65, 0, 65, frag->GetArrayNumber(), 10000, 0, 10000,
                           frag->GetEnergy());
      }

      frag->ClearTransients();
   }
}
