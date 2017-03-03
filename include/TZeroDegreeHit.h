#ifndef ZERODEGREEHIT_H
#define ZERODEGREEHIT_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// TZeroDegreeHit
///
/// This is class that contains the information about a ZDS
/// hit. This class is used to find energy, time, etc.
///
/////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TZeroDegreeHit : public TGRSIDetectorHit {
   public:
      TZeroDegreeHit();
      virtual ~TZeroDegreeHit();
      TZeroDegreeHit(const TZeroDegreeHit&);
      TZeroDegreeHit(const TFragment& frag);
      
   private:
      Int_t    fFilter;
		std::vector<short> fCfdMonitor;
		std::vector<int> fPartialSum;
      
   public:
      /////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter   = x; }   //!<!
      
      /////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     { return fFilter;   }  //!<!
		inline std::vector<short>& GetCfdMonitor() { return fCfdMonitor; }
		inline std::vector<int>& GetPartialSum() { return fPartialSum; }
      
      Int_t GetCfd() const;
		Int_t GetRemainder() const;
      Double_t GetTime(const UInt_t& correction_flag = ETimeFlag::kAll, Option_t* opt = "") const;  ///< Returns a time value to the nearest nanosecond!
		Double_t GetCorrectedTime() const; ///< Returns a time value using the CFD with 1/256 ns intrinsic binning, corrected using GValue

      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps); //!<!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Short_t> &monitor); //!<!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow); //!<!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!<!
      std::vector<Int_t> CalculatePartialSum(); //!<!
      
      bool InFilter(Int_t);                                            //!<!
      
      bool AnalyzeWaveform();                                          //!<!
      
   public:
      void Clear(Option_t *opt = "");		                    //!<!
      void Print(Option_t *opt = "") const;		                    //!<!
      virtual void Copy(TObject&) const;        //!<!
      virtual void Copy(TObject&, bool) const;        //!<!
      
      //Position Not written for ZeroDegree Yet
      
      /// \cond CLASSIMP
      ClassDef(TZeroDegreeHit,3) //Stores the information for a ZeroDegreeHit
      /// \endcond
};
/*! @} */
#endif
