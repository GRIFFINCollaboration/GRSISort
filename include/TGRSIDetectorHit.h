#ifndef TGRSIDETECTORHIT_H
#define TGRSIDETECTORHIT_H


#include "Globals.h"

#include <cstdio>
#include <utility>
#include <vector>

//#include "TChannel.h"
#include "TVector3.h" 
#include "TObject.h" 
#include "Rtypes.h"


////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIDetectorHit                                           //
//                                                            //
// This is class that contains the basic info about detector  //
// hits. This is where the position of a detector is stored.
//                                                            //
////////////////////////////////////////////////////////////////

class TChannel;

class TGRSIDetectorHit : public TObject 	{
   // The bare bones; stuff all detecotors need.  
   // 1. An address.         The whoami for the detector. This is the value used by TChannel::GetChannel(address);
   // 2. An "Energy value."  What this is left the parent class, but it is going to return a double.
   // 3. A   Time  value.     This should be a time value common for detectors (derived from the timestamp)
   //                        Units matter here, I am adobting the ns as the standard.
   // 4. A   Position.       Tvector3s are nice, they make doing geometery trival.  Each hit needs one to determine
   //                        where it is in space, the actual memory of the thing will be stored here.

	public:
		TGRSIDetectorHit(const int &fAddress=0xffffffff)    { address=fAddress; }
		virtual ~TGRSIDetectorHit();

	public:
		virtual void Clear(Option_t* opt = "");	//!
		virtual void Print(Option_t* opt = "");	//!
      //We need a common function for all detectors in here
		//static bool Compare(TGRSIDetectorHit *lhs,TGRSIDetectorHit *rhs); //!

		virtual void SetPosition(const TVector3& temp_ pos) { position = temp_pos; } //!
		virtual TVector3 GetPosition() { return position; }	//!
      
      virtual double GetEnergy() = 0;
      virtual double GetTime()   = 0;  // Returns a time value to the nearest nanosecond!

      virtual void SetAddress(const int &temp_address)    { address = temp_address; } //!
      virtual int  GetAddress()                           { return address; }         //!

      virtual TChannel *GetChannel() { return 0; }

   protected:
      const int address;  //address of the the channel in the DAQ.
      TVector3  position; //Position of hit detector.

	ClassDef(TGRSIDetectorHit,1) //Stores the information for a detector hit
};




#endif
