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

TH1F* GainMatchPlot(TH2* mat, TH2* mat2, Double_t energy1, Double_t energy2){
	TH2* mat_copy = static_cast<TH2*>(mat->Clone());
	mat_copy->Add(mat2);
	return GainMatchPlot(mat_copy,energy1,energy2);
}

