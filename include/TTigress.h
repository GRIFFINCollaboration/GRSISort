#ifndef TTIGRESS_H
#define TTIGRESS_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <set>
#include <stdio.h>
#include <functional>

#include "TMath.h"
#include "TVector3.h" 
#include "TObject.h"
#include "TClonesArray.h"
#include "TRandom2.h"

#include "TGRSIDetector.h" 
#include "TTigressHit.h"
#include "TBgoHit.h"

class TTigress : public TGRSIDetector {
  public:
    enum ETigressBits { // Inherited TObject fBits, Enum via TDetector
      kAddbackSet	= kDetBit0,
      kSuppression	= kDetBit1,
      kTigBit2		= kDetBit2,
      kTigBit3		= kDetBit3,
      kTigBit4		= kDetBit4,
      kTigBit5		= kDetBit5,
      kTigBit6		= kDetBit6,
    };

    enum ETigressGlobalBits {
      kSetBGOWave	= BIT(0),
      kSetCoreWave	= BIT(1),
      kSetSegWave	= BIT(2),
      kSetBGOHits       = BIT(3)
    };

    std::vector<std::vector<TFragment*> > SegmentFragments;

    TTigress();
    TTigress(const TTigress&);
    virtual ~TTigress();

    // Dont know why these were changes to return by reference rather than pointer
    // The tigress group prefer them the old way
    const TTigressHit *GetTigressHit(int i) const { return &fTigressHits.at(i);  }  //!<!
          TTigressHit *GetTigressHit(int i)       { return &fTigressHits.at(i);  }  //!<!        
    const TGRSIDetectorHit *GetHit(int i)   const { return GetTigressHit(i);    }  //!<!
    size_t GetMultiplicity()                const { return fTigressHits.size(); }  //!<!
    static TVector3 GetPosition(int DetNbr ,int CryNbr, int SegNbr, double distance = 0.,bool smear=false);    //!<!
    static TVector3 GetPosition(const TTigressHit&,double distance = 0.);    //!<!

    
    std::vector<TBgoHit> fBgos;
    void AddBGO(TBgoHit& bgo) { fBgos.push_back(bgo);	}	   //!<!
    int GetBGOMultiplicity() const      { return fBgos.size();     }   //!<!
    int GetNBGOs() const      { return fBgos.size();     }   //!<!
    TBgoHit GetBGO( int &i)	   const { return fBgos.at(i);	     }   //!<!
    TBgoHit& GetBGO( int &i)	         { return fBgos.at(i);	     }   //!<!

    // Delete tigress hit from vector (for whatever reason)
    //void DeleteTigressHit(const int& i) { fTigressHits.erase(fTigressHits.begin()+i); } 

    Int_t GetAddbackMultiplicity();
    TTigressHit* GetAddbackHit(const int&);
    void ResetAddback();         //!<!
    UShort_t GetNAddbackFrags(size_t idx) const;

    void AddFragment(TFragment*, TChannel*); //!<!
    void BuildHits();

    TTigress& operator=(const TTigress&); //!<!

#if !defined (__CINT__) && !defined (__CLING__)
    void SetAddbackCriterion(std::function<bool(TTigressHit&, TTigressHit&)> criterion) {
      fAddbackCriterion = criterion;
    }

    std::function<bool(TTigressHit&, TTigressHit&)> GetAddbackCriterion() const {
      return fAddbackCriterion;
    }
    void SetSuppressionCriterion(std::function<bool(TTigressHit&, TBgoHit&)> criterion) {
      fSuppressionCriterion = criterion;
    }
    std::function<bool(TTigressHit&, TBgoHit&)> GetSuppressionCriterion() const {
      return fSuppressionCriterion;
    }
#endif

  private: 
#if !defined (__CINT__) && !defined (__CLING__)
    static std::function<bool(TTigressHit&, TTigressHit&)> fAddbackCriterion;
    static std::function<bool(TTigressHit&, TBgoHit&)> fSuppressionCriterion;
#endif
    static unsigned short fgTigressBits; //!
    std::vector<TTigressHit> fTigressHits;

    static bool fSetSegmentHits;    //!<!
    static bool fSetBGOHits;        //!<!

    static bool fSetCoreWave;       //!<!
    static bool fSetSegmentWave;    //!<!
    static bool fSetBGOWave;        //!<!
    
    static TRandom2 tigress_rand;

    static double GeBluePosition[17][9][3];  //!<!  detector segment XYZ
    static double GeGreenPosition[17][9][3]; //!<!
    static double GeRedPosition[17][9][3];   //!<!
    static double GeWhitePosition[17][9][3]; //!<!
    static double GeBluePositionBack[17][9][3];  //!<!  detector segment XYZ
    static double GeGreenPositionBack[17][9][3]; //!<!
    static double GeRedPositionBack[17][9][3];   //!<!
    static double GeWhitePositionBack[17][9][3]; //!<!

	  //    void ClearStatus();                      // WARNING: this will change the building behavior!
    //		void ClearGlobalStatus() { fTigressBits = 0; }
    static void SetGlobalBit(enum ETigressGlobalBits bit,Bool_t set=true);
    static Bool_t TestGlobalBit(enum ETigressGlobalBits bit) {
      return (bit & fgTigressBits);
    }

    std::vector<TTigressHit> fAddbackHits; //!<! Used to create addback hits on the fly
    std::vector<UShort_t> fAddbackFrags;   //!<! Number of crystals involved in creating in the addback hit

  public:
    static bool SetCoreWave()    { return TestGlobalBit(kSetCoreWave);     }  //!<!
    static bool SetSegmentWave() { return TestGlobalBit(kSetSegWave);      }  //!<!
    static bool SetBGOWave()     { return TestGlobalBit(kSetBGOWave);      }  //!<!
    static bool BGOSuppression[4][4][5]; //!<!

  public:         
    virtual void Clear(Option_t *opt = "");     //!<!
    virtual void Print(Option_t *opt = "") const; //!<!
    virtual void Copy(TObject&) const;           //!<!

/// \cond CLASSIMP
    ClassDef(TTigress,7)  // Tigress Physics structure
/// \endcond
};
/*! @} */
#endif
