#ifndef TCALMANAGER_H
#define TCALMANAGER_H

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"
#include "TNamed.h"
#include "TClass.h"

class TCalManager : public TNamed {
public:
   TCalManager() = default;
   explicit TCalManager(const char* classname);
	TCalManager(const TCalManager&) = default;
	TCalManager(TCalManager&&) noexcept = default;
	TCalManager& operator=(const TCalManager&) = default;
	TCalManager& operator=(TCalManager&&) noexcept = default;
   ~TCalManager();

   TCal*       GetCal(UInt_t chanNum);
   Bool_t      AddToManager(TCal* cal, UInt_t chanNum, Option_t* opt = "");
   Bool_t      AddToManager(TCal* cal, Option_t* opt = "");
   void        RemoveCal(UInt_t channum, Option_t* opt = "");
   void        SetClass(const char* className);
   void        SetClass(TClass* cls);
   const char* GetClass() { return (fClass != nullptr) ? fClass->GetName() : nullptr; }
   void        WriteToChannel() const;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   TCal* operator[](UInt_t channum) { return GetCal(channum); }

private:
   std::map<UInt_t, TCal*> fCalMap;
   TClass*                 fClass{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TCalManager, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
