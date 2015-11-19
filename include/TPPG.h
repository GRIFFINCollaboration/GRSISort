#ifndef TPPG_H
#define TPPG_H

/** \addtogroup Sorting
 *  @{
 */

/*
 * Author:  R.Dunlop, <rdunlop@uoguelph.ca>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


//////////////////////////////////////////////////////////////////////////
///
/// \class TPPG
///
/// The TPPG is designed to hold all of the information about the
/// PPG status.
///
//////////////////////////////////////////////////////////////////////////


#include <map>

#include "TObject.h"
#include "TCollection.h"

#include "Globals.h"

class TPPGData : public TObject {
    public:
       TPPGData();
		 TPPGData(const TPPGData&);
       ~TPPGData(){};

       void Copy(TObject& rhs) const;

       void SetLowTimeStamp(UInt_t lowTime) { fLowTimeStamp = lowTime; SetTimeStamp(); }
       void SetHighTimeStamp(UInt_t highTime) { fHighTimeStamp = highTime; SetTimeStamp();}
       void SetNewPPG(UInt_t newPpg) { fNewPpg = newPpg; }
       void SetOldPPG(UInt_t oldPpg) { fOldPpg = oldPpg; }
       void SetNetworkPacketId(UInt_t packet) { fNetworkPacketId = packet; }

       void SetTimeStamp();

       UInt_t GetLowTimeStamp() const { return fLowTimeStamp; }
       UInt_t GetHighTimeStamp() const { return fHighTimeStamp; }
       uint16_t GetNewPPG() const { return static_cast<uint16_t>(fNewPpg); }
       uint16_t GetOldPPG() const { return static_cast<uint16_t>(fOldPpg); }
       UInt_t GetNetworkPacketId() const {return fNetworkPacketId; }

       Long64_t GetTimeStamp() const { return fTimeStamp;}

       void Print(Option_t* opt = "") const;
       void Clear(Option_t* opt = "");
       

    private:
       ULong64_t fTimeStamp;
       UInt_t fOldPpg;
       UInt_t fNewPpg;
       UInt_t fNetworkPacketId;
       UInt_t fLowTimeStamp;
       UInt_t fHighTimeStamp;

/// \cond CLASSIMP
    ClassDef(TPPGData,2) //Contains PPG data information
/// \endcond
};

class TPPG : public TObject	{
  public:
   enum ppg_pattern{
      kBeamOn     = 0x0001,
      kDecay      = 0x0004,
      kTapeMove   = 0x0008,
      kBackground = 0x0002,
      kSync       = 0xc000,
      kJunk       = 0xFFFF
   };

   typedef std::map<ULong_t,TPPGData*> PPGMap_t;
  public:
    TPPG();
	 TPPG(const TPPG&);
    virtual ~TPPG();

    void Copy(TObject& rhs) const;
   public: 
    void AddData(TPPGData* pat);
    uint16_t GetStatus(ULong64_t time) const;
    ULong64_t GetLastStatusTime(ULong64_t time, ppg_pattern pat = kJunk, bool exact_flag = false ) const;
    Bool_t MapIsEmpty() const;
    std::size_t PPGSize() const {return fPPGStatusMap->size()- 1;}
    Long64_t Merge(TCollection* list);
    void Add(const TPPG* ppg);
    void operator+=(const TPPG& rhs);                           
   
    void SetCycleLength(ULong64_t length) { fCycleLength = length; }

    bool Correct(bool verbose = false);
    ULong64_t GetCycleLength();
    ULong64_t GetNumberOfCycles();
    ULong64_t GetTimeInCycle(ULong64_t real_time);
    ULong64_t GetCycleNumber(ULong64_t real_time);

    ULong64_t GetStatusStart(ppg_pattern);

    const TPPGData* Next();
    const TPPGData* Previous();
    const TPPGData* First();
    const TPPGData* Last();

    virtual void Print(Option_t* opt = "") const;
    virtual void Clear(Option_t* opt = "");

  private:
    PPGMap_t::iterator MapBegin() const { return ++(fPPGStatusMap->begin()); }
    PPGMap_t::iterator MapEnd() const   { return fPPGStatusMap->end(); }
    PPGMap_t::iterator fCurrIterator; //!<!

  private:
    PPGMap_t* fPPGStatusMap;
    ULong64_t fCycleLength;
    std::map<ULong64_t, int> fNumberOfCycleLengths;

/// \cond CLASSIMP
    ClassDef(TPPG,2) //Contains PPG information
/// \endcond
};
/*! @} */
#endif
