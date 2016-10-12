#ifndef MNEMONIC_H
#define MNEMONIC_H

#include <string>
#include "TObject.h"
#include "Globals.h"
#include "TClass.h"

class TMnemonic : public TObject {
   public:
		TMnemonic() :fClassType(NULL) {}
		TMnemonic(const char* name) : TMnemonic() { Parse(name); }
		~TMnemonic() {}

	private:
		int16_t fArrayPosition;
		int16_t fSegment;
		std::string fSystemString;
		std::string fSubSystemString;
		std::string fArraySubPositionString;
		std::string fCollectedChargeString;
		std::string fOutputSensorString;

		int fSystem;
		int fSubSystem;
		int fArraySubPosition;
		int fCollectedCharge;
		int fOutputSensor;

      mutable TClass*   fClassType;          //!<! TGRSIDetector Type that this mnemonic represents
	
	public:
	//standard C++ makes these enumerations global to the class. ie, the name of the enumeration
	//EMnemonic or ESystem has no effect on the clashing of enumerated variable names.
	//These separations exist only to easily see the difference when looking at the code here.
		enum EMnemonic { kA, kB, kC, kD, kE, kF, kG, kI, kL, kM, kN, kP, kQ, kR, kS, kW, kX, kZ, kClear };
		enum ESystem   { kTigress, kSharc, kTriFoil, kRF, kCSM, kSiLi, kS3, kBambino, kTip, kGriffin, kSceptar, kPaces, kLaBr, kTAC, kZeroDegree, kDescant};

		void EnumerateMnemonic(std::string mnemonic_word, int &mnemonic_enum);

		int System() const { return fSystem; }
		int SubSystem() const { return fSubSystem; }
		int ArraySubPosition() const { return fArraySubPosition; }
		int CollectedCharge() const { return fCollectedCharge; }
		int OutputSensor() const { return fOutputSensor; } 
		int16_t ArrayPosition() const {return fArrayPosition; }
		int16_t Segment() const { return fSegment; }

		std::string SystemString() const { return fSystemString; }
		std::string SubSystemString() const { return fSubSystemString; }
		std::string ArraySubPositionString() const { return fArraySubPositionString; }
		std::string CollectedChargeString() const { return fCollectedChargeString; }
		std::string OutputSensorString() const { return fOutputSensorString; } 

		void EnumerateSystem();
		void Parse(std::string *name);
		void Parse(const char *name);

		void SetRFMNEMONIC(std::string *name);

      void SetClassType(TClass* classType)               { fClassType = classType; }
      TClass* GetClassType() const;


		void Print(Option_t *opt="") const;
		void Clear(Option_t *opt=""); 
		
		ClassDef(TMnemonic,1);

};


#endif
