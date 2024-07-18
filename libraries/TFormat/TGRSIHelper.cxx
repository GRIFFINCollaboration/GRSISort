#include "TGRSIHelper.h"
#include "RVersion.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 14, 0)

TGRSIHelper::TGRSIHelper(TList* input)
{
   fPpg          = static_cast<TPPG*>(input->FindObject("TPPG"));
   fRunInfo      = static_cast<TRunInfo*>(input->FindObject("TRunInfo"));
   fUserSettings = static_cast<TUserSettings*>(input->FindObject("UserSettings"));
   // get the analysis options from the input list and assign them to our local analysis options
   // (this might not be needed anymore since the workers aren't started as separate processes but threads)
   *(TGRSIOptions::AnalysisOptions()) = *static_cast<TAnalysisOptions*>(input->FindObject("TAnalysisOptions"));
   // check that we have a parser library
   if(TGRSIOptions::Get()->ParserLibrary().empty()) {
      std::stringstream str;
      str << DRED << "No parser library set!" << RESET_COLOR << std::endl;
      throw std::runtime_error(str.str());
   }
   // loop over all cal-files, val-files, and cut-files we might have (these are full paths!)
   for(int i = 0; input->FindObject(Form("calFile%d", i)) != nullptr; ++i) {
      const char* fileName = static_cast<TNamed*>(input->FindObject(Form("calFile%d", i)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      TChannel::ReadCalFile(fileName);
   }
   for(int i = 0; input->FindObject(Form("valFile%d", i)) != nullptr; ++i) {
      const char* fileName = static_cast<TNamed*>(input->FindObject(Form("valFile%d", i)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      GValue::ReadValFile(fileName);
   }
   for(int i = 0; input->FindObject(Form("cutFile%d", i)) != nullptr; ++i) {
      std::cout << "trying to open " << Form("cutFile%d", i) << std::flush << " = " << input->FindObject(Form("cutFile%d", i)) << std::flush << " with title " << static_cast<TNamed*>(input->FindObject(Form("cutFile%d", i)))->GetTitle() << std::endl;
      const char* fileName = static_cast<TNamed*>(input->FindObject(Form("cutFile%d", i)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      // if we have a relative path and a working directory, combine them
      auto* file = new TFile(fileName);
      if(file != nullptr && file->IsOpen()) {
         TIter iter(file->GetListOfKeys());
         TKey* key = nullptr;
         while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
            if(strcmp(key->GetClassName(), "TCutG") != 0) {
               continue;
            }
            auto* tmpCut = static_cast<TCutG*>(key->ReadObj());
            if(tmpCut != nullptr) {
               fCuts[tmpCut->GetName()] = tmpCut;
            }
         }
      } else {
         std::cout << "Error, failed to open file " << fileName << "!" << std::endl;
         break;
      }
   }
   for(auto& cut : fCuts) {
      std::cout << cut.first << " = " << cut.second << std::endl;
   }

   if(GValue::Size() == 0) {
      std::cout << "No g-values!" << std::endl;
   } else {
      std::cout << GValue::Size() << " g-values" << std::endl;
   }
}

void TGRSIHelper::Setup()
{
   const auto nSlots = ROOT::IsImplicitMTEnabled() ? TGRSIOptions::Get()->GetMaxWorkers() : 1;
   TH1::AddDirectory(false);   // turns off warnings about multiple histograms with the same name because ROOT doesn't manage them anymore
   for(auto i : ROOT::TSeqU(nSlots)) {
      fLists.emplace_back(std::make_shared<std::map<std::string, TList>>());
      fH1.emplace_back(TGRSIMap<std::string, TH1*>());
      fH2.emplace_back(TGRSIMap<std::string, TH2*>());
      fH3.emplace_back(TGRSIMap<std::string, TH3*>());
      fSym.emplace_back(TGRSIMap<std::string, GHSym*>());
      fCube.emplace_back(TGRSIMap<std::string, GCube*>());
      fTree.emplace_back(TGRSIMap<std::string, TTree*>());
      fObject.emplace_back(TGRSIMap<std::string, TObject*>());
      CreateHistograms(i);
      for(auto& it : fH1[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      for(auto& it : fH2[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      for(auto& it : fH3[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      for(auto& it : fSym[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      for(auto& it : fCube[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      for(auto& it : fTree[i]) {
         // trees can only be written into the root-directory due to the way they get merge in Finalize (for now?)
         (*fLists[i])[""].Add(it.second);
      }
      for(auto& it : fObject[i]) {
         // if the key/name of the histogram does not contain a forward slash we put it in the root-directory
         if(it.first.find_last_of('/') == std::string::npos) {
            (*fLists[i])[""].Add(it.second);
         } else {
            // extract the path from the key/name
            auto lastSlash = it.first.find_last_of('/');
            (*fLists[i])[it.first.substr(0, lastSlash)].Add(it.second);
         }
      }
      CheckSizes(i, "use");
   }
   TH1::AddDirectory(true);   // restores old behaviour
}

void TGRSIHelper::Finalize()
{
   /// This function merges all maps of lists into the map of the first slot (slot 0)
   CheckSizes(0, "write");
   // get all objects from the first slot
   auto& res = fLists[0];
   // map to keep track of trees we found
   std::map<TTree*, TList*> treeList;
   // loop over all other slots
   for(auto slot : ROOT::TSeqU(1, fLists.size())) {
      CheckSizes(slot, "write");
      // loop over each TList in the map we merge into
      for(const auto& list : *res) {
         // loop over each object in the list
         for(const auto&& obj : list.second) {
            // check if object exists in the slot's list
            if((*fLists[slot]).at(list.first).FindObject(obj->GetName()) != nullptr) {
               // check object type and merge correspondingly
               if(obj->InheritsFrom(TH1::Class())) {
                  // histograms can just be added together
                  static_cast<TH1*>(obj)->Add(static_cast<TH1*>((*fLists[slot]).at(list.first).FindObject(obj->GetName())));
               } else if(obj->InheritsFrom(TTree::Class())) {
                  // trees are added to the list and merged later
                  auto* tree = static_cast<TTree*>(obj);
                  // std::cout<<slot<<" copied "<<tree->CopyEntries(static_cast<TTree*>((*fLists[slot]).at(list.first).FindObject(obj->GetName())))<<" bytes to tree "<<tree->GetName()<<std::endl;
                  treeList.emplace(tree, new TList);   // emplace does not overwrite existing elements!
                  treeList.at(tree)->Add((*fLists[slot]).at(list.first).FindObject(obj->GetName()));
                  std::cout << slot << ": adding " << treeList.at(tree)->GetSize() << ". " << tree->GetName() << " tree with " << static_cast<TTree*>((*fLists[slot]).at(list.first).FindObject(obj->GetName()))->GetEntries() << " entries" << std::endl;
               } else {
                  std::cerr << "Object '" << obj->GetName() << "' is not a histogram (" << obj->ClassName() << "), don't know what to do with it!" << std::endl;
               }
            } else {
               // only warn about not finding the object in other lists for histograms and trees
               if(obj->InheritsFrom(TH1::Class()) || obj->InheritsFrom(TTree::Class())) {
                  std::cerr << "Failed to find object '" << obj->GetName() << "' in " << slot << ". list" << std::endl;
               }
            }
         }
      }
   }
   // merge the trees in the list (if there are any)
   for(auto& tree : treeList) {
      tree.second->Add(tree.first);
      Long64_t entries = 0;
      int      i       = 0;
      for(const auto&& obj : *tree.second) {
         std::cout << ++i << ". " << tree.first->GetName() << " tree: " << static_cast<TTree*>(obj)->GetEntries() << " entries" << std::endl;
         entries += static_cast<TTree*>(obj)->GetEntries();
      }
      std::cout << "total of " << entries << " entries" << std::endl;
      auto* newTree = TTree::MergeTrees(tree.second);
      std::cout << "Got new tree with " << newTree->GetEntries() << " => " << entries - newTree->GetEntries() << " less than total" << std::endl;
      (*res).at("").Remove(tree.first);
      (*res).at("").Add(newTree);
      //	std::cout<<"Adding "<<tree.second->GetSize()<<" "<<tree.first->GetName()<<" trees to "<<tree.first->GetEntries()<<" entries"<<std::endl;
      //	tree.first->Merge(tree.second);
      //	std::cout<<"Got "<<tree.first->GetEntries()<<" entries"<<std::endl;
   }
   EndOfSort(res);
}

void TGRSIHelper::CheckSizes(unsigned int slot, const char* usage)
{
   /// check size of each object in the output list
   // loop over each TList in the map
   for(auto& list : *fLists[slot]) {
      // loop over each object in the list
      for(const auto&& obj : list.second) {
         TBufferFile buf(TBuffer::kWrite, 10000);
         obj->IsA()->WriteBuffer(buf, obj);
         if(buf.Length() > fSizeLimit) {
            std::stringstream str;
            str << DRED << slot << ". slot: " << obj->ClassName() << " '" << obj->GetName() << "' too large to " << usage << ": " << buf.Length() << " bytes = " << buf.Length() / 1024. / 1024. / 1024. << " GB, removing it!" << RESET_COLOR << std::endl;
            std::cout << str.str();
            // we only remove it from the output list, not deleting the object itself
            // this way the filling of that histogram will still work, it just won't get written to file
            // we remove it from all lists though, not just the first one
            list.second.Remove(obj);
         }
      }
   }
}
#endif
