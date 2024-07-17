#include "GNotifier.h"

#include <iostream>

#include "GValue.h"
#include "TFile.h"
#include "TGRSIOptions.h"

GNotifier* GNotifier::fGNotifier = nullptr;

GNotifier* GNotifier::Get()
{
   if(fGNotifier == nullptr) {
      fGNotifier = new GNotifier;
   }
   return fGNotifier;
}

bool GNotifier::Notify()
{
   // Loads the GValues from the current file of the chain.
   // Does not overwrite parameters set by hand, or by a .val file.
   // TFile *f = gChain->GetCurrentFile();
   // f->Get("GValue");

   for(auto& callback : fCallbacks) {
      callback();
   }

   return true;
}
