#ifndef TS3DATA_H
#define TS3DAtA_H

#include <cstdlib>
#include <vector>
#include <map>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorData.h"

#include "Rtypes.h"

class TS3Data : public TGRSIDetectorData  {

  private:
    std::vector<UShort_t>  fS3_RingDetector;
    std::vector<UShort_t>  fS3_RingNumber;
    std::vector<TFragment> fS3_RingFragment;

    std::vector<UShort_t>  fS3_SectorDetector;
    std::vector<UShort_t>  fS3_SectorNumber;
    std::vector<TFragment> fS3_SectorFragment;

  public:
    TS3Data();
    ~TS3Data();

    void Clear(Option_t *opt = "");
    void Print(Option_t *opt = "") const;

    inline void SetRing_Detector(const UShort_t &DetNbr) { fS3_RingDetector.push_back(DetNbr); } //!
    inline void SetRing_Number(const UShort_t &RingNbr)  { fS3_RingNumber.push_back(RingNbr);  } //!
    inline void SetRing_Fragment(const TFragment &Frag)  { fS3_RingFragment.push_back(Frag);   } //!

    inline void SetSector_Detector(const UShort_t &DetNbr)  { fS3_SectorDetector.push_back(DetNbr); } //!
    inline void SetSector_Number(const UShort_t &SectorNbr) { fS3_SectorNumber.push_back(SectorNbr);} //!
    inline void SetSector_Fragment(const TFragment &Frag)   { fS3_SectorFragment.push_back(Frag);   } //!


    inline void SetRing(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic )  {
      if(!frag||!channel||!mnemonic) return;
      SetRing_Detector(mnemonic->arrayposition);
      SetRing_Number(mnemonic->segment);
      SetRing_Fragment(*frag);
    } //!
    
    inline void SetSector(TFragment *frag, TChannel *channel, MNEMONIC *mnemonic )  {
      if(!frag||!channel||!mnemonic) return;
      SetSector_Detector(mnemonic->arrayposition);
      SetSector_Number(mnemonic->segment);
      SetSector_Fragment(*frag);
    } //!

    inline Int_t GetRingMultiplicity()  {return fS3_RingNumber.size();}

    inline UShort_t  GetRing_Detector(const unsigned int &i)        {return fS3_RingDetector.at(i);}//!
    inline UShort_t  GetRing_Number(const unsigned int &i)          {return fS3_RingNumber.at(i);}//!
    inline TFragment GetRing_Fragment(const unsigned int &i)        {return fS3_RingFragment.at(i);}//!

    inline Int_t GetSectorMultiplicity()  {return fS3_SectorNumber.size();}

    inline UShort_t  GetSector_Detector(const unsigned int &i)       {return fS3_SectorDetector.at(i);}//!
    inline UShort_t  GetSector_Number(const unsigned int &i)         {return fS3_SectorNumber.at(i);}//!
    inline TFragment GetSector_Fragment(const unsigned int &i)       {return fS3_SectorFragment.at(i);}//!

};


#endif
