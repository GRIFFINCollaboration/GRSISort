// Author: Peter C. Bender    06/14

#ifndef TFRAGMENT_H
#define TFRAGMENT_H

#include "Globals.h"

#include <vector>
//#include <stdint.h>
#include <time.h>

#include "Rtypes.h"
#include "TObject.h"

//#ifndef __CINT__
//#include "Globals.h"
//#endif

////////////////////////////////////////////////////////////////
//                                                            //
// TFragment                                                  //
//                                                            //
// This Class contains all of the information in an event     //
// fragment                                                   //
//                                                            //
////////////////////////////////////////////////////////////////

class TFragment : public TObject	{
public:
   TFragment(); 
	TFragment(const TFragment&, int hit = -1); //copy constructor that copies only the requested hit (if hit is in range 0 - Cfd.size())
   virtual ~TFragment(); 

   time_t   MidasTimeStamp;       //->  Timestamp of the MIDAS event  
   Int_t    MidasId;              //->  MIDAS ID
   Long_t   TriggerId;            //->  MasterFilterID in Griffin DAQ  
   Int_t    FragmentId;           //->  Channel Trigger ID
   Int_t    TriggerBitPattern;	 //->  MasterFilterPattern in Griffin DAQ

   Int_t    NetworkPacketNumber;  //->  Network packet number

   Short_t ChannelNumber;         //->  Channel Number    //ryan is going to delete this.
   UInt_t ChannelAddress;         //->  Address of the channel
   std::vector<Int_t> Cfd;        //->  CFD of each pileup hit
   std::vector<Int_t> Zc;         //->  ZC of each pileup hit
   std::vector<Int_t> ccShort;    //->  Integration over the waveform rise (Descant only?)
   std::vector<Int_t> ccLong;     //->  Integration over the wavefrom fall (Descant only?)
   std::vector<Int_t> Led;        //->  LED of each pileup hit
   std::vector<Int_t> Charge;	    //->  The Integrated Charge 

   Int_t TimeStampLow;            //->  Timestamp low bits
   Int_t TimeStampHigh;           //->  Timestamp high bits

	/// Added to combine Grif Fragment  ////

   UInt_t PPG;                    //-> Programmable pattern generator value
   UShort_t DeadTime;	          //-> Deadtime from trigger
   UShort_t NumberOfFilters;      //-> Number of filter patterns passed
   UShort_t NumberOfPileups;      //-> Number of piled up hits 1-3
   UShort_t DataType;             //-> 
   UShort_t DetectorType;         //-> Detector Type (PACES,HPGe, etc)
   UInt_t ChannelId;              //-> Threshold crossing counter for a channel

   std::vector<UShort_t>  KValue; //-> KValue for each pileup hit

   /// *****************************  ////

   std::vector<Short_t>  wavebuffer;//-> waveform words

	int NumberOfHits;              //! transient member to count the number of pile-up hits in the original fragment
	int HitIndex;                  //! transient member indicating which pile-up hit this is in the original fragment
  
   double GetTime()      const; //!
   long   GetTimeStamp() const; //!
   double GetTZero() const; //!
   const char *GetName() const; //!
   double GetEnergy(int iter=0)const; //!
   Float_t GetCharge(int iter=0)const; //!
   long GetTimeStamp_ns(); //!

   Int_t GetCfd(int iter=0)     const { return Cfd.at(iter);     } //!
   Int_t GetLed(int iter=0)     const { return Led.at(iter);     } //!
   Int_t GetZc(int iter=0)      const { return Zc.at(iter);      } //!
   Int_t GetCcShort(int iter=0) const { return ccShort.at(iter); } //!
   Int_t GetCcLong(int iter=0)  const { return ccLong.at(iter);  } //!

   Int_t Get4GCfd(int i=0); //!

   bool IsDetector(const char *prefix, Option_t *opt = "CA") const; //!

   virtual void	Clear(Option_t *opt = ""); //!
   using TObject::Print; 
   virtual void Print(Option_t *opt = "") const; //!
   
   bool Compare(const TFragment &rhs) const { return (GetTimeStamp_ns() < rhs.GetTimeStamp_ns()); }

   bool operator<(const TFragment &rhs) const { return (GetTimeStamp() < rhs.GetTimeStamp()); }
   bool operator>(const TFragment &rhs) const { return (GetTimeStamp() > rhs.GetTimeStamp()); }

   ClassDef(TFragment,5);  // Event Fragments
};
#endif // TFRAGMENT_H
