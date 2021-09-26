#include "TChannel.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include<unordered_map>

#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>

#include "TFile.h"
#include "TKey.h"

#include "StoppableThread.h"

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 *
 * Please indicate changes with your initials.
 *
 *
 */


//NOTE: The fChannelMap is unordered. The easiest way to order it is using std::map<int, int> ordered(unordered.begin(), unordered.end());

/// \cond CLASSIMP
ClassImp(TChannel)
/// \endcond

std::unordered_map<unsigned int, TChannel*>* TChannel::fChannelMap = new std::unordered_map<unsigned int, TChannel*>; // global maps of channels
std::unordered_map<unsigned int, int>* TChannel::fMissingChannelMap = new std::unordered_map<unsigned int, int>; // global map of missing channels
std::unordered_map<int, TChannel*>* TChannel::fChannelNumberMap = new std::unordered_map<int, TChannel*>;

//TClass* TChannel::fMnemonicClass = TMnemonic::Class();
TClassRef TChannel::fMnemonicClass = TClassRef("TMnemonic");

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
	// only set name if it's not empty
	if(strlen(tempName)>0) SetName(tempName);
}

TChannel::TChannel(const TChannel& chan) : TNamed(chan)
{
	/// Makes a copy of a the TChannel.
	Clear();
	*(fMnemonic.Value()) = *(chan.fMnemonic.Value());
	SetAddress(chan.GetAddress());
	SetIntegration(chan.fIntegration);
	SetNumber(chan.fNumber);
	SetStream(chan.fStream);
	SetUserInfoNumber(chan.fUserInfoNumber);
	SetName(chan.GetName());
	SetDigitizerType(chan.fDigitizerTypeString);
	SetTimeOffset(chan.fTimeOffset);
	SetAllENGCoefficients(chan.fENGCoefficients);
	SetENGRanges(chan.fENGRanges);
	SetAllENGChi2(chan.fENGChi2);
	SetENGDriftCoefficents(chan.fENGDriftCoefficents);
	SetCFDCoefficients(chan.fCFDCoefficients);
	SetLEDCoefficients(chan.fLEDCoefficients);
	SetTIMECoefficients(chan.fTIMECoefficients);
	SetEFFCoefficients(chan.fEFFCoefficients);
	SetCTCoefficients(chan.fCTCoefficients);
	SetEnergyNonlinearity(chan.fEnergyNonlinearity);
	SetCFDChi2(chan.fCFDChi2);
	SetLEDChi2(chan.fLEDChi2);
	SetTIMEChi2(chan.fTIMEChi2);
	SetEFFChi2(chan.fEFFChi2);
	SetUseCalFileIntegration(chan.fUseCalFileInt);
	SetWaveParam(chan.GetWaveParam());
	SetDetectorNumber(chan.GetDetectorNumber());
	SetSegmentNumber(chan.GetSegmentNumber());
	SetCrystalNumber(chan.GetCrystalNumber());

	SetClassType(chan.GetClassType());
}

TChannel::TChannel(TChannel* chan)
{
	/// Makes a copy of a the TChannel.
	Clear();
	*(fMnemonic.Value()) = *(chan->fMnemonic.Value());
	SetAddress(chan->GetAddress());
	SetIntegration(chan->fIntegration);
	SetNumber(chan->fNumber);
	SetStream(chan->fStream);
	SetUserInfoNumber(chan->fUserInfoNumber);
	SetName(chan->GetName());
	SetDigitizerType(chan->fDigitizerTypeString);
	SetTimeOffset(chan->fTimeOffset);
	SetAllENGCoefficients(chan->fENGCoefficients);
	SetENGRanges(chan->fENGRanges);
	SetAllENGChi2(chan->fENGChi2);
	SetENGDriftCoefficents(chan->fENGDriftCoefficents);
	SetCFDCoefficients(chan->fCFDCoefficients);
	SetLEDCoefficients(chan->fLEDCoefficients);
	SetTIMECoefficients(chan->fTIMECoefficients);
	SetEFFCoefficients(chan->fEFFCoefficients);
	SetCTCoefficients(chan->fCTCoefficients);
	SetEnergyNonlinearity(chan->fEnergyNonlinearity);
	SetCFDChi2(chan->fCFDChi2);
	SetLEDChi2(chan->fLEDChi2);
	SetTIMEChi2(chan->fTIMEChi2);
	SetEFFChi2(chan->fEFFChi2);
	SetUseCalFileIntegration(chan->fUseCalFileInt);
	SetWaveParam(chan->GetWaveParam());
	SetDetectorNumber(chan->GetDetectorNumber());
	SetSegmentNumber(chan->GetSegmentNumber());
	SetCrystalNumber(chan->GetCrystalNumber());

	SetClassType(chan->GetClassType());
}

void TChannel::SetName(const char* tmpName)
{
	if(strlen(tmpName) == 0) return;
	TNamed::SetName(tmpName);
	// do not parse the default name
	if(strcmp(tmpName, "DefaultTChannel") != 0) {
		fMnemonic.Value()->Parse(GetName());
	}
}

void TChannel::InitChannelInput()
{
	int channels_found = ParseInputData(fFileData.c_str(), "q", EPriority::kRootFile);
	if(gFile != nullptr) {
		std::cout<<"Successfully read "<<channels_found<<" TChannels from "<<CYAN<<gFile->GetName()<<RESET_COLOR<<std::endl;
	} else {
		std::cout<<"Successfully read "<<channels_found<<" TChannels"<<std::endl;
	}
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
	for(auto iter : *fChannelMap) {
		delete iter.second;
		// These maps should point to the same pointers, so this should clear out both
		iter.second = nullptr;
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
			int oldNumber = oldchan->GetNumber();
			oldchan->OverWriteChannel(chan);
			// Need to also update the channel number map RD
			if(oldNumber != oldchan->GetNumber()) {
				// channel number has changed so we need to delete the old one and insert the new one
				fChannelNumberMap->erase(oldNumber);
				if((oldchan->GetNumber() != 0) && (fChannelNumberMap->count(oldchan->GetNumber()) == 0)) {
					fChannelNumberMap->insert(std::make_pair(oldchan->GetNumber(), oldchan));
				}
			}
			return;
		}
		std::cout<<"Trying to add a channel that already exists!"<<std::endl;
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
}

void TChannel::OverWriteChannel(TChannel* chan)
{
	/// Overwrites the current TChannel with chan.
	SetAddress(chan->GetAddress());
	SetIntegration(TPriorityValue<int>(chan->GetIntegration(), EPriority::kForce));
	SetNumber(TPriorityValue<int>(chan->GetNumber(), EPriority::kForce));
	SetStream(TPriorityValue<int>(chan->GetStream(), EPriority::kForce));
	SetUserInfoNumber(TPriorityValue<int>(chan->GetUserInfoNumber(), EPriority::kForce));
	SetDigitizerType(TPriorityValue<std::string>(chan->GetDigitizerTypeString(), EPriority::kForce));
	SetName(chan->GetName());

	SetAllENGCoefficients(TPriorityValue<std::vector<std::vector<Float_t> > >(chan->GetAllENGCoeff(), EPriority::kForce));
	SetENGRanges(TPriorityValue<std::vector<std::pair<double, double> > >(chan->GetENGRanges(), EPriority::kForce));
	SetAllENGChi2(TPriorityValue<std::vector<double> >(chan->GetAllENGChi2(), EPriority::kForce));
	SetENGDriftCoefficents(TPriorityValue<std::vector<Float_t> >(chan->GetENGDriftCoefficents(), EPriority::kForce));
	SetCFDCoefficients(TPriorityValue<std::vector<double> >(chan->GetCFDCoeff(), EPriority::kForce));
	SetLEDCoefficients(TPriorityValue<std::vector<double> >(chan->GetLEDCoeff(), EPriority::kForce));
	SetTIMECoefficients(TPriorityValue<std::vector<double> >(chan->GetTIMECoeff(), EPriority::kForce));
	SetEFFCoefficients(TPriorityValue<std::vector<double> >(chan->GetEFFCoeff(), EPriority::kForce));
	SetCTCoefficients(TPriorityValue<std::vector<double> >(chan->GetCTCoeff(), EPriority::kForce));
	SetEnergyNonlinearity(TPriorityValue<TGraph>(chan->GetEnergyNonlinearity(), EPriority::kForce));

	SetCFDChi2(TPriorityValue<double>(chan->GetCFDChi2(), EPriority::kForce));
	SetLEDChi2(TPriorityValue<double>(chan->GetLEDChi2(), EPriority::kForce));
	SetTIMEChi2(TPriorityValue<double>(chan->GetTIMEChi2(), EPriority::kForce));
	SetEFFChi2(TPriorityValue<double>(chan->GetEFFChi2(), EPriority::kForce));

	SetUseCalFileIntegration(TPriorityValue<bool>(chan->UseCalFileIntegration(), EPriority::kForce));

	SetWaveParam(chan->GetWaveParam());

	SetDetectorNumber(chan->GetDetectorNumber());
	SetSegmentNumber(chan->GetSegmentNumber());
	SetCrystalNumber(chan->GetCrystalNumber());
	SetTimeOffset(TPriorityValue<Long64_t>(chan->GetTimeOffset(), EPriority::kForce));
	SetClassType(chan->GetClassType());
}

void TChannel::AppendChannel(TChannel* chan)
{
	/// Sets the current TChannel to chan
	SetIntegration(chan->fIntegration);
	SetNumber(chan->fNumber);
	SetStream(chan->fStream);
	SetUserInfoNumber(chan->fUserInfoNumber);
	if(strcmp(chan->GetName(), "DefaultTChannel") != 0) SetName(chan->GetName()); // don't overwrite an existing name by the default name
	SetDigitizerType(chan->fDigitizerTypeString);
	SetTimeOffset(chan->fTimeOffset);
	SetAllENGCoefficients(chan->fENGCoefficients);
	SetENGRanges(chan->fENGRanges);
	SetAllENGChi2(chan->fENGChi2);
	SetENGDriftCoefficents(chan->fENGDriftCoefficents);
	SetCFDCoefficients(chan->fCFDCoefficients);
	SetLEDCoefficients(chan->fLEDCoefficients);
	SetTIMECoefficients(chan->fTIMECoefficients);
	SetEFFCoefficients(chan->fEFFCoefficients);
	SetCTCoefficients(chan->fCTCoefficients);
	SetEnergyNonlinearity(chan->fEnergyNonlinearity);
	SetCFDChi2(chan->fCFDChi2);
	SetLEDChi2(chan->fLEDChi2);
	SetTIMEChi2(chan->fTIMEChi2);
	SetEFFChi2(chan->fEFFChi2);
	SetUseCalFileIntegration(chan->fUseCalFileInt);
	SetWaveParam(chan->GetWaveParam());
	SetDetectorNumber(chan->GetDetectorNumber());
	SetSegmentNumber(chan->GetSegmentNumber());
	SetCrystalNumber(chan->GetCrystalNumber());

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
	fAddress = 0xffffffff;
	fIntegration.Reset(0);
	fDigitizerTypeString = TPriorityValue<std::string>();
	//fDigitizerType.Reset(EDigitizer::kDefault);
	fNumber.Reset(0);
	fStream.Reset(0);
	fUserInfoNumber.Reset(0xffffffff);
	fUseCalFileInt.Reset(false);

	fDetectorNumber = -1;
	fSegmentNumber = -1;
	fCrystalNumber = -1;
	fTimeOffset.Reset(0);

	WaveFormShape = WaveFormShapePar();

	fMnemonic = TPriorityValue<TMnemonic*>(static_cast<TMnemonic*>(fMnemonicClass->New()), EPriority::kForce);
	SetName("DefaultTChannel");

	fENGCoefficients.Reset(std::vector<std::vector<Float_t> >());
	fENGRanges.Reset(std::vector<std::pair<double, double> >());
	fENGChi2.Reset(std::vector<double>());
	fENGDriftCoefficents.Reset(std::vector<Float_t>());
	fCFDCoefficients.Reset(std::vector<double>());
	fCFDChi2.Reset(0.0);
	fLEDCoefficients.Reset(std::vector<double>());
	fLEDChi2.Reset(0.0);
	fTIMECoefficients.Reset(std::vector<double>());
	fTIMEChi2.Reset(0.0);
	fEFFCoefficients.Reset(std::vector<double>());
	fEFFChi2.Reset(0.0);
	fCTCoefficients.Reset(std::vector<double>());
	fEnergyNonlinearity.Reset(TGraph());
}

TChannel* TChannel::GetChannel(unsigned int temp_address, bool warn)
{
	/// Returns the TChannel at the specified address. If the address doesn't exist, returns an empty gChannel.

	TChannel* chan = nullptr;
	if(fChannelMap->count(temp_address) == 1) { // found channel
		chan = fChannelMap->at(temp_address);
	}
	if(warn && chan == nullptr) {
		if(fMissingChannelMap->find(temp_address) == fMissingChannelMap->end()) {
			// if there are threads running we're not in interactive mode, so we print a warning about sorting
			if(StoppableThread::AnyThreadRunning()) {
				std::cerr<<RED<<"Failed to find channel for address 0x"<<std::hex<<temp_address<<std::dec<<", this channel won't get sorted properly!"<<RESET_COLOR<<std::endl;
			}
			fMissingChannelMap->insert(std::pair<unsigned int, int>(temp_address, 0));
		}
		++(*fMissingChannelMap)[temp_address];
	}
	return chan;
}

TChannel* TChannel::GetChannelByNumber(int temp_num)
{
	/// Returns the TChannel based on the channel number and not the channel address.
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

	for(auto iter : *fChannelMap) {
		chan                    = iter.second;
		std::string channelName = chan->GetName();
		if(channelName.compare(0, name.length(), name) == 0) {
			return chan;
		}
	}

	return nullptr;
}

std::vector<TChannel*> TChannel::FindChannelByRegEx(const char* ccName)
{
	/// Finds the TChannel by the name of the channel
	std::vector<TChannel*> result;
	if(ccName == nullptr) {
		return result;
	}

	std::regex regex(ccName);

	TChannel* chan;
	for(auto iter : *fChannelMap) {
		chan                    = iter.second;
		std::string channelName = chan->GetName();
		if(std::regex_match(channelName, regex)) {
			result.push_back(chan);
		}
	}

	return result;
}

void TChannel::SetAddress(unsigned int tmpadd)
{
	/// Sets the address of a TChannel and also overwrites that channel if it is in the channel map
	for(auto iter1 : *fChannelMap) {
		if(iter1.second == this) {
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
	fENGCoefficients.Address()->clear();
	fENGRanges.Address()->clear();
	fENGChi2.Address()->clear();
	fENGDriftCoefficents.Address()->clear();
}

void TChannel::DestroyCFDCal()
{
	/// Erases the CFDCoefficients vector
	fCFDCoefficients.Address()->clear();
}

void TChannel::DestroyLEDCal()
{
	/// Erases the LEDCoefficients vector
	fLEDCoefficients.Address()->clear();
}

void TChannel::DestroyTIMECal()
{
	/// Erases the TimeCal vector
	fTIMECoefficients.Address()->clear();
}

void TChannel::DestroyEFFCal()
{
	/// Erases the EffCal vector
	fEFFCoefficients.Address()->clear();
}

void TChannel::DestroyCTCal()
{
	// Erases the CTCal vector
	fCTCoefficients.Address()->clear();
}

void TChannel::DestroyEnergyNonlinearity()
{
	fEnergyNonlinearity.Address()->Set(0);
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

	// We need to add a random number between 0 and 1 before calibrating to avoid
	// binning issues.
	return CalibrateENG(static_cast<double>(charge) + gRandom->Uniform(), temp_int);
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

	if(temp_int == 0) {
		if(fIntegration.Value() != 0) {
			temp_int = fIntegration.Value();
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
	/// integration parameter. If multiple ranges are present, the calibration of the 
	/// range is used. In overlap regions the range used is chosen randomly.
	if(fENGCoefficients.empty()) {
		return charge;
	}
	// select range to use, they should be sorted 
	size_t currentRange = 0;
	if(!fENGRanges.empty()) {
		// we need a boolean to tell us if we found a range as the test 
		// currentRange+1 == fENGRanges.size()
		// fails if the charge is in the overlap between the second-to-last and last range
		// and the last range was selected
		bool foundRange = false;
		for(currentRange = 0; currentRange+1 < fENGRanges.size(); ++currentRange) {
			// check if the current range covers this charge
			if(fENGRanges.Value()[currentRange].first < charge && charge < fENGRanges.Value()[currentRange].second) {
				// check if there is an overlap with the next range in which case we select that one in 50% of the cases
				if(fENGRanges.Value()[currentRange+1].first < charge && charge < fENGRanges.Value()[currentRange+1].second) {
					if(gRandom->Uniform() > 0.5) ++currentRange;
				}
				foundRange = true;
				break;
			}
		}
		if(!foundRange) {
			currentRange = fENGRanges.size()-1;
			if(charge < fENGRanges[currentRange].first || fENGRanges[currentRange].second < charge) {
				std::cerr<<"Charge "<<charge<<" outside all ranges of calibration (first "<<fENGRanges.front().first<<" - "<<fENGRanges.front().second<<", last "<<fENGRanges.back().first<<" - "<<fENGRanges.back().second<<")"<<std::endl;
			}
		}
	}

	// apply the drift correction first
	if(!fENGDriftCoefficents.empty()) {
		double corrCharge = -fENGDriftCoefficents[0]; // ILL subtracts the offset instead of adding it
		for(size_t i = 1; i < fENGDriftCoefficents.size(); i++) {
			corrCharge += fENGDriftCoefficents[i] * pow(charge, i);
		}
		charge = corrCharge;
	}
	// if we had drift correction charge is now the corrected charge, otherwise it is still the charge
	double cal_chg = fENGCoefficients[currentRange][0];
	for(size_t i = 1; i < fENGCoefficients[currentRange].size(); i++) {
		cal_chg += fENGCoefficients[currentRange][i] * pow((charge), i);
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
	//std::cout<<cfd<<":";
	for(size_t i = 0; i < fCFDCoefficients.size(); i++) {
		cal_cfd += fCFDCoefficients[i] * pow(cfd, i);
		//std::cout<<" "<<i<<" - "<<fCFDCoefficients[i]<<" = "<<cal_cfd;
	}
	//std::cout<<std::endl;

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

void TChannel::SetUseCalFileIntegration(const std::string& mnemonic, bool flag, EPriority pr)
{
	/// Writes this UseCalFileIntegration to all channels in the current TChannel Map
	/// that starts with the mnemonic. Use "" to write to ALL channels
	/// WARNING: This is case sensitive!
	std::unordered_map<unsigned int, TChannel*>::iterator mapit;
	std::unordered_map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
	for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
		if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
			mapit->second->SetUseCalFileIntegration(TPriorityValue<bool>(flag, pr));
		}
	}
}

void TChannel::SetIntegration(const std::string& mnemonic, int tmpint, EPriority pr)
{
	// Writes this integration to all channels in the current TChannel Map
	// that starts with the mnemonic. Use "" to write to ALL channels
	// WARNING: This is case sensitive!
	std::unordered_map<unsigned int, TChannel*>::iterator mapit;
	std::unordered_map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
	for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
		if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
			mapit->second->SetIntegration(TPriorityValue<int>(tmpint, pr));
		}
	}
}

void TChannel::SetDigitizerType(const std::string& mnemonic, const char* tmpstr, EPriority pr)
{
	// Writes this digitizer type to all channels in the current TChannel Map
	// that starts with the mnemonic. Use "" to write to ALL channels
	// WARNING: This is case sensitive!
	std::unordered_map<unsigned int, TChannel*>::iterator mapit;
	std::unordered_map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();
	for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++) {
		if(mnemonic.empty() || (strncmp(mapit->second->GetName(), mnemonic.c_str(), mnemonic.size()) == 0)) {
			mapit->second->SetDigitizerType(TPriorityValue<std::string>(tmpstr, pr));
		}
	}
}

void TChannel::PrintCTCoeffs(Option_t*) const
{
	/// Prints out the current TChannel.
	std::cout<<GetName()<<"\t{\n"; //,channelname.c_str();
	std::cout<<"Name:      "<<GetName()<<std::endl;
	std::cout<<"Number:    "<<fNumber<<std::endl;
	std::cout<<std::setfill('0');
	std::cout<<"Address:   0x"<<std::hex<<std::setw(8)<<fAddress<<std::dec<<std::endl;
	for(double fCTCoefficient : fCTCoefficients) {
		std::cout<<fCTCoefficient<<"\t";
	}
	std::cout<<std::endl;
	std::cout<<"}\n";
	std::cout<<"//====================================//\n";
}

void TChannel::Print(Option_t*) const
{
	/// Prints out the current TChannel.
	std::cout<<PrintToString();
}

std::string TChannel::PrintCTToString(Option_t*) const
{
	std::string buffer;
	buffer.append("\n");
	buffer.append(GetName());
	buffer.append("\t{\n"); //,channelname.c_str();
	buffer.append("Name:      ");
	buffer.append(GetName());
	buffer.append("\n");
	buffer.append(Form("Number:    %d\n", fNumber.Value()));
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

std::string TChannel::PrintToString(Option_t*) const
{
	std::ostringstream str;

	str<<GetName()<<"\t{"<<std::endl; //,channelname.c_str();
	str<<"Type:      ";
	if(GetClassType() != nullptr) {
		str<<GetClassType()->GetName()<<std::endl;
	} else {
		str<<"None"<<std::endl;
	}

	str<<"Name:      "<<GetName()<<std::endl;
	str<<"Number:    "<<fNumber<<std::endl;
	str<<std::setfill('0');
	str<<"Address:   0x"<<std::hex<<std::setw(8)<<fAddress<<std::dec<<std::endl;
	str<<std::setfill(' ');
	if(!fDigitizerTypeString.empty()) {
		str<<"Digitizer: "<<fDigitizerTypeString<<std::endl;
	}
	str<<"TimeOffset: "<<fTimeOffset<<std::endl;
	str<<"Integration: "<<fIntegration<<std::endl;
	if(!fENGCoefficients.empty()) {
		for(size_t i = 0; i < fENGCoefficients.size(); ++i) {
			str<<"ENGCoeff:  range "<<i<<"\t";
			for(auto coeff : fENGCoefficients[i]) {
				str<<coeff<<"\t";
			}
			str<<std::endl;
		}
	}
	if(!fENGChi2.empty()) {
		for(size_t i = 0; i < fENGChi2.size(); ++i) {
			str<<"ENGChi2:   range "<<i<<"\t"<<fENGChi2[i]<<std::endl;
		}
	}
	if(!fENGRanges.empty()) {
		for(size_t i = 0; i < fENGRanges.size(); ++i) {
			str<<"ENGRange:   "<<i<<"\t"<<fENGRanges[i].first<<" "<<fENGRanges[i].second<<std::endl;
		}
	}
	if(!fENGDriftCoefficents.empty()) {
		str<<"ENGDrift:   ";
		auto oldPrecision = str.precision();
		str.precision(9);
		for(auto coeff : fENGDriftCoefficents) {
			str<<coeff<<"\t";
		}
		str.precision(oldPrecision);
		str<<std::endl;
	}
	if(!fEFFCoefficients.empty()) {
		str<<"EFFCoeff:  ";
		for(auto coeff : fEFFCoefficients) {
			str<<coeff<<"\t";
		}
		str<<std::endl;
	}
	if(fEFFChi2 != 0) {
		str<<"EFFChi2:   "<<fEFFChi2<<std::endl;
	}
	if(!fCFDCoefficients.empty()) {
		str<<"CFDCoeff:  ";
		for(auto coeff : fCFDCoefficients) {
			str<<coeff<<"\t";
		}
		str<<std::endl;
	}
	if(fEnergyNonlinearity.Value().GetN() > 0) {
		str<<"EnergyNonlinearity:  ";
		double* x = fEnergyNonlinearity.Value().GetX();
		double* y = fEnergyNonlinearity.Value().GetY();
		for(int i = 0; i < fEnergyNonlinearity.Value().GetN(); ++i) {
			str<<x[i]<<"\t"<<y[i]<<"\t";
		}
		str<<std::endl;
	}
	if(!fCTCoefficients.empty()) {
		str<<"CTCoeff:  ";
		for(auto coeff : fCTCoefficients) {
			str<<coeff<<"\t";
		}
		str<<std::endl;
	}
	if(!fTIMECoefficients.empty()) {
		str<<"TIMECoeff: ";
		for(auto coeff : fTIMECoefficients) {
			str<<coeff<<"\t";
		}
		str<<std::endl;
	}
	if(fUseCalFileInt) {
		str<<"FileInt: "<<fUseCalFileInt<<std::endl;
	}
	if(UseWaveParam()) {
		str<<"RiseTime: "<<WaveFormShape.TauRise<<std::endl;
		str<<"DecayTime: "<<WaveFormShape.TauDecay<<std::endl;
		str<<"BaseLine: "<<WaveFormShape.BaseLine<<std::endl;
	}
	str<<"}"<<std::endl;
	str<<"//====================================//"<<std::endl;
	std::string buffer = str.str();

	return buffer;
}

void TChannel::WriteCalFile(const std::string& outfilename)
{
	/// prints the context of addresschannelmap formatted correctly to stdout if
	/// no file name is passed to the function.  If a file name is passed to the function
	/// prints the context of addresschannelmap formatted correctly to a file with the given
	/// name.  This will earse and rewrite the file if the file already exisits!

	std::vector<TChannel*> chanVec;
	for(auto iter : *fChannelMap) {
		if(iter.second != nullptr) {
			chanVec.push_back(iter.second);
		}
	}

	//This orders channels nicely
	std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

	if(outfilename.length() > 0) {
		std::ofstream calout;
		calout.open(outfilename.c_str());
		for(auto iter_vec : chanVec) {
			std::string chanstr = iter_vec->PrintToString();
			calout<<chanstr.c_str();
			calout<<std::endl;
		}
		calout<<std::endl;
		calout.close();
	} else {
		for(auto iter_vec : chanVec) {
			iter_vec->Print();
		}
	}
}

void TChannel::WriteCTCorrections(const std::string& outfilename)
{
	std::vector<TChannel*> chanVec;
	for(auto iter : *fChannelMap) {
		if(iter.second != nullptr) {
			chanVec.push_back(iter.second);
		}
	}

	//This ordered channels nicely
	std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

	if(outfilename.length() > 0) {
		std::ofstream calout;
		calout.open(outfilename.c_str());
		for(auto iter_vec : chanVec) {
			std::string chanstr = iter_vec->PrintCTToString();
			calout<<chanstr.c_str();
			calout<<std::endl;
		}
		calout<<std::endl;
		calout.close();
	} else {
		for(auto iter_vec : chanVec) {
			iter_vec->PrintCTCoeffs();
		}
	}
}

void TChannel::WriteCalBuffer(Option_t*)
{
	/// writes any TChannels in memory to the internal buffer
	/// fFileData.  Can be used to over write info that is there
	/// or create the buffer if the channels originated from the odb.

	std::vector<TChannel*> chanVec;
	for(auto iter : *fChannelMap) {
		if(iter.second != nullptr) {
			chanVec.push_back(iter.second);
		}
	}

	//This ordered channels nicely
	std::sort(chanVec.begin(), chanVec.end(), TChannel::CompareChannels);

	std::string data;

	for(auto iter_vec : chanVec) {
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

	return ReadFile(tempf);
}

Int_t TChannel::ReadCalFromFile(TFile* tempf, Option_t*)
{
	/// Reads the TChannel information from a TFile if it has already been written to that File.
	if(tempf == nullptr) {
		return 0;
	}

	return ReadFile(tempf);
}

Int_t TChannel::ReadCalFromTree(TTree* tree, Option_t*)
{
	/// Reads the TChannel information from a Tree if it has already been written to that Tree.
	if(tree == nullptr) {
		return 0;
	}

	TFile* tempf = tree->GetCurrentFile();

	return ReadFile(tempf);
}

Int_t TChannel::ReadFile(TFile* tempf)
{
	TList* list  = tempf->GetListOfKeys();
	TIter  iter(list);
	while(TKey* key = static_cast<TKey*>(iter.Next())) {
		if((key == nullptr) || (strcmp(key->GetClassName(), "TChannel") != 0)) {
			continue;
		}
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

	auto* buffer = new char[length+1];//+1 for the null character to terminate the string
	infile.seekg(0, std::ios::beg);
	infile.read(buffer, length);
	buffer[length] = '\0';

	int channels_found = ParseInputData(const_cast<const char*>(buffer), "", EPriority::kInputFile);
	SaveToSelf(infilename.c_str());

	fChannelNumberMap->clear(); // This isn't the nicest way to do this but will keep us consistent.

	for(auto mapiter : *fChannelMap) {
		fChannelNumberMap->insert(std::make_pair(mapiter.second->GetNumber(), mapiter.second));
	}

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

Int_t TChannel::ParseInputData(const char* inputdata, Option_t* opt, EPriority pr)
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
		trim(line);
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

		//*************************************//
		if(closebrace != std::string::npos) {
			brace_open = false;
			if(channel != nullptr) {
				TChannel* currentchan = GetChannel(channel->GetAddress(), false);
				if(currentchan == nullptr) {
					AddChannel(channel); // consider using a default option here
					newchannels++;
				} else {
					currentchan->UpdateChannel(channel);
					delete channel;
					newchannels++;
				}
			} else {
				delete channel;
			}
			channel = nullptr;
			name.clear();
		}
		//*************************************//
		if(openbrace != std::string::npos) {
			brace_open = true;
			name       = line.substr(0, openbrace);
			trim(name);
			channel    = new TChannel("");
			if(!name.empty()) channel->SetName(name.c_str());
		}
		//*************************************//
		if(brace_open) {
			size_t ntype = line.find(':');
			if(ntype != std::string::npos) {
				std::string type = line.substr(0, ntype);
				line             = line.substr(ntype + 1, line.length());
				trim(line);
				std::istringstream ss(line);
				// transform type to upper case
				std::transform(type.begin(), type.end(), type.begin(), ::toupper);
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
					channel->SetIntegration(TPriorityValue<int>(tempint, pr));
				} else if(type.compare("NUMBER") == 0) {
					int tempnum;
					ss >> tempnum;
					channel->SetNumber(TPriorityValue<int>(tempnum, pr));
				} else if(type.compare("TIMEOFFSET") == 0) {
					Long64_t tempoff;
					ss >> tempoff;
					channel->SetTimeOffset(TPriorityValue<Long64_t>(tempoff, pr));
				} else if(type.compare("STREAM") == 0) {
					int tempstream;
					ss >> tempstream;
					channel->SetStream(TPriorityValue<int>(tempstream, pr));
				} else if(type.compare("DIGITIZER") == 0) {
					channel->SetDigitizerType(TPriorityValue<std::string>(line, pr));
				} else if(type.compare("ENGCHI2") == 0) {
					size_t range = line.find("range");
					if(range != std::string::npos) {
						line             = line.substr(range + 5, line.length());
						trim(line);
						ss.str(line);
						ss>>range;
					} else {
						range = 0;
					}
					double tempdbl;
					ss >> tempdbl;
					channel->SetENGChi2(TPriorityValue<double>(tempdbl, pr), range);
				} else if(type.compare("CFDCHI2") == 0) {
					double tempdbl;
					ss >> tempdbl;
					channel->SetCFDChi2(TPriorityValue<double>(tempdbl, pr));
				} else if(type.compare("LEDCHI2") == 0) {
					double tempdbl;
					ss >> tempdbl;
					channel->SetLEDChi2(TPriorityValue<double>(tempdbl, pr));
				} else if(type.compare("TIMECHI2") == 0) {
					double tempdbl;
					ss >> tempdbl;
					channel->SetTIMEChi2(TPriorityValue<double>(tempdbl, pr));
				} else if(type.compare("EFFCHI2") == 0) {
					double tempdbl;
					ss >> tempdbl;
					channel->SetEFFChi2(TPriorityValue<double>(tempdbl, pr));
				} else if(type.compare("ENGCOEFF") == 0) {
					size_t range = line.find("range");
					if(range != std::string::npos) {
						line             = line.substr(range + 5, line.length());
						trim(line);
						ss.str(line);
						ss>>range;
					} else {
						range = 0;
					}
					if(range == 0) {
						channel->DestroyENGCal();
						channel->fENGCoefficients.SetPriority(pr);
					}
					double value;
					while(!(ss >> value).fail()) {
						channel->AddENGCoefficient(value, range);
					}
				} else if(type.compare("ENGRANGE") == 0) {
					size_t range;
					double low;
					double high;
					ss>>range>>low>>high;
					channel->SetENGRange(std::make_pair(low, high), range);
				} else if(type.compare("ENGDRIFT") == 0) {
					double value;
					while(!(ss>>value).fail()) {
						channel->AddENGDriftCoefficent(value);
					}
				} else if(type.compare("LEDCOEFF") == 0) {
					channel->DestroyLEDCal();
					channel->fLEDCoefficients.SetPriority(pr);
					double value;
					while(!(ss >> value).fail()) {
						channel->AddLEDCoefficient(value);
					}
				} else if(type.compare("CFDCOEFF") == 0) {
					channel->DestroyCFDCal();
					channel->fCFDCoefficients.SetPriority(pr);
					double value;
					while(!(ss >> value).fail()) {
						channel->AddCFDCoefficient(value);
					}
				} else if((type.compare("TIMECOEFF") == 0) || (type.compare("WALK") == 0)) {
					channel->DestroyTIMECal();
					channel->fTIMECoefficients.SetPriority(pr);
					double value;
					while(!(ss >> value).fail()) {
						channel->AddTIMECoefficient(value);
					}
				} else if(type.compare("CTCOEFF") == 0) {
					channel->DestroyCTCal();
					channel->fCTCoefficients.SetPriority(pr);
					double value;
					while(!(ss >> value).fail()) {
						channel->AddCTCoefficient(value);
					}
				} else if(type.compare("ENERGYNONLINEARITY") == 0) {
					channel->DestroyEnergyNonlinearity();
					channel->fEnergyNonlinearity.SetPriority(pr);
					double x, y;
					while(!(ss>>x>>y).fail()) {
						channel->AddEnergyNonlinearityPoint(x, y);
					}
					channel->SetupEnergyNonlinearity();	
				} else if(type.compare("EFFCOEFF") == 0) {
					channel->DestroyEFFCal();
					channel->fEFFCoefficients.SetPriority(pr);
					double value;
					while(!(ss >> value).fail()) {
						channel->AddEFFCoefficient(value);
					}
				} else if(type.compare("FILEINT") == 0) {
					int tempstream;
					ss >> tempstream;
					if(tempstream > 0) {
						channel->SetUseCalFileIntegration(TPriorityValue<bool>(true, pr));
					} else {
						channel->SetUseCalFileIntegration(TPriorityValue<bool>(false, pr));
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

void TChannel::trim(std::string& line)
{
	/// Removes whitespace from  the string 'line'
	line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
				return !std::isspace(ch);
				}));
	line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch) { 
				return !std::isspace(ch);
				}).base(), line.end());
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
			fMnemonicClass->Streamer(R__b);
		}
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
			fMnemonicClass->Streamer(R__b);
		}
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
		std::cout<<"No TChannels found to write."<<std::endl;
		return 0;
	}
	if(fileptr == nullptr) {
		fileptr = gDirectory->GetFile();
	}

	// check if we got a file
	if(fileptr == nullptr) {
		std::cout<<"Error, no file provided and no file open (gDirectory = "<<gDirectory->GetName()<<")!"<<std::endl;
		return 0;
	}
	fileptr->cd();
	std::string oldoption = std::string(fileptr->GetOption());
	if(oldoption == "READ") {
		fileptr->ReOpen("UPDATE");
	}
	if(!gDirectory) { // we don't compare to nullptr here, as ROOT >= 6.24.00 uses the TDirectoryAtomicAdapter structure with a bool() operator
		std::cout<<"No file opened to write to."<<std::endl;
	}
	TIter iter(gDirectory->GetListOfKeys());

	bool        found         = false;
	std::string defaultName    = "Channel";
	std::string defaultTitle   = "TChannel";
	std::string channelbuffer = fFileData; //fFileData is the old TChannel information read from file
	WriteCalBuffer(); //replaces fFileData with the current channels
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
			defaultName.assign(ch->GetName());
			defaultTitle.assign(ch->GetTitle());
		}
		std::string cname = key->ReadObj()->GetName();
		cname.append(";*");
		gDirectory->Delete(cname.c_str());
		TChannel::DeleteAllChannels();
	}

	stdout = originalstdout; // Restore stdout

	ParseInputData(savedata.c_str(), "q", EPriority::kRootFile);
	SaveToSelf(savedata.c_str());
	TChannel::ParseInputData(channelbuffer.c_str(), "q", EPriority::kRootFile);
	c = TChannel::GetDefaultChannel();
	c->SetNameTitle(defaultName.c_str(), defaultTitle.c_str());
	c->Write("Channel", TObject::kOverwrite);

	ParseInputData(savedata.c_str(), "q", EPriority::kRootFile);
	SaveToSelf(savedata.c_str());

	std::cout<<"  "<<GetNumberOfChannels()<<" TChannels saved to "<<gDirectory->GetFile()->GetName()<<std::endl;
	if(oldoption == "READ") {
		std::cout<<"  Returning "<<gDirectory->GetFile()->GetName()<<" to \"READ\" mode."<<std::endl;
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

	fDetectorNumber = static_cast<int32_t>(fMnemonic.Value()->ArrayPosition());
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
			fSegmentNumber = static_cast<int32_t>(fMnemonic.Value()->Segment());
		}
	}

	return fSegmentNumber;
}

int TChannel::GetCrystalNumber() const
{
	if(fCrystalNumber > -1) {
		return fCrystalNumber;
	}

	fCrystalNumber = fMnemonic.Value()->NumericArraySubPosition();

	return fCrystalNumber;
}

double TChannel::GetEnergyNonlinearity(double en) const
{
	int nPoints = fEnergyNonlinearity.Value().GetN();
	if(nPoints < 1) return 0.;
	if(en < fEnergyNonlinearity.Value().GetX()[0] || fEnergyNonlinearity.Value().GetX()[nPoints-1] < en) return 0.;
	return fEnergyNonlinearity.Value().Eval(en);
}

void TChannel::SetupEnergyNonlinearity()
{
	fEnergyNonlinearity.Address()->Sort();
	fEnergyNonlinearity.Address()->SetBit(TGraph::kIsSortedX);
	fEnergyNonlinearity.Address()->SetName(Form("EnergyNonlinearity0x%04x", fAddress));
	fEnergyNonlinearity.Address()->SetTitle(Form("Energy nonlinearity of channel 0x%04x", fAddress));
}

void TChannel::ReadEnergyNonlinearities(TFile* file, const char* graphName, bool all)
{
	/// Read energy nonlinearities as TGraphErrors from provided root file using "graphName%x" as names
	TList* list  = file->GetListOfKeys();
	TIter  iter(list);
	while(TKey* key = static_cast<TKey*>(iter.Next())) {
		TClass* cl = TClass::GetClass(key->GetClassName());
		if(!cl->InheritsFrom(TGraph::Class()) || strncmp(key->GetName(), graphName, strlen(graphName)) != 0) {
			continue;
		}
		// get address from keys name
		std::stringstream str;
		str<<std::hex<<(key->GetName()+strlen(graphName));
		unsigned int address;
		str>>address;
		if(GetChannel(address) != nullptr) {
			GetChannel(address)->fEnergyNonlinearity.Set(*(static_cast<TGraph*>(key->ReadObj())), EPriority::kRootFile);
			GetChannel(address)->SetupEnergyNonlinearity();
		} else if(all) {
			TChannel* newChannel = new TChannel("");
			newChannel->SetAddress(address);
			newChannel->fEnergyNonlinearity.Set(*(static_cast<TGraph*>(key->ReadObj())), EPriority::kRootFile);
			newChannel->SetupEnergyNonlinearity();
			AddChannel(newChannel);
		}
	}
}
