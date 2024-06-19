#include "TCal.h"

/// \cond CLASSIMP
ClassImp(TCal)
/// \endcond

TCal::TCal()
{
   /// Default constructor
   InitTCal();
}

TCal::TCal(const char* name, const char* title)
{
   /// Constructor for naming the calibration
   InitTCal();
   SetNameTitle(name, title);
   // fgraph->SetNameTitle(name,title);
}

TCal::~TCal()
{
   /// Default dtor
}

TCal::TCal(const TCal& copy) : TGraphErrors(copy)
{
   /// Copy constructor
   InitTCal();
   copy.Copy(*this);
}

void TCal::SetNucleus(TNucleus* nuc, Option_t*)
{
   /// Sets the nucleus to be calibrated against
   if(nuc == nullptr) {
      Error("SetNucleus", "Nucleus does not exist");
      return;
   }
   if(fNuc != nullptr) {
      Warning("SetNucleus", "Overwriting nucleus: %s", fNuc->GetName());
   }
   fNuc = nuc;
}

void TCal::Copy(TObject& obj) const
{
   /// Copies the TCal.
   static_cast<TCal&>(obj).fChan = fChan;
   // Things to make deep copies of
   if(fFitFunc != nullptr) {
      *(static_cast<TCal&>(obj).fFitFunc) = *fFitFunc;
   }

   // Members to make shallow copies of
   static_cast<TCal&>(obj).fNuc = fNuc;
   TNamed::Copy(static_cast<TGraphErrors&>(obj));
}

Bool_t TCal::SetChannel(const TChannel* chan)
{
   /// Sets the channel being calibrated
   if(chan == nullptr) {
      Error("SetChannel", "TChannel does not exist");
      return false;
   }
   // Set our TRef to point at the TChannel
   fChan = const_cast<TChannel*>(chan);
   return true;
}

void TCal::WriteToAllChannels(const std::string& mnemonic)
{
   /// Writes this calibration to all channels in the current TChannel Map
   std::unordered_map<unsigned int, TChannel*>::iterator mapIt;
   std::unordered_map<unsigned int, TChannel*>*          chanMap  = TChannel::GetChannelMap();
   TChannel*                                             origChan = GetChannel();
   for(mapIt = chanMap->begin(); mapIt != chanMap->end(); mapIt++) {
      if(mnemonic.empty() || (strncmp(mapIt->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
         SetChannel(mapIt->second);
         WriteToChannel();
      }
   }
   std::cout << std::endl;
   if(origChan != nullptr) {
      SetChannel(origChan);
   }
}

std::vector<Double_t> TCal::GetParameters() const
{
   /// Returns all of the parameters in the current TCal.
   std::vector<Double_t> paramList;
   if(GetFitFunction() == nullptr) {
      Error("GetParameters", "Function has not been fitted yet");
      return paramList;
   }

   Int_t nParams = GetFitFunction()->GetNpar();

   for(int i = 0; i < nParams; i++) {
      paramList.push_back(GetParameter(i));
   }

   return paramList;
}

Double_t TCal::GetParameter(size_t parameter) const
{
   /// Returns the parameter at the index parameter
   if(GetFitFunction() == nullptr) {
      Error("GetParameter", "Function have not been fitted yet");
      return 0;
   }
   return GetFitFunction()->GetParameter(parameter);   // Root does all of the checking for us.
}

Bool_t TCal::SetChannel(UInt_t chanNum)
{
   /// Sets the channel for the calibration to the channel number channum. Returns
   /// 0 if the channel does not exist
   TChannel* chan = TChannel::GetChannelByNumber(chanNum);
   if(chan == nullptr) {
      Error("SetChannel", "Channel Number %d does not exist in current memory.", chanNum);
      return false;
   }
   return SetChannel(chan);
}

TChannel* TCal::GetChannel() const
{
   /// Gets the channel being pointed to by the TCal. Returns 0 if no channel
   /// is set.
   return static_cast<TChannel*>(fChan.GetObject());   // Gets the object pointed at by the TRef and casts it to a
                                                       // TChannel
}

void TCal::SetHist(TH1* hist)
{
   /// Sets this histogram pointed to. TCal does NOT take ownership so you cannot delete this
   /// histogram as long as you want to access the hist in the TCal/write it out. I will add this
   /// functionality if I get annoyed enough with the way it is.
   fHist = hist;
}

void TCal::Clear(Option_t*)
{
   /// Clears the calibration. Does not delete nuclei or channels.
   fNuc  = nullptr;
   fChan = nullptr;
   TGraphErrors::Clear();
}

void TCal::Print(Option_t*) const
{
   /// Prints calibration information
   if(GetChannel() != nullptr) {
      std::cout << "Channel Number: " << GetChannel()->GetNumber() << std::endl;
   } else {
   }

   if(fFitFunc != nullptr) {
      std::cout << std::endl
                << "*******************************" << std::endl;
      std::cout << " Fit:" << std::endl;
      fFitFunc->Print();
      std::cout << std::endl
                << "*******************************" << std::endl;
   } else {
      std::cout << "Parameters: FIT NOT SET" << std::endl;
   }

   std::cout << "Nucleus: ";
   if(GetNucleus() != nullptr) {
      std::cout << GetNucleus()->GetName() << std::endl;
   } else {
      std::cout << "NOT SET" << std::endl;
   }
}

void TCal::InitTCal()
{
   /// Initiallizes the TCal.
   /* fgraph = new TGraphErrors;*/
   fFitFunc = nullptr;
   fChan    = nullptr;
   fNuc     = nullptr;
   fHist    = nullptr;
   Clear();
}
