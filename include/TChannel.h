#ifndef TCHANNEL_H
#define TCHANNEL_H

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


 /////////////////////////////////////////////////////////////////////////
 //                                                                     //
 // TChannel                                                            //
 //                                                                     //
 // The TChannel is designed to hold all non-essential                  // 
 // information of a TFragment (name, energy coeff, etc..)              //
 // that would otherwise clog up the FragmentTree.  The TChannel class  //
 // contains a static map to every channel make retrieval fairly        //
 // easy.  The TChannel class also contains the ability to              //
 // read and write a custom calibration file to set or                  //
 // save the TChannel information.                                      //
 //                                                                     //
 /////////////////////////////////////////////////////////////////////////


#include<string>
#include<cmath>
#include<utility>
#include<map>

#include<TNamed.h>
#include<TRandom.h>
#include<TList.h>
#include<TTree.h>

#include"TFragment.h"
#include"Globals.h"

class TChannel : public TNamed	{

  public:
    static TChannel *GetChannel(unsigned int temp_address); 
    static TChannel *GetChannelByNumber(int temp_numebr);
    static TChannel *FindChannelByName(const char *name);

    TChannel();
    TChannel(const char*);
    TChannel(TChannel*);

    virtual ~TChannel(); 

    static int  GetNumberOfChannels() { return fChannelMap->size(); }
    static void AddChannel(TChannel*,Option_t *opt="");
    static int  UpdateChannel(TChannel*,Option_t *opt="");

    static std::map<unsigned int,TChannel*> *GetChannelMap() { return fChannelMap; }
    static void DeleteAllChannels();

    static bool Compare(const TChannel&,const TChannel&);

  private:
    unsigned int	   address;                                 //The address of the digitizer
    int		         integration;                             //The charge integration setting
    std::string      channelname;                             //The name of the channel (MNEMONIC)
    std::string      type_name;
    std::string      digitizertype;
    int 	            number;
    int		         stream;
    int              userinfonumber;

    std::vector<double> ENGCoefficients;  //Energy calibration coeffs (low to high order)
    double ENGChi2;                       //Chi2 of the energy calibration
    std::vector<double> CFDCoefficients;  //CFD calibration coeffs (low to high order)
    double CFDChi2;                       //Chi2 of the CFD calibration
    std::vector<double> LEDCoefficients;  //LED calibration coeffs (low to high order)
    double LEDChi2;                       //Chi2 of LED calibration
    std::vector<double> TIMECoefficients; //Time calibration coeffs (low to high order)
    double TIMEChi2;                      //Chi2 of the Time calibration
    std::vector<double> EFFCoefficients;  //Efficiency calibration coeffs (low to high order)
    double EFFChi2;                       //Chi2 of Efficiency calibration

    static std::map<unsigned int,TChannel*> *fChannelMap; //A map to all of the channels based on address
    static std::map<int,TChannel*> *fChannelNumberMap;    //A map of TChannels based on channel number
    static void UpdateChannelNumberMap();
	 void OverWriteChannel(TChannel*);
	 void AppendChannel(TChannel*);

    void SetENGCoefficients(std::vector<double> tmp) { ENGCoefficients = tmp; }
    void SetCFDCoefficients(std::vector<double> tmp) { CFDCoefficients = tmp; }
    void SetLEDCoefficients(std::vector<double> tmp) { LEDCoefficients = tmp; }
    void SetTIMECoefficients(std::vector<double> tmp){ TIMECoefficients = tmp; }
    void SetEFFCoefficients(std::vector<double> tmp) { EFFCoefficients = tmp;}
	
	 static void trim(std::string *, const std::string & trimChars = " \f\n\r\t\v");

  public:
    inline void SetAddress(unsigned int tmpadd) 	  {address = tmpadd;}
    inline void SetChannelName(const char *tmpname)  {channelname.assign(tmpname);} 
    inline void SetNumber(int tmpnum)	              {number = tmpnum;}
    inline void SetIntegration(int tmpint)	        {integration = tmpint;}
    inline void SetStream(int tmpstream)	           {stream = tmpstream;}
    inline void SetUserInfoNumber(int tempinfo)      {userinfonumber = tempinfo;}
    inline void SetDigitizerType(const char *tmpstr) {digitizertype.assign(tmpstr);}
    inline void SetTypeName(std::string tmpstr)      {type_name = tmpstr;}
   

    int	GetNumber()		           { return number;  }
    unsigned int	GetAddress()     { return address; }
    int GetIntegration()           { return integration; }
    int GetStream()                { return stream; }
    int GetUserInfoNumber()        { return userinfonumber;}
    const char *GetChannelName()   { return channelname.c_str(); }
    const char *GetDigitizerType() { return digitizertype.c_str(); }
		//write the rest of the gettters/setters...

    double GetENGChi2()  { return ENGChi2; }
    double GetCFDChi2()  { return CFDChi2; }
    double GetLEDChi2()  { return LEDChi2; }
    double GetTIMEChi2() { return TIMEChi2; }
    double GetEFFChi2()  { return EFFChi2;} 

    std::vector<double> GetENGCoeff()  { return ENGCoefficients;}
    std::vector<double> GetCFDCoeff()  { return CFDCoefficients;}
    std::vector<double> GetLEDCoeff()  { return LEDCoefficients;}
    std::vector<double> GetTIMECoeff() { return TIMECoefficients;}
    std::vector<double> GetEFFCoeff()  { return EFFCoefficients;}

    inline void AddENGCoefficient(double temp)  { ENGCoefficients.push_back(temp);}
    inline void AddCFDCoefficient(double temp)  { CFDCoefficients.push_back(temp);}
    inline void AddLEDCoefficient(double temp)  { LEDCoefficients.push_back(temp);}
    inline void AddTIMECoefficient(double temp) { TIMECoefficients.push_back(temp);}
    inline void AddEFFCoefficient(double temp)  { EFFCoefficients.push_back(temp);}

    inline void SetENGChi2(double temp)  { ENGChi2 = temp; }
    inline void SetCFDChi2(double temp)  { CFDChi2 = temp; }
    inline void SetLEDChi2(double temp)  { LEDChi2 = temp; }
    inline void SetTIMEChi2(double temp) { TIMEChi2 = temp; }
    inline void SetEFFChi2(double temp)  { EFFChi2 = temp; } 

    //void CalibrateFragment(TFragment*);

    double CalibrateENG(double);
    double CalibrateENG(int);
    double CalibrateCFD(double);
    double CalibrateCFD(int);
    double CalibrateLED(double);
    double CalibrateLED(int); 
    double CalibrateTIME(double);
    double CalibrateTIME(int);
    double CalibrateEFF(double);

    void DestroyCalibrations();

    void DestroyENGCal();
    void DestroyCFDCal();
    void DestroyLEDCal();
    void DestroyTIMECal();
    void DestroyEFFCal();

    static void ReadCalFromTree(TTree*,Option_t *opt="overwrite");
    static Int_t ReadCalFile(const char *filename = "");
    static void WriteCalFile(std::string outfilename = "");

    virtual void Print(Option_t *opt = "");
    virtual void Clear(Option_t *opt = "");
    //static  void PrintAll(Option_t *opt = "");      

    ClassDef(TChannel,3) //Contains the Digitizer Information
};
#endif
