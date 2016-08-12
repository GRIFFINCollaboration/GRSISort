#include "GNotifier.h"

#include <iostream>

#include "GValue.h"
#include "TFile.h"
#include "TGRSIOptions.h"

GNotifier *GNotifier::fGNotifier = 0;

GNotifier *GNotifier::Get() {
  if(!fGNotifier)
    fGNotifier = new GNotifier;
  return fGNotifier;
}

GNotifier::GNotifier() { }

GNotifier::~GNotifier() { }


bool GNotifier::Notify() {
  // Loads the GValues from the current file of the chain.
  // Does not overwrite parameters set by hand, or by a .val file.
  TFile *f = gChain->GetCurrentFile();
  f->Get("GValue");

  for(auto& callback : callbacks){
    callback();
  }

  return true;
}
