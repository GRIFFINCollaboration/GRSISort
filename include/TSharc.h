#ifndef TSHARC_H
#define TSHARC_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>
#include <map>
#include <set>

#include "TMath.h"
#include "TVector3.h"

#include "Globals.h"
#include "TGRSIDetector.h"
#include "TSharcHit.h"

class TSharc : public TGRSIDetector  {
	public:
		TSharc();
		virtual ~TSharc();
		TSharc(const TSharc& rhs);

	public: 
		TSharcHit*        GetSharcHit(const int& i);  
		TGRSIDetectorHit* GetHit(const int& i);
		static TVector3 GetPosition(int detector, int frontstrip, int backstrip, double X=0.00, double Y=0.00, double Z=0.00);  //!<! 
		static double GetXOffset()  { return fXoffset; }
		static double GetYOffset()  { return fYoffset; }
		static double GetZOffset()  { return fZoffset; }
		static TVector3 GetOffset() { return TVector3(fXoffset,fYoffset,fZoffset); } 
		static void   SetXYZOffset(const double x,const double y,const double z) { fXoffset =x; fYoffset=y; fZoffset=z; }


		int GetSize() const { return fSharcHits.size();} //!<!
		Short_t GetMultiplicity() const { return fSharcHits.size(); } 

		virtual void Copy(TObject&) const;         //!<!
		virtual void Clear(Option_t * = "");       //!<!
		virtual void Print(Option_t * = "") const; //!<!

		TSharc& operator=(const TSharc& rhs)  { if(this!=&rhs) rhs.Copy(*this); return *this; }//!<!

		void AddFragment(TFragment*, MNEMONIC*); //!<!
		void BuildHits();   //no need to build any hits, everything already done in AddFragment;  
                        //we still need to build hits as one hit is composed of multiple fragments.  pcb.

	protected:
		void PushBackHit(TGRSIDetectorHit* sharcHit) { fSharcHits.push_back(*(static_cast<TSharcHit*>(sharcHit))); };

	private:
		std::vector <TSharcHit> fSharcHits;
		int  CombineHits(TSharcHit*,TSharcHit*,int,int);        //!<!
		void RemoveHits(std::vector<TSharcHit>*,std::set<int>*);  //!<!

		std::vector<TFragment> fFrontFragments; //! 
		std::vector<TFragment> fBackFragments;  //! 
		std::vector<TFragment> fPadFragments;  //! 


	private: 
		static double fXoffset;  //!<!
		static double fYoffset;  //!<!
		static double fZoffset;  //!<!

		// various sharc dimensions set in mm, taken from IOP SHARC white paper
		static double fXdim; // total X dimension of all boxes
		static double fYdim; // total Y dimension of all boxes
		static double fZdim; // total Z dimension of all boxes
		static double fRdim; // Rmax-Rmin for all QQQs 
		static double fPdim; // QQQ quadrant angular range (degrees)
		// BOX dimensions
		static double fXposUB;
		static double fYminUB; 
		static double fZminUB; 
		static double fXposDB; 
		static double fYminDB; 
		static double fZminDB; 
		// QQQ dimensions
		static double fZposUQ;    
		static double fRmaxUQ;
		static double fRminUQ;
		static double fPminUQ; // degrees
		static double fZposDQ;    
		static double fRmaxDQ;
		static double fRminDQ;
		static double fPminDQ; // degrees
		// segmentation
		//static const int frontstripslist[16]   ;
		//static const int backstripslist[16]    ;        
		//pitches
		static  double fStripFPitch;
		static  double fStripBPitch;
		static  double fRingPitch;
		static  double fSegmentPitch; // angular pitch, degrees

/// \cond CLASSIMP
		ClassDef(TSharc,7)
/// \endcond
};
/*! @} */
#endif
