#include "TTdrPlasticHit.h"

#include <iostream>
#include <algorithm>
#include <climits>

#include "Globals.h"
#include "TTdrPlastic.h"
#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TTdrPlasticHit)
/// \endcond

TTdrPlasticHit::TTdrPlasticHit()
{
	// Default Constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TTdrPlasticHit::~TTdrPlasticHit() = default;

TTdrPlasticHit::TTdrPlasticHit(const TTdrPlasticHit& rhs) : TGRSIDetectorHit()
{
	// Copy Constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
	rhs.Copy(*this);
}

TTdrPlasticHit::TTdrPlasticHit(const TFragment& frag) : TGRSIDetectorHit(frag)
{
	if(TTdrPlastic::SetWave()) {
		if(frag.GetWaveform()->empty()) {
			printf("Warning, TTdrPlastic::SetWave() set, but data waveform size is zero!\n");
		}
		if(false) {
			std::vector<Short_t> x;
			// Need to reorder waveform data for S1507 data from December 2014
			// All pairs of samples are swapped.
			// The first two samples are also delayed by 8.
			// We choose to throw out the first 2 samples (junk) and the last 6 samples (convience)
			x              = *(frag.GetWaveform());
			size_t  length = x.size() - (x.size() % 8);
			Short_t temp;

			if(length > 8) {
				for(size_t i = 0; i < length - 8; i += 8) {
					x[i]     = x[i + 9];
					x[i + 1] = x[i + 8];
					temp     = x[i + 2];
					x[i + 2] = x[i + 3];
					x[i + 3] = temp;
					temp     = x[i + 4];
					x[i + 4] = x[i + 5];
					x[i + 5] = temp;
					temp     = x[i + 6];
					x[i + 6] = x[i + 7];
					x[i + 7] = temp;
				}
				x.resize(length - 8);
			}
			SetWaveform(x);
		} else {
			frag.CopyWave(*this);
		}
		if(!GetWaveform()->empty()) {
			//            printf("Analyzing waveform, current cfd = %d\n",dethit.GetCfd());
			AnalyzeWaveform();
			//            printf("%s analyzed waveform, cfd = %d\n",analyzed ?
			//            "successfully":"unsuccessfully",dethit.GetCfd());
		}
	}
}

void TTdrPlasticHit::Copy(TObject& rhs) const
{
	// Copies a TTdrPlasticHit
	TGRSIDetectorHit::Copy(rhs);
	static_cast<TTdrPlasticHit&>(rhs).fFilter = fFilter;
}

void TTdrPlasticHit::Copy(TObject& obj, bool waveform) const
{
	Copy(obj);
	if(waveform) {
		CopyWave(obj);
	}
}

TVector3 TTdrPlasticHit::GetPosition(Double_t) const
{
	// Gets the position of the current TTdrPlasticHit
	return TTdrPlastic::GetPosition(GetDetector());
}

TVector3 TTdrPlasticHit::GetPosition() const
{
	// Gets the position of the current TTdrPlasticHit
	return GetPosition(GetDefaultDistance());
}

bool TTdrPlasticHit::InFilter(Int_t)
{
	// check if the desired filter is in wanted filter;
	// return the answer;
	return true;
}

void TTdrPlasticHit::Clear(Option_t*)
{
	// Clears the TdrPlasticHit
	fFilter = 0;
	TGRSIDetectorHit::Clear();
}

void TTdrPlasticHit::Print(Option_t*) const
{
	// Prints the TdrPlasticHit. Returns:
	// Detector
	// Energy
	// Time
	printf("TdrPlastic Detector: %i\n", GetDetector());
	printf("TdrPlastic hit energy: %.2f\n", GetEnergy());
	printf("TdrPlastic hit time:   %.lf\n", GetTime());
}

bool TTdrPlasticHit::AnalyzeWaveform()
{
	// Calculates the cfd time from the waveform
	bool error = false;
	if(fWaveform.empty()) {
		return false; // Error!
	}

	std::vector<Int_t>   baselineCorrections(8, 0);
	std::vector<Short_t> smoothedWaveform;

	// all timing algorithms use interpolation with this many steps between two samples (all times are stored as
	// integers)
	unsigned int interpolationSteps  = 256;
	int          delay               = 8;
	double       attenuation         = 24. / 64.;
	int          halfsmoothingwindow = 0; // 2*halfsmoothingwindow + 1 = number of samples in moving window.

	// baseline algorithm: correct each adc with average of first two samples in that adc
	for(size_t i = 0; i < 8 && i < fWaveform.size(); ++i) {
		baselineCorrections[i] = fWaveform[i];
	}
	for(size_t i = 8; i < 16 && i < fWaveform.size(); ++i) {
		baselineCorrections[i - 8] =
			((baselineCorrections[i - 8] + fWaveform[i]) + ((baselineCorrections[i - 8] + fWaveform[i]) > 0 ? 1 : -1)) >>
			1;
	}
	for(size_t i = 0; i < fWaveform.size(); ++i) {
		fWaveform[i] -= baselineCorrections[i % 8];
	}

	SetCfd(CalculateCfd(attenuation, delay, halfsmoothingwindow, interpolationSteps));

	return !error;
}

Int_t TTdrPlasticHit::CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow,
		unsigned int interpolationSteps)
{
	// Used when calculating the CFD from the waveform
	std::vector<Short_t> monitor;

	return CalculateCfdAndMonitor(attenuation, delay, halfsmoothingwindow, interpolationSteps, monitor);
}

Int_t TTdrPlasticHit::CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow,
		unsigned int interpolationSteps, std::vector<Short_t>& monitor)
{
	// Used when calculating the CFD from the waveform

	Short_t monitormax = 0;

	bool armed = false;

	Int_t cfd = 0;

	std::vector<Short_t> smoothedWaveform;

	if(fWaveform.empty()) {
		return INT_MAX; // Error!
	}

	if(static_cast<unsigned int>(fWaveform.size()) > delay + 1) {

		if(halfsmoothingwindow > 0) {
			smoothedWaveform = TTdrPlasticHit::CalculateSmoothedWaveform(halfsmoothingwindow);
		} else {
			smoothedWaveform = fWaveform;
		}

		monitor.resize(smoothedWaveform.size() - delay);
		monitor[0] = attenuation * smoothedWaveform[delay] - smoothedWaveform[0];
		if(monitor[0] > monitormax) {
			armed      = true;
			monitormax = monitor[0];
		}

		for(unsigned int i = delay + 1; i < smoothedWaveform.size(); ++i) {
			monitor[i - delay] = attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay];
			if(monitor[i - delay] > monitormax) {
				armed      = true;
				monitormax = monitor[i - delay];
			} else {
				if(armed && monitor[i - delay] < 0) {
					armed = false;
					if(monitor[i - delay - 1] - monitor[i - delay] != 0) {
						// Linear interpolation.
						cfd = (i - delay - 1) * interpolationSteps +
							(monitor[i - delay - 1] * interpolationSteps) / (monitor[i - delay - 1] - monitor[i - delay]);
					} else {
						// Should be impossible, since monitor[i-delay-1] => 0 and monitor[i-delay] > 0
						cfd = 0;
					}
				}
			}
		}
	} else {
		monitor.resize(0);
	}

	return cfd;
}

std::vector<Short_t> TTdrPlasticHit::CalculateSmoothedWaveform(unsigned int halfsmoothingwindow)
{
	// Used when calculating the CFD from the waveform

	if(fWaveform.empty()) {
		return std::vector<Short_t>(); // Error!
	}

	std::vector<Short_t> smoothedWaveform(std::max(static_cast<size_t>(0), fWaveform.size() - 2 * halfsmoothingwindow),
			0);

	for(size_t i = halfsmoothingwindow; i < fWaveform.size() - halfsmoothingwindow; ++i) {
		for(int j = -static_cast<int>(halfsmoothingwindow); j <= static_cast<int>(halfsmoothingwindow); ++j) {
			smoothedWaveform[i - halfsmoothingwindow] += fWaveform[i + j];
		}
	}

	return smoothedWaveform;
}

std::vector<Short_t> TTdrPlasticHit::CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow)
{
	// Used when calculating the CFD from the waveform

	if(fWaveform.empty()) {
		return std::vector<Short_t>(); // Error!
	}

	std::vector<Short_t> smoothedWaveform;

	if(halfsmoothingwindow > 0) {
		smoothedWaveform = TTdrPlasticHit::CalculateSmoothedWaveform(halfsmoothingwindow);
	} else {
		smoothedWaveform = fWaveform;
	}

	std::vector<Short_t> monitor(std::max(static_cast<size_t>(0), smoothedWaveform.size() - delay), 0);

	for(size_t i = delay; i < smoothedWaveform.size(); ++i) {
		monitor[i - delay] = attenuation * smoothedWaveform[i] - smoothedWaveform[i - delay];
	}

	return monitor;
}
