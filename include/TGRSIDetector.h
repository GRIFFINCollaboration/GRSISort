#ifndef TGRSIDETECTOR_H
#define TGRSIDETECTOR_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <vector>

#include "TObject.h"
#include "TVector3.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TDetector.h"

class TGRSIDetectorHit;

/////////////////////////////////////////////////////////////////
///
/// \class TGRSIDetector
///
/// This is an abstract class that contains the basic info
/// about a detector. This is where the hits are built and
/// the data is filled.
///
/////////////////////////////////////////////////////////////////


class TGRSIDetector : public TDetector	{
  public:
    TGRSIDetector();
    TGRSIDetector(const TGRSIDetector&);
    virtual ~TGRSIDetector();

  public: 
    //virtual TGRSIDetectorHit* GetHit(const Int_t idx = 0) { AbstractMethod("GetHit()"); return 0;}
    virtual void AddFragment(TFragment*, TChannel*) = 0; //!<! = 0; //!
    virtual void BuildHits() {}

    virtual void Copy(TObject&) const;              //!<!
    virtual void Clear(Option_t *opt = "");         //!<!
    virtual void Print(Option_t *opt = "") const;   //!<!

    //      virtual void AddHit(TGRSIDetectorHit* hit, Option_t *opt ="") {}        //!<!

    //  void Init();

    //virtual void   SetMidasTimestamp(Long_t time) { fMidasTimestamp = time; }
    //virtual Long_t GetMidasTimestamp() const { return fMidasTimestamp; }

  protected:
    void CopyFragment(TFragment *frag);
  private:
    
    //Long_t fMidasTimestamp;

    /// \cond CLASSIMP
	 ClassDef(TGRSIDetector,1) //Abstract class for detector systems 
	 /// \endcond
};
/*! @} */
#endif
