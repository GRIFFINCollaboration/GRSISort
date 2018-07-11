#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <string>
#include "TObject.h"
#include "Globals.h"
#include "TClass.h"

class TMnemonic : public TObject {
public:
   TMnemonic() : fClassType(nullptr) { Clear(); }
   TMnemonic(const char* name) : TMnemonic() { Parse(name); }
   ~TMnemonic() override = default;

   // standard C++ makes these enumerations global to the class. ie, the name of the enumeration
   // EMnemonic or ESystem has no effect on the clashing of enumerated variable names.
   // These separations exist only to easily see the difference when looking at the code here.
   enum class EMnemonic { kA, kB, kC, kD, kE, kF, kG, kI, kL, kM, kN, kP, kQ, kR, kS, kT, kW, kX, kZ, kClear };
   enum class ESystem {
      kTigress,
      kSharc,
      kTriFoil,
      kRF,
      kCSM,
      kSiLi,
      kSiLiS3,
      kGeneric,
      kS3,
      kBambino,
      kTip,
      kGriffin,
      kSceptar,
      kPaces,
      kLaBr,
      kTAC,
      kZeroDegree,
      kDescant,
		kGriffinBgo,
		kLaBrBgo,
      kFipps,
		kBgo,
		kTdrClover,
		kTdrCloverBgo,
		kTdrTigress,
		kTdrTigressBgo,
		kTdrSiLi,
		kTdrPlastic,
		kClear
   };
   enum class EDigitizer { kDefault, kGRF16, kGRF4G, kTIG10, kTIG64, kCAEN8, kPixie, kFastPixie, kPixieTapeMove };

   ESystem   System() const { return fSystem; }
   EMnemonic SubSystem() const { return fSubSystem; }
   EMnemonic ArraySubPosition() const { return fArraySubPosition; }
   EMnemonic CollectedCharge() const { return fCollectedCharge; }
   EMnemonic OutputSensor() const { return fOutputSensor; }
   int16_t   ArrayPosition() const { return fArrayPosition; }
   int16_t   Segment() const { return fSegment; }

   std::string SystemString() const { return fSystemString; }
   std::string SubSystemString() const { return fSubSystemString; }
   std::string ArraySubPositionString() const { return fArraySubPositionString; }
   std::string CollectedChargeString() const { return fCollectedChargeString; }
   std::string OutputSensorString() const { return fOutputSensorString; }

   void Parse(std::string* name);
   void Parse(const char* name);

   static EDigitizer EnumerateDigitizer(std::string name);

   void SetRFMNEMONIC(std::string* name);

   void SetClassType(TClass* classType) { fClassType = classType; }
   TClass*                   GetClassType() const;

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   int16_t     fArrayPosition;
   int16_t     fSegment;
   std::string fSystemString;
   std::string fSubSystemString;
   std::string fArraySubPositionString;
   std::string fCollectedChargeString;
   std::string fOutputSensorString;

   ESystem fSystem;
   EMnemonic fSubSystem;
   EMnemonic fArraySubPosition;
   EMnemonic fCollectedCharge;
   EMnemonic fOutputSensor;

   mutable TClass* fClassType; //!<! TGRSIDetector Type that this mnemonic represents

   void EnumerateSystem();
   void EnumerateMnemonic(std::string mnemonic_word, EMnemonic& mnemonic_enum);

   /// \cond CLASSIMP
   ClassDefOverride(TMnemonic, 1)
   /// \endcond
};

#endif
