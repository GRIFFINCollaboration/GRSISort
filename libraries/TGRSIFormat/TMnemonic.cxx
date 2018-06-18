#include "TMnemonic.h"

#include <algorithm>

// Detector dependent includes
#include "TGriffin.h"
#include "TSceptar.h"
#include "TTigress.h"
#include "TTip.h"
#include "TTAC.h"
#include "TLaBr.h"
#include "TSharc.h"
#include "TCSM.h"
#include "TTriFoil.h"
#include "TRF.h"
#include "TS3.h"
#include "TPaces.h"
#include "TDescant.h"
#include "TZeroDegree.h"
#include "TSiLi.h"
#include "TGenericDetector.h"
#include "TFipps.h"

ClassImp(TMnemonic)

void TMnemonic::Clear(Option_t*)
{
   fArrayPosition = -1;
   fSegment       = -1;
   fSystemString.clear();
   fSystem = ESystem::kClear;
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
		case 'I': mnemonic_enum = EMnemonic::kI; break;
		case 'L': mnemonic_enum = EMnemonic::kL; break;
		case 'M': mnemonic_enum = EMnemonic::kM; break;
		case 'N': mnemonic_enum = EMnemonic::kN; break;
		case 'P': mnemonic_enum = EMnemonic::kP; break;
		case 'Q': mnemonic_enum = EMnemonic::kQ; break;
		case 'R': mnemonic_enum = EMnemonic::kR; break;
		case 'S': mnemonic_enum = EMnemonic::kS; break;
		case 'W': mnemonic_enum = EMnemonic::kW; break;
		case 'X': mnemonic_enum = EMnemonic::kX; break;
		case 'Z': mnemonic_enum = EMnemonic::kZ; break;
		default: mnemonic_enum  = EMnemonic::kClear;
	};
}

void TMnemonic::EnumerateSystem()
{
   // Enumerating the fSystemString must come after the total mnemonic has been parsed as the details of other parts of
   // the mnemonic must be known
   if(fSystemString.compare("TI") == 0) {
      fSystem = ESystem::kTigress;
   } else if(fSystemString.compare("SH") == 0) {
      fSystem = ESystem::kSharc;
   } else if(fSystemString.compare("TR") == 0) {
      fSystem = ESystem::kTriFoil;
   } else if(fSystemString.compare("RF") == 0) {
      fSystem = ESystem::kRF;
   } else if(fSystemString.compare("SP") == 0) {
      if(SubSystem() == EMnemonic::kI) {
         fSystem = ESystem::kSiLi;
      } else {
         fSystem = ESystem::kSiLiS3;
      }
   } else if(fSystemString.compare("GD") == 0) {
         fSystem = ESystem::kGeneric;
   } else if(fSystemString.compare("CS") == 0) {
      fSystem = ESystem::kCSM;
   } else if(fSystemString.compare("GR") == 0) {
      fSystem = ESystem::kGriffin;
   } else if(fSystemString.compare("SE") == 0) {
      fSystem = ESystem::kSceptar;
   } else if(fSystemString.compare("PA") == 0) {
      fSystem = ESystem::kPaces;
   } else if(fSystemString.compare("DS") == 0) {
      fSystem = ESystem::kDescant;
   } else if((fSystemString.compare("DA") == 0) || (fSystemString.compare("LB") == 0) ) {
      if(CollectedCharge() == EMnemonic::kN) {
         fSystem = ESystem::kLaBr;
      } else {
         fSystem = ESystem::kTAC;
      }
   } else if(fSystemString.compare("BA") == 0) {
      fSystem = ESystem::kS3;
   } else if(fSystemString.compare("ZD") == 0) {
      fSystem = ESystem::kZeroDegree;
   } else if(fSystemString.compare("TP") == 0) {
      fSystem = ESystem::kTip;
   } else if(fSystemString.compare("FI") == 0) {
      fSystem = ESystem::kFipps;
   } else {
      fSystem = ESystem::kClear;
   }
}

TMnemonic::EDigitizer TMnemonic::EnumerateDigitizer(std::string name)
{
   std::transform(name.begin(), name.end(), name.begin(), ::toupper);
   if(name.compare("GRF16") == 0) {
      return EDigitizer::kGRF16;
   }
   if(name.compare("GRF4G") == 0) {
      return EDigitizer::kGRF4G;
   }
   if(name.compare("TIG10") == 0) {
      return EDigitizer::kTIG10;
   }
   if(name.compare("TIG64") == 0) {
      return EDigitizer::kTIG64;
   }
   if(name.compare("CAEN8") == 0) {
      return EDigitizer::kCAEN8;
   }
   return EDigitizer::kDefault;
}

void TMnemonic::Parse(std::string* name)
{
   if((name == nullptr) || name->length() < 9) {
      if((name->length() < 1) && (name->compare(0, 2, "RF") == 0)) {
         SetRFMNEMONIC(name);
      }
      return;
   }
   std::string buf;
   fSystemString.assign(*name, 0, 2);
   fSubSystemString.assign(*name, 2, 1);
   EnumerateMnemonic(fSubSystemString, fSubSystem);
   buf.clear();
   buf.assign(*name, 3, 2);
   fArrayPosition = static_cast<uint16_t>(atoi(buf.c_str()));
   fArraySubPositionString.assign(*name, 5, 1);
   EnumerateMnemonic(fArraySubPositionString, fArraySubPosition);
   fCollectedChargeString.assign(*name, 6, 1);
   EnumerateMnemonic(fCollectedChargeString, fCollectedCharge);
   buf.clear();
   buf.assign(*name, 7, 2);
   fSegment = static_cast<uint16_t>(atoi(buf.c_str()));
   fOutputSensorString.assign(*name, 9, 1);
   EnumerateMnemonic(fOutputSensorString, fOutputSensor);
   // Enumerating the fSystemString must come last as the details of other parts of
   // the mnemonic must be known
   EnumerateSystem();

   if(fSystem == ESystem::kSiLi) {
      buf.clear();
      buf.assign(*name, 7, 2);
      fSegment = static_cast<uint16_t>(strtol(buf.c_str(), nullptr, 16));
   }

   return;
}

void TMnemonic::Parse(const char* name)
{
   std::string sname = name;
   Parse(&sname);
}

void TMnemonic::SetRFMNEMONIC(std::string* name)
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

void TMnemonic::Print(Option_t*) const
{
   printf("======MNEMONIC ======\n");
   printf("fArrayPosition           = %i\n", fArrayPosition);
   printf("fSegment                 = %i\n", fSegment);
   printf("fSystemString            = %s\n", fSystemString.c_str());
   printf("fSubSystemString         = %s\n", fSubSystemString.c_str());
   printf("fArraySubPositionString  = %s\n", fArraySubPositionString.c_str());
   printf("fCollectedChargeString   = %s\n", fCollectedChargeString.c_str());
   printf("fOutputSensorString      = %s\n", fOutputSensorString.c_str());
   printf("===============================\n");
}

TClass* TMnemonic::GetClassType() const
{
   if(fClassType != nullptr) {
      return fClassType;
   }

   switch(System()) {
		case ESystem::kTigress:    fClassType = TTigress::Class(); break;
		case ESystem::kSharc:      fClassType = TSharc::Class(); break;
		case ESystem::kTriFoil:    fClassType = TTriFoil::Class(); break;
		case ESystem::kRF:         fClassType = TRF::Class(); break;
		case ESystem::kSiLi:       fClassType = TSiLi::Class(); break;
		case ESystem::kS3:         fClassType = TS3::Class(); break;
		case ESystem::kSiLiS3:     fClassType = TS3::Class(); break;
		case ESystem::kCSM:        fClassType = TCSM::Class(); break;
		case ESystem::kGriffin:    fClassType = TGriffin::Class(); break;
		case ESystem::kSceptar:    fClassType = TSceptar::Class(); break;
		case ESystem::kPaces:      fClassType = TPaces::Class(); break;
		case ESystem::kDescant:    fClassType = TDescant::Class(); break;
		case ESystem::kLaBr:       fClassType = TLaBr::Class(); break;
		case ESystem::kTAC:        fClassType = TTAC::Class(); break;
		case ESystem::kZeroDegree: fClassType = TZeroDegree::Class(); break;
		case ESystem::kTip:        fClassType = TTip::Class(); break;
		case ESystem::kFipps:      fClassType = TFipps::Class(); break;
		case ESystem::kGeneric:    fClassType = TGenericDetector::Class(); break;
		default:                              fClassType = nullptr;
   };

   return fClassType;
}
