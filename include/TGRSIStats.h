#ifndef TGRSISTATS_H
#define TGRSISTATS_H

#include <map>
#include "TObject.h"



class TGRSIStats : public TObject {
   public:
      TGRSIStats(){};//For ROOT only. Do not call this.

   private:
      TGRSIStats(int);
   public:
      static TGRSIStats *GetStats(int temp_address); 
      static int GetNumberOfStats() { return fStatsMap->size(); }  

   private:
     static std::map<int,TGRSIStats*> *fStatsMap; //A Map of channel number to a set of stats
     static time_t fLowestMidasTimeStamp;         //The lowest midas time stamp recorded during a sub-run
     static time_t fHighestMidasTimeStamp;        //This highest midas time stamp recorded during a sub-run

     static Int_t fLowestNetworkPacket;           //The lowest network packet recorded during a sub-run
     static Int_t fHighestNetworkPacket;          //The highest network packet recorded durin a sub-run

     int fStatAddress;                            //The address that the stats correspon to

     unsigned long fDeadTime;                     //The total dead-time recorded in a channel
     int fLostEvents;                             //The number of lost events in a channel
     int fLastChannelIdSeen;                      //The last channelId that came from the channel

     static Long_t fGoodEvents;                   //The total number of good events in a sub-run

   public:
      virtual ~TGRSIStats();
      static std::map<int,TGRSIStats*> *GetMap() { return fStatsMap;}
      static int GetSize() { if(fStatsMap) return fStatsMap->size(); else return 0; }

      int GetAddress() const { return fStatAddress; }
      unsigned long GetDeadTime() const {return fDeadTime;}
      void IncDeadTime(int dtime) { fDeadTime += dtime; }
      void IncLostEvent(int lnum = 1) { fLostEvents+=lnum; } 

      void SetLastChannelId(int last) { fLastChannelIdSeen = last;  }

      static void IncGoodEvents(){ fGoodEvents++;}

      static inline void SetLowestMidasTimeStamp(time_t low) { fLowestMidasTimeStamp = low; }
      static inline void SetHighestMidasTimeStamp(time_t high) { fHighestMidasTimeStamp = high; }
      static inline time_t GetLowestMidasTimeStamp()  { return fLowestMidasTimeStamp; }
      static inline time_t GetHighestMidasTimeStamp() { return fHighestMidasTimeStamp; }
      
      static inline void SetLowestNetworkPacket(Int_t low) { fLowestNetworkPacket = low; }
      static inline void SetHighestNetworkPacket(Int_t high) {fHighestNetworkPacket = high; }
      static inline Int_t GetLowestNetworkPacket()  { return fLowestNetworkPacket; }
      static inline Int_t GetHighestNetworkPacket() { return fHighestNetworkPacket; }
      static inline Double_t GetMissingNetworkPackets() { return (fHighestNetworkPacket - fLowestNetworkPacket)/fGoodEvents;}

      static int GetRunTime() { return fHighestMidasTimeStamp - fLowestMidasTimeStamp; }


      virtual void Print(Option_t *opt = "") const;
      virtual void Clear(Option_t *opt = "");



   ClassDef(TGRSIStats,1) //A Helper class designed to record stats from each channel
};


#endif
