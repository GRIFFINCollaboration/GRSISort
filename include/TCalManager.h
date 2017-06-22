#ifndef TCALMANAGER_H__
#define TCALMANAGER_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"
#include "TNamed.h"
#include "TClass.h"

class TCalManager : public TNamed {
public:
   TCalManager();
   TCalManager(const char* classname);
   ~TCalManager() override;

public:
   TCal* GetCal(UInt_t channum);
   Bool_t AddToManager(TCal* cal, UInt_t channum, Option_t* opt = "");
   Bool_t AddToManager(TCal* cal, Option_t* opt = "");
   void RemoveCal(UInt_t channum, Option_t* opt = "");
   void SetClass(const char* classname);
   void SetClass(const TClass* cl);
   const char* GetClass() { return fClass ? fClass->GetName() : nullptr; }
   void        WriteToChannel() const;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   TCal* operator[](UInt_t channum) { return GetCal(channum); }

private:
   typedef std::map<UInt_t, TCal*> CalMap;
   CalMap  fCalMap;
   TClass* fClass;

   /// \cond CLASSIMP
   ClassDefOverride(TCalManager, 1);
   /// \endcond
};
/*! @} */
#endif
