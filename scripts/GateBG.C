
TH1* ProjectionXBGP(TH2* matrix, Int_t gate_low, Int_t gate_high, Int_t bg_low = -1, Int_t bg_high = -1) {

   //The Projection functions are inclusive on the bins, we need to take this into account when coming up with our scale factor.
   //Find the counts in the background of the TPeak.
   //Get the background of the last peak fit
   if(!GPeak::GetLastFit()){
      std::cout << "Missing a Fit to use, not performing BG correction!!!" << std::endl;
      return matrix->ProjectionX(Form("gated_%s_%d_to_%d_px",matrix->GetName(),gate_low,gate_high),gate_low,gate_high); ;
   }

   GPeak *peak = GPeak::GetLastFit();
   //The peak should have came from the y-axis projection. We need to use this histogram to do things like get bin width, and the coord->bin mapping
   TH1* y_projection = matrix->ProjectionY();
   Double_t bin_width = y_projection->GetBinWidth(1);

   //Integrate the background 
   Double_t bg_counts_under_peak = peak->Background()->Integral( y_projection->GetXaxis()->GetBinLowEdge(gate_low),y_projection->GetXaxis()->GetBinUpEdge(gate_high))/bin_width;
   Double_t bg_counts_in_bg_gate = y_projection->Integral(bg_low,bg_high);
   if(bg_low == -1 && bg_high == -1)
      bg_counts_in_bg_gate = 1.;

   
   Double_t scale_factor = bg_counts_under_peak/bg_counts_in_bg_gate;
   std::cout << "Background counts = " << bg_counts_under_peak << std::endl;
   std::cout << "Scale factor      = " << scale_factor << std::endl;
   //Use Get Axis and low edge, high edge etc.
   //First thing we need to do is project out the matrix using the gate.
   TH1* gated_histogram = matrix->ProjectionX(Form("gated_%s_%d_to_%d_px",matrix->GetName(),gate_low,gate_high),gate_low,gate_high);
   TH1* bg_histogram = matrix->ProjectionX(Form("bg_%s_%d_to_%d_px",matrix->GetName(),bg_low,bg_high),bg_low,bg_high);

   //Clone the gated histogram so that we can subtract the bg from it
   TH1* corrected_histogram = static_cast<TH1*>(gated_histogram->Clone());
   corrected_histogram->SetName(Form("corrected_%s_%d_%d_px",matrix->GetName(),gate_low,gate_high));
   corrected_histogram->Add(bg_histogram,-scale_factor);

   return corrected_histogram;

}

TH1* ProjectionYBGP(TH2* matrix, Int_t gate_low, Int_t gate_high, Int_t bg_low = -1, Int_t bg_high = -1) {

   //The Projection functions are inclusive on the bins, we need to take this into account when coming up with our scale factor.
   //Find the counts in the background of the TPeak.
   //Get the background of the last peak fit
   if(!GPeak::GetLastFit()){
      std::cout << "Missing a Fit to use, not performing BG correction!!!" << std::endl;
      return matrix->ProjectionY(Form("gated_%s_%d_to_%d_px",matrix->GetName(),gate_low,gate_high),gate_low,gate_high); ;
   }

   GPeak *peak = GPeak::GetLastFit();
   //The peak should have came from the y-axis projection. We need to use this histogram to do things like get bin width, and the coord->bin mapping
   TH1* x_projection = matrix->ProjectionX();
   Double_t bin_width = x_projection->GetBinWidth(1);

   //Integrate the background 
   Double_t bg_counts_under_peak = peak->Background()->Integral( x_projection->GetXaxis()->GetBinLowEdge(gate_low),y_projection->GetXaxis()->GetBinUpEdge(gate_high))/bin_width;
   Double_t bg_counts_in_bg_gate = x_projection->Integral(bg_low,bg_high);
   if(bg_low == -1 && bg_high == -1)
      bg_counts_in_bg_gate = 1.;

   
   Double_t scale_factor = bg_counts_under_peak/bg_counts_in_bg_gate;
   std::cout << "Background counts = " << bg_counts_under_peak << std::endl;
   std::cout << "Scale factor      = " << scale_factor << std::endl;
   //Use Get Axis and low edge, high edge etc.
   //First thing we need to do is project out the matrix using the gate.
   TH1* gated_histogram = matrix->ProjectionY(Form("gated_%s_%d_to_%d_py",matrix->GetName(),gate_low,gate_high),gate_low,gate_high);
   TH1* bg_histogram = matrix->ProjectionY(Form("bg_%s_%d_to_%d_py",matrix->GetName(),bg_low,bg_high),bg_low,bg_high);

   //Clone the gated histogram so that we can subtract the bg from it
   TH1* corrected_histogram = static_cast<TH1*>(gated_histogram->Clone());
   corrected_histogram->SetName(Form("corrected_%s_%d_%d_py",matrix->GetName(),gate_low,gate_high));
   corrected_histogram->Add(bg_histogram,-scale_factor);

   return corrected_histogram;

}
