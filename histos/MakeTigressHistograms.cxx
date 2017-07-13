#include "TRuntimeObjects.h"
#include "TFragment.h"
#include "TSceptar.h"
#include "TChannel.h"
#include "TTigress.h"
#include "TSharc.h"
#include "TTriFoil.h"

TCutG* prot = nullptr;
TCutG* trit = nullptr;
TCutG* tita = nullptr;

extern "C" void MakeFragmentHistograms(TRuntimeObjects& obj)
{
	std::shared_ptr<const TFragment> frag = obj.GetFragment();
	TChannel*                        chan = frag->GetChannel();

	if(prot == nullptr) {
		TDirectory* current = gDirectory;
		TFile       f("cuts.root");
		prot = dynamic_cast<TCutG*>(f.Get("p"));
		trit = dynamic_cast<TCutG*>(f.Get("t"));
		tita = dynamic_cast<TCutG*>(f.Get("ti"));
		f.Close();
		current->cd();
		printf("LOADED CUTS!\n");
		fflush(stdout);
	}
	// static long first_timestamp = 0;
	if(frag != nullptr && chan != nullptr) {
		obj.FillHistogram("channel", 2500, 0, 2500, frag->GetChannelNumber());
		obj.FillHistogram("channel_charge", 2500, 0, 2500, frag->GetChannelNumber(), 3000, 0, 30000, frag->GetCharge());
		obj.FillHistogram("channel_energy", 2500, 0, 2500, frag->GetChannelNumber(), 10000, 0, 10000, frag->GetEnergy());
		if(chan->GetClassType() == TSceptar::Class()) {
			obj.FillHistogram("sceptar_charge", 4000, 0, 4000, frag->GetCharge());
		}else{

		}
		int sharc_m = frag->GetSharcMesyBoard();

		if(sharc_m>-1) {
			obj.FillHistogram("sharc","mes_trigger_id", 100000, 0, 1000000, frag->GetTriggerId(),
					40, 0, 40, sharc_m);
		}
	}
}


extern "C" void MakeAnalysisHistograms(TRuntimeObjects& obj)
{
	std::shared_ptr<TSharc>   sharc   = obj.GetDetector<TSharc>();
	std::shared_ptr<TTigress> tigress = obj.GetDetector<TTigress>();
	std::shared_ptr<TTriFoil> tf      = obj.GetDetector<TTriFoil>();

	if(sharc != nullptr) {
		obj.FillHistogram("sharc_size", 10, 0, 10, sharc->GetSize());

		for(Int_t i = 0; i < sharc->GetSize(); i++) {

			TSharcHit* shit = sharc->GetSharcHit(i);

			obj.FillHistogram("sharc_diag", "phi_theta", 200, -10, 190, shit->GetTheta() * 180.0 / 3.14159, 400, -200, 200,
					shit->GetPosition().Phi() * 180.0 / 3.14159);

			obj.FillHistogram("sharc_diag", "energy_theta", 200, -10, 190, shit->GetTheta() * 180.0 / 3.14159, 5000, 0,
					50000, shit->GetEnergy());
			obj.FillHistogram("sharc_diag", "detnum", 20, 0, 20, shit->GetDetector());

			obj.FillHistogram("sharc_diag", Form("front_back_%02d", shit->GetDetector()), 60, 0, 60, shit->GetFrontStrip(),
					60, 0, 60, shit->GetBackStrip());

			obj.FillHistogram("phi_theta", 200, -10, 190, shit->GetThetaDeg(), 400, -200, 200,
					shit->GetPosition().Phi() * 180.0 / 3.1415926535);

			obj.FillHistogram("energy_theta", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000, shit->GetEnergy());
			if(tf && tf->TBeam() >= 200 && tf->TBeam() > 260) {
				obj.FillHistogram("energy_theta_200_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
						shit->GetEnergy());
			}
			if(tf && tf->TBeam() >= 230 && tf->TBeam() > 260) {
				obj.FillHistogram("energy_theta_230_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
						shit->GetEnergy());
			}
			if(tf && tf->TBeam() >= 240 && tf->TBeam() > 260) {
				obj.FillHistogram("energy_theta_240_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
						shit->GetEnergy());
			}

			obj.FillHistogram("denergy_theta", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000, shit->GetDeltaE());

			obj.FillHistogram("deltaE_energy", 5000, 0, 50000, shit->GetEnergy(), 1000, 0, 10000, shit->GetDeltaE());

			if(shit->GetDetector() > 8 || shit->GetPadCharge() > 10) {
				obj.FillHistogram("energy_theta_clean", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
						shit->GetEnergy());
				if(tf && tf->TBeam() >= 200 && tf->TBeam() > 260) {
					obj.FillHistogram("energy_theta_clean_200_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
							shit->GetEnergy());
				}
				if(tf && tf->TBeam() >= 230 && tf->TBeam() > 260) {
					obj.FillHistogram("energy_theta_clean_230_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
							shit->GetEnergy());
				}
				if(tf && tf->TBeam() >= 240 && tf->TBeam() > 260) {
					obj.FillHistogram("energy_theta_clean_240_260", 200, -10, 190, shit->GetThetaDeg(), 5000, 0, 50000,
							shit->GetEnergy());
				}
			}

			obj.FillHistogram("detectors_energy", Form("energy_theta_%02i", shit->GetDetector()), 200, -10, 190,
					shit->GetThetaDeg(), 5000, 0, 50000, shit->GetEnergy());

			obj.FillHistogram("detectors_energy", Form("denergy_theta_%02i", shit->GetDetector()), 200, -10, 190,
					shit->GetThetaDeg(), 5000, 0, 50000, shit->GetDeltaE());

			obj.FillHistogram("detectors_energy", Form("energy_vs_frontstrip_%02i", shit->GetDetector()), 50, 0, 50,
					shit->GetFrontStrip(), 5000, 0, 50000, shit->GetEnergy());

			obj.FillHistogram("detectors_energy", Form("energy_vs_backstrip_%02i", shit->GetDetector()), 50, 0, 50,
					shit->GetBackStrip(), 5000, 0, 50000, shit->GetEnergy());

			if(shit->GetPadCharge() > 10) {

				obj.FillHistogram("detectors_energy", Form("deltaE_padE_%02i", shit->GetDetector()), 5000, 0, 50000,
						shit->GetPadE(), 1000, 0, 10000, shit->GetDeltaE());

				obj.FillHistogram("deltaE_padE", 5000, 0, 50000, shit->GetPadE(), 1000, 0, 10000, shit->GetDeltaE());

				if(shit->GetFrontStrip() > 9 && shit->GetBackStrip() < 13) {
					obj.FillHistogram("detectors_energy", Form("padcharge_vs_backstrip_%02i", shit->GetDetector()), 50, 0,
							50, shit->GetBackStrip(), 5000, 0, 50000, shit->GetPadCharge());
					obj.FillHistogram("detectors_energy", Form("padcharge_vs_bstheta_%02i", shit->GetDetector()), 90, 0, 90,
							shit->GetThetaDeg(), 5000, 0, 50000, shit->GetPadCharge());
				}
				obj.FillHistogram("detectors_energy", Form("padcharge_vs_theta_%02i", shit->GetDetector()), 90, 0, 90,
						shit->GetThetaDeg(), 5000, 0, 50000, shit->GetPadCharge());
				obj.FillHistogram("detectors_energy", Form("padenergy_vs_theta_%02i", shit->GetDetector()), 90, 0, 90,
						shit->GetThetaDeg(), 2000, 0, 40000, shit->GetPadE());
			}

			if(tigress != nullptr) {
				for(Short_t t = 0; t < tigress->GetMultiplicity(); t++) {
					TTigressHit* thit = tigress->GetTigressHit(t);
					obj.FillHistogram("tigress", "gamma_sharc_energies", 2000, 0, 10000, shit->GetEnergy(), 4000, 0, 2000,
							thit->GetEnergy());
					obj.FillHistogram("tigress", "gamma_theta", 180, 0, 180, shit->GetThetaDeg(), 8000, 0, 4000,
							thit->GetDoppler(0.105));
					if(shit->GetDetector() < 9) {
						obj.FillHistogram("tigress", "gammas_downstream", 4000, 0, 4000, thit->GetDoppler(0.105));
						obj.FillHistogram("tigress", "gammas_downstream_vs_shitenergy", 5000, 0, 20000, shit->GetEnergy(),
								1000, 0, 4000, thit->GetDoppler(0.105));
					} else {
						obj.FillHistogram("tigress", "gammas_upstream", 4000, 0, 4000, thit->GetDoppler(0.105));
						obj.FillHistogram("tigress", "gammas_upstream_vs_shitenergy", 5000, 0, 20000, shit->GetEnergy(), 1000,
								0, 4000, thit->GetDoppler(0.105));
					}
					obj.FillHistogram("tigress", "all_doppler", 4000, 0, 8000, thit->GetDoppler(0.105));
					double beta = 0.0;
					for(int j = 0; j < 50; j++) {
						beta += 0.12 / 50.;
						obj.FillHistogram("tigress", "gamma_sharc_energies", 2000, 0, 10000, thit->GetDoppler(beta), 50, 0,
								0.12, beta);
					}
				}
			}
		}
	}
}
