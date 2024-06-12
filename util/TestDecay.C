TH1F* TestSingleDecay(TSingleDecay* decay, Int_t nPoints = 10000)
{
   TH1F* decay_curve = (TH1F*)(gROOT->Get("decay_curve"));
   TH1F* tmp_curve   = (TH1F*)(gROOT->Get("tmp_curve"));
   if(decay_curve) delete decay_curve;
   if(tmp_curve) delete tmp_curve;
   decay_curve = new TH1F("decay_curve", "", 100, 0, 10);
   decay_curve->FillRandom(decay->GetTotalDecayFunc()->GetName(), nPoints);
   decay_curve->Draw();

   return (TH1F*)(decay_curve->Clone("tmp_curve"));
}

TDecayChain* TestDecayChain()
{
   TDecayChain* chain = new TDecayChain(2);
   chain->GetDecay(0)->SetHalfLife(1);
   chain->GetDecay(0)->SetIntensity(100);
   //  chain->GetDecay(0)->SetEfficiency(0.0);
   chain->GetDecay(1)->SetHalfLife(5);
   chain->SetRange(0, 100);
   // chain->GetDecay(2)->SetHalfLife();

   //  chain->Draw();
   //  chain->DrawComponents("same");
   // chain->GetDecay(2)->Draw("same");

   //  TCanvas* c1 = new TCanvas;
   //  chain->GetDecay(1)->Draw();

   return chain;
}

TH1F* TestChainFit(Int_t nPoints = 10000)
{
   TDecayChain* chain       = TestDecayChain();
   TH1F*        decay_curve = (TH1F*)(gROOT->Get("decay_curve"));
   TH1F*        tmp_curve   = (TH1F*)(gROOT->Get("tmp_curve"));
   if(decay_curve) delete decay_curve;
   if(tmp_curve) delete tmp_curve;
   decay_curve = new TH1F("decay_curve", "", 1000, 0, 100);
   decay_curve->FillRandom(chain->GetChainFunc()->GetName(), nPoints);
   decay_curve->Draw();

   // chain->Fit(decay_curve);
   return (TH1F*)(decay_curve->Clone("tmp_curve"));
}

void TestDraw(Int_t generation)
{
   TSingleDecay* dec1     = new TSingleDecay;
   Int_t         halflife = 1;
   dec1->SetIntensity(100);
   dec1->SetHalfLife(halflife++);
   std::vector<TSingleDecay*> declist;
   declist.push_back(dec1);
   for(int i = 1; i < generation; i++) {
      TSingleDecay* curDec = new TSingleDecay(dec1);
      curDec->SetHalfLife(halflife++);
      dec1 = curDec;
      declist.push_back(curDec);
   }
   declist.at(generation - 1)->Draw();
}

TDecay* TestDecay()
{
   TDecayChain* chain  = new TDecayChain(2);
   TDecayChain* chain2 = new TDecayChain(1);

   chain->GetDecay(0)->SetHalfLife(8);
   chain->GetDecay(1)->SetHalfLife(6);
   chain2->GetDecay(0)->SetHalfLife(1);

   chain->GetDecay(0)->SetName("name1");
   chain->GetDecay(1)->SetName("name2");
   chain2->GetDecay(0)->SetName("name3");

   chain->GetDecay(0)->SetIntensity(700);
   chain2->GetDecay(0)->SetIntensity(200);

   chain2->GetDecay(0)->SetDecayId(12);
   chain->GetDecay(1)->SetDecayId(3);

   //  chain2->GetDecay(0)->SetDecayId(1);

   std::vector<TDecayChain*> list;
   list.push_back(chain);
   list.push_back(chain2);
   TDecay* mydecay = new TDecay(list);

   //  decay->SetHalfLife(12,5);
   mydecay->SetBackground(40);
   mydecay->SetRange(0, 100);

   TFile f("testdecay.root", "recreate");
   mydecay->SetName("mydecay");
   mydecay->Write();
   f.Close();

   return mydecay;
}

TDecay* GenDecay()
{
   TDecayChain* chain  = new TDecayChain(3);
   TDecayChain* chain2 = new TDecayChain(1);

   chain->GetDecay(0)->SetHalfLife(6);
   chain->GetDecay(0)->SetDecayId(kBlack);
   chain->GetDecay(1)->SetHalfLife(2);
   chain->GetDecay(1)->SetDecayId(kBlue);
   chain->GetDecay(2)->SetDecayId(kGreen);
   chain->GetDecay(2)->SetHalfLife(10);
   //   chain->GetDecay(0)->SetDecayRate(log(2)/5);
   //   chain2->GetDecay(0)->SetDecayRate(log(2)/1);

   // chain->GetDecay(1)->SetHalfLife();
   chain2->GetDecay(0)->SetDecayId(kBlue);
   //  chain2->GetDecay(0)->SetHalfLife(7);

   chain->GetDecay(0)->SetIntensity(100);
   chain2->GetDecay(0)->SetIntensity(20);
   //   chain2->GetDecay(0)->SetIntensity(10);

   //  chain2->GetDecay(0)->SetDecayId(12);
   //  chain->GetDecay(1)->SetDecayId(12);
   // chain2->GetDecay(0)->SetDecayId(1);

   std::vector<TDecayChain*> list;
   list.push_back(chain);
   list.push_back(chain2);
   TDecay* mydecay = new TDecay(list);
   //   decay->SetHalfLifeLimits(1,4,6);

   // decay->SetHalfLife(12,10);
   mydecay->SetBackground(10);
   mydecay->SetRange(0, 100);
   mydecay->Print();

   return mydecay;
}

TH1F* TestFullFit(Int_t nPoints = 100000)
{
   TDecay* mydecay     = GenDecay();
   TH1F*   decay_curve = (TH1F*)(gROOT->Get("decay_curve"));
   TH1F*   tmp_curve   = (TH1F*)(gROOT->Get("tmp_curve"));
   if(decay_curve) delete decay_curve;
   if(tmp_curve) delete tmp_curve;
   decay_curve = new TH1F("decay_curve", "decay_curve", 1000, 0, 100);
   // for(int i =0;i<nPoints;++i){
   //  decay_curve->Fill(decay->GetFitFunc()->GetRandom());
   //  }
   decay_curve->FillRandom(mydecay->GetFitFunc()->GetName(), nPoints);
   //  decay_curve->FillRandom("tmpfit2",nPoints);
   decay_curve->Draw();
   printf("creating fitter\n");
   mydecay->SetRange(0, 99);
   mydecay->Print();
   mydecay->GetChain(0)->GetDecay(0)->SetIntensity(400);
   mydecay->GetChain(1)->GetDecay(0)->SetIntensity(20);
   //  decay->SetParameters();
   decay_curve->Draw();
   //  TLMFitter *fitter = new TLMFitter;
   //  fitter->Fit(decay_curve,decay->GetFunc());
   // delete fitter;
   mydecay->Fit(decay_curve, "V");
   return (TH1F*)(decay_curve->Clone("tmp_curve"));
   // return decay_curve;
}

TDecay* TestFileDecay(TFile* file)
{
   TDecayChain* chain = new TDecayChain(1);
   chain->GetDecay(0)->SetHalfLife(19.1);
   chain->GetDecay(0)->SetIntensity(1000000);
   std::vector<TDecayChain*> vec;
   vec.push_back(chain);
   TDecay* decay = new TDecay(vec);
   decay->SetBackground(50);
   decay->SetRange(0, 499);

   TH1F* decay_curve = (TH1F*)(file->Get("h1"));

   //   TLMFitter *fitter = new TLMFitter;
   //   fitter->Fit(decay_curve,decay->GetFunc());
   decay->Fit(decay_curve);
   return decay;
}
