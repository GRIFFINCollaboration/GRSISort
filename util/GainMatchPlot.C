TH1F* GainMatchPlot(TH2* mat, Double_t energy1, Double_t energy2){
   TCanvas *c = new TCanvas;
   c->Divide(1,2);
   c->cd(1);
   mat->GetYaxis()->SetRangeUser(energy1-20.,energy1+20.);
   mat->DrawCopy("colz");
   c->cd(2);
   mat->GetYaxis()->SetRangeUser(energy2-20.,energy2+20.);
   mat->DrawCopy("colz");
   mat->GetYaxis()->UnZoom();
}
