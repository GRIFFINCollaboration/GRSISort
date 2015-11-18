#include <iostream>
#include "TSceptar.h"
#include <TRandom.h>
#include <TMath.h>

////////////////////////////////////////////////////////////
//                    
// TSceptar
//
// The TSceptar class defines the observables and algorithms used
// when analyzing GRIFFIN data. It includes detector positions,
// etc. 
//
////////////////////////////////////////////////////////////

/// \cond CLASSIMP
ClassImp(TSceptar)
/// \endcond


bool TSceptar::fSetWave = false;

TVector3 TSceptar::gPaddlePosition[21] = { 
   //Sceptar positions from Evan; Thanks Evan.
   TVector3(0,0,1),
   TVector3(14.3025, 4.6472, 22.8096),  
   TVector3(0, 15.0386, 22.8096), 
   TVector3(-14.3025, 4.6472, 22.8096),  
   TVector3(-8.8395, -12.1665, 22.8096),  
   TVector3(8.8395, -12.1665, 22.8096),     
   TVector3(19.7051, 6.4026, 6.2123),  
   TVector3(0, 20.7192, 6.2123),  
   TVector3(-19.7051, 6.4026, 6.2123),  
   TVector3(-12.1784, -16.7622, 6.2123),  
   TVector3(12.1784, -16.7622, 6.2123),     
   TVector3(19.7051, 6.4026, -6.2123),  
   TVector3(0, 20.7192, -6.2123),  
   TVector3(-19.7051, 6.4026, -6.2123),  
   TVector3(-12.1784, -16.7622, -6.2123),  
   TVector3(12.1784, -16.7622, -6.2123),
   TVector3(14.3025, 4.6472, -22.8096),  
   TVector3(0, 15.0386, -22.8096),  
   TVector3(-14.3025, 4.6472, -22.8096),  
   TVector3(-8.8395, -12.1665, -22.8096),  
   TVector3(8.8395, -12.1665, -22.8096)
};


TSceptar::TSceptar() {
   //Default Constructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   //Class()->AddRule("TSceptar sceptar_hits attributes=NotOwner");
   //Class()->AddRule("TSceptar sceptardata attributes=NotOwner");
   Clear();
}

TSceptar::~TSceptar()	{
   //Default Destructor
}

TSceptar::TSceptar(const TSceptar& rhs) : TGRSIDetector() {
   //Copy Contructor
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	rhs.Copy(*this);
}

void TSceptar::Clear(Option_t *opt)	{
	//Clears all of the hits
	//The Option "all" clears the base class.
   if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
		TGRSIDetector::Clear(opt);
   }
	fSceptarHits.clear();
}

void TSceptar::Copy(TObject &rhs) const {
   //Copies a TSceptar
  TGRSIDetector::Copy(rhs);

  static_cast<TSceptar&>(rhs).fSceptarHits    = fSceptarHits;
}                                       

TSceptar& TSceptar::operator=(const TSceptar& rhs) {
   rhs.Copy(*this);
   return *this;
}

void TSceptar::Print(Option_t *opt) const	{
	//Prints out TSceptar Multiplicity, currently does little.
	printf("%lu fSceptarHits\n",fSceptarHits.size());
}

void TSceptar::PushBackHit(TGRSIDetectorHit *scHit) {
   //Adds a Hit to the list of TSceptar Hits
   fSceptarHits.push_back(*(static_cast<TSceptarHit*>(scHit)));
}

void TSceptar::AddFragment(TFragment* frag, MNEMONIC* mnemonic) {
	//Builds the SCEPTAR Hits directly from the TFragment. Basically, loops through the data for an event and sets observables. 
	//This is done for both SCEPTAR and it's suppressors.
	if(frag == NULL || mnemonic == NULL) {
		return;
	}

   Clear("");

	for(size_t i = 0; i < frag->Charge.size(); ++i) {
		TSceptarHit hit;
		hit.SetAddress(frag->ChannelAddress);
		hit.SetTimeStamp(frag->GetTimeStamp());
		hit.SetCfd(frag->GetCfd(i));
		hit.SetCharge(frag->GetCharge(i));
	  
      if(TSceptar::SetWave()){
         if(frag->wavebuffer.size() == 0) {
            printf("Warning, TSceptar::SetWave() set, but data waveform size is zero!\n");
         }
         hit.SetWaveform(frag->wavebuffer);
         if(hit.GetWaveform()->size() > 0) {
				printf("Analyzing waveform, current cfd = %d\n",hit.GetCfd());
            bool analyzed = hit.AnalyzeWaveform();
            printf("%s analyzed waveform, cfd = %d\n",analyzed ? "successfully":"unsuccessfully",hit.GetCfd());
         }
      }

		AddHit(&hit);
	}
}

TGRSIDetectorHit* TSceptar::GetHit(const Int_t& idx){
   //Gets the TSceptarHit at index idx. 
   return GetSceptarHit(idx);
}

TSceptarHit* TSceptar::GetSceptarHit(const int& i) {
   try{
      return &fSceptarHits.at(i);   
   }
   catch (const std::out_of_range& oor){
      std::cerr << ClassName() << " is out of range: " << oor.what() << std::endl;
      throw grsi::exit_exception(1);
   }
   return NULL;
}
