#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#if !defined (__CINT__) && !defined (__CLING__)
#include <tuple>
#endif

#include "TMath.h"
#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TGRSIDetectorHit.h"

class TTigressHit : public TGRSIDetectorHit {
  public:
    TTigressHit();
    TTigressHit(const TTigressHit&);
    TTigressHit(const TFragment& frag);
    void CopyFragment(const TFragment& frag);
    virtual ~TTigressHit();

  private:
    //UShort_t fFirstSegment;        
    //Float_t  fFirstSegmentCharge; //!<!

    std::vector<TGRSIDetectorHit> fSegments;

    bool fBgoFired;
    Float_t fTimeFit;
    Float_t fSig2Noise;

/*     //need to do sudo tracking to build addback. */
/*     TVector3 fLastHit;                //!<! */
/* #if !defined (__CINT__) && !defined (__CLING__) */
/*     std::tuple<int,int,int> fLastPos; //!<! */
/* #endif */

  public:
    void SetHit() {}
    /////////////////////////    /////////////////////////////////////
    void SetCore(const TTigressHit& core)           { core.Copy(*this);  }               //!<!
    void AddSegment(const TGRSIDetectorHit& seg)    { fSegments.push_back(seg);  } //!<!
    //    void AddBGO(const TGRSIDetectorHit& bgo)        { fBgos.push_back(bgo);  }     //!<!
    //void SetInitalHit(const int &i)     { fFirstSegment = i; }        //!<!

    /////////////////////////    /////////////////////////////////////
    //int GetCrystal()   const;           //{  return crystal;      }    //!<!
    //int GetInitialHit() const           {  return fFirstSegment;  }      //!<!

    void SetWavefit(TFragment&);
    void SetWavefit();
    Double_t GetSignalToNoise() const  { return fSig2Noise;  } //!<!
    Double_t GetFitTime()       const  { return fTimeFit;   }  //!<!

    UShort_t GetArrayNumber() const  {  
      int number = 4*(GetDetector()-1) + GetCrystal(); 
      return number;
    }

    inline double GetDoppler(double beta, TVector3 *vec=0) { 
      if(vec==0) {
        vec = GetBeamDirection();
      }
      double tmp = 0;
      double gamma = 1/(sqrt(1-pow(beta,2)));
      tmp = this->GetEnergy()*gamma *(1 - beta*TMath::Cos(GetPosition().Angle(*vec)));
      return tmp;
    }

    bool BGOFired() const { return fBgoFired; }
    void SetBGOFired(bool fired) { fBgoFired = fired;}
 
    int GetTimeToTrigger() { return (fTimeStamp&0x7fffff)-(fCfd>>4); }

    int GetSegmentMultiplicity()        const { return fSegments.size(); }  //!<!
    int GetNSegments()                  const { return fSegments.size(); }  //!<!
    /* int GetBGOMultiplicity()            const { return fBgos.size();     }  //!<! */
    /* int GetNBGOs()                      const { return fBgos.size();     }  //!<! */

    const TGRSIDetectorHit& GetSegmentHit(int i) const { return fSegments.at(i);  }  //!<!
    /* const TGRSIDetectorHit& GetBGO(int i)     const { return fBgos.at(i);      }  //!<! */
    const TGRSIDetectorHit& GetCore()         const { return *this;            }  //!<!
    
    const std::vector<TGRSIDetectorHit>& GetSegmentVec() const { return fSegments; }
    /* const std::vector<TGRSIDetectorHit>& GetBGOVec()     const { return fBgos; } */

    /* modified by Momiyama and Niikura on Aug. 23, 2016 */
    /* int GetFirstSeg() const { if(fSegments.size()>0) return fSegments.front().GetSegment(); return -1; } */
    /* int GetLastSeg()  const { if(fSegments.size()>0) return fSegments.back().GetSegment(); return -1; } */
    int GetFirstSeg() const { if(fSegments.size()>0) return fSegments.front().GetSegment(); return 0; }
    int GetLastSeg()  const { if(fSegments.size()>0) return fSegments.back().GetSegment(); return 0; }

    static bool Compare(TTigressHit lhs, TTigressHit rhs);        //!<!
    static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);  //!<!

    void SumHit(TTigressHit*);                                    //!<!

    TVector3 GetPosition(Double_t dist) const; 
    TVector3 GetPosition() const; 
    TVector3 GetLastPosition(Double_t dist) const;
    TVector3 GetLastPosition() const;

  private:
    Double_t GetDefaultDistance() const { return 110.; }

  public:
    virtual void Clear(Option_t *opt = "");                        //!<!
    virtual void Copy(TObject&) const;                             //!<!
    virtual void Print(Option_t *opt = "") const;                  //!<!

    void SortSegments() { std::sort(fSegments.begin(),fSegments.end()); } //!<!

    /// \cond CLASSIMP
    ClassDef(TTigressHit,4)
      /// \endcond
};
/*! @} */
#endif
