#ifndef TCHANNEL_H
#define TCHANNEL_H

#include<string>
#include<cmath>
#include<utility>
#include<map>

#include<TNamed.h>
#include<TRandom.h>
#include<TList.h>
//#include<TIter.h>

#include"TFragment.h"
#include"Globals.h"

class TChannel : public TNamed	{

   public:
      //static TChannel *NewChannel(int temp_adress, 
      //                            int temp_number=0, 
      //                            char *temp_name = "");
      static TChannel *GetChannel(int temp_address); 
      //static TChannel *GetChannel(const char *temp_name = "");
      virtual ~TChannel(); 

      static int GetNumberOfChannels() { return fChannelList->GetSize(); }
      static TIter *GetChannelIter() { TIter *iter = new TIter(fChannelList); return iter;}

      TChannel();
      static void CopyChannel(TChannel*,TChannel*);

  private:
      TChannel(const char *address);
      static TChannel *fTChannel;

      unsigned int	address;
		int				integration;
      std::string    channelname;
      std::string    type_name;
		std::string    digitizertype;
      int 			   number;
		int				stream;
      int            userinfonumber;

		std::vector<double> ENGCoefficients;  double ENGChi2;
      std::vector<double> CFDCoefficients;  double CFDChi2;
		std::vector<double> LEDCoefficients;  double LEDChi2;
		std::vector<double> TIMECoefficients; double TIMEChi2;

      static TList *fChannelList;
      static std::map<int,TChannel*> *fChannelMap;

      void SetChannel(int taddress, 
		                int tnumber = 0, 
		                std::string tname = "");
  public:
      inline void SetAddress(int &tmpadd) 			   {address = tmpadd;};
      inline void SetChannelName(const char *tmpname)	{channelname.assign(tmpname);} 
      inline void SetNumber(int &tmpnum)				   {number = tmpnum;}
      inline void SetStream(int &tmpstream)			   {stream = tmpstream;}
      inline void SetUserInfoNumber(int &tempinfo)    {userinfonumber = tempinfo;}
      inline void SetDigitizerType(std::string &tmpstr) {digitizertype = tmpstr;}
      inline void SetTypeName(std::string &tmpstr)    {type_name = tmpstr;}
   

      int	GetNumber()		 	         { return number;  }
      int	GetAddress() 			      { return address; }
      int   GetIntegration()           { return integration;   }
      int   GetStream()                { return stream;  }
      int   GetUserInfoNumber()        { return userinfonumber;}
		const char *GetChannelName()	   { return channelname.c_str();	 }
      const char *GetDigitizerType()   { return digitizertype.c_str(); }
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

		virtual void Print(Option_t *opt = "");
                using TNamed::Print;
		virtual void Clear(Option_t *opt = "");
      static  void PrintAll(Option_t *opt = "");      


	  ClassDef(TChannel,2)
};
#endif






