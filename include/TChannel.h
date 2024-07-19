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
#include <unordered_map>

#include "TNamed.h"
#include "TRandom.h"
#include "TList.h"
#include "TTree.h"
#include "TGraph.h"
#include "TMnemonic.h"
#include "TClassRef.h"
#include "Globals.h"
#include "TPriorityValue.h"

enum class EDigitizer : char;
class TMnemonic;

class TChannel : public TNamed {
public:
   static TChannel*              GetChannel(unsigned int temp_address, bool warn = false);
   static TChannel*              GetChannelByNumber(int temp_num);
   static TChannel*              FindChannelByName(const char* ccName);
   static std::vector<TChannel*> FindChannelByRegEx(const char* ccName);

   TChannel();
   explicit TChannel(const char*);
   explicit TChannel(TChannel*);
   TChannel(const TChannel&);
   TChannel(TChannel&&);

   TChannel& operator=(const TChannel& rhs);
   TChannel& operator=(TChannel&&);

   ~TChannel() override;

   static size_t GetNumberOfChannels() { return fChannelMap->size(); }
   static void   AddChannel(TChannel*, Option_t* opt = "");
   static int    UpdateChannel(TChannel*, Option_t* opt = "");

   static std::unordered_map<unsigned int, TChannel*>* GetChannelMap() { return fChannelMap; }
   static std::unordered_map<unsigned int, int>*       GetMissingChannelMap() { return fMissingChannelMap; }
   static void                                         DeleteAllChannels();

   static bool CompareChannels(const TChannel*, const TChannel*);

   static TChannel* GetDefaultChannel();

   static void      SetMnemonicClass(const TClassRef& cls) { fMnemonicClass = cls; }
   static TClassRef GetMnemonicClass() { return fMnemonicClass; }

private:
   unsigned int                fAddress{0};       // The address of the digitizer
   TPriorityValue<int>         fIntegration{1};   // The charge integration setting
   TPriorityValue<std::string> fDigitizerTypeString;
   TPriorityValue<EDigitizer>  fDigitizerType;
   TPriorityValue<int>         fTimeStampUnit{0};
   TPriorityValue<int>         fNumber{0};
   TPriorityValue<int>         fStream{0};
   TPriorityValue<bool>        fUseCalFileInt{false};

   mutable int fDetectorNumber{-1};
   mutable int fSegmentNumber{-1};
   mutable int fCrystalNumber{-1};

   TPriorityValue<Long64_t>   fTimeOffset;
   TPriorityValue<double>     fTimeDrift;   ///< Time drift factor
   TPriorityValue<TMnemonic*> fMnemonic;
   static TClassRef           fMnemonicClass;

   TPriorityValue<std::vector<std::vector<Float_t>>>      fENGCoefficients;       ///< Energy calibration coeffs (low to high order)
   TPriorityValue<std::vector<std::pair<double, double>>> fENGRanges;             ///< Range of energy calibrations
   TPriorityValue<std::vector<double>>                    fENGChi2;               ///< Chi2 of the energy calibration
   TPriorityValue<std::vector<Float_t>>                   fENGDriftCoefficents;   ///< Energy drift coefficents (applied after energy calibration has been applied)
   TPriorityValue<std::vector<double>>                    fCFDCoefficients;       ///< CFD calibration coeffs (low to high order)
   TPriorityValue<double>                                 fCFDChi2;               ///< Chi2 of the CFD calibration
   TPriorityValue<std::vector<double>>                    fLEDCoefficients;       ///< LED calibration coeffs (low to high order)
   TPriorityValue<double>                                 fLEDChi2;               ///< Chi2 of LED calibration
   TPriorityValue<std::vector<double>>                    fTIMECoefficients;      ///< Time calibration coeffs (low to high order)
   TPriorityValue<double>                                 fTIMEChi2;              ///< Chi2 of the Time calibration
   TPriorityValue<std::vector<double>>                    fEFFCoefficients;       ///< Efficiency calibration coeffs (low to high order)
   TPriorityValue<double>                                 fEFFChi2;               ///< Chi2 of Efficiency calibration
   TPriorityValue<std::vector<double>>                    fCTCoefficients;        ///< Cross talk coefficients
   TPriorityValue<TGraph>                                 fEnergyNonlinearity;    ///< Energy nonlinearity as TGraph, is used as E=E+GetEnergyNonlinearity(E), so y should be E(source)-calibration(peak)

   struct WaveFormShapePar {
      bool   InUse;
      double BaseLine;
      double TauDecay;
      double TauRise;
   };

   WaveFormShapePar WaveFormShape{};

   static std::unordered_map<unsigned int, TChannel*>* fChannelMap;          // A map to all of the channels based on address
   static std::unordered_map<unsigned int, int>*       fMissingChannelMap;   // A map to all of the missing channels based on address
   static std::unordered_map<int, TChannel*>*          fChannelNumberMap;    // A map of TChannels based on channel number
   void                                                OverWriteChannel(TChannel*);
   void                                                AppendChannel(TChannel*);

   void SetupEnergyNonlinearity();   // sort energy nonlinearity graph and set name/title

   static std::vector<TChannel*> SortedChannels();

public:
   void        SetName(const char* tmpName) override;
   void        SetAddress(unsigned int tmpadd);
   inline void SetNumber(const TPriorityValue<int>& tmp)
   {
      if(fNumber == tmp) { return; }
      // channel number has changed so we need to delete the old one and insert the new one
      fChannelNumberMap->erase(fNumber.Value());
      fNumber = tmp;
      if((fNumber.Value() != 0) && (fChannelNumberMap->count(fNumber.Value()) == 0)) {
         fChannelNumberMap->insert(std::make_pair(fNumber.Value(), this));
      }
   }
   inline void SetIntegration(const TPriorityValue<int>& tmp) { fIntegration = tmp; }
   static void SetIntegration(const std::string& mnemonic, int tmpint, EPriority pr);
   inline void SetStream(const TPriorityValue<int>& tmp) { fStream = tmp; }
   void        SetDigitizerType(const TPriorityValue<std::string>& tmp);
   static void SetDigitizerType(const std::string& mnemonic, const char* tmpstr, EPriority prio);
   inline void SetTimeOffset(const TPriorityValue<Long64_t>& tmp) { fTimeOffset = tmp; }
   inline void SetTimeDrift(const TPriorityValue<double>& tmp) { fTimeDrift = tmp; }

   void SetDetectorNumber(int tempint) { fDetectorNumber = tempint; }
   void SetSegmentNumber(int tempint) { fSegmentNumber = tempint; }
   void SetCrystalNumber(int tempint) { fCrystalNumber = tempint; }

   double           GetTime(Long64_t timestamp, Float_t cfd, double energy) const;
   int              GetDetectorNumber() const;
   int              GetSegmentNumber() const;
   int              GetCrystalNumber() const;
   const TMnemonic* GetMnemonic() const;
   TClass*          GetClassType() const;
   void             SetClassType(TClass* cl_type);

   int          GetNumber() const { return fNumber.Value(); }
   unsigned int GetAddress() const { return fAddress; }
   int          GetIntegration() const { return fIntegration.Value(); }
   int          GetStream() const { return fStream.Value(); }
   const char*  GetDigitizerTypeString() const { return fDigitizerTypeString.c_str(); }
   EDigitizer   GetDigitizerType() const { return fDigitizerType.Value(); }
   int          GetTimeStampUnit() const { return fTimeStampUnit.Value(); }
   Long64_t     GetTimeOffset() const { return fTimeOffset.Value(); }
   double       GetTimeDrift() const { return fTimeDrift.Value(); }
   // write the rest of the gettters/setters...

   std::vector<double> GetAllENGChi2() const { return fENGChi2.Value(); }
   double              GetENGChi2(size_t range = 0) const { return fENGChi2.Value()[range]; }
   double              GetCFDChi2() const { return fCFDChi2.Value(); }
   double              GetLEDChi2() const { return fLEDChi2.Value(); }
   double              GetTIMEChi2() const { return fTIMEChi2.Value(); }
   double              GetEFFChi2() const { return fEFFChi2.Value(); }

   void        SetUseCalFileIntegration(const TPriorityValue<bool>& tmp = TPriorityValue<bool>(true, EPriority::kUser)) { fUseCalFileInt = tmp; }
   static void SetUseCalFileIntegration(const std::string& mnemonic, bool flag, EPriority pr);
   bool        UseCalFileIntegration() { return fUseCalFileInt.Value(); }

   std::vector<std::vector<Float_t>>      GetAllENGCoeff() const { return fENGCoefficients.Value(); }
   std::vector<Float_t>                   GetENGCoeff(size_t range = 0) const { return fENGCoefficients.Value()[range]; }
   std::vector<double>                    GetCFDCoeff() const { return fCFDCoefficients.Value(); }
   std::vector<double>                    GetLEDCoeff() const { return fLEDCoefficients.Value(); }
   std::vector<double>                    GetTIMECoeff() const { return fTIMECoefficients.Value(); }
   std::vector<double>                    GetEFFCoeff() const { return fEFFCoefficients.Value(); }
   std::vector<double>                    GetCTCoeff() const { return fCTCoefficients.Value(); }
   TGraph                                 GetEnergyNonlinearity() const { return fEnergyNonlinearity.Value(); }
   double                                 GetEnergyNonlinearity(double eng) const;
   std::vector<std::pair<double, double>> GetENGRanges() const { return fENGRanges.Value(); }
   std::pair<double, double>              GetENGRange(size_t range) const { return fENGRanges.Value()[range]; }
   std::vector<Float_t>                   GetENGDriftCoefficents() const { return fENGDriftCoefficents.Value(); }

   inline void AddENGCoefficient(Float_t temp, size_t range = 0)
   {
      if(range >= fENGCoefficients.size()) { fENGCoefficients.resize(range + 1); }
      fENGCoefficients.Address()->at(range).push_back(temp);
   }
   inline void AddENGDriftCoefficent(Float_t temp) { fENGDriftCoefficents.Address()->push_back(temp); }
   inline void AddCFDCoefficient(double temp) { fCFDCoefficients.Address()->push_back(temp); }
   inline void AddLEDCoefficient(double temp) { fLEDCoefficients.Address()->push_back(temp); }
   inline void AddTIMECoefficient(double temp) { fTIMECoefficients.Address()->push_back(temp); }
   inline void AddEFFCoefficient(double temp) { fEFFCoefficients.Address()->push_back(temp); }
   inline void AddCTCoefficient(double temp) { fCTCoefficients.Address()->push_back(temp); }
   void        AddEnergyNonlinearityPoint(double x, double y) { fEnergyNonlinearity.Address()->SetPoint(fEnergyNonlinearity.Address()->GetN(), x, y); }

   inline void ResizeENG(size_t size)
   {
      fENGCoefficients.resize(size);
      fENGChi2.resize(size);
      fENGRanges.resize(size);
   }

   void SetAllENGCoefficients(const TPriorityValue<std::vector<std::vector<Float_t>>>& tmp) { fENGCoefficients = tmp; }
   void SetENGCoefficients(std::vector<Float_t> tmp, size_t range = 0)
   {
      if(range >= fENGCoefficients.size()) { fENGCoefficients.resize(range + 1); }
      fENGCoefficients.Address()->at(range) = tmp;
   }
   void SetENGRanges(const TPriorityValue<std::vector<std::pair<double, double>>>& tmp) { fENGRanges = tmp; }
   void SetENGRange(const std::pair<double, double>& tmp, const size_t& range)
   {
      if(range >= fENGRanges.size()) { fENGRanges.resize(range + 1); }
      fENGRanges.Address()->at(range) = tmp;
   }
   void SetENGDriftCoefficents(const TPriorityValue<std::vector<Float_t>>& tmp) { fENGDriftCoefficents = tmp; }
   void SetCFDCoefficients(const TPriorityValue<std::vector<double>>& tmp) { fCFDCoefficients = tmp; }
   void SetLEDCoefficients(const TPriorityValue<std::vector<double>>& tmp) { fLEDCoefficients = tmp; }
   void SetTIMECoefficients(const TPriorityValue<std::vector<double>>& tmp) { fTIMECoefficients = tmp; }
   void SetEFFCoefficients(const TPriorityValue<std::vector<double>>& tmp) { fEFFCoefficients = tmp; }
   void SetCTCoefficients(const TPriorityValue<std::vector<double>>& tmp) { fCTCoefficients = tmp; }
   void SetEnergyNonlinearity(const TPriorityValue<TGraph>& tmp) { fEnergyNonlinearity = tmp; }

   inline void SetAllENGChi2(const TPriorityValue<std::vector<double>>& tmp) { fENGChi2 = tmp; }
   inline void SetENGChi2(const TPriorityValue<double>& tmp, const size_t& range = 0)
   {
      if(tmp.Priority() >= fENGChi2.Priority()) {
         if(range >= fENGChi2.size()) { fENGChi2.resize(range + 1); }
         fENGChi2.Address()->at(range) = tmp.Value();
      }
   }
   inline void SetCFDChi2(const TPriorityValue<double>& tmp) { fCFDChi2 = tmp; }
   inline void SetLEDChi2(const TPriorityValue<double>& tmp) { fLEDChi2 = tmp; }
   inline void SetTIMEChi2(const TPriorityValue<double>& tmp) { fTIMEChi2 = tmp; }
   inline void SetEFFChi2(const TPriorityValue<double>& tmp) { fEFFChi2 = tmp; }

   inline void SetWaveRise(const double& temp)
   {
      WaveFormShape.TauRise = temp;
      SetUseWaveParam();
   }
   inline void SetWaveDecay(const double& temp)
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

   double CalibrateENG(double) const;
   double CalibrateENG(double, int temp_int) const;
   double CalibrateENG(int, int temp_int = 0) const;

   double CalibrateCFD(double) const;
   double CalibrateCFD(int) const;

   double CalibrateLED(double) const;
   double CalibrateLED(int) const;

   double        CalibrateTIME(double) const;
   double        CalibrateTIME(int) const;
   inline double GetTZero(double tempd) const { return CalibrateTIME(tempd); }
   inline double GetTZero(int tempi) const { return CalibrateTIME(tempi); }

   double CalibrateEFF(double) const;

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
   static Int_t ReadCalFile(std::ifstream& infile);
   static Int_t ReadCalFile(const char* filename = "");
   static Int_t ParseInputData(const char* inputdata = "", Option_t* opt = "", EPriority prio = EPriority::kUser);
   static void  WriteCalFile(const std::string& outfilename = "");
   static void  WriteCTCorrections(const std::string& outfilename = "");
   static void  WriteCalBuffer(Option_t* opt = "");
   static void  ReadEnergyNonlinearities(TFile*, const char* graphName = "EnergyNonlinearity0x", bool all = false);

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;
   // static  void PrintAll(Option_t* opt = "");
   std::string PrintToString(Option_t* opt = "") const;
   std::string PrintCTToString(Option_t* opt = "") const;
   void        PrintCTCoeffs(Option_t* opt = "") const;

   static int WriteToRoot(TFile* fileptr = nullptr);

private:
   // the follow is to make the custom streamer
   // stuff play nice.  pcb.
   static std::string fFileData;
   static void        InitChannelInput();
   static void        SaveToSelf();
   static void        SaveToSelf(const char*);

   static Int_t ReadFile(TFile* tempf);

   /// \cond CLASSIMP
   ClassDefOverride(TChannel, 6) // NOLINT
   /// \endcond
};
/*! @} */
#endif
