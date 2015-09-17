#ifndef TTIGRESS_H
#define TTIGRESS_H


#include <vector>
#include <iostream>
#include <set>
#include <stdio.h>

#include "TTigressHit.h"
#ifndef __CINT__
#include "TTigressData.h"
#include "TBGOData.h"
#else
class TTigressData;
class TBGOData;
#endif

#include "TMath.h"
#include "TVector3.h" 
#include "TObject.h"
#include "TClonesArray.h"

#include "TGRSIDetector.h" 

using namespace std;

class TTigress : public TGRSIDetector {

	public:
		TTigress();
      TTigress(const TTigress&);
		virtual ~TTigress();

	public: 
      void BuildHits(TDetectorData *data =0,Option_t *opt = ""); //!
		//void BuildHits(TTigressData *data = 0,TBGOData *bdata = 0,Option_t *opt="");	//!
		void BuildAddBack(Option_t *opt="");	//!
		//void BuildCloverAddBack(Option_t *opt="");	//!

		TTigressHit *GetTigressHit(const int i) {	return (TTigressHit*)tigress_hits.At(i);	    } //!
      TGRSIDetectorHit *GetHit(const int i)   { return GetTigressHit(i);       } //!
		Int_t GetMultiplicity()	                { return tigress_hits.GetEntries(); }	//!

		TTigressHit *GetAddBackHit(int i)   {	return (TTigressHit*)addback_hits.At(i);	       } //!
		Int_t GetAddBackMultiplicity()	   {	return addback_hits.GetEntries(); }	//!

		//TTigressHit *GetCloverAddBackHit(int i) { return clover_addback_hits.At(i);     } //!
		//Int_t GetCloverAddBackMultiplicity()    { return clover_addback_hits.GetEntries(); } //!

      void AddTigressHit(const TTigressHit&); 
      void AddAddBackHit(const TTigressHit&); 
      //void AddCloverAddBackHit(const TTigressHit&); 

      void PushBackHit(TGRSIDetectorHit* hit) { }

		//static void   SetBeta(double b) 		{	TTigress::beta = b;	} 		//!
		//static void   DopplerCorrect(TTigressHit *);							//!

		static TVector3 GetPosition(int DetNbr ,int CryNbr, int SegNbr, double distance = 110.);		//!
		//TVector3 GetPosition(TTigressHit *,int distance=0);									//!

		void FillData(TFragment*,TChannel*,MNEMONIC*); //!
		void BuildHits(TFragment*,MNEMONIC*); //!
		void FillBGOData(TFragment*,TChannel*,MNEMONIC*); //!

	private: 
		TTigressData *tigdata;        //!
		TBGOData     *bgodata;        //!

		//std::vector <TTigressHit> tigress_hits;
		//std::vector <TTigressHit> addback_hits;
		//std::vector <TTigressHit> clover_addback_hits;			
      TClonesArray tigress_hits;
      TClonesArray addback_hits;
      //TClonesArray clover_addback_hits;

		//static double beta;

		static bool fSetSegmentHits;			   //!
		static bool fSetBGOHits;					//!
		
		static bool fSetCoreWave;					//!
		static bool fSetSegmentWave;			   //!
		static bool fSetBGOWave;					//!

    	static double GeBlue_Position[17][9][3];	//!	detector segment XYZ
		static double GeGreen_Position[17][9][3];	//!
		static double GeRed_Position[17][9][3];	//!
		static double GeWhite_Position[17][9][3];	//!

	public:
		static bool SetSegmentHits() 	 { return fSetSegmentHits;	}	//!
		static bool SetBGOHits()     	 { return fSetBGOHits;	    }	//!

		static bool SetCoreWave()    { return fSetCoreWave;	    }	//!
		static bool SetSegmentWave() { return fSetSegmentWave;  }	//!
		static bool SetBGOWave()	 { return fSetBGOWave;		}     //!

	public:         
		virtual void Clear(Option_t *opt = "");		 //!
		virtual void Print(Option_t *opt = "") const; //!
      virtual void Copy(TTigress&) const;           //!

   ClassDef(TTigress,1)  // Tigress Physics structure


};








#endif


