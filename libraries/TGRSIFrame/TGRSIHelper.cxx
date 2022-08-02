#include "TGRSIHelper.h"

TGRSIHelper::TGRSIHelper(TList* input) {
	fPpg = static_cast<TPPG*>(input->FindObject("TPPG"));
	fRunInfo = static_cast<TRunInfo*>(input->FindObject("TRunInfo"));
	// get the analysis options from the input list and assign them to our local analysis options
	// (this might not be needed anymore since the workers aren't started as separate processes but threads)
	*(TGRSIOptions::AnalysisOptions()) = *static_cast<TAnalysisOptions*>(input->FindObject("TAnalysisOptions"));
	// check that we have a parser library
	if(TGRSIOptions::Get()->ParserLibrary().empty()) {
		std::stringstream str;
		str<<DRED<<"No parser library set!"<<RESET_COLOR<<std::endl;
		throw std::runtime_error(str.str());
	}
	// loop over all cal-files, val-files, and cut-files we might have (these are full paths!)
	int i = 0;
	while(input->FindObject(Form("calFile%d", i)) != nullptr) {
		const char* fileName = static_cast<TNamed*>(input->FindObject(Form("calFile%d", i)))->GetTitle();
		if(fileName[0] == 0) {
			std::cout<<"Error, empty file name!"<<std::endl;
			break;
		}
		TChannel::ReadCalFile(fileName);
		++i;
	}
	i = 0;
	while(input->FindObject(Form("valFile%d", i)) != nullptr) {
		const char* fileName = static_cast<TNamed*>(input->FindObject(Form("valFile%d", i)))->GetTitle();
		if(fileName[0] == 0) {
			std::cout<<"Error, empty file name!"<<std::endl;
			break;
		}
		GValue::ReadValFile(fileName);
		++i;
	}
	i = 0;
	while(input->FindObject(Form("cutFile%d", i)) != nullptr) {
		std::cout<<"trying to open "<<Form("cutFile%d", i)<<std::flush<<" = "<<input->FindObject(Form("cutFile%d", i))<<std::flush<<" with title "<<static_cast<TNamed*>(input->FindObject(Form("cutFile%d", i)))->GetTitle()<<std::endl;
		const char* fileName = static_cast<TNamed*>(input->FindObject(Form("cutFile%d", i)))->GetTitle();
		if(fileName[0] == 0) {
			std::cout<<"Error, empty file name!"<<std::endl;
			break;
		}
		// if we have a relative path and a working directory, combine them
		auto file = new TFile(fileName);
		if(file != nullptr && file->IsOpen()) {
			TIter iter(file->GetListOfKeys());
			TKey* key = nullptr;
			while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
				if(strcmp(key->GetClassName(), "TCutG") != 0) {
					continue;
				}
				TCutG* tmpCut = static_cast<TCutG*>(key->ReadObj());
				if(tmpCut != nullptr) {
					fCuts[tmpCut->GetName()] = tmpCut;
				}
			}
		} else {
			std::cout<<"Error, failed to open file "<<fileName<<"!"<<std::endl;
			break;
		}
		++i;
	}
	for(auto cut : fCuts) {
		std::cout<<cut.first<<" = "<<cut.second<<std::endl;
	}

	if(GValue::Size() == 0) {
		std::cout<<"No g-values!"<<std::endl;
	} else {
		std::cout<<GValue::Size()<<" g-values"<<std::endl;
	}
}

void TGRSIHelper::Setup() {
	std::cout<<__PRETTY_FUNCTION__<<" calling setup of TGRSIHelper"<<std::endl;
	const auto nSlots = ROOT::IsImplicitMTEnabled() ? ROOT::GetThreadPoolSize() : 1;
	TH1::AddDirectory(false); // turns off warnings about multiple histograms with the same name because ROOT doesn't manage them anymore
	for(auto i : ROOT::TSeqU(nSlots)) {
		fLists.emplace_back(std::make_shared<TList>());
		fH1.emplace_back(TGRSIMap<std::string, TH1*>());
		fH2.emplace_back(TGRSIMap<std::string, TH2*>());
		CreateHistograms(i);
		for(auto it : fH1[i]) {
			fLists[i]->Add(it.second);
		}
		for(auto it : fH2[i]) {
			fLists[i]->Add(it.second);
		}
		CheckSizes(i, "use");
	}
	TH1::AddDirectory(true); // restores old behaviour
}

void TGRSIHelper::Finalize() {
	CheckSizes(0, "write");
	auto &res = fLists[0];
	for(auto slot : ROOT::TSeqU(1, fLists.size())) {
		CheckSizes(slot, "write");
		for(const auto&& obj : *res) {
			if(obj->InheritsFrom(TH1::Class())) {
				if(fLists[slot]->FindObject(obj->GetName()) != nullptr) {
					static_cast<TH1*>(obj)->Add(static_cast<TH1*>(fLists[slot]->FindObject(obj->GetName())));
				} else {
					std::cerr<<"Failed to find object '"<<obj->GetName()<<"' in "<<slot<<". list"<<std::endl;
				}
			} else {
				std::cerr<<"Object '"<<obj->GetName()<<"' is not a histogram ("<<obj->ClassName()<<"), don't know what to do with it!"<<std::endl;
			}
		}
	}
}

void TGRSIHelper::CheckSizes(unsigned int slot, const char* usage) {
	// check size of each object in the output list
	for(const auto&& obj : *fLists[slot]) {
		TBufferFile b(TBuffer::kWrite, 10000);
		obj->IsA()->WriteBuffer(b, obj);
		if(b.Length() > SIZE_LIMIT) {
			std::stringstream str;
			str<<DRED<<slot<<". slot: "<<obj->ClassName()<<" '"<<obj->GetName()<<"' too large to "<<usage<<": "<<b.Length()<<" bytes = "<<b.Length()/1024./1024./1024.<<" GB, removing it!"<<RESET_COLOR<<std::endl;
			std::cout<<str.str();
			// we only remove it from the output list, not deleting the object itself
			// this way the filling of that histogram will still work, it just won't get written to file
			// we remove it from all lists though, not just the first one
			fLists[slot]->Remove(obj);
		}
	}
}

