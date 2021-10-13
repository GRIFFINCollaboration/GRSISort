#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <string>
#include <iostream>

#include "TObject.h"
#include "TClass.h"

#include "Globals.h"
#include "TPriorityValue.h"

enum class EDigitizer : char;// { kDefault };

class TMnemonic : public TObject {
public:
   TMnemonic() : fClassType(nullptr) { Clear(); }
   TMnemonic(const char* name) : TMnemonic() { Parse(name); }
   ~TMnemonic() override = default;

   // standard C++ makes these enumerations global to the class. ie, the name of the enumeration
   // EMnemonic or ESystem has no effect on the clashing of enumerated variable names.
   // These separations exist only to easily see the difference when looking at the code here.
   enum class EMnemonic { kA, kB, kC, kD, kE, kF, kG, kH, kI, kJ, kK, kL, kM, kN, kO, kP, kQ, kR, kS, kT, kU, kV, kW, kX, kY, kZ, kClear };

   virtual EMnemonic SubSystem() const { return fSubSystem; }
   virtual EMnemonic ArraySubPosition() const { return fArraySubPosition; }
	virtual int       NumericArraySubPosition() const;
   virtual EMnemonic CollectedCharge() const { return fCollectedCharge; }
   virtual EMnemonic OutputSensor() const { return fOutputSensor; }
   virtual int16_t   ArrayPosition() const { return fArrayPosition; }
   virtual int16_t   Segment() const { return fSegment; }

   virtual std::string SystemString() const { return fSystemString; }
   virtual std::string SubSystemString() const { return fSubSystemString; }
   virtual std::string ArraySubPositionString() const { return fArraySubPositionString; }
   virtual std::string CollectedChargeString() const { return fCollectedChargeString; }
   virtual std::string OutputSensorString() const { return fOutputSensorString; }

   virtual void Parse(std::string* name);
   virtual void Parse(const char* name);

   virtual void EnumerateDigitizer(TPriorityValue<std::string>&, TPriorityValue<EDigitizer>&, TPriorityValue<int>&) { }

   virtual void SetRFMnemonic(std::string* name);

   virtual void SetClassType(TClass* classType) { fClassType = classType; }
   virtual TClass*                   GetClassType() const;

   virtual void Print(Option_t* opt = "") const override;
   virtual void Clear(Option_t* opt = "") override;

protected:
   int16_t     fArrayPosition;
   int16_t     fSegment;
   std::string fSystemString;
   std::string fSubSystemString;
   std::string fArraySubPositionString;
   std::string fCollectedChargeString;
   std::string fOutputSensorString;

   EMnemonic fSubSystem;
   EMnemonic fArraySubPosition;
   EMnemonic fCollectedCharge;
   EMnemonic fOutputSensor;

   mutable TClass* fClassType; //!<! TGRSIDetector Type that this mnemonic represents

   void EnumerateMnemonic(std::string mnemonic_word, EMnemonic& mnemonic_enum);

   /// \cond CLASSIMP
   ClassDefOverride(TMnemonic, 1)
   /// \endcond
};

#endif
