
#include <map>
#include <TObject.h>



class TGRSIStats : public TObject {

   private:
      TGRSIStats(int);

   public:
      static TGRSIStats *GetStats(int temp_address); 
      static int GetNumberOfStats() { return fStatsMap->size(); }  

   private:
     static std::map<int,TGRSIStats*> *fStatsMap;

     int fStatAddress;

     int fDeadTime;
     int fLostEvents;
     int fLastChannelIdSeen;

   public:
      ~TGRSIStats();


      int GetAddress() { return fStatAddress; }
      int GetDeadTime() {return fDeadTime*10;}
      void IncDeadTime(int dtime) { fDeadTime += dtime; }
      void IncLostEvent(int lnum = 1) { fLostEvents+=lnum; }

      void SetLastChannelId(int last) { fLastChannelIdSeen = last;  }


      virtual void Print(Option_t *opt = "");
      virtual void Clear(Option_t *opt = "");



   ClassDef(TGRSIStats,1)
};



