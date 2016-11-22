
void InitChannels(TTree *tree = (TTree*)gROOT->FindObjectAny("FragmentTree")) {
   if(!tree)
      return;
   TList *list = tree->GetUserInfo();
   TIter *iter = new TIter(list);
   TChannel *chan = 0;
   while(chan = (TChannel*)iter->Next()) {
      TChannel *cur_chan = TChannel::GetChannel(chan->GetAddress());
      TChannel::CopyChannel(cur_chan,chan);
   }

}

