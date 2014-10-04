
#include <map>
#include <TObject.h>



class TGRSIStats : public TObject {
   public:
      TGRSIStats(){};//For ROOT only. Do not call this.

   private:
      TGRSIStats(int);
   public:
      static TGRSIStats *GetStats(int temp_address); 
      static int GetNumberOfStats() { return fStatsMap->size(); }  

   private:
     static std::map<int,TGRSIStats*> *fStatsMap; 
     static time_t fLowestMidasTimeStamp;
     static time_t fHighestMidasTimeStamp;

     static Int_t fLowestNetworkPacket;
     static Int_t fHighestNetworkPacket;

     int fStatAddress;

     unsigned long fDeadTime;
     int fLostEvents;
     int fLastChannelIdSeen;

     static Long_t fGoodEvents;

   public:
      ~TGRSIStats();
      static std::map<int,TGRSIStats*> *GetMap() { return fStatsMap;}
      static int GetSize() { if(fStatsMap) return fStatsMap->size(); else return 0; }

      int GetAddress() { return fStatAddress; }
      unsigned long GetDeadTime() {return fDeadTime;}
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

      static int GetRunTime() {return fHighestMidasTimeStamp - fLowestMidasTimeStamp; }


      virtual void Print(Option_t *opt = "");
      virtual void Clear(Option_t *opt = "");



   ClassDef(TGRSIStats,1)
};



