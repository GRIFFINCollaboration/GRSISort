#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <string>
#include <iostream>
#include <sstream>

#include "TObject.h"
#include "TClass.h"

#include "Globals.h"
#include "TPriorityValue.h"
#include "TChannel.h"

enum class EDigitizer : char;

class TChannel;

class TMnemonic : public TObject {
public:
   TMnemonic() : fClassType(nullptr) { Clear(); }
   explicit TMnemonic(const char* name) : TMnemonic() { Parse(name); }
   TMnemonic(const TMnemonic&) = default;
   TMnemonic(TMnemonic&&)      = default;
   ~TMnemonic()                = default;

   TMnemonic& operator=(const TMnemonic&) = default;
   TMnemonic& operator=(TMnemonic&&)      = default;

   // standard C++ makes these enumerations global to the class. ie, the name of the enumeration
   // EMnemonic or ESystem has no effect on the clashing of enumerated variable names.
   // These separations exist only to easily see the difference when looking at the code here.
   enum class EMnemonic { kA,
                          kB,
                          kC,
                          kD,
                          kE,
                          kF,
                          kG,
                          kH,
                          kI,
                          kJ,
                          kK,
                          kL,
                          kM,
                          kN,
                          kO,
                          kP,
                          kQ,
                          kR,
                          kS,
                          kT,
                          kU,
                          kV,
                          kW,
                          kX,
                          kY,
                          kZ,
                          kClear };

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

   virtual void EnumerateDigitizer(TPriorityValue<std::string>&, TPriorityValue<EDigitizer>&, TPriorityValue<int>&)
   {
      if(fPrint) {
         std::cerr << RED << ClassName() << ": No data library set, some things like timing won't work!" << RESET_COLOR << std::endl;
         fPrint = false;
      }
   }

   virtual void Segment(int16_t val) { fSegment = val; }
   virtual void SetRFMnemonic(std::string* name);

   // this needs to be const because we call this from GetClassType as well, which is const
   // that is also why fClassType is mutable (?)
   // but does GetClassType need to be const?
   virtual void    SetClassType(TClass* classType) const { fClassType = classType; }
   virtual TClass* GetClassType() const;

   virtual double GetTime(Long64_t timestamp, Float_t cfd, double energy, const TChannel* channel) const;

   void Print(Option_t* opt = "") const override;
   void Print(std::ostringstream& str) const;
   void Clear(Option_t* opt = "") override;

   void        SetName(const char* val) { fName = val; }
   void        SetName(const std::string& val) { fName = val; }
   const char* GetName() const override { return fName.c_str(); }

private:
   std::string fName;
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

   mutable TClass* fClassType;   //!<! TGRSIDetector Type that this mnemonic represents

   void EnumerateMnemonic(std::string mnemonic_word, EMnemonic& mnemonic_enum);

   static bool fPrint;

   /// \cond CLASSIMP
   ClassDefOverride(TMnemonic, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
