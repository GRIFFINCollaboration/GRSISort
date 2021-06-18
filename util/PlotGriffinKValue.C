void PlotGriffinKValue(const char* filename, double eLow = 0., double eHigh = 0.)
{
	TFile* file = new TFile(filename);

	gStyle->SetOptTitle(0);
	gStyle->SetOptStat(0);

	TCanvas* canv1 = new TCanvas;

	canv1->SetLogz();
	static_cast<TH2D*>(file->Get("hKP"))->Draw("colz");

	TCanvas* canv2 = new TCanvas;

	canv2->SetLogy();
	TH1D* hE = static_cast<TH1D*>(file->Get("hE"));
	hE->GetXaxis()->CenterTitle();
	hE->GetYaxis()->CenterTitle();
	hE->GetXaxis()->SetRangeUser(1., hE->GetYaxis()->GetBinUpEdge(hE->GetYaxis()->GetLast()));
	hE->SetLineColor(kBlack);
	if(eLow < eHigh) {
		hE->GetXaxis()->SetRangeUser(eLow, eHigh);
	}
	hE->Draw();

	TH1D* hE_noPU = static_cast<TH1D*>(file->Get("hE_noPU"));
	hE_noPU->SetLineColor(kBlue);
	hE_noPU->Draw("same");

	TH1D* hE_2h = static_cast<TH1D*>(file->Get("hE_2h"));
	hE_2h->SetLineColor(kRed);
	hE_2h->Draw("same");

	TH1D* hE_3h = static_cast<TH1D*>(file->Get("hE_3h"));
	hE_3h->SetLineColor(kOrange);
	hE_3h->Draw("same");

	TH1D* hE_2hc = static_cast<TH1D*>(file->Get("hE_2hc"));
	hE_2hc->SetLineColor(kBlack);
	hE_2hc->Draw("same");

	hE->SetTitle("all hits");
	hE_noPU->SetTitle("no pile-up");
	hE_2h->SetTitle("two piled-up hits");
	hE_3h->SetTitle("three piled-up hits");
	hE_2hc->SetTitle("two piled-up hits with two integrations");
	canv2->BuildLegend();

	TCanvas* canv3 = new TCanvas("canv3", "Energy vs k-value plots", 700, 2000);

	canv3->Divide(2,4);
	canv3->SetLogz();

	canv3->cd(1);
	TH2D* hEK_1h2 = static_cast<TH2D*>(file->Get("hEK_1h2"));
	if(eLow < eHigh) {
		hEK_1h2->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_1h2->Draw("colz");

	canv3->cd(2);
	TH2D* hEK_2h2 = static_cast<TH2D*>(file->Get("hEK_2h2"));
	if(eLow < eHigh) {
		hEK_2h2->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_2h2->Draw("colz");

	canv3->cd(3);
	TH2D* hEK_1h2c = static_cast<TH2D*>(file->Get("hEK_1h2c"));
	if(eLow < eHigh) {
		hEK_1h2c->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_1h2c->Draw("colz");

	canv3->cd(4);
	TH2D* hEK_2h2c = static_cast<TH2D*>(file->Get("hEK_2h2c"));
	if(eLow < eHigh) {
		hEK_2h2c->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_2h2c->Draw("colz");

	canv3->cd(5);
	TH2D* hEK_1h3 = static_cast<TH2D*>(file->Get("hEK_1h3"));
	if(eLow < eHigh) {
		hEK_1h3->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_1h3->Draw("colz");

	canv3->cd(6);
	TH2D* hEK_2h3 = static_cast<TH2D*>(file->Get("hEK_2h3"));
	if(eLow < eHigh) {
		hEK_2h3->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_2h3->Draw("colz");

	canv3->cd(7);
	TH2D* hEK_3h3 = static_cast<TH2D*>(file->Get("hEK_3h3"));
	if(eLow < eHigh) {
		hEK_3h3->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_3h3->Draw("colz");

	canv3->cd(8);
	TH2D* hEK_noPU = static_cast<TH2D*>(file->Get("hEK_noPU"));
	if(eLow < eHigh) {
		hEK_noPU->GetYaxis()->SetRangeUser(eLow, eHigh);
	}
	hEK_noPU->Draw("colz");
}
