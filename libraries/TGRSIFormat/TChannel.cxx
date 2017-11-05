#include "TChannel.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>

#include <vector>
#include <sstream>
#include <algorithm>

#include "TFile.h"
#include "TKey.h"

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 *
 * Please indicate changes with your initials.
 *
 *
 */

/// \cond CLASSIMP
ClassImp(TChannel)
/// \endcond

std::map<unsigned int, TChannel*>* TChannel::fChannelMap =
	new std::map<unsigned int, TChannel*>; // global maps of channels
std::map<int, TChannel*>* TChannel::fChannelNumberMap = new std::map<int, TChannel*>;

std::string TChannel::fFileName;
std::string TChannel::fFileData;

TChannel::TChannel()
{
   Clear();
} // default constructor need to write to root file.

TChannel::~TChannel() = default;

TChannel::TChannel(const char* tempName)
{
   Clear();
   SetName(tempName);
}

TChannel::TChannel(TChannel* chan)
{
   /// Makes a copy of a the TChannel.
   SetAddress(chan->GetAddress());
   SetIntegration(chan->GetIntegration());
   SetNumber(chan->GetNumber());
   SetStream(chan->GetStream());
   SetUserInfoNumber(chan->GetUserInfoNumber());
   SetName(chan->GetName()); // SetName also sets the mnemonic
   SetDigitizerType(chan->GetDigitizerTypeString());

   SetENGCoefficients(chan->GetENGCoeff());
   SetCFDCoefficients(chan->GetCFDCoeff());
   SetLEDCoefficients(chan->GetLEDCoeff());
   SetTIMECoefficients(chan->GetTIMECoeff());
   SetEFFCoefficients(chan->GetEFFCoeff());
   SetCTCoefficients(chan->GetCTCoeff());

   SetENGChi2(chan->GetENGChi2());
   SetCFDChi2(chan->GetCFDChi2());
   SetLEDChi2(chan->GetLEDChi2());
   SetTIMEChi2(chan->GetTIMEChi2());
   SetEFFChi2(chan->GetEFFChi2());
   SetUseCalFileIntegration(chan->UseCalFileIntegration());
   SetWaveParam(chan->GetWaveParam());

   SetDetectorNumber(chan->GetDetectorNumber());
   SetSegmentNumber(chan->GetSegmentNumber());
   SetCrystalNumber(chan->GetCrystalNumber());
   SetTimeOffset(chan->GetTimeOffset());
   SetClassType(chan->GetClassType());
}

void TChannel::SetName(const char* tmpName)
{
   TNamed::SetName(tmpName);
   fMnemonic.Clear();
   fMnemonic.Parse(GetName());
}

void TChannel::InitChannelInput()
{
   int channels_found = ParseInputData(fFileData.c_str(), "q");
   printf("Successfully read %i TChannels from" CYAN " %s" RESET_COLOR "\n", channels_found, gFile->GetName());
}

bool TChannel::CompareChannels(const TChannel& chana, const TChannel& chanb)
{
   /// Compares the names of the two TChannels. Returns true if the names are the
   /// same, false if different.
   std::string namea;
   namea.assign(static_cast<TChannel>(chana).GetName());

   return namea.compare(static_cast<TChannel>(chanb).GetName()) < 0;
}

void TChannel::DeleteAllChannels()
{
   /// Safely deletes fChannelMap and fChannelNumberMap
   std::map<unsigned int, TChannel*>::iterator iter;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
      if(iter->second != nullptr) {
         delete iter->second;
      }
      // These maps should point to the same pointers, so this should clear out both
      iter->second = nullptr;
   }
   fChannelMap->clear();
   fChannelNumberMap->clear();
}

void TChannel::AddChannel(TChannel* chan, Option_t* opt)
{
   /// Add a TChannel to fChannelMap. If the TChannel doesn't exist, create a new TChannel and add that the fChannelMap.
   /// Options:
   ///        "overwrite" -  The TChannel in the fChannelMap at the same address is overwritten.
   ///                       If this option is not specified, an Error is returned if the TChannel already
   ///                       exists in the fChannelMap.
   ///        "save"      -  The temporary channel is not deleted after being placed in the map.
   if(chan == nullptr) {
      return;
   }
   if(fChannelMap->count(chan->GetAddress()) == 1) { // if this channel exists
      if(strcmp(opt, "overwrite") == 0) {
         TChannel* oldchan = GetChannel(chan->GetAddress());
         oldchan->OverWriteChannel(chan);
         // Need to also update the channel number map RD
         UpdateChannelNumberMap();
         return;
      }
      printf("Trying to add a channel that already exists!\n");
      return;
   }
   if((chan->GetAddress() & 0x00ffffff) == 0x00ffffff) {
      // this is the default tigress value for i am not there.
      // we should not imclude it in the map.
      delete chan;
   } else {
      // We need to update the channel maps to correspond to the new channel that has been added.
      fChannelMap->insert(std::make_pair(chan->GetAddress(), chan));
      if((chan->GetNumber() != 0) && (fChannelNumberMap->count(chan->GetNumber()) == 0)) {
         fChannelNumberMap->insert(std::make_pair(chan->GetNumber(), chan));
      }
   }

   return;
}

void TChannel::OverWriteChannel(TChannel* chan)
{
   /// Overwrites the current TChannel with chan.
   SetAddress(chan->GetAddress());
   SetIntegration(chan->GetIntegration());
   SetNumber(chan->GetNumber());
   SetStream(chan->GetStream());
   SetUserInfoNumber(chan->GetUserInfoNumber());
   SetDigitizerType(chan->GetDigitizerTypeString());
   SetName(chan->GetName());

   SetENGCoefficients(chan->GetENGCoeff());
   SetCFDCoefficients(chan->GetCFDCoeff());
   SetLEDCoefficients(chan->GetLEDCoeff());
   SetTIMECoefficients(chan->GetTIMECoeff());
   SetEFFCoefficients(chan->GetEFFCoeff());
   SetCTCoefficients(chan->GetCTCoeff());

   SetENGChi2(chan->GetENGChi2());
   SetCFDChi2(chan->GetCFDChi2());
   SetLEDChi2(chan->GetLEDChi2());
   SetTIMEChi2(chan->GetTIMEChi2());
   SetEFFChi2(chan->GetEFFChi2());

   SetUseCalFileIntegration(chan->UseCalFileIntegration());

   SetWaveParam(chan->GetWaveParam());

   SetDetectorNumber(chan->GetDetectorNumber());
   SetSegmentNumber(chan->GetSegmentNumber());
   SetCrystalNumber(chan->GetCrystalNumber());
   SetTimeOffset(chan->GetTimeOffset());
   SetClassType(chan->GetClassType());
}

void TChannel::AppendChannel(TChannel* chan)
{
   /// Sets the current TChannel to chan
   if(chan->GetIntegration() != 0) {
      SetIntegration(chan->GetIntegration());
   }
   if(chan->GetNumber() != 0) {
      SetNumber(chan->GetNumber());
   }
   if(chan->GetStream() != 0) {
      SetStream(chan->GetStream());
   }
   if(chan->GetUserInfoNumber() != 0 && chan->GetUserInfoNumber() != -1) {
      SetUserInfoNumber(chan->GetUserInfoNumber());
   }
   if(strlen(chan->GetName()) > 0) {
      SetName(chan->GetName());
   }
   if(strlen(chan->GetDigitizerTypeString()) > 0) {
      SetDigitizerType(chan->GetDigitizerTypeString());
   }
   if(chan->GetTimeOffset() != 0.0) {
      SetTimeOffset(chan->GetTimeOffset());
   }

   if(!chan->GetENGCoeff().empty()) {
      SetENGCoefficients(chan->GetENGCoeff());
   }
   if(!chan->GetCFDCoeff().empty()) {
      SetCFDCoefficients(chan->GetCFDCoeff());
   }
   if(!chan->GetLEDCoeff().empty()) {
      SetLEDCoefficients(chan->GetLEDCoeff());
   }
   if(!chan->GetTIMECoeff().empty()) {
      SetTIMECoefficients(chan->GetTIMECoeff());
   }
   if(!chan->GetEFFCoeff().empty()) {
      SetEFFCoefficients(chan->GetEFFCoeff());
   }
   if(!chan->GetCTCoeff().empty()) {
      SetCTCoefficients(chan->GetCTCoeff());
   }

   if(chan->GetENGChi2() != 0.0) {
      SetENGChi2(chan->GetENGChi2());
   }
   if(chan->GetCFDChi2() != 0.0) {
      SetCFDChi2(chan->GetCFDChi2());
   }
   if(chan->GetLEDChi2() != 0.0) {
      SetLEDChi2(chan->GetLEDChi2());
   }
   if(chan->GetTIMEChi2() != 0.0) {
      SetTIMEChi2(chan->GetTIMEChi2());
   }
   if(chan->GetEFFChi2() != 0.0) {
      SetEFFChi2(chan->GetEFFChi2());
   }
	SetUseCalFileIntegration(chan->UseCalFileIntegration());
   if(chan->UseWaveParam()) {
      SetWaveParam(chan->GetWaveParam());
   }

   if(chan->GetDetectorNumber() > -1) {
      SetDetectorNumber(chan->GetDetectorNumber());
   }
   if(chan->GetSegmentNumber() > -1) {
      SetSegmentNumber(chan->GetSegmentNumber());
   }
   if(chan->GetCrystalNumber() > -1) {
      SetCrystalNumber(chan->GetCrystalNumber());
   }

   SetClassType(chan->GetClassType());
}

int TChannel::UpdateChannel(TChannel* chan, Option_t*)
{
   /// If there is information in the chan, the current TChannel with the same address is updated with that information.
   if(chan == nullptr) {
      return 0;
   }
   TChannel* oldchan = GetChannel(chan->GetAddress()); // look for already existing channel at this address
   if(oldchan == nullptr) {
      return 0;
   }
   oldchan->AppendChannel(chan);

   return 0;
}

TChannel* TChannel::GetDefaultChannel()
{
   if(!fChannelMap->empty()) {
      return fChannelMap->begin()->second;
   }
   return nullptr;
}

void TChannel::Clear(Option_t*)
{
   /// Clears all fields of a TChannel. There are currently no options to be specified.
   fAddress        = 0xffffffff;
   fIntegration    = 0;
	fTypeName.clear();
	fDigitizerTypeString.clear();
	fDigitizerType  = TMnemonic::EDigitizer::kDefault;
   fNumber         = 0;
   fStream         = 0;
   fUserInfoNumber = 0xffffffff;
   fUseCalFileInt  = false;

   fDetectorNumber = -1;
   fSegmentNumber  = -1;
   fCrystalNumber  = -1;
   fTimeOffset     = 0.0;

   WaveFormShape   = WaveFormShapePar();

   SetName("DefaultTChannel");

   fENGCoefficients.clear();
   fENGChi2        = 0.0;
   fCFDCoefficients.clear();
   fCFDChi2        = 0.0;
   fLEDCoefficients.clear();
   fLEDChi2        = 0.0;
   fTIMECoefficients.clear();
   fTIMEChi2        = 0.0;
   fEFFCoefficients.clear();
   fEFFChi2        = 0.0;
   fCTCoefficients.clear();
   fENGChi2        = 0.0;
}

TChannel* TChannel::GetChannel(unsigned int temp_address)
{
   /// Returns the TChannel at the specified address. If the address doesn't exist, returns an empty gChannel.

   TChannel* chan = nullptr;
   //    if(temp_address == 0 || temp_address == 0xffffffff) {//default (nullptr) address, return 0;
   //	      return chan;
   //    }
   if(fChannelMap->count(temp_address) == 1) { // found channel
      chan = fChannelMap->at(temp_address);
   }
   return chan;
}

TChannel* TChannel::GetChannelByNumber(int temp_num)
{
   /// Returns the TChannel based on the channel number and not the channel address.
   //  if(fChannelMap->size() != fChannelNumberMap->size()) {
   // We should just always update this map before we use it
   UpdateChannelNumberMap();
   //    }
   TChannel* chan = nullptr;
   try {
      chan = fChannelNumberMap->at(temp_num);
   } catch(const std::out_of_range& oor) {
      return nullptr;
   }
   return chan;
}

TChannel* TChannel::FindChannelByName(const char* ccName)
{
   /// Finds the TChannel by the name of the channel
   TChannel* chan = nullptr;
   if(ccName == nullptr) {
      return chan;
   }

   std::string name = ccName;
   if(name.length() == 0) {
      return chan;
   }

   std::map<unsigned int, TChannel*>::iterator iter;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
      chan                    = iter->second;
      std::string channelName = chan->GetName();
      if(channelName.compare(0, name.length(), name) == 0) {
         break;
      }
      chan = nullptr;
   }
   // either comes out normally as null or breaks out with some TChannel [SC]

   return chan;
}

void TChannel::UpdateChannelNumberMap()
{
   /// Updates the fChannelNumberMap based on the entries in the fChannelMap. This should be called before using the
   /// fChannelNumberMap.
   std::map<unsigned int, TChannel*>::iterator mapiter;
   fChannelNumberMap->clear(); // This isn't the nicest way to do this but will keep us consistent.

   for(mapiter = fChannelMap->begin(); mapiter != fChannelMap->end(); mapiter++) {
      fChannelNumberMap->insert(std::make_pair(mapiter->second->GetNumber(), mapiter->second));
   }
   /*
      for(iter1 = fChannelMap->begin(); iter1 != fChannelMap->end(); iter1++) {
      if(fChannelNumberMap->count(iter1->second->GetNumber())==0) {
      fChannelNumberMap->insert(std::make_pair(iter1->second->GetNumber(),iter1->second));
      }
      else{
      fChannelNumberMap.find(iter1->second->GetNumber())->second = iter1->second
      }
      }*/
}

void TChannel::SetAddress(unsigned int tmpadd)
{
   /// Sets the address of a TChannel and also overwrites that channel if it is in the channel map
   std::map<unsigned int, TChannel*>::iterator iter1;
   for(iter1 = fChannelMap->begin(); iter1 != fChannelMap->end(); iter1++) {
      if(iter1->second == this) {
         std::cout<<"Channel at address: 0x"<<std::hex<<fAddress
                  <<" already exists. Please use AddChannel() or OverWriteChannel() to change this TChannel"
                  <<std::dec<<std::endl;
         break;
      }
   }
   fAddress = tmpadd;
}

void TChannel::DestroyENGCal()
{
   /// Erases the ENGCoefficients vector
   fENGCoefficients.clear();
}

void TChannel::DestroyCFDCal()
{
   /// Erases the CFDCoefficients vector
   fCFDCoefficients.clear();
}

void TChannel::DestroyLEDCal()
{
   /// Erases the LEDCoefficients vector
   fLEDCoefficients.clear();
}

void TChannel::DestroyTIMECal()
{
   /// Erases the TimeCal vector
   fTIMECoefficients.clear();
}

void TChannel::DestroyEFFCal()
{
   /// Erases the EffCal vector
   fEFFCoefficients.clear();
}

void TChannel::DestroyCTCal()
{
   // Erases the CTCal vector
   fCTCoefficients.clear();
}

void TChannel::DestroyCalibrations()
{
   /// Erases all Cal vectors
   DestroyENGCal();
   DestroyCFDCal();
   DestroyLEDCal();
   DestroyTIMECal();
   DestroyEFFCal();
   DestroyCTCal();
}

double TChannel::CalibrateENG(int charge, int temp_int)
{
   /// Returns the calibrated energy of the channel when a charge is passed to it.
   /// This is done by first adding a random number between 0 and 1 to the charge
   /// bin. This is then taken and divided by the integration parameter. The
   /// polynomial energy calibration formula is then applied to get the calibrated
   /// energy.
   if(charge == 0) {
      return 0.0000;
   }

   // int temp_int = 1; //125.0;
   if(temp_int == 0) {
      if(fIntegration != 0) {
         temp_int = fIntegration; // the 4 is the dis.
      } else {
         temp_int = 1;
      }
   }

   // We need to add a random number between 0 and 1 before calibrating to avoid
   // binning issues.
   return CalibrateENG((static_cast<double>(charge) + gRandom->Uniform()) / static_cast<double>(temp_int));
}

double TChannel::CalibrateENG(double charge, int temp_int)
{
   /// Returns the calibrated energy of the channel when a charge is passed to it.
   /// This is divided by the integration parameter. The
   /// polynomial energy calibration formula is then applied to get the calibrated
   /// energy.
   if(charge == 0) {
      return 0.0000;
   }

   // int temp_int = 1; //125.0;
   if(temp_int == 0) {
      if(fIntegration != 0) {
         temp_int = fIntegration; // the 4 is the dis.
      } else {
         temp_int = 1;
      }
   }

   return CalibrateENG((charge) / static_cast<double>(temp_int));
}

double TChannel::CalibrateENG(double charge)
{
   /// Returns the calibrated energy. The polynomial energy calibration formula is
   /// applied to get the calibrated energy. This function does not use the
   /// integration parameter.
   if(fENGCoefficients.empty()) {
      return charge;
   }
   double cal_chg = fENGCoefficients[0];
   for(size_t i = 1; i < fENGCoefficients.size(); i++) {
      cal_chg += fENGCoefficients[i] * pow((charge), i);
   }
   return cal_chg;
}

double TChannel::CalibrateCFD(int cfd)
{
   /// Calibrates the CFD properly.
   return CalibrateCFD(static_cast<double>(cfd) + gRandom->Uniform());
}

double TChannel::CalibrateCFD(double cfd)
{
   /// Returns the calibrated CFD. The polynomial CFD calibration formula is
   /// applied to get the calibrated CFD.
   if(fCFDCoefficients.empty()) {
      return cfd;
   }

   double cal_cfd = 0.0;
   for(size_t i = 0; i < fCFDCoefficients.size(); i++) {
      cal_cfd += fCFDCoefficients[i] * pow(cfd, i);
   }

   return cal_cfd;
}

double TChannel::CalibrateLED(int led)
{
   /// Calibrates the LED
   return CalibrateLED(static_cast<double>(led) + gRandom->Uniform());
}

double TChannel::CalibrateLED(double led)
{
   /// Returns the calibrated LED. The polynomial LED calibration formula is
   /// applied to get the calibrated LED.
   if(fLEDCoefficients.empty()) {
      return led;
   }

   double cal_led = 0.0;
   for(size_t i = 0; i < fLEDCoefficients.size(); i++) {
      cal_led += fLEDCoefficients[i] * pow(led, i);
   }
   return cal_led;
}

double TChannel::CalibrateTIME(int chg)
{
   /// Calibrates the time spectrum
   if(fTIMECoefficients.size() != 3 || (chg < 1)) {
      return 0.0000;
   }
   return CalibrateTIME((CalibrateENG(chg)));
}

double TChannel::CalibrateTIME(double energy)
{
   /// uses the values stored in TIMECOefficients to calculate a
   /// "walk correction" factor.  This function returns the correction
   /// not an adjusted time stamp!   pcb.
   if(fTIMECoefficients.size() != 3 || (energy < 3.0)) {
      return 0.0000;
   }

   double timeCorrection = 0.0;

   timeCorrection = fTIMECoefficients.at(0) + (fTIMECoefficients.at(1) * pow(energy, fTIMECoefficients.at(2)));

   return timeCorrection;
}

double TChannel::CalibrateEFF(double)
{
   /// This needs to be added
   return 1.0;
}

void TChannel::SetUseCalFileIntegration(const std::string& mnemonic, bool flag)
{
   /// Writes this UseCalFileIntegration to all channels in the current TChannel Map
   /// that starts with the mnemonic. Use "" to write to ALL channels
   /// WARNING: This is case sensitive!
   std::map<unsigned int, TChannel*>::iterator mapit;
   std::map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
   for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
      if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
         mapit->second->SetUseCalFileIntegration(flag);
      }
   }
}

void TChannel::SetIntegration(const std::string& mnemonic, int tmpint)
{
   // Writes this integration to all channels in the current TChannel Map
   // that starts with the mnemonic. Use "" to write to ALL channels
   // WARNING: This is case sensitive!
   std::map<unsigned int, TChannel*>::iterator mapit;
   std::map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
   for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
      if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
         mapit->second->SetIntegration(tmpint);
      }
   }
}

void TChannel::SetDigitizerType(const std::string& mnemonic, const char* tmpstr)
{
   // Writes this digitizer type to all channels in the current TChannel Map
   // that starts with the mnemonic. Use "" to write to ALL channels
   // WARNING: This is case sensitive!
   std::map<unsigned int, TChannel*>::iterator mapit;
   std::map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
   for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
      if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
         mapit->second->SetDigitizerType(tmpstr);
      }
   }
}

void TChannel::PrintCTCoeffs(Option_t*) const
{
   /// Prints out the current TChannel.
   std::cout<<GetName()<<"\t{\n"; //,channelname.c_str();
   std::cout<<"Name:      "<<GetName()<<"\n";
   std::cout<<"Number:    "<<fNumber<<"\n";
   std::cout<<std::setfill('0');
   std::cout<<"Address:   0x"<<std::hex<<std::setw(8)<<fAddress<<std::dec<<"\n";
   for(double fCTCoefficient : fCTCoefficients) {
      std::cout<<fCTCoefficient<<"\t";
   }
   std::cout<<"\n";
   std::cout<<"}\n";
   std::cout<<"//====================================//\n";
}

void TChannel::Print(Option_t*) const
{
   /// Prints out the current TChannel.
   std::cout<<GetName()<<"\t{\n"; //,channelname.c_str();
   std::cout<<"Type:      ";
   if(GetClassType() != nullptr) {
      std::cout<<GetClassType()->GetName()<<std::endl;
   } else {
      std::cout<<"None"<<std::endl;
   }

   std::cout<<"Name:      "<<GetName()<<"\n";
   std::cout<<"Number:    "<<fNumber<<"\n";
   std::cout<<std::setfill('0');
   std::cout<<"Address:   0x"<<std::hex<<std::setw(8)<<fAddress<<std::dec<<"\n";
   std::cout<<std::setfill(' ');
   std::cout<<"Digitizer: "<<fDigitizerTypeString<<"\n";
   std::cout<<"TimeOffset: "<<fTimeOffset<<"\n";
   std::cout<<"EngCoeff:  ";
   for(float fENGCoefficient : fENGCoefficients) {
      std::cout<<fENGCoefficient<<"\t";
   }
   std::cout<<"\n";
   std::cout<<"Integration: "<<fIntegration<<"\n";
   std::cout<<"ENGChi2:   "<<fENGChi2<<"\n";
   std::cout<<"EffCoeff:  ";
   for(double fEFFCoefficient : fEFFCoefficients) {
      std::cout<<fEFFCoefficient<<"\t";
   }
   std::cout<<"\n";
   std::cout<<"EFFChi2:   "<<fEFFChi2<<"\n";
   if(!fCTCoefficients.empty()) {
      std::cout<<"CTCoeff:  ";
      for(double fCTCoefficient : fCTCoefficients) {
         std::cout<<fCTCoefficient<<"\t";
      }
      std::cout<<"\n";
   }
   if(!fTIMECoefficients.empty()) {
      std::cout<<"TIMECoeff: ";
      for(double fTIMECoefficient : fTIMECoefficients) {
         std::cout<<fTIMECoefficient<<"\t";
      }
      std::cout<<"\n";
   }
   if(fUseCalFileInt) {
      std::cout<<"FileInt: "<<fUseCalFileInt<<"\n";
   }
   if(UseWaveParam()) {
      std::cout<<"RiseTime: "<<WaveFormShape.TauRise<<"\n";
      std::cout<<"DecayTime: "<<WaveFormShape.TauDecay<<"\n";
      std::cout<<"BaseLine: "<<WaveFormShape.BaseLine<<"\n";
   }
   std::cout<<"}\n";
   std::cout<<"//====================================//\n";
}

std::string TChannel::PrintCTToString(Option_t*)
{
   std::string buffer;
   buffer.append("\n");
   buffer.append(GetName());
   buffer.append("\t{\n"); //,channelname.c_str();
   buffer.append("Name:      ");
   buffer.append(GetName());
   buffer.append("\n");
   buffer.append(Form("Number:    %d\n", fNumber));
   buffer.append(Form("Address:   0x%08x\n", fAddress));
   if(!fCTCoefficients.empty()) {
      buffer.append("CTCoeff:  ");
      for(double fCTCoefficient : fCTCoefficients) {
         buffer.append(Form("%f\t", fCTCoefficient));
      }
      buffer.append("\n");
   }
   buffer.append("}\n");

   buffer.append("//====================================//\n");

   return buffer;
}

std::string TChannel::PrintToString(Option_t*)
{
   std::string buffer;
   buffer.append("\n");
   buffer.append(GetName());
   buffer.append("\t{\n"); //,channelname.c_str();
   buffer.append("Type:      ");
   if(GetClassType() != nullptr) {
      buffer.append(Form("%s\n", GetClassType()->GetName()));
   } else {
      buffer.append("None\n");
   }

   buffer.append("Name:      ");
   buffer.append(GetName());
   buffer.append("\n");
   buffer.append(Form("Number:    %d\n", fNumber));
   buffer.append(Form("Address:   0x%08x\n", fAddress));
   buffer.append(Form("Digitizer: %s\n", fDigitizerTypeString.c_str()));
   buffer.append("EngCoeff:  ");
   for(float fENGCoefficient : fENGCoefficients) {
      buffer.append(Form("%f\t", fENGCoefficient));
   }
   buffer.append("\n");
   buffer.append(Form("Integration: %d\n", fIntegration));
   buffer.append(Form("TimeOffset: %lf\n", fTimeOffset));
   buffer.append(Form("ENGChi2:     %f\n", fENGChi2));
   buffer.append("EffCoeff:  ");
   for(double fEFFCoefficient : fEFFCoefficients) {
      buffer.append(Form("%f\t", fEFFCoefficient));
   }
   buffer.append("\n");
   buffer.append(Form("EFFChi2:   %f\n", fEFFChi2));
   if(!fCTCoefficients.empty()) {
      buffer.append("CTCoeff:  ");
      for(double fCTCoefficient : fCTCoefficients) {
         buffer.append(Form("%f\t", fCTCoefficient));
      }
      buffer.append("\n");
   }
   if(!fTIMECoefficients.empty()) {
      buffer.append("TIMECoeff:  ");
      for(double fTIMECoefficient : fTIMECoefficients) {
         buffer.append(Form("%f\t", fTIMECoefficient));
      }
      buffer.append("\n");
   }
   buffer.append(Form("FileInt: %d\n", static_cast<int>(fUseCalFileInt)));
   if(UseWaveParam()) {
      buffer.append(Form("RiseTime: %f\n", WaveFormShape.TauRise));
      buffer.append(Form("DecayTime: %f\n", WaveFormShape.TauDecay));
      buffer.append(Form("BaseLine: %f\n", WaveFormShape.BaseLine));
   }

   buffer.append("}\n");

   buffer.append("//====================================//\n");

   return buffer;
}

void TChannel::WriteCalFile(const std::string& outfilename)
{
   /// prints the context of addresschannelmap formatted correctly to stdout if
   /// no file name is passed to the function.  If a file name is passed to the function
   /// prints the context of addresschannelmap formatted correctly to a file with the given
   /// name.  This will earse and rewrite the file if the file already exisits!

   std::map<unsigned int, TChannel*>::iterator iter;
   std::vector<TChannel>           chanVec;
   std::vector<TChannel>::iterator iter_vec;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
      if(iter->second != nullptr) {
         chanVec.push_back(*iter->second);
      }
   }

   std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

   /*
      std::stringstream buffer;
      std::streambuf* std_out = std::cout.rdbuf(buffer.rdbuf());
      WriteCalFile();
      fFileData.assign(buffer.str());
      std::cout.rdbuf(std_out);
      */

   if(outfilename.length() > 0) {
      std::ofstream calout;
      calout.open(outfilename.c_str());
      for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++) {
         std::string chanstr = iter_vec->PrintToString();
         calout<<chanstr.c_str();
         calout<<std::endl;
      }
      calout<<std::endl;
      calout.close();
   } else {
      for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++) {
         iter_vec->Print();
      }
   }

   /*
      FILE* c_outputfile;
      if(outfilename.length()>0) {
      c_outputfile = freopen (outfilename.c_str(),"w",stdout);
      }
      for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++)   {
      iter_vec->Print();
      }
      if(outfilename.length()>0) {
      fclose(c_outputfile);
      int fd = open("/dev/tty", O_WRONLY);
      stdout = fdopen(fd, "w");
      }
      */
   return;
}

void TChannel::WriteCTCorrections(const std::string& outfilename)
{

   std::map<unsigned int, TChannel*>::iterator iter;
   std::vector<TChannel>           chanVec;
   std::vector<TChannel>::iterator iter_vec;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
      if(iter->second != nullptr) {
         chanVec.push_back(*iter->second);
      }
   }

   std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

   if(outfilename.length() > 0) {
      std::ofstream calout;
      calout.open(outfilename.c_str());
      for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++) {
         std::string chanstr = iter_vec->PrintCTToString();
         calout<<chanstr.c_str();
         calout<<std::endl;
      }
      calout<<std::endl;
      calout.close();
   } else {
      for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++) {
         iter_vec->PrintCTCoeffs();
      }
   }

   return;
}

void TChannel::WriteCalBuffer(Option_t*)
{
   /// writes any TChannels in memory to the internal buffer
   /// fFileData.  Can be used to over write info that is there
   /// or create the buffer if the channels originated from the odb.

   std::map<unsigned int, TChannel*>::iterator iter;
   std::vector<TChannel>           chanVec;
   std::vector<TChannel>::iterator iter_vec;
   for(iter = fChannelMap->begin(); iter != fChannelMap->end(); iter++) {
      if(iter->second != nullptr) {
         chanVec.push_back(*iter->second);
      }
   }

   std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

   /*
      std::stringstream buffer;
      std::streambuf* std_out = std::cout.rdbuf(buffer.rdbuf());
      WriteCalFile();
      fFileData.assign(buffer.str());
      std::cout.rdbuf(std_out);
      */

   std::string data;

   for(iter_vec = chanVec.begin(); iter_vec != chanVec.end(); iter_vec++) {
      data.append(iter_vec->PrintToString());
   }
   fFileData.clear();
   fFileData = data;
}

Int_t TChannel::ReadCalFromCurrentFile(Option_t*)
{

   if(!gFile) {
      return 0;
   }

   TFile* tempf = gFile->CurrentFile();
   TList* list  = tempf->GetListOfKeys();
   TIter  iter(list);

   // while(TObject *obj = ((TKey*)(iter.Next()))->ReadObj()) {
   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TChannel") != 0)) {
         continue;
      }
      // TObject *  obj = key->ReadObj();
      // if(obj && !obj->InheritsFrom("TChannel"))
      //   continue;
      // TChannel *c = (TChannel*)obj;
      // TChannel *c = (TChannel*)key->ReadObj();
      key->ReadObj();
      return GetNumberOfChannels();
   }
   return 0;
}

Int_t TChannel::ReadCalFromFile(TFile* tempf, Option_t*)
{
   /// Reads the TChannel information from a TFile if it has already been written to that File.
   if(tempf == nullptr) {
      return 0;
   }

   TList* list = tempf->GetListOfKeys();
   TIter  iter(list);

   // while(TObject *obj = ((TKey*)(iter.Next()))->ReadObj()) {
   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TChannel") != 0)) {
         continue;
      }
      key->ReadObj();
      return GetNumberOfChannels();
   }
   return 0;
}

Int_t TChannel::ReadCalFromTree(TTree* tree, Option_t*)
{
   /// Reads the TChannel information from a Tree if it has already been written to that Tree.
   if(tree == nullptr) {
      return 0;
   }

   TFile* tempf = tree->GetCurrentFile();
   TList* list  = tempf->GetListOfKeys();
   TIter  iter(list);

   // while(TObject *obj = ((TKey*)(iter.Next()))->ReadObj()) {
   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TChannel") != 0)) {
         continue;
      }
      // TObject *  obj = key->ReadObj();
      // if(obj && !obj->InheritsFrom("TChannel"))
      //   continue;
      // TChannel *c = (TChannel*)obj;
      // TChannel *c = (TChannel*)key->ReadObj();
      key->ReadObj();
      return GetNumberOfChannels();
   }
   return 0;
}

Int_t TChannel::ReadCalFile(const char* filename)
{
   /// Makes TChannels from a cal file to be used as the current calibration until grsisort
   /// is closed. Returns the number of channels properly read in.
   std::string infilename;
   infilename.append(filename);

   if(infilename.length() == 0) {
      return -1;
   }

   printf("Reading from calibration file:" CYAN " %s" RESET_COLOR ".....", filename);
   std::ifstream infile;
   infile.open(infilename.c_str());
   if(!infile.is_open()) {
      printf(DRED "could not open file." RESET_COLOR "\n");
      return -2;
   }
   infile.seekg(0, std::ios::end);
   int length = infile.tellg();
   if(length < 1) {
      return -2;
   }

   auto* buffer = new char[length];
   infile.seekg(0, std::ios::beg);
   infile.read(buffer, length);

   int channels_found = ParseInputData(const_cast<const char*>(buffer));
   SaveToSelf(infilename.c_str());
   UpdateChannelNumberMap();
   return channels_found;
}

void TChannel::SaveToSelf(const char* fname)
{
   if(fFileName.length() == 0) {
      fFileName.assign(fname);
   } else if(fFileName.compare(fname) == 0) {
      // do nothing.
   } else {
      // also do nothing, but perhaps jest append the names toghter...
   }
   std::stringstream buffer;
   std::streambuf*   std_out = std::cout.rdbuf(buffer.rdbuf());
   WriteCalFile();
   fFileData.assign(buffer.str());
   std::cout.rdbuf(std_out);
}

Int_t TChannel::ParseInputData(const char* inputdata, Option_t* opt)
{
   std::istringstream infile(inputdata);

   TChannel* channel = nullptr;

   std::string line;
   int         linenumber  = 0;
   int         newchannels = 0;

   // bool creatednewchannel = false;
   bool brace_open = false;
   // int detector = 0;
   std::string name;

   // Parse the cal file. This is useful because if the cal file contains something that
   // the parser does not recognize, it just skips it!
   while(std::getline(infile, line)) {
      linenumber++;
      trim(&line);
      size_t comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(line.length() == 0u) {
         continue;
      }
      size_t openbrace  = line.find('{');
      size_t closebrace = line.find('}');
      size_t colon      = line.find(':');

      if(openbrace == std::string::npos && closebrace == std::string::npos && colon == std::string::npos) {
         continue;
      }
      // printf("line : %s\n",line.c_str());

      //*************************************//
      if(closebrace != std::string::npos) {
         // printf("brace closed.\n");
         // channel->Print();
         brace_open = false;
         if(channel != nullptr) { // && (channel->GetAddress()!=0) )
            TChannel* currentchan = GetChannel(channel->GetAddress());
            if(currentchan == nullptr) {
               AddChannel(channel); // consider using a default option here
               newchannels++;
            } else {
               //				 currentchan->Print();
               currentchan->UpdateChannel(channel);
               delete channel;
               newchannels++;
            }
         } else {
            delete channel;
         }
         channel = nullptr;
         name.clear();
         // detector = 0;
      }
      //*************************************//
      if(openbrace != std::string::npos) {
         brace_open = true;
         name       = line.substr(0, openbrace);
         channel    = new TChannel(""); // GetChannel(0);
         channel->SetName(name.c_str());
      }
      //*************************************//
      if(brace_open) {
         size_t ntype = line.find(':');
         if(ntype != std::string::npos) {
            std::string type = line.substr(0, ntype);
            line             = line.substr(ntype + 1, line.length());
            trim(&line);
            std::istringstream ss(line);
            int                j = 0;
            while(type[j] != 0) {
               char c    = *(type.c_str() + j);
               c         = toupper(c);
               type[j++] = c;
            }
            // printf("type = %s\n",type.c_str());
            if(type.compare("NAME") == 0) {
               channel->SetName(line.c_str());
            } else if(type.compare("ADDRESS") == 0) {
               unsigned int tempadd = 0;
               ss >> tempadd;
               if(tempadd == 0) { // maybe it is in hex...
                  std::stringstream newss;
                  newss<<std::hex<<line;
                  newss >> tempadd;
               }
               tempadd = tempadd & 0x00ffffff; // front end number is not included in the odb...
               channel->SetAddress(tempadd);
            } else if(type.compare("INTEGRATION") == 0) {
               int tempint;
               ss >> tempint;
               channel->SetIntegration(tempint);
            } else if(type.compare("NUMBER") == 0) {
               int tempnum;
               ss >> tempnum;
               channel->SetNumber(tempnum);
            } else if(type.compare("TIMEOFFSET") == 0) {
               int tempoff;
               ss >> tempoff;
               channel->SetTimeOffset(tempoff);
            } else if(type.compare("STREAM") == 0) {
               int tempstream;
               ss >> tempstream;
               channel->SetStream(tempstream);
            } else if(type.compare("DIGITIZER") == 0) {
               int k = 0;
               while(type[k] != 0) {
                  char c    = *(type.c_str() + j);
                  c         = toupper(c);
                  type[k++] = c;
               }
               channel->SetDigitizerType(line.c_str());
            } else if(type.compare("ENGCHI2") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetENGChi2(tempdbl);
            } else if(type.compare("CFDCHI2") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetCFDChi2(tempdbl);
            } else if(type.compare("LEDCHI2") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetLEDChi2(tempdbl);
            } else if(type.compare("TIMECHI2") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetTIMEChi2(tempdbl);
            } else if(type.compare("EFFCHI2") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetEFFChi2(tempdbl);
            } else if(type.compare("ENGCOEFF") == 0) {
               channel->DestroyENGCal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddENGCoefficient(value);
               }
            } else if(type.compare("LEDCOEFF") == 0) {
               channel->DestroyLEDCal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddLEDCoefficient(value);
               }
            } else if(type.compare("CFDCOEFF") == 0) {
               channel->DestroyCFDCal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddCFDCoefficient(value);
               }
            } else if(type.compare("TIMECOEFF") == 0) {
               channel->DestroyTIMECal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddTIMECoefficient(value);
               }
            } else if(type.compare("CTCOEFF") == 0) {
               channel->DestroyCTCal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddCTCoefficient(value);
               }
            } else if(type.compare("WALK") == 0) {
               channel->DestroyTIMECal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddTIMECoefficient(value);
               }
            } else if(type.compare("EFFCOEFF") == 0) {
               channel->DestroyEFFCal();
               double value;
               while(!(ss >> value).fail()) {
                  channel->AddEFFCoefficient(value);
               }
            } else if(type.compare("FILEINT") == 0) {
               int tempstream;
               ss >> tempstream;
               if(tempstream > 0) {
                  channel->SetUseCalFileIntegration(true);
               } else {
                  channel->SetUseCalFileIntegration(false);
               }
            } else if(type.compare("RISETIME") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetWaveRise(tempdbl);
            } else if(type.compare("DECAYTIME") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetWaveDecay(tempdbl);
            } else if(type.compare("BASELINE") == 0) {
               double tempdbl;
               ss >> tempdbl;
               channel->SetWaveBaseLine(tempdbl);
            } else {
            }
         }
      }
   }
   if(strcmp(opt, "q") != 0) {
      printf("parsed %i lines.\n", linenumber);
   }

   return newchannels;
}

void TChannel::trim(std::string* line, const std::string& trimChars)
{
   /// Removes the the string "trimCars" from  the string 'line'
   if(line->length() == 0) {
      return;
   }
   std::size_t found = line->find_first_not_of(trimChars);
   if(found != std::string::npos) {
      *line = line->substr(found, line->length());
   }
   found = line->find_last_not_of(trimChars);
   if(found != std::string::npos) {
      *line = line->substr(0, found + 1);
   }
   return;
}

void TChannel::Streamer(TBuffer& R__b)
{
   SetBit(kCanDelete);
   UInt_t R__s, R__c;
   if(R__b.IsReading()) { // reading from file
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if(R__v != 0) {
      }
      TNamed::Streamer(R__b);
      {
         TString R__str;
         R__str.Streamer(R__b);
         fFileName.assign(R__str.Data());
      }
      {
         TString R__str;
         R__str.Streamer(R__b);
         fFileData.assign(R__str.Data());
      }
      InitChannelInput();
      R__b.CheckByteCount(R__s, R__c, TChannel::IsA());
   } else { // writing to file
      R__c = R__b.WriteVersion(TChannel::IsA(), true);
      TNamed::Streamer(R__b);
      {
         TString R__str = fFileName.c_str();
         R__str.Streamer(R__b);
      }
      {
         TString R__str = fFileData.c_str();
         R__str.Streamer(R__b);
      }
      R__b.SetByteCount(R__c, true);
   }
}

int TChannel::WriteToRoot(TFile* fileptr)
{
   /// Writes Cal File information to the tree
   TChannel* c = GetDefaultChannel();
   // Maintain old gDirectory info
   TDirectory* savdir = gDirectory;

   if(c == nullptr) {
      printf("No TChannels found to write.\n");
   }
   if(fileptr == nullptr) {
      fileptr = gDirectory->GetFile();
   }

   fileptr->cd();
   std::string oldoption = std::string(fileptr->GetOption());
   if(oldoption == "READ") {
      fileptr->ReOpen("UPDATE");
   }
   if(!gDirectory) {
      printf("No file opened to write to.\n");
   }
   TIter iter(gDirectory->GetListOfKeys());

   // printf("1 Number of Channels: %i\n",GetNumberOfChannels());
   // gDirectory->ls();

   bool        found         = false;
   std::string mastername    = "TChannel";
   std::string mastertitle   = "TChannel";
   std::string channelbuffer = fFileData;
   // std::map<std::string,int> indexmap;
   WriteCalBuffer();
   std::string savedata = fFileData;

   FILE* originalstdout = stdout;
   int   fd             = open("/dev/null", O_WRONLY); // turn off stdout.
   stdout               = fdopen(fd, "w");

   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TChannel") != 0)) {
         continue;
      }
      if(!found) {
         found        = true;
         TChannel* ch = static_cast<TChannel*>(key->ReadObj());
         mastername.assign(ch->GetName());
         mastertitle.assign(ch->GetTitle());
      }
      std::string cname = key->ReadObj()->GetName();
      // TFile* f = gDirectory->GetFile();
      cname.append(";*");
      gDirectory->Delete(cname.c_str());
      // indexmap[cname]++;
      // std::string cnamei = cname; cnamei.append(Form(";%i",indexmap[cname]));
      // printf("cnamei = %s\n",cnamei.c_str());
      // gDirectory->Delete(cnamei.c_str());
      TChannel::DeleteAllChannels();
   }

   stdout = originalstdout; // Restore stdout

   ParseInputData(savedata.c_str(), "q");
   SaveToSelf(savedata.c_str());
   // printf("1 Number of Channels: %i\n",GetNumberOfChannels());
   // gDirectory->ls();
   TChannel::ParseInputData(channelbuffer.c_str(), "q");
   c = TChannel::GetDefaultChannel();
   c->SetNameTitle(mastername.c_str(), mastertitle.c_str());
   c->Write("", TObject::kOverwrite);

   ParseInputData(savedata.c_str(), "q");
   SaveToSelf(savedata.c_str());

   // printf("1 Number of Channels: %i\n",GetNumberOfChannels());
   // gDirectory->ls();
   // TChannel::DeleteAllChannels();
   // gDirectory->GetFile()->Get("c->GetName()");
   printf("  %i TChannels saved to %s.\n", GetNumberOfChannels(), gDirectory->GetFile()->GetName());
   if(oldoption == "READ") {
      printf("  Returning %s to \"%s\" mode.\n", gDirectory->GetFile()->GetName(), oldoption.c_str());
      fileptr->ReOpen("READ");
   }
   savdir->cd(); // Go back to original gDirectory
   return GetNumberOfChannels();
}

int TChannel::GetDetectorNumber() const
{
   if(fDetectorNumber > -1) { //||fDetectorNumber==0x0fffffff)
      return fDetectorNumber;
   }

   fDetectorNumber = static_cast<int32_t>(fMnemonic.ArrayPosition());
   return fDetectorNumber;
}

int TChannel::GetSegmentNumber() const
{
   if(fSegmentNumber > -1) {
      return fSegmentNumber;
   }

   std::string name = GetName();
   if(name.length() < 10) {
      fSegmentNumber = 0;
   } else {
      TString str = name[9];
      if(str.IsDigit()) {
         std::string buf;
         buf.clear();
         buf.assign(name, 7, 3);
         fSegmentNumber = (int32_t)atoi(buf.c_str());
      } else {
         fSegmentNumber = static_cast<int32_t>(fMnemonic.Segment());
      }
   }

   return fSegmentNumber;
}

int TChannel::GetCrystalNumber() const
{
   if(fCrystalNumber > -1) {
      return fCrystalNumber;
   }

   switch(fMnemonic.ArraySubPosition()) {
		case TMnemonic::EMnemonic::kB:
			fCrystalNumber = 0;
			break;
		case TMnemonic::EMnemonic::kG:
			fCrystalNumber = 1;
			break;
		case TMnemonic::EMnemonic::kR:
			fCrystalNumber = 2;
			break;
		case TMnemonic::EMnemonic::kW:
			fCrystalNumber = 3;
			break;
		default:
			fCrystalNumber = 5;
			break;
   };

   // printf("%s: %c\t%i\n",__PRETTY_FUNCTION__,color,fCrystalNumber);
   return fCrystalNumber;
}
