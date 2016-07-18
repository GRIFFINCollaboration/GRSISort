// Author: Peter C. Bender    06/14

/** \addtogroup Sorting
 *  @{
 */

#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"
#include "TGRSIDetectorHit.h"
#include "TPPG.h"

#include <vector>
#include <time.h>

#include "Rtypes.h"

/////////////////////////////////////////////////////////////////
///
/// \class TFragment
///
/// This Class contains all of the information in an event
/// fragment
///
/////////////////////////////////////////////////////////////////

class TFragment : public TGRSIDetectorHit	{
  public:
    TFragment(); 
    TFragment(const TFragment&); 
    virtual ~TFragment(); 

    time_t   MidasTimeStamp;       //->  Timestamp of the MIDAS event  
    Int_t    MidasId;              //->  MIDAS ID
    Long_t   TriggerId;            //->  MasterFilterID in Griffin DAQ  
    Int_t    FragmentId;           //->  Channel Trigger ID
    Int_t    TriggerBitPattern;	 //->  MasterFilterPattern in Griffin DAQ
    Int_t    NetworkPacketNumber;  //->  Network packet number

    Int_t  Zc;     
    Int_t  ccShort;
    Int_t  ccLong; 
    Int_t  Led;    

    /// Added to combine Grif Fragment  ////

    UInt_t PPG;                    //-> Programmable pattern generator value
    UShort_t DeadTime;	           //-> Deadtime from trigger
    UShort_t NumberOfFilters;      //-> Number of filter patterns passed
    UShort_t NumberOfPileups;      //-> Number of piled up hits 1-3
    UShort_t DataType;             //-> 
    UShort_t DetectorType;         //-> Detector Type (PACES,HPGe, etc)
    UInt_t ChannelId;              //-> Threshold crossing counter for a channel
    //UInt_t AcceptedChannelId;    //-> Accepted threshold crossing counter for a channel

    /// *****************************  ////
    void SetZc(Int_t val)         { Zc     = val; }

    TPPG* fPPG; //!<!

    int NumberOfHits;  //!<! transient member to count the number of pile-up hits in the original fragment
    int HitIndex;    //!<! transient member indicating which pile-up hit this is in the original fragment

    double GetTZero() const; //!<!
    Long_t GetTimeStamp_ns() const; //!<!
    ULong64_t GetTimeInCycle(); //!<!
    ULong64_t GetCycleNumber(); //!<!

    time_t GetMidasTimeStamp()      const { return MidasTimeStamp; }  //!<!
    Int_t  GetChannelNumber()      const { TChannel *c=GetChannel();if(c) return c->GetNumber(); return -1; }  //!<!
    Int_t GetZc()                   const { return Zc; } //!<! 
    Int_t GetLed()                  const { return Led; }  //!<!
    Int_t GetCcShort()              const { return ccShort; }  //!<!
    Int_t GetCcLong()               const { return ccLong; }  //!<!
    UShort_t GetDeadTime()          const { return DeadTime; }  //!<!
    UInt_t GetChannelId()           const { return ChannelId; }  //!<!

    Int_t Get4GCfd(size_t i=0) const; //!<!

    bool IsDetector(const char *prefix, Option_t *opt = "CA") const; //!<!
    //int  GetColor(Option_t *opt = "") const; //!<!
    //bool HasWave() const { return (wavebuffer.size()>0) ?  true : false; } //!<!

    virtual void Clear(Option_t *opt = ""); //!<!
    virtual void Print(Option_t *opt = "") const; //!<!

    //virtual Int_t GetDetector() const; //!<!
    //virtual Int_t GetSegment() const;	 //!<!

    bool operator<(const TFragment& rhs) const { return (GetTimeStamp() < rhs.GetTimeStamp()); }
    bool operator>(const TFragment& rhs) const { return (GetTimeStamp() > rhs.GetTimeStamp()); }

    /// \cond CLASSIMP
    ClassDef(TFragment,6);  // Event Fragments
    /// \endcond
};
/*! @} */
#endif // TFRAGMENT_H
