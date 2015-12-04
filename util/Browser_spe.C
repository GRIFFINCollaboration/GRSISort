
#include"load_spe.C"

void Browser_spe(const char *hname)
{
  TH1F *histogram = new TH1F;
  load_spe(hname,histogram);
  histogram->Draw();
  gPad->Update();
}
