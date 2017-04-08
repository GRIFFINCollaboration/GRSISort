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
#include "TMnemonic.h"
#include "TClass.h"
#include "Globals.h"

class TChannel : public TNamed	{
   public:
      static TChannel* GetChannel(unsigned int temp_address); 
      static TChannel* GetChannelByNumber(int temp_numebr);
      static TChannel* FindChannelByName(const char *name);

      TChannel();
      TChannel(const char*);
      TChannel(TChannel*);

      virtual ~TChannel(); 

      static int  GetNumberOfChannels() { return fChannelMap->size(); }
      static void AddChannel(TChannel*,Option_t *opt="");
      static int  UpdateChannel(TChannel*,Option_t *opt="");

      static std::map<unsigned int,TChannel*>* GetChannelMap() { return fChannelMap; }
      static void DeleteAllChannels();

      static bool CompareChannels(const TChannel&,const TChannel&);

      static TChannel* GetDefaultChannel();

   private:
      unsigned int	   fAddress;                                 //The address of the digitizer
      int		         fIntegration;                             //The charge integration setting
      std::string       fTypeName;
      std::string       fDigitizerTypeString;
      int               fDigitizerType;
      int 	        fNumber;
      int		fStream;
      int               fUserInfoNumber;
      bool              fUseCalFileInt;

      mutable int       fDetectorNumber;
      mutable int       fSegmentNumber;

      mutable int       fCrystalNumber; 
      double            fTimeOffset;
	   TMnemonic         fMnemonic;


      std::vector<Float_t> fENGCoefficients;  //Energy calibration coeffs (low to high order)
      double fENGChi2;                       //Chi2 of the energy calibration
      std::vector<double> fCFDCoefficients;  //CFD calibration coeffs (low to high order)
      double fCFDChi2;                       //Chi2 of the CFD calibration
      std::vector<double> fLEDCoefficients;  //LED calibration coeffs (low to high order)
      double fLEDChi2;                       //Chi2 of LED calibration
      std::vector<double> fTIMECoefficients; //Time calibration coeffs (low to high order)
      double fTIMEChi2;                      //Chi2 of the Time calibration
      std::vector<double> fEFFCoefficients;  //Efficiency calibration coeffs (low to high order)
      double fEFFChi2;                       //Chi2 of Efficiency calibration

      typedef struct WaveFormShapePar{
		bool InUse;
		double BaseLine;
		double TauDecay;
		double TauRise;
	}WaveFormShapePar;
	
	WaveFormShapePar WaveFormShape;
      
      
      std::vector<double> fCTCoefficients;   //Cross talk coefficients

      static std::map<unsigned int,TChannel*>* fChannelMap; //A map to all of the channels based on address
      static std::map<int,TChannel*>* fChannelNumberMap;    //A map of TChannels based on channel number
      static void UpdateChannelNumberMap();
      static void UpdateChannelMap();
      void OverWriteChannel(TChannel*);
      void AppendChannel(TChannel*);

      void SetENGCoefficients(std::vector<Float_t> tmp) { fENGCoefficients = tmp;  }
      void SetCFDCoefficients(std::vector<double> tmp)  { fCFDCoefficients = tmp;  }
      void SetLEDCoefficients(std::vector<double> tmp)  { fLEDCoefficients = tmp;  }
      void SetTIMECoefficients(std::vector<double> tmp) { fTIMECoefficients = tmp; }
      void SetEFFCoefficients(std::vector<double> tmp)  { fEFFCoefficients = tmp;  }
      void SetCTCoefficients(std::vector<double> tmp)  { fCTCoefficients = tmp;   } 

      static void trim(std::string* , const std::string & trimChars = " \f\n\r\t\v");


   public:
      void SetName(const char* tmpName);
      void SetAddress(unsigned int tmpadd);
      inline void SetNumber(int tmpnum)	             { fNumber = tmpnum; UpdateChannelNumberMap(); }
      inline void SetIntegration(int tmpint)	          { fIntegration = tmpint; }
      static void SetIntegration(std::string mnemonic,int tmpint);
      inline void SetStream(int tmpstream)	          { fStream = tmpstream; }
      inline void SetUserInfoNumber(int tempinfo)      { fUserInfoNumber = tempinfo; }
      inline void SetDigitizerType(const char* tmpstr) { fDigitizerTypeString.assign(tmpstr); fDigitizerType = TMnemonic::EnumerateDigitizer(fDigitizerTypeString); }
      static void SetDigitizerType(std::string mnemonic, const char* tmpstr);
      inline void SetTypeName(std::string tmpstr)      { fTypeName = tmpstr; }
      inline void SetTimeOffset(double tmpto)          { fTimeOffset = tmpto; }

      void SetDetectorNumber(int tempint)   { fDetectorNumber = tempint; }
      void SetSegmentNumber(int tempint)    { fSegmentNumber = tempint; }
      void SetCrystalNumber(int tempint)    { fCrystalNumber = tempint; }
      
      int GetDetectorNumber() const; 
      int GetSegmentNumber()  const;  
      int GetCrystalNumber()  const;  
      const TMnemonic* GetMnemonic() const  { return &fMnemonic; }
	 	TClass* GetClassType() const { return fMnemonic.GetClassType(); }
		void SetClassType(TClass* cl_type) { fMnemonic.SetClassType(cl_type); }

      int GetNumber() const                      { return fNumber;  }
      unsigned int GetAddress() const            { return fAddress; }
      int GetIntegration() const                 { return fIntegration; }
      int GetStream() const                      { return fStream; }
      int GetUserInfoNumber() const              { return fUserInfoNumber;}
      const char* GetDigitizerTypeString() const { return fDigitizerTypeString.c_str(); }
      int GetDigitizerType() const               { return fDigitizerType; }
      double GetTimeOffset() const               { return fTimeOffset; }
      //write the rest of the gettters/setters...

      double GetENGChi2()  const { return fENGChi2; }
      double GetCFDChi2()  const { return fCFDChi2; }
      double GetLEDChi2()  const { return fLEDChi2; }
      double GetTIMEChi2() const { return fTIMEChi2; }
      double GetEFFChi2()  const { return fEFFChi2; } 

      void SetUseCalFileIntegration(bool flag=true) { fUseCalFileInt = flag;}
      static void SetUseCalFileIntegration(std::string mnemonic,bool flag);
      bool UseCalFileIntegration() { return fUseCalFileInt; }

      std::vector<Float_t> GetENGCoeff() const { return fENGCoefficients;}
      std::vector<double> GetCFDCoeff()  const { return fCFDCoefficients;}
      std::vector<double> GetLEDCoeff()  const { return fLEDCoefficients;}
      std::vector<double> GetTIMECoeff() const { return fTIMECoefficients;}
      std::vector<double> GetEFFCoeff()  const { return fEFFCoefficients;}
      std::vector<double> GetCTCoeff()   const { return fCTCoefficients;}

      inline void AddENGCoefficient(Float_t temp) { fENGCoefficients.push_back(temp); }
      inline void AddCFDCoefficient(double temp)  { fCFDCoefficients.push_back(temp); }
      inline void AddLEDCoefficient(double temp)  { fLEDCoefficients.push_back(temp); }
      inline void AddTIMECoefficient(double temp) { fTIMECoefficients.push_back(temp);}
      inline void AddEFFCoefficient(double temp)  { fEFFCoefficients.push_back(temp);}
      inline void AddCTCoefficient(double temp)  { fCTCoefficients.push_back(temp);}


      inline void SetENGChi2(double temp)  { fENGChi2 = temp; }
      inline void SetCFDChi2(double temp)  { fCFDChi2 = temp; }
      inline void SetLEDChi2(double temp)  { fLEDChi2 = temp; }
      inline void SetTIMEChi2(double temp) { fTIMEChi2 = temp; }
      inline void SetEFFChi2(double temp)  { fEFFChi2 = temp; } 
      
      inline void SetWaveRise(double temp)  { WaveFormShape.TauRise = temp; SetUseWaveParam();}
      inline void SetWaveDecay(double temp)  { WaveFormShape.TauDecay = temp; SetUseWaveParam();}
      inline void SetWaveBaseLine(double temp)  { WaveFormShape.BaseLine = temp; SetUseWaveParam();}
      inline void SetUseWaveParam(bool temp=true)  { WaveFormShape.InUse=temp; }
      inline void SetWaveParam(WaveFormShapePar temp)  { WaveFormShape=temp; }
      
       
      double GetWaveRise()  const { return WaveFormShape.TauRise; }
      double GetWaveDecay()  const { return WaveFormShape.TauDecay; }
      double GetWaveBaseLine()  const { return WaveFormShape.BaseLine; }
      bool UseWaveParam()  const { return WaveFormShape.InUse; }
      WaveFormShapePar GetWaveParam() const { return WaveFormShape; }
      

      double CalibrateENG(double);
      double CalibrateENG(double, int integration);
      double CalibrateENG(int,int integration=0);

      double CalibrateCFD(double);
      double CalibrateCFD(int);

      double CalibrateLED(double);
      double CalibrateLED(int); 

      double CalibrateTIME(double);
      double CalibrateTIME(int);
      inline double GetTZero(double tempd) { return CalibrateTIME(tempd); }
      inline double GetTZero(int    tempi) { return CalibrateTIME(tempi); }

      double CalibrateEFF(double);

      void DestroyCalibrations();

      void DestroyENGCal();
      void DestroyCFDCal();
      void DestroyLEDCal();
      void DestroyTIMECal();
      void DestroyEFFCal();
      void DestroyCTCal();

      static Int_t ReadCalFromCurrentFile(Option_t *opt="overwrite");
      static Int_t ReadCalFromTree(TTree*,Option_t* opt="overwrite");
      static Int_t ReadCalFromFile(TFile* tempf,Option_t* opt="overwrite");
      static Int_t ReadCalFile(const char* filename = "");
      static Int_t ParseInputData(const char* inputdata = "",Option_t* opt = "");
      static void WriteCalFile(std::string outfilename = "");
      static void WriteCTCorrections(std::string outfilename = "");
      static void WriteCalBuffer(Option_t* opt ="");

      virtual void Print(Option_t* opt = "") const;
      virtual void Clear(Option_t* opt = "");
      //static  void PrintAll(Option_t* opt = "");      
      std::string PrintToString(Option_t* opt="");
      std::string PrintCTToString(Option_t* opt="");
      void PrintCTCoeffs(Option_t* opt = "") const;

      static int WriteToRoot(TFile* fileptr = 0);

   private:
      // the follow is to make the custom streamer 
      // stuff play nice.  pcb.
      static std::string fFileName;
      static std::string fFileData;
      static void InitChannelInput();
      static void SaveToSelf(const char*);

      /// \cond CLASSIMP
      ClassDef(TChannel,5) //Contains the Digitizer Information
         /// \endcond
};
/*! @} */
#endif
