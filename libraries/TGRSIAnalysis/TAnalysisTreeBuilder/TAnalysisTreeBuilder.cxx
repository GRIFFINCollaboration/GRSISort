
#include "Globals.h"

#include <TVirtualIndex.h>
#include <TTreeIndex.h>
#include <TSystem.h>

#include "TAnalysisTreeBuilder.h"
#include "TGRSIOptions.h"

#include <TStopwatch.h>

#include <TGRSIOptions.h>

//#include "TSharcData.h"


TEventQueue *TEventQueue::fPtrToQue = 0;

TEventQueue::TEventQueue() { }

TEventQueue *TEventQueue::Get() {
   if(!fPtrToQue)
      fPtrToQue = new TEventQueue;
   return fPtrToQue;
}

void TEventQueue::Add(std::vector<TFragment> *event) {
  Get()->Add_Instance(event);
}

std::vector<TFragment> *TEventQueue::PopEntry() {
  return Get()->PopEntry_Instance();
}

int TEventQueue::Size() {
  return Get()->Size_Instance();
}

void TEventQueue::Add_Instance(std::vector<TFragment> *event) {
   ///Thread-safe method for adding events to the event queue. 
   m_event.lock();
   fEventQueue.push(event);
   m_event.unlock();
   //UnsetLock();
   return;
}

std::vector<TFragment> *TEventQueue::PopEntry_Instance() {
	///Thread-safe method for taking an event out of the event queue
   std::vector<TFragment> *temp;
   m_event.lock();
   temp = fEventQueue.front();
   fEventQueue.pop();
   m_event.unlock();
   return temp;
}

int TEventQueue::Size_Instance() {
	///Thread-safe method for checking the size of the event queue
   int temp;
   m_event.lock();
   temp = fEventQueue.size();
   m_event.unlock();
   return temp;
}

TWriteQueue *TWriteQueue::fPtrToQue = 0;

TWriteQueue::TWriteQueue() { }

//TWriteQueue::~TWriteQueue() { }

TWriteQueue *TWriteQueue::Get() {
   ///Returns a pointer to the write queue
   if(!fPtrToQue)
      fPtrToQue = new TWriteQueue;
   return fPtrToQue;
}

void TWriteQueue::Add(std::map<std::string, TDetector*> *event) {
  Get()->Add_Instance(event);
}

std::map<std::string, TDetector*> *TWriteQueue::PopEntry() {
  return Get()->PopEntry_Instance();
}

int TWriteQueue::Size() {
  return Get()->Size_Instance();
}

void TWriteQueue::Add_Instance(std::map<std::string, TDetector*> *event) {
   ///Thread-safe method for adding to the event queue
   m_write.lock();
   fWriteQueue.push(event);
   m_write.unlock();
   return;
}

std::map<std::string, TDetector*> *TWriteQueue::PopEntry_Instance() {
   ///Thread safe method for taking an event out of the write queue
   std::map<std::string, TDetector*> *temp;
   m_write.lock();
   temp = fWriteQueue.front();
   fWriteQueue.pop();
   m_write.unlock();
   return temp;
}

int TWriteQueue::Size_Instance() {
   ///Thread-safe method for checking the size of the event queue
   int temp;
   m_write.lock();
   temp = fWriteQueue.size();
   m_write.unlock();
   return temp;
}

///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///
///************************************************///


ClassImp(TAnalysisTreeBuilder)

//This sets the minimum amount of memory that root can hold a tree in.
const size_t TAnalysisTreeBuilder::MEM_SIZE = (size_t)1024*(size_t)1024*(size_t)1024*(size_t)1; // 16 GB

TAnalysisTreeBuilder* TAnalysisTreeBuilder::fAnalysisTreeBuilder = 0;

//Reset the statistics of the analysis tree builder
long TAnalysisTreeBuilder::fEntries     = 0;
int  TAnalysisTreeBuilder::fFragmentsIn = 0;
int  TAnalysisTreeBuilder::fAnalysisIn  = 0;
int  TAnalysisTreeBuilder::fAnalysisOut = 0;


TAnalysisTreeBuilder* TAnalysisTreeBuilder::Get() {
   ///Returns an instance of the singleton AnalysisTreeBuilder
   if(fAnalysisTreeBuilder == 0)
      fAnalysisTreeBuilder = new TAnalysisTreeBuilder;
   return fAnalysisTreeBuilder;
}

TAnalysisTreeBuilder::TAnalysisTreeBuilder() {
   ///The Default ctor for the analysis tree builder
   fFragmentChain = 0;
   fCurrentFragTree = 0;
   fCurrentFragFile = 0;
   fCurrentAnalysisTree = 0;
   fCurrentAnalysisFile = 0;
   fCurrentRunInfo = 0;
   fCurrentPPG = 0;

   fCurrentFragPtr = 0;

   tigress = 0;//new TTigress;
   sharc = 0;//new TSharc;
   triFoil = 0;//new TTriFoil;
   rf = 0;
   csm = 0;//new TCSM;
   sili = 0;
   s3 = 0;
   tip = 0;

   griffin = 0;//new TGriffin;
   sceptar = 0;//new TSceptar;
   paces   = 0;//new TPaces;
   descant = 0;//new TDescant;
   //dante->Clear();
   //zerodegree->Clear();

}


//TAnalysisTreeBuilder::~TAnalysisTreeBuilder() { }

void TAnalysisTreeBuilder::StartMakeAnalysisTree(int argc, char** argv) {
   ///Sets up a fragment chain from the list of fragment files sent to
   ///grsisort. This fragment chain then gets sorted by timestamp.
   if(argc==1) {
      SetUpFragmentChain(TGRSIOptions::GetInputRoot());
   } else {
      return;
   }
   SortFragmentChain();   

}


void TAnalysisTreeBuilder::InitChannels() {
   ///Initializes the channels from a cal file on the command line when 
   ///grsisort is started. If no cal file is input on the command line
   ///grsisort attempts to read the calibration from the fragment tree
   ///if it exists.
   
   if(!fCurrentFragTree)
      return;

   //Delete channels from memory incase there is something in there still
   TChannel::DeleteAllChannels(); 
   //Try to read the calibration data from the fragment tree
	TChannel::ReadCalFromTree(fCurrentFragTree);

   //If we find an input cal file, we overwrite what the tree calibration is with that cal file
   if(!TGRSIOptions::GetInputCal().empty()) {
      for(size_t x = 0; x < TGRSIOptions::GetInputCal().size(); ++x) {
         TChannel::ReadCalFile(TGRSIOptions::GetInputCal().at(x).c_str());
      }
   }
   printf("AnalysisTreeBuilder:  read in %i TChannels.\n", TChannel::GetNumberOfChannels());
}  



void TAnalysisTreeBuilder::SetUpFragmentChain(std::vector<std::string> infiles) {
   ///Makes a TChain of all of the fragments input on the command line. This may not work as
   ///desired for the Griffin DAQ. In this case histograms should be summed during post
   ///processing.
   TChain *chain = new TChain("FragmentTree");
   for(size_t x = 0; x < infiles.size(); ++x) 
      chain->Add(infiles.at(x).c_str());
   SetUpFragmentChain(chain);
   
}

void TAnalysisTreeBuilder::SetUpFragmentChain(TChain *chain) {
   ///Sets the fFragment chain from the TChain chain.
   if(fFragmentChain)
      delete fFragmentChain;
   fFragmentChain = chain;
   fFragmentChain->CanDeleteRefs(true); 
}

void TAnalysisTreeBuilder::SortFragmentChain() {
   ///Sorts the fragment chain by tree number.
   if(!fFragmentChain)
      return;
   
   int ntrees = fFragmentChain->GetNtrees();
   int nChainEntries = fFragmentChain->GetEntries();
   int treeNumber, lastTreeNumber = -1;

   printf("Found %i trees with %i total fragments.\n",ntrees,nChainEntries);

   for(int i=0;i<nChainEntries;i++) {
      fFragmentChain->LoadTree(i);
      treeNumber = fFragmentChain->GetTreeNumber();
      if(treeNumber != lastTreeNumber) {
         if(lastTreeNumber == -1) {
            printf(DYELLOW "Sorting tree[%d]: %s" RESET_COLOR "\n",treeNumber+1,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
         } else {
            printf(DYELLOW "Changing to tree[%d/%d]: %s" RESET_COLOR "\n",treeNumber+1,ntrees,fFragmentChain->GetTree()->GetCurrentFile()->GetName());
            printf(DYELLOW "    Switched from tree[%d] at chain entry number %i" RESET_COLOR "\n",lastTreeNumber,i);
         }
         lastTreeNumber = treeNumber;
      }  else {
         continue;
      }
      fCurrentFragTree = fFragmentChain->GetTree();
      int nentries = fCurrentFragTree->GetEntries();
      SetupFragmentTree();
      SetupOutFile();
      SetupAnalysisTree();

      fSortFragmentDone = false;
      fPrintStatus = true;
      fProcessThread = new std::thread(&TAnalysisTreeBuilder::ProcessEvent,this);
      fWriteThread = new std::thread(&TAnalysisTreeBuilder::WriteAnalysisTree,this);
      fStatusThread = new std::thread(&TAnalysisTreeBuilder::Status,this);

      fFragmentsIn = 0;
      fAnalysisIn  = 0;
      fAnalysisOut = 0;
      if(fCurrentRunInfo->Griffin())
         SortFragmentTreeByTimeStamp();
      else
         SortFragmentTree();
      fSortFragmentDone = true;

      fProcessThread->join();
      fWriteThread->join();
      fPrintStatus = false;
      
      CloseAnalysisFile();
      printf("\n");
      i+=(nentries);//-10);
      fCurrentFragFile->Close("r");
   }
   printf("Finished chain sort.\n");
   return;
}

void TAnalysisTreeBuilder::SortFragmentTree() {
   ///Sorts the fragment tree based on the TreeIndex major name.
   ///It then puts the fragment into the event Q.
   long major_min = (long) fCurrentFragTree->GetMinimum(fCurrentFragTree->GetTreeIndex()->GetMajorName());
   if(major_min<0)
      major_min = 0;
   long major_max = (long) fCurrentFragTree->GetMaximum(fCurrentFragTree->GetTreeIndex()->GetMajorName());

   TTreeIndex *index = (TTreeIndex*)fCurrentFragTree->GetTreeIndex();
   fEntries = index->GetN();

   for(int j=major_min;j<=major_max;j++) {
      std::vector<TFragment> *event = new std::vector<TFragment>;
      int fragno = 1;
      while(fCurrentFragTree->GetEntryWithIndex(j,fragno++) != -1) {
         fFragmentsIn++;
         event->push_back(*fCurrentFragPtr);
      }
      if(event->empty()) {
         delete event;
      } else {
         TEventQueue::Add(event);
      }
   }
   printf("\n");
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}


void TAnalysisTreeBuilder::SortFragmentTreeByTimeStamp() {
   ///Suprisingly, sorts the fragment tree by times stamps.
   ///It then takes the sorted fragments and puts them into the eventQ.
   TFragment *currentFrag = 0;

   //Find the TFragment branch of the tree
   fCurrentFragTree->SetBranchAddress("TFragment",&currentFrag);

   fEntries = fCurrentFragTree->GetEntries();

   //this vector holds all fragments of one built event and get's passed to the event queue
   std::vector<TFragment>* event = NULL;
	//this multiset is used to sort the fragments we're reading sequentially (but not necessarily time-ordered) from the tree
	std::multiset<TFragment,std::less<TFragment> > sortedFragments;//less is the default ordering option, could be left out

   //loop over all of the fragments in the tree 
   for(int x = 0; x < fEntries; ++x) {
      if(fCurrentFragTree->GetEntry(x) == -1 ) {
         //GetEntry Reads all branches of entry and returns total number of bytes read.
         //This means that if this if statement passes, we have had an I/O error.
         printf(DRED "FIRE!!!" RESET_COLOR  "\n");
         continue;
      }
      fFragmentsIn++;//Now that we have read a new entry, we need to increment our counter

		//pull the different pile-up hits apart and put the into the sorted buffer as different fragments
		for(size_t hit = 0; hit < currentFrag->Cfd.size(); ++hit) {
			try {
				sortedFragments.insert(TFragment(*currentFrag, hit));
			} catch (std::bad_alloc& e) {
				//failed to insert the fragment, check overall size of the multiset
				if(sortedFragments.size() < TGRSIRunInfo::BufferSize()) {
					printf(BG_RED WHITE "Ran out of memory trying to insert the %dth fragment into the multiset, minimum requested size of multiset is %d, aborting!" RESET_COLOR "\n", (int) sortedFragments.size(), (int) TGRSIRunInfo::BufferSize());
					abort();
				} else {
					//Wait and try inserting again. If that fails as well we can't do anything else, so we don't catch exceptions here
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					sortedFragments.insert(TFragment(*currentFrag, hit));					
				}
			}
		}

		///We start putting fragments into a build event when either the time 
		///difference between the first and the last fragment is larger than a 
		///given value, or if the size of the multiset is above a given threshold
		///and the first and last of the fragments are at least BuildWindow apart.
 		long firstTimeStamp = (*(sortedFragments.begin())).GetTimeStamp();
		long lastTimeStamp = (*(std::prev(sortedFragments.end()))).GetTimeStamp();
		if(//(lastTimeStamp - firstTimeStamp > TGRSIRunInfo::BufferDuration()) ||
			(sortedFragments.size() > TGRSIRunInfo::BufferSize() && (lastTimeStamp - firstTimeStamp > TGRSIRunInfo::BuildWindow()))) {
			//We are now in a situation where we think that no more fragments will be read that will end up at the beginning of the multiset.
			//So we can put everything that is in the BuildWindow of the first fragment into our built event.
			//For this we loop through the fragments in the multiset, put them into the new event vector, and once the time difference is larger than BuildWindow,
			//we put the event into the queue, erase the fragments from the multiset, and stop the loop.
			event = new std::vector<TFragment>;
			for(auto it = sortedFragments.begin(); it != sortedFragments.end(); ++it) {
				if(it->GetTimeStamp() - firstTimeStamp <= TGRSIRunInfo::BuildWindow()) {
					event->push_back(*it);
					if(TGRSIRunInfo::IsMovingWindow()){
						firstTimeStamp = it->GetTimeStamp(); //THIS IS FOR MOVING WINDOW
					}
				} else {
					//Add the event to the event Q.
					TEventQueue::Get()->Add(event);
					//erase the fragments [first,current), i.e. from the first to the current, but not the current one itself
					sortedFragments.erase(sortedFragments.begin(),it);
					//this screws up the iterators, so never remove the break statement after this!
					break;
				}
			}
		}
   }//loop over entries

	printf("Finished reading %d fragments, built %d events, got %d fragments left\t\t\t\t\t\t\t\t\t\t\n", fFragmentsIn, fAnalysisIn, (int) sortedFragments.size());

   //In case we have fragments left after all of the fragments have been processed, we add them to the queue now.
	while(!sortedFragments.empty()) {
 		long firstTimeStamp = (*(sortedFragments.begin())).GetTimeStamp();
		event = new std::vector<TFragment>;
		auto it = sortedFragments.begin();
		for(; it != sortedFragments.end(); ++it) {
			if(it->GetTimeStamp() - firstTimeStamp <= TGRSIRunInfo::BuildWindow()) {
				event->push_back(*it);
				if(TGRSIRunInfo::IsMovingWindow()){
					firstTimeStamp = it->GetTimeStamp(); //THIS IS FOR MOVING WINDOW
				}
			} else {
				//Add the event to the event Q.
				TEventQueue::Get()->Add(event);
				//erase the fragments [first,current), i.e. from the first to the current, but not the current one itself
				sortedFragments.erase(sortedFragments.begin(),it);
				//this screws up the iterators, so never remove the break statement after this!
				break;
			}
		}
		//if this is the last fragments, just fill them
		if(it == sortedFragments.end()) {
			//Add the event to the event Q.
			TEventQueue::Get()->Add(event);
			//erase the fragments [first,current), i.e. from the first to the current, but not the current one itself
			sortedFragments.erase(sortedFragments.begin(),it);
		}
	}//while sortedFragments isn't empty
   
   //Drop the TFragmentBranch from the Cache so we aren't still holding it in memory
   fCurrentFragTree->DropBranchFromCache(fCurrentFragTree->GetBranch("TFragment"),true);
   fCurrentFragTree->SetCacheSize(0);
   return;
}




void TAnalysisTreeBuilder::SetupFragmentTree() {
   ///Set up the fragment Tree to be sorted on time stamps or trigger Id's. This also reads the the run info out of the fragment tree.
   fCurrentFragFile = fCurrentFragTree->GetCurrentFile();
	if(fCurrentFragFile == NULL) {
		printf("Failed to get current fragment file\n");
		return;
	}

   std::string tmpRunInfoFileName = TGRSIRunInfo::Get()->GetRunInfoFileName();
   //Set the run info file to what is stored in the fragment tree
   fCurrentRunInfo  = (TGRSIRunInfo*)fCurrentFragFile->Get("TGRSIRunInfo");
	if(fCurrentRunInfo == NULL) {
		printf("Failed to get current run info\n");
	}

   //overwrite the relevent information using the loaded info file.
   //First check if there was a file
   if((tmpRunInfoFileName.length()<1)){
      //This does nothing
   }
   else{
      printf("Reading from Run info: %s\n",tmpRunInfoFileName.c_str());
      fCurrentRunInfo->ReadInfoFile(tmpRunInfoFileName.c_str());
   }
   if(fCurrentRunInfo) {
   //   TGRSIRunInfo::ReadInfoFromFile(fCurrentRunInfo);
      fCurrentRunInfo->Print("a");
   }

   fCurrentPPG = (TPPG*)fCurrentFragFile->Get("TPPG");
   if(fCurrentPPG){//We do this because not every run has PPG
      printf("Found PPG data\n");
      if(!fCurrentPPG->Correct())
         printf("Errors in PPG that could not be corrected\n");
   }
   //Intialize the TChannel Information
   InitChannels();

   //Set the branch to point at the Fragment Tree.
   TBranch *branch = fCurrentFragTree->GetBranch("TFragment");
   //Make the fCurrentFragPtr point at the Fragment Tree.
   branch->SetAddress(&fCurrentFragPtr);
   //Load the fragment tree into the MEM_SIZE (described in the header and above) into memory
   //fCurrentFragTree->LoadBaskets(MEM_SIZE);   
   printf(DRED "\t MEM_SIZE = %zd" RESET_COLOR  "\n", MEM_SIZE);
   return;
}

void TAnalysisTreeBuilder::SetupOutFile() {
   ///Sets up the anaysistree.root file to write the created events into
   if(!fCurrentRunInfo)
      return;
   std::string outfilename;
   if(fCurrentRunInfo->SubRunNumber() == -1)
     outfilename = Form("analysis%05i.root",fCurrentRunInfo->RunNumber());
   else
     outfilename = Form("analysis%05i_%03i.root",fCurrentRunInfo->RunNumber(),fCurrentRunInfo->SubRunNumber());
   //We add the output analysis file to the "input root files" in case we want to do something with that file after we finish 
   //sorting it.
   TGRSIOptions::AddInputRootFile(outfilename);
   if(fCurrentAnalysisFile)
      delete fCurrentAnalysisFile;
   fCurrentAnalysisFile = new TFile(outfilename.c_str(),"recreate");
   fCurrentAnalysisFile->SetCompressionSettings(1);
   printf("created output file: %s\n",fCurrentAnalysisFile->GetName());
   return;
}

void TAnalysisTreeBuilder::SetupAnalysisTree() { 
   ///Sets up the analysis tree by creating branches of the available detector systems. The available detector systems
   ///are set in the RunInfo of the fragment tree. When the analysis tree sorting begins, an output is created on screen
   ///that tells you which detector systems were found in the RunInfo.
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   //Create a new Analysis Tree in the analysistree.root file.
   fCurrentAnalysisFile->cd();
   if(fCurrentAnalysisTree)
      delete fCurrentAnalysisTree;
   fCurrentAnalysisTree = new TTree("AnalysisTree","AnalysisTree");

   //Load in the run info
   TGRSIRunInfo *info = fCurrentRunInfo;
   TTree *tree = fCurrentAnalysisTree;

   //Set new branches in the analysis tree if the run info in the fragment tree says the detectors are in the data stream
   int basketSize = 128000; //128000;
   if(info->Tigress())   { tree->Bronch("TTigress","TTigress",&tigress); }//, basketSize); } 
   if(info->Sharc())     { tree->Bronch("TSharc","TSharc",&sharc); }//, basketSize); } 
   if(info->TriFoil())   { tree->Bronch("TTriFoil","TTriFoil",&triFoil); }//, basketSize); } 
   if(info->RF())        { tree->Bronch("TRF","TRF",&rf); }//, basketSize); } 
   if(info->CSM())       { tree->Bronch("TCSM","TCSM",&csm); }//, basketSize); } 
   if(info->Spice())     { tree->Bronch("TSiLi","TSiLi",&sili); tree->Bronch("TS3","TS3",&s3); }//, basketSize); } 
   if(info->Tip())       { tree->Bronch("TTip","TTip",&tip); }//, basketSize); } 

   if(info->Griffin())   { tree->Bronch("TGriffin","TGriffin",&griffin, basketSize, 99);}// branch->SetAddress(0);} 
   if(info->Sceptar())   { tree->Bronch("TSceptar","TSceptar",&sceptar, basketSize, 99);}// branch->SetAddress(0);} 
   if(info->Paces())     { tree->Bronch("TPaces","TPaces",&paces, basketSize,99); }//, basketSize); } 
   //if(info->Dante())     { tree->Bronch("TDante","TDante",&dante); }//, basketSize); } 
   //if(info->ZeroDegree()){ tree->Bronch("TZeroDegree","TZeroDegree",&zerodegree); }//, basketSize); } 
   if(info->Descant())   { tree->Bronch("TDescant","TDescant",&descant); }//, basketSize);

   //tree->SetAutoFlush(-300000000);
   //tree->SetCacheSizeAux(true);

   //tree->SetAutoFlush(-500000000);
   //tree->SetCacheSize();
   //tree->AddBranchToCache("*",true);
   //tree->SetAutoSave(10000000);
   printf("created AnalysisTree\n");
   return;  
}

void TAnalysisTreeBuilder::ClearActiveAnalysisTreeBranches() {
   ///Clears the current analysis tree branches.
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;

   if(info->Tigress())   { tigress->Clear(); }
   if(info->Sharc())     { sharc->Clear(); }
   if(info->TriFoil())   { triFoil->Clear(); }
   if(info->RF())        { rf->Clear(); } 
   if(info->CSM())       { csm->Clear(); }
   if(info->Spice())     { sili->Clear(); s3->Clear(); } 
   if(info->Tip())       { tip->Clear(); } 
//printf("clearing griffin 0x08%x\n",griffin);
//griffin->Print();
   if(info->Griffin())   { griffin->Clear(); }
   if(info->Sceptar())   { sceptar->Clear(); }
   if(info->Paces())     { paces->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   if(info->Descant())   { descant->Clear();}
   //printf("ClearActiveAnalysisTreeBranches done\n");
}


void TAnalysisTreeBuilder::ResetActiveAnalysisTreeBranches() {
   ///Clears the current analysis tree branches.
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;
   TGRSIRunInfo *info = fCurrentRunInfo;

   if(info->Tigress())   { tigress = 0; }//->Clear(); }
   if(info->Sharc())     { sharc = 0; }//->Clear(); }
   if(info->TriFoil())   { triFoil = 0; }//->Clear(); }
   if(info->RF())        { rf = 0;  } 
   if(info->CSM())       { csm = 0; }//->Clear(); }
   if(info->Spice())     { sili = 0; s3 = 0; } 
   if(info->Tip())       { tip = 0; } 
//printf("clearing griffin 0x08%x\n",griffin);
//griffin->Print();
   if(info->Griffin())   { griffin = 0; }//->Clear(); }
   if(info->Sceptar())   { sceptar = 0; }//->Clear(); }
   if(info->Paces())     { paces = 0; }//->Clear(); } 
   //if(info->Dante())     { dante->Clear(); } 
   //if(info->ZeroDegree()){ zerodegree->Clear(); } 
   if(info->Descant())   { descant = 0; }//->Clear();}
   //printf("ClearActiveAnalysisTreeBranches done\n");
}

void TAnalysisTreeBuilder::BuildActiveAnalysisTreeBranches(std::map<std::string, TDetector*> *detectors) {
   ///Build the hits in each of the detectors.
   if(!fCurrentAnalysisFile || !fCurrentRunInfo)
      return;

   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      det->second->BuildHits();
   }
}

void TAnalysisTreeBuilder::FillWriteQueue(std::map<std::string, TDetector*> *detectors) {
   ///Fill the write Q with the built hits in each of the detectors.
   fAnalysisIn++;
   TWriteQueue::Add(detectors);
}

void TAnalysisTreeBuilder::WriteAnalysisTree() {
   ///Write the analysis tree to the root file. We do this by filling the analysis tree with each
   ///of the built events in the write Q.
   while(TWriteQueue::Size() > 0 || TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TWriteQueue::Size() == 0) {
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         continue;
      }
      std::map<std::string, TDetector*> *detectors = TWriteQueue::PopEntry();
      fAnalysisOut++;

      FillAnalysisTree(detectors);
   }
}

void TAnalysisTreeBuilder::FillAnalysisTree(std::map<std::string, TDetector*> *detectors) {
   ///Fill the analysis Tree with the built events. Each detector gets its own branch in the analysis tree
   if(!fCurrentAnalysisTree || !detectors) {
      printf("returned from fill without filling (%p %p)!\n",(void*) fCurrentAnalysisTree, (void*) detectors);   
      return;
   }   
   
   // clear branches
   //ClearActiveAnalysisTreeBranches();	
   ResetActiveAnalysisTreeBranches();	
   

   //Fill the detector map with TDetector classes if the mnemonic of the detector is in the map.
   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      if(det->first.compare(0,2,"TI") == 0) {
         tigress = (TTigress*) det->second;
      } else if(det->first.compare(0,2,"SH") == 0) {
         sharc = (TSharc*) det->second;
      } else if(det->first.compare(0,2,"TR") == 0) {
         triFoil = (TTriFoil*) det->second;
      } else if(det->first.compare(0,2,"RF") == 0) {
         rf = (TRF*) det->second;
      } else if(det->first.compare(0,2,"CS") == 0) {
         csm = (TCSM*) det->second;
      } else if(det->first.compare(0,3,"SPI") == 0) {
         sili = (TSiLi*) det->second;
      } else if(det->first.compare(0,3,"SPE") == 0) {
         s3   = (TS3*) det->second;
      } else if(det->first.compare(0,2,"GR") == 0) {
         griffin = (TGriffin*) det->second;
      } else if(det->first.compare(0,2,"SE") == 0) {
         sceptar = (TSceptar*) det->second;
      } else if(det->first.compare(0,2,"DS") == 0) {
         descant = (TDescant*) det->second;
      } else if(det->first.compare(0,2,"PA") == 0) {
         paces = (TPaces*) det->second;
      } else if(det->first.compare(0,2,"TP") == 0) {
         tip = (TTip*) det->second;
      } 
   }
   fCurrentAnalysisTree->Fill();
   
   //ClearActiveAnalysisTreeBranches();	
   //Zero the detectors in the detector map
   for(auto det = detectors->begin(); det != detectors->end(); det++) {
      delete det->second;
      det->second = 0;
   }
   delete detectors;
   detectors = 0;

   return;
}

void TAnalysisTreeBuilder::CloseAnalysisFile() {
   ///Safely close the analysis file.
   if(!fCurrentAnalysisFile)
      return;

   ///******************************////
   ///******************************////
   // this will be removed and put into a seperate thread later.

   printf("Writing file %s\n",fCurrentAnalysisFile->GetName());
   ///******************************////
   ///******************************////

   if(TChannel::GetNumberOfChannels()>0) {
     TChannel *c = TChannel::GetDefaultChannel();
     c->Write();
   }
   fCurrentRunInfo->Write();
   if(fCurrentPPG){
      printf("Writing PPG Data\n");
      fCurrentPPG->Write("TPPG",TObject::kSingleKey);
   }
   //TChannel::DeleteAllChannels();

   fCurrentAnalysisFile->cd();
   if(fCurrentAnalysisTree)
      fCurrentAnalysisTree->Write();
   fCurrentAnalysisFile->Close();

   fCurrentAnalysisTree = 0;
   fCurrentAnalysisFile = 0;
   return;
}


void TAnalysisTreeBuilder::ProcessEvent() {
   ///Process the event. We do this by filling a map of the detectors, and put the hits into the appropriate detector class.
   ///This is important because different detector systems will need different experimental data/conditions to be set.
   while(TEventQueue::Size() > 0 || !fSortFragmentDone) {
      if(TEventQueue::Size() == 0) { 
         std::this_thread::sleep_for(std::chrono::milliseconds(100));
         continue; 
      }
      
      //We need to pull the event out of the Event Q
      std::vector<TFragment> *event = TEventQueue::PopEntry();
      MNEMONIC mnemonic;
      std::map<std::string, TDetector*> *detectors = new std::map<std::string, TDetector*>;
      for(size_t i=0;i<event->size();i++) {
      
         TChannel *channel = TChannel::GetChannel(event->at(i).ChannelAddress);
         if(!channel)
            continue;
         ClearMNEMONIC(&mnemonic);
         //if(strlen(channel->GetChannelName())>0)
         //  printf("chnnel->GetChannelName() = %s\n",channel->GetChannelName());
         ParseMNEMONIC(channel->GetChannelName(),&mnemonic);
         
         //We use the MNEMONIC in order to figure out what detector we want to put the set of fragments into
         if(mnemonic.system.compare("TI")==0) {
            if(detectors->find("TI") == detectors->end()) {
					(*detectors)["TI"] = new TTigress;
            }
            (*detectors)["TI"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("SH")==0) {
            if(detectors->find("SH") == detectors->end()) {
               (*detectors)["SH"] = new TSharc;
            }
            (*detectors)["SH"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("Tr")==0) {	
            if(detectors->find("Tr") == detectors->end()) {
               (*detectors)["Tr"] = new TTriFoil;
            }
            (*detectors)["Tr"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("RF")==0) {	
            if(detectors->find("RF") == detectors->end()) {
               (*detectors)["RF"] = new TRF;
            }
            (*detectors)["RF"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("SP")==0) {
            if(mnemonic.subsystem.compare("I")==0) {
               if(detectors->find("SPI") == detectors->end()) {
                  (*detectors)["SPI"] = new TSiLi;
               }
               (*detectors)["SPI"]->AddFragment(&(event->at(i)), &mnemonic);
            } else {
               if(detectors->find("SPE") == detectors->end()) {
                  (*detectors)["SPE"] = new TS3;
               }
               (*detectors)["SPE"]->AddFragment(&(event->at(i)), &mnemonic);
            }
         } else if(mnemonic.system.compare("CS")==0) {	
            if(detectors->find("CS") == detectors->end()) {
               (*detectors)["CS"] = new TCSM;
            }
            (*detectors)["CS"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("GR")==0) {
            if(detectors->find("GR") == detectors->end()) {
               (*detectors)["GR"] = new TGriffin;
            }
            (*detectors)["GR"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("SE")==0) {
            if(detectors->find("SE") == detectors->end()) {
               (*detectors)["SE"] = new TSceptar;
            }
            (*detectors)["SE"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("PA")==0) {	
            if(detectors->find("PA") == detectors->end()) {
               (*detectors)["PA"] = new TPaces;
            }
            (*detectors)["PA"]->AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("DS")==0) {	
            if(detectors->find("DS") == detectors->end()) {
               (*detectors)["DS"] = new TDescant;
            }
            (*detectors)["DS"]->AddFragment(&(event->at(i)), &mnemonic);
         //} else if(mnemonic.system.compare("DA")==0) {	
         //	AddFragment(&(event->at(i)), &mnemonic);
         //} else if(mnemonic.system.compare("ZD")==0) {	
         //	AddFragment(&(event->at(i)), &mnemonic);
         } else if(mnemonic.system.compare("TP")==0) {	
            if(detectors->find("TP") == detectors->end()) {
               (*detectors)["TP"] = new TTip;
            }
            (*detectors)["TP"]->AddFragment(&(event->at(i)), &mnemonic);
         }
      }

      if(!detectors->empty()) {
         BuildActiveAnalysisTreeBranches(detectors);
         FillWriteQueue(detectors);
      }
      delete event;
   }
}

void TAnalysisTreeBuilder::Print(Option_t *opt) const {
   ///Prints information about the Q's and threads in the AnalysisTreeBuilding process
   if(fCurrentFragFile && fCurrentAnalysisFile)
      printf(DMAGENTA " %s/%s" RESET_COLOR  "\n",fCurrentFragFile->GetName(),fCurrentAnalysisFile->GetName());
   printf(DMAGENTA " fSortFragmentDone         = %s" RESET_COLOR "\n",fSortFragmentDone ? "true":"false");
   printf(DYELLOW  " TEventQueue::Size()       = %i" RESET_COLOR "\n",TEventQueue::Size());
   printf(DBLUE    " TWriteQueue::Size()       = %i" RESET_COLOR "\n",TWriteQueue::Size());
   printf(DGREEN   " fFragmentsIn/fAnalysisOut = %i / %i" RESET_COLOR "\n",fFragmentsIn,fAnalysisOut);  
   printf(GREEN    " std::thread::hardware_concurrency = %u" RESET_COLOR "\n",std::thread::hardware_concurrency());
   printf(DMAGENTA " ==========================================" RESET_COLOR "\n");
   return;
}

void TAnalysisTreeBuilder::Status() {
   ///Prints the current status for each of the threads in the TAnalysisTree Building process
   TStopwatch w;
   w.Start();
   bool fragmentsDone = false;
   bool sortingDone = false;
   while(fPrintStatus) {
//      printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
//             "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%.1f seconds." SHOW_CURSOR "\r",
//             fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());
      if(!sortingDone) {
         printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
					 (100.*fFragmentsIn)/fEntries, fAnalysisIn, fAnalysisOut, fAnalysisIn > 0 ? (100.*fAnalysisOut)/fAnalysisIn:0., fAnalysisOut/w.RealTime(), w.RealTime());
      } else {
         if(fAnalysisOut > 0) {
            printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                   DGREEN "   write speed: %9.1f built events/second." RESET_COLOR "  %3.1f seconds, %.1f seconds remaining." SHOW_CURSOR "\r",
                  (100.*fFragmentsIn)/fEntries, fAnalysisIn, fAnalysisOut, (100.*fAnalysisOut)/fAnalysisIn, fAnalysisOut/w.RealTime(), w.RealTime(), ((double)(fAnalysisIn-fAnalysisOut))/fAnalysisOut*w.RealTime());
         } else {
            printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
                   DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
                  (100.*fFragmentsIn)/fEntries, fAnalysisIn, fAnalysisOut, (100.*fAnalysisOut)/fAnalysisIn, fAnalysisOut/w.RealTime(), w.RealTime());
         }
      }
      //we insert a newline (thus preserving the last status), if we just finished getting all fragment, or finished removing fragments from the event queue
      if(!fragmentsDone && fFragmentsIn == fEntries && fEntries != 0) {
         printf("\n");
         fragmentsDone = true;
      }
      if(fragmentsDone && !sortingDone && TEventQueue::Size() == 0) {
         printf("\n");
         sortingDone = true;
      }
      w.Continue(); 
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
//   printf(DYELLOW HIDE_CURSOR "%12i / %12ld " RESET_COLOR "/" DBLUE " %12i " RESET_COLOR "/" DCYAN " %12i " RESET_COLOR "/" DRED " %12i " RESET_COLOR "/" DGREEN " %12i " RESET_COLOR
//          "    processed fragments / # of fragments/ # of events / event queue size / write queue size / events written.\t%.1f seconds." SHOW_CURSOR "\n",
//          fFragmentsIn, fEntries, fAnalysisIn, TEventQueue::Size(), TWriteQueue::Size(), fAnalysisOut, w.RealTime());
   if(fAnalysisOut > 0) {
      printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
             DGREEN "   write speed: %9.1f built events/second." RESET_COLOR "  %.1f seconds, %.1f seconds remaining." SHOW_CURSOR "\r",
            (100.*fFragmentsIn)/fEntries, fAnalysisIn, fAnalysisOut, (100.*fAnalysisOut)/fAnalysisIn, fAnalysisOut/w.RealTime(), w.RealTime(), ((double)(fAnalysisIn-fAnalysisOut))/fAnalysisOut*w.RealTime());
   } else {
      printf(DYELLOW HIDE_CURSOR "Fragments: %.1f %%," DBLUE "   %9i built events," DRED "   written: %9i = %.1f %%," 
             DGREEN "   write speed: %9.1f built events/second." RESET_COLOR " %3.1f seconds." SHOW_CURSOR "\r",
            (100.*fFragmentsIn)/fEntries, fAnalysisIn, fAnalysisOut, (100.*fAnalysisOut)/fAnalysisIn, fAnalysisOut/w.RealTime(), w.RealTime());
   }

   return;
}


