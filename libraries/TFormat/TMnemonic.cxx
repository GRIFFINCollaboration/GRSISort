#include "TMnemonic.h"

#include <algorithm>
#include <iostream>

bool TMnemonic::fPrint = true;

void TMnemonic::Clear(Option_t*)
{
   fArrayPosition = -1;
   fSegment       = -1;
   fSystemString.clear();
   fSubSystemString.clear();
   fSubSystem = EMnemonic::kClear;
   fArraySubPositionString.clear();
   fArraySubPosition = EMnemonic::kClear;
   fCollectedChargeString.clear();
   fCollectedCharge = EMnemonic::kClear;
   fOutputSensorString.clear();
   fOutputSensor = EMnemonic::kClear;
}

void TMnemonic::EnumerateMnemonic(std::string mnemonic_word, EMnemonic& mnemonic_enum)
{
   char mnemonic_char = mnemonic_word[0];
   switch(mnemonic_char) {
   case 'A': mnemonic_enum = EMnemonic::kA; break;
   case 'B': mnemonic_enum = EMnemonic::kB; break;
   case 'C': mnemonic_enum = EMnemonic::kC; break;
   case 'D': mnemonic_enum = EMnemonic::kD; break;
   case 'E': mnemonic_enum = EMnemonic::kE; break;
   case 'F': mnemonic_enum = EMnemonic::kF; break;
   case 'G': mnemonic_enum = EMnemonic::kG; break;
   case 'H': mnemonic_enum = EMnemonic::kH; break;
   case 'I': mnemonic_enum = EMnemonic::kI; break;
   case 'J': mnemonic_enum = EMnemonic::kJ; break;
   case 'K': mnemonic_enum = EMnemonic::kK; break;
   case 'L': mnemonic_enum = EMnemonic::kL; break;
   case 'M': mnemonic_enum = EMnemonic::kM; break;
   case 'N': mnemonic_enum = EMnemonic::kN; break;
   case 'O': mnemonic_enum = EMnemonic::kO; break;
   case 'P': mnemonic_enum = EMnemonic::kP; break;
   case 'Q': mnemonic_enum = EMnemonic::kQ; break;
   case 'R': mnemonic_enum = EMnemonic::kR; break;
   case 'S': mnemonic_enum = EMnemonic::kS; break;
   case 'T': mnemonic_enum = EMnemonic::kT; break;
   case 'U': mnemonic_enum = EMnemonic::kU; break;
   case 'V': mnemonic_enum = EMnemonic::kV; break;
   case 'W': mnemonic_enum = EMnemonic::kW; break;
   case 'X': mnemonic_enum = EMnemonic::kX; break;
   case 'Y': mnemonic_enum = EMnemonic::kY; break;
   case 'Z': mnemonic_enum = EMnemonic::kZ; break;
   default: mnemonic_enum = EMnemonic::kClear;
   };
}

void TMnemonic::Parse(std::string* name)
{
   if((name == nullptr) || name->length() < 9) {
      // ??? has this ever worked? How can we compare 2 characters from a string with a length < 1, i.e. an empty string?
      // or use a null pointer for this?
      if((name->length() < 1) && (name->compare(0, 2, "RF") == 0)) {
         SetRFMnemonic(name);
      }
      return;
   }
   SetName(*name);
   std::string buf;
   fSystemString.assign(*name, 0, 2);
   fSubSystemString.assign(*name, 2, 1);
   EnumerateMnemonic(fSubSystemString, fSubSystem);
   buf.clear();
   buf.assign(*name, 3, 2);
   fArrayPosition = static_cast<int16_t>(atoi(buf.c_str()));
   // TIP is a Bad Mnemonic and uses 3 characters for array position this may be changed in the future - S. Gillespie
   if(fSystemString == "TP") {
      fArraySubPositionString.assign(*name, 5, 2);
   } else {
      fArraySubPositionString.assign(*name, 5, 1);
   }
   EnumerateMnemonic(fArraySubPositionString, fArraySubPosition);
   fCollectedChargeString.assign(*name, 6, 1);
   EnumerateMnemonic(fCollectedChargeString, fCollectedCharge);
   buf.clear();
   buf.assign(*name, 7, 2);
   fSegment = static_cast<int16_t>(atoi(buf.c_str()));
   fOutputSensorString.assign(*name, 9, 1);
   EnumerateMnemonic(fOutputSensorString, fOutputSensor);
}

void TMnemonic::Parse(const char* name)
{
   std::string sname = name;
   Parse(&sname);
}

void TMnemonic::SetRFMnemonic(std::string* name)
{
   fSystemString.assign(*name, 0, 2);
   fSubSystemString.assign("X");
   EnumerateMnemonic(fSubSystemString, fSubSystem);
   fArrayPosition = 0;
   fArraySubPositionString.assign("X");
   EnumerateMnemonic(fArraySubPositionString, fArraySubPosition);
   fCollectedChargeString.assign("X");
   EnumerateMnemonic(fCollectedChargeString, fCollectedCharge);
   fSegment = 0;
   fOutputSensorString.assign("X");
   EnumerateMnemonic(fOutputSensorString, fOutputSensor);
}

void TMnemonic::Print(std::ostringstream& str) const
{
   str << "fArrayPosition           = " << fArrayPosition << std::endl;
   str << "fSegment                 = " << fSegment << std::endl;
   str << "fSystemString            = " << fSystemString << std::endl;
   str << "fSubSystemString         = " << fSubSystemString << std::endl;
   str << "fArraySubPositionString  = " << fArraySubPositionString << std::endl;
   str << "fCollectedChargeString   = " << fCollectedChargeString << std::endl;
   str << "fOutputSensorString      = " << fOutputSensorString << std::endl;
}

void TMnemonic::Print(Option_t*) const
{
   std::ostringstream str;
   str << "====== MNEMONIC ======" << std::endl;
   Print(str);
   str << "===============================" << std::endl;
   std::cout << str.str();
}

int TMnemonic::NumericArraySubPosition() const
{
   switch(fArraySubPosition) {
   case TMnemonic::EMnemonic::kB:
      return 0;
   case TMnemonic::EMnemonic::kG:
      return 1;
   case TMnemonic::EMnemonic::kR:
      return 2;
   case TMnemonic::EMnemonic::kW:
      return 3;
   default:
      return 5;
   };

   // return statement here instead of default case
   // to make sure compiler doesn't warn us about missing return
   return 5;
}

TClass* TMnemonic::GetClassType() const
{
   return fClassType;
}

double TMnemonic::GetTime(Long64_t timestamp, Float_t, double, const TChannel* channel) const
{
   return static_cast<double>((timestamp + gRandom->Uniform()) * channel->GetTimeStampUnit());
}
