#include "TCal.h"

/// \cond CLASSIMP
ClassImp(TCal)
/// \endcond

TCal::TCal() {
   ///Default constructor
	InitTCal();
}

TCal::TCal(const char* name, const char* title) {
	///Constructor for naming the calibration
	InitTCal();
	SetNameTitle(name,title);
	//fgraph->SetNameTitle(name,title);
}

TCal::~TCal() {
	///Default dtor
	fNuc = NULL;
	//fgraph = NULL;
	fHist = NULL;
}

TCal::TCal(const TCal& copy) : TGraphErrors(copy) {
	///Copy constructor
	InitTCal();
	copy.Copy(*this);
}

void TCal::SetNucleus(TNucleus* nuc,Option_t * opt) {
	///Sets the nucleus to be calibrated against
	if(!nuc) {
		Error("SetNucleus","Nucleus does not exist");
		return;
	}
	if(fNuc)
		Warning("SetNucleus","Overwriting nucleus: %s",fNuc->GetName());
	fNuc = nuc;
}

void TCal::Copy(TObject &obj) const {
	///Copies the TCal.
	static_cast<TCal&>(obj).fChan = fChan;
	//Things to make deep copies of
	if(fFitFunc)
		*(static_cast<TCal&>(obj).fFitFunc)   =  *fFitFunc;

	//Members to make shallow copies of
	static_cast<TCal&>(obj).fNuc        =  fNuc;
	TNamed::Copy(static_cast<TGraphErrors&>(obj));
}

Bool_t TCal::SetChannel(const TChannel* chan) {
	///Sets the channel being calibrated
	if(!chan) {
		Error("SetChannel","TChannel does not exist");
		printf("%p\n", static_cast<const void*>(chan));
		return false;
	}
	//Set our TRef to point at the TChannel
	fChan = const_cast<TChannel*>(chan);
	return true;
}

void TCal::WriteToAllChannels(std::string mnemonic) {
	///Writes this calibration to all channels in the current TChannel Map
	std::map<unsigned int,TChannel*>::iterator mapIt;
	std::map<unsigned int,TChannel*>* chanMap = TChannel::GetChannelMap();
	TChannel* origChan = GetChannel();
	for(mapIt = chanMap->begin(); mapIt != chanMap->end(); mapIt++){
		if(!mnemonic.size() || !strncmp(mapIt->second->GetName(),mnemonic.c_str(),mnemonic.size())){
			SetChannel(mapIt->second);
			WriteToChannel();
		}
	}
	printf("\n");
	if(origChan)
		SetChannel(origChan);
}

std::vector<Double_t> TCal::GetParameters() const {
	///Returns all of the parameters in the current TCal.
	std::vector<Double_t> paramList;
	if(!GetFitFunction()) {
		Error("GetParameters","Function has not been fitted yet");
		return paramList;
	}

	Int_t nParams = GetFitFunction()->GetNpar();

	for(int i = 0; i < nParams; i++)
		paramList.push_back(GetParameter(i));

	return paramList;
}

Double_t TCal::GetParameter(size_t parameter) const {
	///Returns the parameter at the index parameter
	if(!GetFitFunction()) {
		Error("GetParameter","Function have not been fitted yet");
		return 0;
	}
	return GetFitFunction()->GetParameter(parameter); //Root does all of the checking for us.
}

Bool_t TCal::SetChannel(UInt_t chanNum) {
	///Sets the channel for the calibration to the channel number channum. Returns 
	///0 if the channel does not exist
	TChannel* chan = TChannel::GetChannelByNumber(chanNum);
	if(!chan) {
		Error("SetChannel","Channel Number %d does not exist in current memory.",chanNum);
		return false;
	} else {
		return SetChannel(chan);
	}
}

TChannel* TCal::GetChannel() const {
	///Gets the channel being pointed to by the TCal. Returns 0 if no channel
	///is set.
	return static_cast<TChannel*>(fChan.GetObject()); //Gets the object pointed at by the TRef and casts it to a TChannel
}

void TCal::SetHist(TH1* hist) {
	///Sets this histogram pointed to. TCal does NOT take ownership so you cannot delete this
	///histogram as long as you want to access the hist in the TCal/write it out. I will add this
	///functionality if I get annoyed enough with the way it is.
	fHist = hist;
}   

void TCal::Clear(Option_t *opt) {
	///Clears the calibration. Does not delete nuclei or channels.
	fNuc = NULL;
	fChan = NULL;
	TGraphErrors::Clear();
}

void TCal::Print(Option_t *opt) const{
	///Prints calibration information
	if(GetChannel())
		printf("Channel Number: %u\n",GetChannel()->GetNumber());
	else
		printf("Channel Number: NOT SET\n");


	if(fFitFunc) {
		printf("\n*******************************\n");
		printf(" Fit:\n");      
		fFitFunc->Print();
		printf("\n*******************************\n");   
	} else {
		printf("Parameters: FIT NOT SET\n");
	}

	printf("Nucleus: ");
	if(GetNucleus())
		printf("%s\n",GetNucleus()->GetName());
	else
		printf("NOT SET\n");
}

void TCal::InitTCal() {
	///Initiallizes the TCal.
	/* fgraph = new TGraphErrors;*/
	fFitFunc = NULL;
	fChan = NULL;
	fNuc = NULL;
	fHist = NULL;
	Clear();
}
