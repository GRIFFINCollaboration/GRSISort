


{

	TH1D *hist = (TH1D*)(GetOutputList()->FindObject(Form("Charge_0x%04x",fragment->ChannelAddress)));
	if(hist) hist->Fill(fragment->Charge.at(0)/512.0);

	hist = (TH1D*)(GetOutputList()->FindObject(Form("Energy_0x%04x",fragment->ChannelAddress)));
	if(hist) hist->Fill(channel->CalibrateENG(fragment->Charge.at(0)));
        
	TH1D *histcfd = (TH1D*)(GetOutputList()->FindObject(Form("Cfd_0x%04x",fragment->ChannelAddress)));
        if(histcfd) histcfd->Fill(fragment->Cfd.at(0));

	TH2D *mat = (TH2D*)(GetOutputList()->FindObject("hp_charge"));
	if(mat) mat->Fill(channel->GetNumber(),fragment->Charge.at(0)/512.0);

	hist = (TH1D*)(GetOutputList()->FindObject("test"));
	if(hist) hist->Fill(channel->GetNumber());


}
