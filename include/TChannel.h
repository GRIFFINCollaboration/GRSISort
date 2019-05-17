#ifndef TCHANNEL_H
#define TCHANNEL_H

/** \addtogroup Sorting
 *  @{
 */

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 *
 * Please indicate changes with your initials.
 *
 *
 */

//////////////////////////////////////////////////////////////////////////
///
/// \class TChannel
///
/// The TChannel is designed to hold all non-essential
/// information of a TFragment (now the same as a hit; name, energy coeff, etc..)
/// that would otherwise clog up the FragmentTree.  The TChannel class
/// contains a static map to every channel make retrieval fairly
/// easy.  The TChannel class also contains the ability to
/// read and write a custom calibration file to set or
/// save the TChannel information. Most of the information is read out
/// of the ODB information in the MIDAS file.
///
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cmath>
#include <utility>
#include <map>

#include "TNamed.h"
#include "TRandom.h"
#include "TList.h"
#include "TTree.h"
#include "TGraph.h"
#include "TMnemonic.h"
#include "TClassRef.h"
#include "Globals.h"
#include "TPriorityValue.h"

class TChannel : public TNamed {
public:
   static TChannel* GetChannel(unsigned int temp_address, bool warn = true);
   static TChannel* GetChannelByNumber(int temp_num);
   static TChannel* FindChannelByName(const char* ccName);
   static std::vector<TChannel*> FindChannelByRegEx(const char* ccName);

   TChannel();
   TChannel(const char*);
   TChannel(TChannel*);
	TChannel(const TChannel&);

   ~TChannel() override;

   static int  GetNumberOfChannels() { return fChannelMap->size(); }
   static void AddChannel(TChannel*, Option_t* opt = "");
   static int UpdateChannel(TChannel*, Option_t* opt = "");

   static std::map<unsigned int, TChannel*>* GetChannelMap() { return fChannelMap; }
   static std::map<unsigned int, int>* GetMissingChannelMap() { return fMissingChannelMap; }
   static void DeleteAllChannels();

   static bool CompareChannels(const TChannel&, const TChannel&);

   static TChannel* GetDefaultChannel();

	static void      SetMnemonicClass(TClassRef cl) { fMnemonicClass = cl; }
	static TClassRef GetMnemonicClass()             { return fMnemonicClass; }

private:
   unsigned int fAddress;     // The address of the digitizer
   TPriorityValue<int>          fIntegration; // The charge integration setting
   TPriorityValue<std::string>  fDigitizerTypeString;
	TPriorityValue<EDigitizer>   fDigitizerType;
   TPriorityValue<int>          fTimeStampUnit;
   TPriorityValue<int>          fNumber;
   TPriorityValue<int>          fStream;
   TPriorityValue<int>          fUserInfoNumber;
   TPriorityValue<bool>         fUseCalFileInt;

   mutable int fDetectorNumber;
   mutable int fSegmentNumber;
   mutable int fCrystalNumber;

   TPriorityValue<Long64_t>    fTimeOffset;
   TPriorityValue<TMnemonic*>  fMnemonic;
	static TClassRef  			 fMnemonicClass;

   TPriorityValue<std::vector<Float_t> > fENGCoefficients;  // Energy calibration coeffs (low to high order)
   TPriorityValue<double>                fENGChi2;          // Chi2 of the energy calibration
   TPriorityValue<std::vector<double> >  fCFDCoefficients;  // CFD calibration coeffs (low to high order)
   TPriorityValue<double>                fCFDChi2;          // Chi2 of the CFD calibration
   TPriorityValue<std::vector<double> >  fLEDCoefficients;  // LED calibration coeffs (low to high order)
   TPriorityValue<double>                fLEDChi2;          // Chi2 of LED calibration
   TPriorityValue<std::vector<double> >  fTIMECoefficients; // Time calibration coeffs (low to high order)
   TPriorityValue<double>                fTIMEChi2;         // Chi2 of the Time calibration
   TPriorityValue<std::vector<double> >  fEFFCoefficients;  // Efficiency calibration coeffs (low to high order)
   TPriorityValue<double>                fEFFChi2;          // Chi2 of Efficiency calibration
   TPriorityValue<std::vector<double> >  fCTCoefficients;   // Cross talk coefficients
	TPriorityValue<TGraph>                fEnergyNonlinearity; // Energy nonlinearity as spline

   struct WaveFormShapePar {
      bool   InUse;
      double BaseLine;
      double TauDecay;
      double TauRise;
   };

   WaveFormShapePar WaveFormShape;

   static std::map<unsigned int, TChannel*>* fChannelMap;       // A map to all of the channels based on address
   static std::map<unsigned int, int>* fMissingChannelMap;      // A map to all of the missing channels based on address
   static std::map<int, TChannel*>*          fChannelNumberMap; // A map of TChannels based on channel number
   static void UpdateChannelNumberMap();
   static void UpdateChannelMap();
   void        OverWriteChannel(TChannel*);
   void        AppendChannel(TChannel*);

   void SetENGCoefficients(TPriorityValue<std::vector<Float_t> > tmp) { fENGCoefficients = tmp; }
   void SetCFDCoefficients(TPriorityValue<std::vector<double> > tmp) { fCFDCoefficients = tmp; }
   void SetLEDCoefficients(TPriorityValue<std::vector<double> > tmp) { fLEDCoefficients = tmp; }
   void SetTIMECoefficients(TPriorityValue<std::vector<double> > tmp) { fTIMECoefficients = tmp; }
   void SetEFFCoefficients(TPriorityValue<std::vector<double> > tmp) { fEFFCoefficients = tmp; }
   void SetCTCoefficients(TPriorityValue<std::vector<double> > tmp) { fCTCoefficients = tmp; }
	void SetEnergyNonlinearity(TPriorityValue<TGraph> tmp) { fEnergyNonlinearity = tmp; }

	void SetupEnergyNonlinearity(); // sort energy nonlinearity graph and set name/title

   static void trim(std::string*, const std::string& trimChars = " \f\n\r\t\v");

public:
   void SetName(const char* tmpName) override;
   void SetAddress(unsigned int tmpadd);
   inline void SetNumber(TPriorityValue<int> tmp)
   {
      fNumber = tmp;
      UpdateChannelNumberMap();
   }
   inline void SetIntegration(TPriorityValue<int> tmp) { fIntegration = tmp; }
   static void SetIntegration(const std::string& mnemonic, int tmpint, EPriority pr);
   inline void SetStream(TPriorityValue<int> tmp) { fStream = tmp; }
   inline void SetUserInfoNumber(TPriorityValue<int> tmp) { fUserInfoNumber = tmp; }
   inline void SetDigitizerType(TPriorityValue<std::string> tmp)
   {
      fDigitizerTypeString = tmp;
      fMnemonic.Value()->EnumerateDigitizer(fDigitizerTypeString, fDigitizerType, fTimeStampUnit);
   }
   static void SetDigitizerType(const std::string& mnemonic, const char* tmpstr, EPriority pr);
   inline void SetTimeOffset(TPriorityValue<Long64_t> tmp) { fTimeOffset = tmp; }

   void SetDetectorNumber(int tempint) { fDetectorNumber = tempint; }
   void SetSegmentNumber(int tempint) { fSegmentNumber = tempint; }
   void SetCrystalNumber(int tempint) { fCrystalNumber = tempint; }

   int               GetDetectorNumber() const;
   int               GetSegmentNumber() const;
   int               GetCrystalNumber() const;
   const TMnemonic*  GetMnemonic() const { return fMnemonic.Value(); }
   TClass*           GetClassType() const { return fMnemonic.Value()->GetClassType(); }
   void              SetClassType(TClass* cl_type) { fMnemonic.Value()->SetClassType(cl_type); }

   int          GetNumber() const { return fNumber.Value(); }
   unsigned int GetAddress() const { return fAddress; }
   int          GetIntegration() const { return fIntegration.Value(); }
   int          GetStream() const { return fStream.Value(); }
   int          GetUserInfoNumber() const { return fUserInfoNumber.Value(); }
   const char*  GetDigitizerTypeString() const { return fDigitizerTypeString.Value().c_str(); }
	EDigitizer   GetDigitizerType() const { return fDigitizerType.Value(); }
	int          GetTimeStampUnit() const { return fTimeStampUnit.Value(); }
   Long64_t     GetTimeOffset() const { return fTimeOffset.Value(); }
   // write the rest of the gettters/setters...

   double GetENGChi2() const { return fENGChi2.Value(); }
   double GetCFDChi2() const { return fCFDChi2.Value(); }
   double GetLEDChi2() const { return fLEDChi2.Value(); }
   double GetTIMEChi2() const { return fTIMEChi2.Value(); }
   double GetEFFChi2() const { return fEFFChi2.Value(); }

   void SetUseCalFileIntegration(TPriorityValue<bool> tmp = TPriorityValue<bool>(true, EPriority::kUser)) { fUseCalFileInt = tmp; }
   static void SetUseCalFileIntegration(const std::string& mnemonic, bool flag, EPriority pr);
   bool UseCalFileIntegration() { return fUseCalFileInt.Value(); }

   std::vector<Float_t> GetENGCoeff() const { return fENGCoefficients.Value(); }
   std::vector<double>  GetCFDCoeff() const { return fCFDCoefficients.Value(); }
   std::vector<double>  GetLEDCoeff() const { return fLEDCoefficients.Value(); }
   std::vector<double>  GetTIMECoeff() const { return fTIMECoefficients.Value(); }
   std::vector<double>  GetEFFCoeff() const { return fEFFCoefficients.Value(); }
   std::vector<double>  GetCTCoeff() const { return fCTCoefficients.Value(); }
	TGraph               GetEnergyNonlinearity() const { return fEnergyNonlinearity.Value(); }
	double               GetEnergyNonlinearity(double en) const;

   inline void AddENGCoefficient(Float_t temp) { fENGCoefficients.Address()->push_back(temp); }
   inline void AddCFDCoefficient(double temp) { fCFDCoefficients.Address()->push_back(temp); }
   inline void AddLEDCoefficient(double temp) { fLEDCoefficients.Address()->push_back(temp); }
   inline void AddTIMECoefficient(double temp) { fTIMECoefficients.Address()->push_back(temp); }
   inline void AddEFFCoefficient(double temp) { fEFFCoefficients.Address()->push_back(temp); }
   inline void AddCTCoefficient(double temp) { fCTCoefficients.Address()->push_back(temp); }
	void AddEnergyNonlinearityPoint(double x, double y) { fEnergyNonlinearity.Address()->SetPoint(fEnergyNonlinearity.Address()->GetN(), x, y); }

   inline void SetENGChi2(TPriorityValue<double> tmp) { fENGChi2 = tmp; }
   inline void SetCFDChi2(TPriorityValue<double> tmp) { fCFDChi2 = tmp; }
   inline void SetLEDChi2(TPriorityValue<double> tmp) { fLEDChi2 = tmp; }
   inline void SetTIMEChi2(TPriorityValue<double> tmp) { fTIMEChi2 = tmp; }
   inline void SetEFFChi2(TPriorityValue<double> tmp) { fEFFChi2 = tmp; }

	inline void SetWaveRise(double temp)
	{
		WaveFormShape.TauRise = temp;
		SetUseWaveParam();
	}
	inline void SetWaveDecay(double temp)
	{
		WaveFormShape.TauDecay = temp;
		SetUseWaveParam();
	}
	inline void SetWaveBaseLine(double temp)
	{
		WaveFormShape.BaseLine = temp;
		SetUseWaveParam();
	}

	inline void SetUseWaveParam(bool temp = true) { WaveFormShape.InUse = temp; }
	inline void SetWaveParam(WaveFormShapePar temp) { WaveFormShape = temp; }

   double           GetWaveRise() const { return WaveFormShape.TauRise; }
   double           GetWaveDecay() const { return WaveFormShape.TauDecay; }
   double           GetWaveBaseLine() const { return WaveFormShape.BaseLine; }
   bool             UseWaveParam() const { return WaveFormShape.InUse; }
   WaveFormShapePar GetWaveParam() const { return WaveFormShape; }

   double CalibrateENG(double);
   double CalibrateENG(double, int temp_int);
   double CalibrateENG(int, int temp_int = 0);

   double CalibrateCFD(double);
   double CalibrateCFD(int);

   double CalibrateLED(double);
   double CalibrateLED(int);

   double        CalibrateTIME(double);
   double        CalibrateTIME(int);
   inline double GetTZero(double tempd) { return CalibrateTIME(tempd); }
   inline double GetTZero(int tempi) { return CalibrateTIME(tempi); }

   double CalibrateEFF(double);

   void DestroyCalibrations();

   void DestroyENGCal();
   void DestroyCFDCal();
   void DestroyLEDCal();
   void DestroyTIMECal();
   void DestroyEFFCal();
   void DestroyCTCal();
	void DestroyEnergyNonlinearity();

   static Int_t ReadCalFromCurrentFile(Option_t* opt = "overwrite");
   static Int_t ReadCalFromTree(TTree*, Option_t* opt = "overwrite");
   static Int_t ReadCalFromFile(TFile* tempf, Option_t* opt = "overwrite");
   static Int_t ReadCalFile(const char* filename = "");
   static Int_t ParseInputData(const char* inputdata = "", Option_t* opt = "", EPriority pr = EPriority::kUser);
   static void WriteCalFile(const std::string& outfilename = "");
   static void WriteCTCorrections(const std::string& outfilename = "");
   static void WriteCalBuffer(Option_t* opt = "");

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
   // static  void PrintAll(Option_t* opt = "");
   std::string PrintToString(Option_t* opt = "");
   std::string PrintCTToString(Option_t* opt = "");
   void PrintCTCoeffs(Option_t* opt = "") const;

   static int WriteToRoot(TFile* fileptr = nullptr);

private:
   // the follow is to make the custom streamer
   // stuff play nice.  pcb.
   static std::string fFileName;
   static std::string fFileData;
   static void        InitChannelInput();
   static void        SaveToSelf(const char*);

   /// \cond CLASSIMP
   ClassDefOverride(TChannel, 5) // Contains the Digitizer Information
   /// \endcond
};
/*! @} */
#endif
