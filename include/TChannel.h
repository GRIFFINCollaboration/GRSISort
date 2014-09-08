#ifndef TCHANNEL_H
#define TCHANNEL_H

/*
 * Author:  P.C. Bender, <pcbend@gmail.com>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


/* 
 * The TChannel is designed to hold all non-essential 
 * information of a TFragment (name, energy coeff, etc..)
 * that would otherwise clog up the FragmentTree.  The TChannel class
 * contains a static map to every channel make retrieval fairly 
 * easy.  The TChannel class also contains the ability to 
 * read and write a custom calibration file to set or 
 * save the TChannel information.
 *
 */






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
    //static TChannel *GetChannel(int temp_address);   
    static TChannel *GetChannel(int temp_address); 
    // static TChannel *GetChannelByNumber(int temp_numebr);
    static TChannel *GetChannelByNumber(int temp_numebr);

    TChannel();
    TChannel(const char*);
    TChannel(TChannel*);

    virtual ~TChannel(); 

    static int  GetNumberOfChannels() { return fChannelMap->size(); }
    static void AddChannel(TChannel*,Option_t *opt="");
    static int  UpdateChannel(TChannel*,Option_t *opt="");

    static std::map<int,TChannel*> *GetChannelMap() { return fChannelMap; }
    static void DeleteAllChannels();

  private:
    static TChannel *gChannel;                            //The Global Channel address. Holds the last channel read. 

    int	         address;                                 //The address of the digitizer
    int		 integration;                             //The charge integration setting
    std::string  channelname;                             
    std::string  type_name;
    std::string  digitizertype;
    int 	 number;
    int		 stream;
    int          userinfonumber;

    std::vector<double> ENGCoefficients;  double ENGChi2;
    std::vector<double> CFDCoefficients;  double CFDChi2;
    std::vector<double> LEDCoefficients;  double LEDChi2;
    std::vector<double> TIMECoefficients; double TIMEChi2;

    //static TList *fChannelList;
    static std::map<int,TChannel*> *fChannelMap;          //A map to all of the channels based on address
    static std::map<int,TChannel*> *fChannelNumberMap;
    static void UpdateChannelNumberMap();

    void SetENGCoefficients(std::vector<double> tmp) { ENGCoefficients = tmp; }
    void SetCFDCoefficients(std::vector<double> tmp) { CFDCoefficients = tmp; }
    void SetLEDCoefficients(std::vector<double> tmp) { LEDCoefficients = tmp; }
    void SetTIMECoefficients(std::vector<double> tmp){ TIMECoefficients = tmp; }

		static void trim(std::string *, const std::string & trimChars = " \f\n\r\t\v");

  public:
    inline void SetAddress(int tmpadd) 		     {address = tmpadd;}
    inline void SetChannelName(const char *tmpname)  {channelname.assign(tmpname);} 
    inline void SetNumber(int tmpnum)	             {number = tmpnum;}
    inline void SetIntegration(int tmpint)	     {integration = tmpint;}
    inline void SetStream(int tmpstream)	     {stream = tmpstream;}
    inline void SetUserInfoNumber(int tempinfo)      {userinfonumber = tempinfo;}
    inline void SetDigitizerType(const char *tmpstr) {digitizertype.assign(tmpstr);}
    inline void SetTypeName(std::string tmpstr)      {type_name = tmpstr;}
   

    int	GetNumber()		   { return number;  }
    int	GetAddress()               { return address; }
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

    std::vector<double> GetENGCoeff()  { return ENGCoefficients;}
    std::vector<double> GetCFDCoeff()  { return CFDCoefficients;}
    std::vector<double> GetLEDCoeff()  { return LEDCoefficients;}
    std::vector<double> GetTIMECoeff() { return TIMECoefficients;}


    inline void AddENGCoefficient(double temp)  { ENGCoefficients.push_back(temp);}
    inline void AddCFDCoefficient(double temp)  { CFDCoefficients.push_back(temp);}
    inline void AddLEDCoefficient(double temp)  { LEDCoefficients.push_back(temp);}
    inline void AddTIMECoefficient(double temp) { TIMECoefficients.push_back(temp);}

    inline void SetENGChi2(double temp)  { ENGChi2 = temp; }
    inline void SetCFDChi2(double temp)  { CFDChi2 = temp; }
    inline void SetLEDChi2(double temp)  { LEDChi2 = temp; }
    inline void SetTIMEChi2(double temp) { TIMEChi2 = temp; }

    //void CalibrateFragment(TFragment*);

    double CalibrateENG(double);
    double CalibrateENG(int);
    double CalibrateCFD(double);
    double CalibrateCFD(int);
    double CalibrateLED(double);
    double CalibrateLED(int); 
    double CalibrateTIME(double);
    double CalibrateTIME(int);


    void DestroyCalibrations();

    void DestroyENGCal();
    void DestroyCFDCal();
    void DestroyLEDCal();
    void DestroyTIMECal();

    static void ReadCalFromTree(TTree*);
    static void ReadCalFile(const char *filename = "");
    static void WriteCalFile(std::string outfilename = "");


    virtual void Print(Option_t *opt = "");
    virtual void Clear(Option_t *opt = "");
    //static  void PrintAll(Option_t *opt = "");      


    ClassDef(TChannel,3) //Contains the Digitizer Information
};
#endif

