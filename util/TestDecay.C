TH1F* TestDecay(TDecay *decay,Int_t nPoints=10000){
   TH1F* decay_curve = (TH1F*)(gROOT->Get("decay_curve"));
   TH1F* tmp_curve = (TH1F*)(gROOT->Get("tmp_curve"));
   if(decay_curve) delete decay_curve;
   if(tmp_curve) delete tmp_curve;
   decay_curve = new TH1F("decay_curve","",100,0,10);
   decay_curve->FillRandom(decay->GetTotalDecayFunc()->GetName(),nPoints);
   decay_curve->Draw();

   return (TH1F*)(decay_curve->Clone("tmp_curve"));


}

void TestDecayChain(){
   TDecayChain *chain = new TDecayChain(2);
   chain->GetDecay(0)->SetHalfLife(1);
   chain->GetDecay(0)->SetIntensity(100);
 //  chain->GetDecay(0)->SetEfficiency(0.0);
   chain->GetDecay(1)->SetHalfLife(5);
   //chain->GetDecay(2)->SetHalfLife();

   chain->Draw();
   chain->DrawComponents("same");
  // chain->GetDecay(2)->Draw("same");


   TCanvas* c1 = new TCanvas;
   chain->GetDecay(1)->Draw();
   

}

void TestDraw(Int_t generation){
   TDecay *dec1 = new TDecay;
   Int_t halflife = 1;
   dec1->SetIntensity(100);
   dec1->SetHalfLife(halflife++);
   std::vector<TDecay*> declist;
   declist.push_back(dec1);
   for(int i=1; i< generation; i++){
      TDecay *curDec = new TDecay(dec1);
      curDec->SetHalfLife(halflife++);
      dec1 = curDec;
      declist.push_back(curDec);
   }
   declist.at(generation-1)->Draw();
}


