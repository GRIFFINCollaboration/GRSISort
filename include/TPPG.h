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

class TPPGData : public TObject {
   enum ppg_pattern{
      kBeamOn     = 0x1,
      kDecay      = 0x2,
      kTapeMove   = 0x4,
      kBackground = 0x8,
      kJunk       = 0xF,
   };
    public:
       TPPGData();
		 TPPGData(const TPPGData&);
       ~TPPGData(){};

       void Copy(TObject& rhs) const;

       void SetLowTimeStamp(Int_t low_time) { flowtimestamp = low_time; SetTimeStamp(); }
       void SetHighTimeStamp(Int_t high_time) { fhightimestamp = high_time; SetTimeStamp();}
       void SetNewPPG(Int_t new_ppg) { fnew_ppg = new_ppg; }
       void SetOldPPG(Int_t old_ppg) { fold_ppg = old_ppg; }
       void SetNetworkPacketId(Int_t packet) { fNetworkPacketId = packet; }

       void SetTimeStamp();

       Int_t GetLowTimeStamp() const { return flowtimestamp; }
       Int_t GetHighTimeStamp() const { return fhightimestamp; }
       Int_t GetNewPPG() const { return fnew_ppg; }
       Int_t GetOldPPG() const { return fold_ppg; }
       Int_t GetNetworkPacketId() const {return fNetworkPacketId; }

       Long64_t GetTimeStamp() const { return ftimestamp;}

       void Print(Option_t *opt = "") const;
       void Clear(Option_t *opt = "");
       

    private:
       Long64_t ftimestamp;
       Int_t fold_ppg;
       Int_t fnew_ppg;
       Int_t fNetworkPacketId;
       Int_t flowtimestamp;
       Int_t fhightimestamp;

    ClassDef(TPPGData,1) //Contains PPG data information
};

class TPPG : public TObject	{
  public:

   typedef std::map<Double_t,TPPGData*> PPGMap_t;
  public:
    TPPG();
    virtual ~TPPG(); 

   public: 
    void AddData(TPPGData* pat);
    uint16_t GetStatus(Double_t time) const;
    Bool_t MapIsEmpty() const;
    std::size_t PPGSize() const {return fPPGStatusMap->size()- 1;}

    virtual void Print(Option_t *opt = "") const;
    virtual void Clear(Option_t *opt = "");

  private:
    PPGMap_t::iterator MapBegin() const { return ++(fPPGStatusMap->begin()); }
    PPGMap_t::iterator MapEnd() const   { return fPPGStatusMap->end(); }

  private:
    PPGMap_t *fPPGStatusMap;

    ClassDef(TPPG,1) //Contains PPG information
};
#endif
