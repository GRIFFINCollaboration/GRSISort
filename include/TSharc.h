#ifndef TSHARC_H
#define TSHARC_H

#include <vector>
#include <cstdio>
#include <map>

#ifndef __CINT__
#include <tuple>
#include <iterator>
#include <algorithm>
#endif

#include <utility>
#include <set>

#include "TSharcData.h"
#include "TSharcHit.h"
#include "SRIMManager.h"

#include <TMath.h>
#include <TVector3.h>
#include <TObject.h>
#include <TNamed.h>
#include "Globals.h"

#ifndef PI
#define PI                       (TMath::Pi())
#endif

class TSharc : public TObject	{
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
      inline static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X=0.00, double Y=0.00, double Z=0.00);	//! 

		void BuildHits(TSharcData*,Option_t * = "");			   //!

		virtual void Clear(Option_t * = "");		//!
		virtual void Print(Option_t * = "");		//!


	private: 

		// various sharc dimensions set in mm, taken from IOP SHARC white paper
		static const double Xdim        = +72.0; // total X dimension of all boxes
	   static const double Ydim        = +72.0; // total Y dimension of all boxes
	   static const double Zdim        = +48.0; // total Z dimension of all boxes
	   static const double Rdim        = +32.0; // Rmax-Rmin for all QQQs 
	   static const double Pdim        = +81.6; // QQQ quadrant angular range (degrees)
    // BOX dimensions
		static const double XposUB      = +42.5;
		static const double YminUB      = -36.0; 
		static const double ZminUB      = -5.00; 
		static const double XposDB      = +40.5; 
		static const double YminDB      = -36.0; 
		static const double ZminDB      = +9.00; 
    // QQQ dimensions
		static const double ZposUQ      = -66.5;    
		static const double RminUQ      = +9.00;
		static const double PminUQ      = +2.00; // degrees
		static const double ZposDQ      = +74.5;    
		static const double RminDQ      = +9.00;
		static const double PminDQ      = +6.40; // degrees
    // segmentation
		static const int frontstripslist[16]   ;
		static const int backstripslist[16]    ;        
    //pitches
		static const double stripFpitch        ;
		static const double stripBpitch        ;
    static const double ringpitch          ;
    static const double segmentpitch       ; // angular pitch, degrees

   ClassDef(TSharc,5)  // Sharc Analysis structure
};


#endif
