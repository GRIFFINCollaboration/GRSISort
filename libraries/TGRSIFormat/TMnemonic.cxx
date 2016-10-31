#include "TMnemonic.h"

#include <algorithm>

//Detector dependent includes
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


ClassImp(TMnemonic)

void TMnemonic::Clear(Option_t *opt) {
	fArrayPosition = -1;
	fSegment       = -1;
	fSystemString.clear();
	fSubSystemString.clear(); fSubSystem = kClear;
	fArraySubPositionString.clear(); fArraySubPosition = kClear;
	fCollectedChargeString.clear(); fCollectedCharge = kClear;
	fOutputSensorString.clear(); fOutputSensor = kClear;

}

void TMnemonic::EnumerateMnemonic(std::string mnemonic_word, int &mnemonic_enum){

	char mnemonic_char = mnemonic_word[0];

	switch(mnemonic_char){
		case 'A':
			mnemonic_enum = kA;
			break;
		case 'B':
			mnemonic_enum = kB;
			break;
		case 'C':
			mnemonic_enum = kC;
			break;
		case 'D':
			mnemonic_enum = kD;
			break;
		case 'E':
			mnemonic_enum = kE;
			break;
		case 'F':
			mnemonic_enum = kF;
			break;
		case 'G':
			mnemonic_enum = kG;
			break;
		case 'I':
			mnemonic_enum = kI;
			break;
		case 'L':
			mnemonic_enum = kL;
			break;
		case 'M':
			mnemonic_enum = kM;
			break;
		case 'N':
			mnemonic_enum = kN;
			break;
		case 'P':
			mnemonic_enum = kP;
			break;
		case 'Q':
			mnemonic_enum = kQ;
			break;
		case 'R':
			mnemonic_enum = kR;
			break;
		case 'S':
			mnemonic_enum = kS;
			break;
		case 'W':
			mnemonic_enum = kW;
			break;
		case 'X':
			mnemonic_enum = kX;
			break;
		case 'Z':
			mnemonic_enum = kZ;
			break;
		default: 
			mnemonic_enum = kClear;
	};
}

void TMnemonic::EnumerateSystem(){
	//Enumerating the fSystemString must come after the total mnemonic has been parsed as the details of other parts of
	//the mnemonic must be known
	if(fSystemString.compare("TI")==0) {
		fSystem = kTigress;
	} else if (fSystemString.compare("SH")==0) {
		fSystem = kSharc;
	} else if(fSystemString.compare("TR")==0) {	
		fSystem = kTriFoil;
	} else if(fSystemString.compare("RF")==0) {	
		fSystem = kRF;
	} else if(fSystemString.compare("SP")==0) {
		if(SubSystem() == kI) {
			fSystem = kSiLi;
		} else {
			fSystem = kS3;
		}
	} else if(fSystemString.compare("CS")==0) {	
		fSystem = kCSM;
	} else if(fSystemString.compare("GR")==0) {
		fSystem = kGriffin;
	} else if(fSystemString.compare("SE")==0) {
		fSystem = kSceptar;
	} else if(fSystemString.compare("PA")==0) {	
		fSystem = kPaces;
	} else if(fSystemString.compare("DS")==0) {	
		fSystem = kDescant;
	} else if(fSystemString.compare("DA")==0) {
		if(CollectedCharge() == kN) {
			fSystem = kLaBr;
		} else {
			fSystem = kTAC;
		}
	} else if(fSystemString.compare("BA")==0) {
		fSystem = kS3;
	} else if(fSystemString.compare("ZD")==0) {	
		fSystem = kZeroDegree;
	} else if(fSystemString.compare("TP")==0) {	
		fSystem = kTip;
	} else {
		fSystem = kClear;
	}
}

int TMnemonic::EnumerateDigitizer(std::string& name) {
	std::transform(name.begin(), name.end(), name.begin(), ::toupper);
	if(name.compare("GRF16") == 0) return kGRF16;
	if(name.compare("GRF4G") == 0) return kGRF4G;
	if(name.compare("TIG10") == 0) return kTIG10;
	if(name.compare("TIG64") == 0) return kTIG64;
	return kDefault;
}

void TMnemonic::Parse(std::string *name){
	if(!name || name->length()<9) {
		if((name->length()<1) && (name->compare(0,2,"RF")==0))
			SetRFMNEMONIC(name);
		return;
	}   
	std::string buf;
	fSystemString.assign(*name,0,2);
	fSubSystemString.assign(*name,2,1);
	EnumerateMnemonic(fSubSystemString,fSubSystem);
	buf.clear(); buf.assign(*name,3,2);
	fArrayPosition = (uint16_t)atoi(buf.c_str());
	fArraySubPositionString.assign(*name,5,1);
	EnumerateMnemonic(fArraySubPositionString,fArraySubPosition);
	fCollectedChargeString.assign(*name,6,1);
	EnumerateMnemonic(fCollectedChargeString,fCollectedCharge);
	buf.clear(); buf.assign(*name,7,2);
	fSegment = (uint16_t)atoi(buf.c_str());
	fOutputSensorString.assign(*name,9,1);
	EnumerateMnemonic(fOutputSensorString,fOutputSensor);
	//Enumerating the fSystemString must come last as the details of other parts of
	//the mnemonic must be known
	EnumerateSystem();
	
	if(fSystem == kSiLi){
		buf.clear(); buf.assign(*name,7,2);
		fSegment = (uint16_t)strtol(buf.c_str(), NULL, 16);
	}
	
	return;
}

void TMnemonic::Parse(const char *name){
	std::string sname = name;
	Parse(&sname);
	return;
}

void TMnemonic::SetRFMNEMONIC(std::string *name){
	fSystemString.assign(*name,0,2);
	fSubSystemString.assign("X");
	EnumerateMnemonic(fSubSystemString,fSubSystem);
	fArrayPosition = 0;
	fArraySubPositionString.assign("X");
	EnumerateMnemonic(fArraySubPositionString,fArraySubPosition);
	fCollectedChargeString.assign("X");
	EnumerateMnemonic(fCollectedChargeString,fCollectedCharge);   
	fSegment = 0;
	fOutputSensorString.assign("X");
	EnumerateMnemonic(fOutputSensorString,fOutputSensor);

}

void TMnemonic::Print(Option_t * opt) const{
	printf("======MNEMONIC ======\n");
	printf("fArrayPosition           = %i\n", fArrayPosition);
	printf("fSegment                 = %i\n", fSegment);
	printf("fSystemString            = %s\n", fSystemString.c_str());
	printf("fSubSystemString         = %s\n", fSubSystemString.c_str());
	printf("fArraySubPositionString  = %s\n", fArraySubPositionString.c_str());
	printf("fCollectedChargeString   = %s\n", fCollectedChargeString.c_str());
	printf("fOutputSensorString      = %s\n", fOutputSensorString.c_str());
	printf("===============================\n");
	return;
}

TClass* TMnemonic::GetClassType() const {
	if(fClassType != nullptr)
		return fClassType;

   switch(System()){
      case TMnemonic::kTigress:
         fClassType = TTigress::Class();
         break;
      case TMnemonic::kSharc:
         fClassType = TSharc::Class();
         break;
      case TMnemonic::kTriFoil:
         fClassType = TTriFoil::Class();
         break;
      case TMnemonic::kRF:
         fClassType = TRF::Class();
         break;
      case TMnemonic::kSiLi:
         fClassType = TSiLi::Class();
         break;
      case TMnemonic::kS3:
         fClassType = TS3::Class();
         break;
      case TMnemonic::kCSM:
         fClassType = TCSM::Class();
         break;
      case TMnemonic::kGriffin:
         fClassType = TGriffin::Class();
         break;
      case TMnemonic::kSceptar:
         fClassType = TSceptar::Class();
         break;
      case TMnemonic::kPaces:
         fClassType = TPaces::Class();
         break;
      case TMnemonic::kDescant:
         fClassType = TDescant::Class();
         break;
      case TMnemonic::kLaBr:
         fClassType = TLaBr::Class();
         break;
      case TMnemonic::kTAC:
         fClassType = TTAC::Class();
         break;
      case TMnemonic::kZeroDegree:
         fClassType = TZeroDegree::Class();
         break;
      case TMnemonic::kTip:
         fClassType = TTip::Class();
         break;
      default:
         fClassType = nullptr;
   };

   return fClassType;
}
