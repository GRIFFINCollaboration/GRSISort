void DroppedData(TFile *file) {
   //Finds the number of dropped Midas events as well as the number of lost events over the network.
   TTree* tree = (TTree*) file->Get("FragmentTree");

   long entries = tree->GetEntries();
   double packetRatio = ((double)entries)/((double)(tree->GetMaximum("NetworkPacketNumber") - tree->GetMinimum("NetworkPacketNumber")));
//   double filterRatio = ((double)entries)/((double)(tree->GetMaximum("MidasId") - tree->GetMinimum("MidasId")));

   double packetLost = tree->GetMaximum("NetworkPacketNumber") - tree->GetMinimum("NetworkPacketNumber") - entries;
//   double filterLost = tree->GetMaximum("MidasId") - tree->GetMinimum("MidasId") - entries;

   std::cout<<"Got "<<packetRatio*100.<<" % of the network packets (lost "<<packetLost<<" packets"<<std::endl;
//   std::cout<<"Got "<<packetRatio*100.<<" % of the network packets and "<<filterRatio*100.<<" % of the master filter IDs (lost "<<packetLost<<" packets and "<<filterLost<<" IDs)"<<std::endl;
}
