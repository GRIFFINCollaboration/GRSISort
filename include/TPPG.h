#ifndef TPPG_H
#define TPPG_H

/*
 * Author:  R.Dunlop, <rdunlop@uoguelph.ca>
 * 
 * Please indicate changes with your initials.
 * 
 *
 */


 /////////////////////////////////////////////////////////////////////////
 //                                                                     //
 // TPPG                                                                //
 //                                                                     //
 // The TPPG is designed to hold all of the information about the       //
 // PPG status.
 //
 /////////////////////////////////////////////////////////////////////////


#include <map>

#include "TObject.h"
#include "Globals.h"
#include "TCollection.h"

class TPPGData : public TObject {
    public:
       TPPGData();
		 TPPGData(const TPPGData&);
       ~TPPGData(){};

       void Copy(TObject& rhs) const;

       void SetLowTimeStamp(UInt_t low_time) { flowtimestamp = low_time; SetTimeStamp(); }
       void SetHighTimeStamp(UInt_t high_time) { fhightimestamp = high_time; SetTimeStamp();}
       void SetNewPPG(UInt_t new_ppg) { fnew_ppg = new_ppg; }
       void SetOldPPG(UInt_t old_ppg) { fold_ppg = old_ppg; }
       void SetNetworkPacketId(UInt_t packet) { fNetworkPacketId = packet; }

       void SetTimeStamp();

       UInt_t GetLowTimeStamp() const { return flowtimestamp; }
       UInt_t GetHighTimeStamp() const { return fhightimestamp; }
       uint16_t GetNewPPG() const { return (uint16_t)fnew_ppg; }
       uint16_t GetOldPPG() const { return (uint16_t)fold_ppg; }
       UInt_t GetNetworkPacketId() const {return fNetworkPacketId; }

       Long64_t GetTimeStamp() const { return ftimestamp;}

       void Print(Option_t *opt = "") const;
       void Clear(Option_t *opt = "");
       

    private:
       ULong64_t ftimestamp;
       UInt_t fold_ppg;
       UInt_t fnew_ppg;
       UInt_t fNetworkPacketId;
       UInt_t flowtimestamp;
       UInt_t fhightimestamp;

    ClassDef(TPPGData,1) //Contains PPG data information
};

class TPPG : public TObject	{
  public:
   enum ppg_pattern{
      kBeamOn     = 0x0001,
      kDecay      = 0x0004,
      kTapeMove   = 0x0008,
      kBackground = 0x0002,
      kSync       = 0xc000,
      kJunk = 0xFFFF
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
    ULong64_t GetLastStatusTime(ULong64_t time, ppg_pattern pat = kJunk, bool exact_flag = false );
    Bool_t MapIsEmpty() const;
    std::size_t PPGSize() const {return fPPGStatusMap->size()- 1;}
    Long64_t Merge(TCollection *list);
    void Add(const TPPG* ppg);
    void operator+=(const TPPG& rhs);                           
   
    void SetCycleLength(ULong64_t length) { fCycleLength = length; }

    bool Correct(bool verbose = false);
    ULong64_t GetCycleLength();
    ULong64_t GetNumberOfCycles();
    ULong64_t GetTimeInCycle(ULong64_t real_time);
    ULong64_t GetCycleNumber(ULong64_t real_time);

    ULong64_t GetStatusStart(ppg_pattern);

    TPPGData* const Next();
    TPPGData* const Previous();
    TPPGData* const First();
    TPPGData* const Last();

    virtual void Print(Option_t *opt = "") const;
    virtual void Clear(Option_t *opt = "");

  private:
    PPGMap_t::iterator MapBegin() const { return ++(fPPGStatusMap->begin()); }
    PPGMap_t::iterator MapEnd() const   { return fPPGStatusMap->end(); }
    PPGMap_t::iterator fcurrIterator; //!

  private:
    PPGMap_t *fPPGStatusMap;
    ULong64_t fCycleLength;
    std::map<ULong64_t, int> fNumberOfCycleLengths;

    ClassDef(TPPG,2) //Contains PPG information
};
#endif
