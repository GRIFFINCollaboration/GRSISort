#ifndef TSHARC_H
#define TSHARC_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <map>
#include <set>

#ifndef __CINT__
#endif

#include "TMath.h"
#include "TVector3.h"

#include "TGRSIDetector.h"

#ifndef __CINT__
#include "TSharcData.h"
#else
class TSharcData;
#endif
#include "TSharcHit.h"

#ifndef PI
#define PI                       (TMath::Pi())
#endif

class TSharc : public TGRSIDetector	{
	public:
		TSharc();
		~TSharc();

	private:
		std::vector <TSharcHit> sharc_hits;
		int  CombineHits(TSharcHit*,TSharcHit*,int,int);				//!
		void RemoveHits(std::vector<TSharcHit>*,std::set<int>*);	//!

	public: 
		inline Short_t    GetNumberOfHits()   	{return sharc_hits.size();}	//->
		inline TSharcHit *GetHit(int i)		   {return &sharc_hits.at(i);}	//->
      static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X=0.00, double Y=0.00, double Z=0.00);	//! 

		void BuildHits(TGRSIDetectorData *sd=0,Option_t * = "");			   //!

		int GetMultiplicity() { return sharc_hits.size(); } //!

		virtual void Clear(Option_t * = "");		//!
		virtual void Print(Option_t * = "");		//!

    //TSharcData *GetData() { return &data; }  //!
		void FillData(TFragment*,TChannel*,MNEMONIC*); //!

	private: 

    TSharcData *data;    //!

		// various sharc dimensions set in mm, taken from IOP SHARC white paper
		static double Xdim; // total X dimension of all boxes
	  static double Ydim; // total Y dimension of all boxes
	  static double Zdim; // total Z dimension of all boxes
	  static double Rdim; // Rmax-Rmin for all QQQs 
	  static double Pdim; // QQQ quadrant angular range (degrees)
    // BOX dimensions
		static double XposUB;
		static double YminUB; 
		static double ZminUB; 
		static double XposDB; 
		static double YminDB; 
		static double ZminDB; 
    // QQQ dimensions
		static double ZposUQ;    
		static double RminUQ;
		static double PminUQ; // degrees
		static double ZposDQ;    
		static double RminDQ;
		static double PminDQ; // degrees
    // segmentation
		//static const int frontstripslist[16]   ;
		//static const int backstripslist[16]    ;        
    //pitches
		static  double stripFpitch;
		static  double stripBpitch;
    static  double ringpitch;
    static  double segmentpitch; // angular pitch, degrees

   ClassDef(TSharc,5)  
};


#endif
