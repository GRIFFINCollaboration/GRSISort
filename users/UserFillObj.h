



   TH1D *hist = (TH1D*)(GetOutputList()->FindObject(Form("Charge_0x%04x",fragment->GetChannelAddress())));
	if(hist) hist->Fill(fragment->GetCharge(0));

   hist = (TH1D*)(GetOutputList()->FindObject(Form("Energy_0x%04x",fragment->GetChannelAddress())));
	if(hist) hist->Fill(fragment->GetEnergy(0));

	TH1D *histcfd = (TH1D*)(GetOutputList()->FindObject(Form("Cfd_0x%04x",fragment->GetChannelAddress())));
   if(histcfd) histcfd->Fill(fragment->GetCfd(0));

	TH2D *mat = (TH2D*)(GetOutputList()->FindObject("hp_charge"));
   if(mat && fragment->GetDetectorType() <= 1 ) mat->Fill(channel->GetNumber(),fragment->GetCharge(0));

	mat = (TH2D*)(GetOutputList()->FindObject("hp_energy"));
   if(mat && fragment->GetDetectorType() <= 1) mat->Fill(channel->GetNumber(),fragment->GetEnergy());

   hist = (TH1D*)(GetOutputList()->FindObject("test"));
	if(hist) hist->Fill(channel->GetNumber());

	hist = (TH1D*)(GetOutputList()->FindObject("PPG"));
   if(hist) hist->Fill(fragment->GetPPGWord());
   
   hist = (TH1D*)(GetOutputList()->FindObject("EnergySum"));
   if(hist && fragment->GetDetectorType() <= 1) hist->Fill(fragment->GetEnergy());

	hist = (TH1D*)(GetOutputList()->FindObject("ChannelId"));
   if(hist) hist->Fill(fragment->GetChannelId());

	hist = (TH1D*)(GetOutputList()->FindObject("MidasId"));
   if(hist) hist->Fill(fragment->GetMidasId());

	hist = (TH1D*)(GetOutputList()->FindObject("MidasTimeStamp"));
   if(hist) hist->Fill(fragment->GetMidasTimeStamp());

	hist = (TH1D*)(GetOutputList()->FindObject("TriggerId"));
   if(hist) hist->Fill(fragment->GetTriggerId());

	hist = (TH1D*)(GetOutputList()->FindObject("Charge_nofilter"));
   if(hist && fragment->GetTriggerId() < 0) hist->Fill(fragment->GetCharge(0));

   hist = (TH1D*)(GetOutputList()->FindObject("Charge_filter"));
   if(hist && fragment->GetTriggerId() > 0) hist->Fill(fragment->GetCharge(0)); 

	hist = (TH1D*)(GetOutputList()->FindObject("TriggerPattern"));
   if(hist && fragment->GetTriggerBitPattern()>-1) hist->Fill(fragment->GetTriggerBitPattern());


