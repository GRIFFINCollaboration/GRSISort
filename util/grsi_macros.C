void InitChannels(TTree* tree = static_cast<TTree*>(gROOT->FindObjectAny("FragmentTree")))
{
   if(tree == nullptr) { return; }
   TList*    list = tree->GetUserInfo();
   TIter*    iter = new TIter(list);
   TChannel* chan = 0;
   while(chan = static_cast<TChannel*>(iter->Next())) {
      TChannel* cur_chan = TChannel::GetChannel(chan->GetAddress());
      TChannel::CopyChannel(cur_chan, chan);
   }
}
