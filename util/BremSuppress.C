
BremSuppress(TH2* hp_mat, Double_t threshold)
{

   static const Int_t nScDet = 20;   // hp_mat->GetXaxis()->GetNbins();
   static const Int_t nGrDet = 64;   // hp_mat->GetYaxis()->GetNbins();

   Int_t firstScDet = 1;
   Int_t firstGrDet = 1;
   Int_t lastScDet  = 20;
   Int_t lastGrDet  = 64;

   //  Bool_t *supp_flag = new Bool_t[nScDet+1][nGrDet+1];
   Bool_t supp_flag[nScDet + 1][nGrDet + 1];

   for(int sc = 0; sc <= lastScDet; ++sc) {
      for(int gr = 0; gr <= lastGrDet; ++gr) {
         supp_flag[sc][gr] = false;
      }
   }
   for(int sc = firstScDet; sc <= lastScDet; ++sc) {
      for(int gr = firstGrDet; gr <= lastGrDet; ++gr) {
         //   cout<<sc<<" "<<gr<<": "<<threshold<<" "<<hp_mat->GetBinContent(sc,gr)<<" "<<(hp_mat->GetBinContent(sc,gr)>threshold)<<endl;
         if(hp_mat->GetBinContent(sc, gr) > threshold)
            supp_flag[sc][gr] = true;
         else
            supp_flag[sc][gr] = false;
      }
   }
   /*
      for(int sc=0; sc<=lastScDet;++sc){
         for(int gr=0; gr<=lastGrDet;++gr){
            cout<<sc<<" "<<gr<<": "<<supp_flag[sc][gr]<<endl;
         }
      }
   */
   printf("const Int_t nScDet = %d;\n", nScDet + 1);
   printf("const Int_t nGrDet = %d;\n", nGrDet + 1);
   printf("Bool_t supp_flag[nScDet][nGrDet] = {\n");
   for(int sc = 0; sc <= lastScDet; ++sc) {
      printf("{ ");
      for(int gr = 0; gr <= lastGrDet; ++gr) {
         //   printf("%d,%d,%d: ",sc,gr,supp_flag[sc][gr]);
         if(gr != (lastGrDet))
            printf("%s, ", supp_flag[sc][gr] ? "true " : "false");
         else
            printf("%s ", supp_flag[sc][gr] ? "true " : "false");
      }
      if(sc != (lastScDet))
         printf("},//%d\n", hp_mat->GetXaxis()->GetBinLowEdge(sc));
      else
         printf("}//%d\n", hp_mat->GetXaxis()->GetBinLowEdge(sc));
   }
   printf("};\n");
   hp_mat->SetMinimum(threshold);
   hp_mat->Draw("colz");
}
