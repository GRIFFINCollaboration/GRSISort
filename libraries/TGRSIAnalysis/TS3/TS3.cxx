#include "TS3.h"
#include "TMnemonic.h"

#include <cmath>
#include "TMath.h"

#include "TGRSIOptions.h"

/// \cond CLASSIMP
ClassImp(TS3)
/// \endcond

int TS3::fRingNumber = 24;
int TS3::fSectorNumber  = 32;

double TS3::fOffsetPhiCon = 0.5 * TMath::Pi(); // Offset between connector and sector 0 (viewed from sector side)
double TS3::fOffsetPhiSet =
   -22.5 * TMath::Pi() / 180.; // Phi rotation of connector in setup // -90 for bambino -22.5 for SPICE
double TS3::fOuterDiameter  = 70.;
double TS3::fInnerDiameter  = 22.;
double TS3::fTargetDistance = 31.;

// Default tigress unpacking settings
TTransientBits<UShort_t> TS3::fgS3Bits = static_cast<std::underlying_type<TS3::ES3GlobalBits>::type>(TS3::ES3GlobalBits::kMultHit);

Int_t  TS3::fFrontBackTime = 75;
double TS3::fFrontBackEnergy = 0.9;
double TS3::fFrontBackOffset = 0;

TS3::TS3()
{
   Clear();
}

TS3::~TS3() = default;

TS3& TS3::operator=(const TS3& rhs)
{
   rhs.Copy(*this);
   return *this;
}

TS3::TS3(const TS3& rhs) : TGRSIDetector()
{
   rhs.Copy(*this);
}

void TS3::Copy(TObject& rhs) const
{
   TGRSIDetector::Copy(rhs);
   static_cast<TS3&>(rhs).fS3RingHits   = fS3RingHits;
   static_cast<TS3&>(rhs).fS3SectorHits = fS3SectorHits;
}

void TS3::AddFragment(const std::shared_ptr<const TFragment>& frag, TChannel* chan)
{
   /// This function creates TS3Hits for each fragment and stores them in separate front and back vectors
   if(frag == nullptr || chan == nullptr) {
      return;
   }

   TS3Hit dethit(*frag); // Moved upstream/downstream switch into hit ctor

   if(chan->GetMnemonic()->CollectedCharge() == TMnemonic::EMnemonic::kN) {
      dethit.SetRingNumber(frag->GetSegment());
      dethit.SetSectorNumber(0);

      if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting()) {
         dethit.SetWavefit(*frag);
      }

      fS3RingHits.push_back(std::move(dethit));
   } else {
      dethit.SetRingNumber(0);
      dethit.SetSectorNumber(frag->GetSegment());

      if(TGRSIOptions::AnalysisOptions()->IsWaveformFitting()) {
         dethit.SetWavefit(*frag);
      }

      fS3SectorHits.push_back(std::move(dethit));
   }
}

void TS3::SetBitNumber(enum ES3Bits bit, Bool_t set)
{
   // Used to set the flags that are stored in TS3.
   fS3Bits.SetBit(bit, set);
}

Int_t TS3::GetPixelMultiplicity()
{
   // Creates a vector of TS3Hits based on front/back coincidences
   // Returns the size of the resultant vector

   BuildPixels();

   return fS3Hits.size();
}

void TS3::BuildPixels()
{
   // Constructs the front/back coincidences to create pixels based on energy and time differences
   // Energy and time differences can be changed using the SetFrontBackEnergy and SetFrontBackTime functions
   // Shared rings and sectors can be constructed, by default they are not.
   // To enable shared hits, use SetMultiHit function

   // if the pixels have been reset (or never set), clear the pixel hits //MUST BE FIRST
   if(!fS3Bits.TestBit(ES3Bits::kPixelsSet)) {
      fS3Hits.clear();
   }
	
   if(fS3RingHits.empty() || fS3SectorHits.empty()) {
      return;
   }

   if(fS3Hits.empty()) {

      // We are going to want energies several times
      // So build a quick vector
      std::vector<double> EneR, EneS;
      std::vector<bool>   UsedRing, UsedSector;
      for(auto& fS3RingHit : fS3RingHits) {
         EneR.push_back(fS3RingHit.GetEnergy());
         UsedRing.push_back(false);
      }
      for(auto& fS3SectorHit : fS3SectorHits) {
         EneS.push_back(fS3SectorHit.GetEnergy());
         UsedSector.push_back(false);
      }

      // new
      /// Loop over two vectors and build energy+time matching hits
      for(size_t i = 0; i < fS3RingHits.size(); ++i) {
         for(size_t j = 0; j < fS3SectorHits.size(); ++j) {
	    if(fS3RingHits[i].GetArrayPosition()!=fS3SectorHits[j].GetArrayPosition())continue;

            if(abs(fS3RingHits[i].GetTime() - fS3SectorHits[j].GetTime())*1.6 < fFrontBackTime) { // check time
               if((EneR[i] - fFrontBackOffset) * fFrontBackEnergy < EneS[j] &&
                  (EneS[j] - fFrontBackOffset) * fFrontBackEnergy < EneR[i]) { // if time is good check energy

                  // Now we have accepted a good event, build it
                  if(SectorPreference()) {
                     TS3Hit dethit = fS3SectorHits[j]; // Sector defines all data ring just gives position
                     dethit.SetRingNumber(fS3RingHits[i].GetRing());
                     fS3Hits.push_back(dethit);
                  } else {
                     TS3Hit dethit = fS3RingHits[i]; // Ring defines all data sector just gives position (default)
                     dethit.SetSectorNumber(fS3SectorHits[j].GetSector());
                     fS3Hits.push_back(dethit);
                  }

                  // Although set to used for MultiHit, continue to check all combinations in this loop.
                  UsedRing[i]   = true;
                  UsedSector[j] = true;
                  // This is desired behaviour for telescope, debugging, etc, when one would set fFrontBackEnergy=0 and
                  // build all combinations
               }
            }
         }
      }

      if(MultiHit()) {

         int ringcount   = 0;
         int sectorcount = 0;
         for(auto&& i : UsedRing) {
            if(!i) {
               ringcount++;
            }
         }

         for(auto&& i : UsedSector) {
            if(!i) {
               sectorcount++;
            }
         }

         /// If we have parts of hit left here they are possibly a shared strip hit not easy singles
         if(ringcount > 1 || sectorcount > 1) {

            // Shared Ring loop
            for(size_t i = 0; i < fS3RingHits.size(); ++i) {
               if(UsedRing.at(i)) {
                  continue;
               }
               for(size_t j = 0; j < fS3SectorHits.size(); ++j) {
                  if(UsedSector.at(j)) {
                     continue;
                  }
                  if(fS3RingHits[i].GetArrayPosition()!=fS3SectorHits[j].GetArrayPosition())continue;
                  
                  for(size_t k = j + 1; k < fS3SectorHits.size(); ++k) {
                     if(UsedSector.at(k)) {
                        continue;
                     }
                     if(fS3SectorHits[j].GetArrayPosition()!=fS3SectorHits[k].GetArrayPosition())continue;

                     if(abs(fS3RingHits[i].GetTime() - fS3SectorHits[j].GetTime())*1.6 < fFrontBackTime &&
                        abs(fS3RingHits[i].GetTime() - fS3SectorHits[k].GetTime())*1.6 < fFrontBackTime) { // check time
                        if((EneR[i] - fFrontBackOffset) * fFrontBackEnergy < (EneS[j] + EneS[k]) &&
                           (EneS[j] + EneS[k] - fFrontBackOffset) * fFrontBackEnergy < EneR[i]) { // if time is good check energy

                           int SectorSep = fS3SectorHits[j].GetSector() - fS3SectorHits[k].GetSector();
                           if(abs(SectorSep) == 1 || abs(SectorSep) == fSectorNumber) {
                              // Same ring and neighbour sectors, almost certainly charge sharing
                              // Experiments with breakup might get real mult2 events like this but most will be
                              // charge
                              // sharing

                              if(KeepShared()) {
                                 TS3Hit dethit = fS3RingHits[i]; // Ring defines all data sector just gives position
                                 // Selecting one of the sectors is currently the best class allows, some loss of
                                 // position information
                                 if(fS3SectorHits[k].GetEnergy() < fS3SectorHits[j].GetEnergy()) {
                                    dethit.SetSectorNumber(fS3SectorHits[j].GetSector());
                                 } else {
                                    dethit.SetSectorNumber(fS3SectorHits[k].GetSector());
                                 }
                                 fS3Hits.push_back(dethit);
                              }
                           } else {
                              // 2 separate hits with shared ring

                              // Now we have accepted a good event, build it
                              TS3Hit dethit = fS3SectorHits[j]; // Sector now defines all data ring just gives position
                              dethit.SetRingNumber(fS3RingHits[i].GetRing());
                              fS3Hits.push_back(dethit);

                              // Now we have accepted a good event, build it
                              TS3Hit dethitB = fS3SectorHits[k]; // Sector now defines all data ring just gives position
                              dethitB.SetRingNumber(fS3RingHits[i].GetRing());
                              fS3Hits.push_back(dethitB);
                           }

                           UsedRing[i]   = true;
                           UsedSector[j] = true;
                           UsedSector[k] = true;
                        }
                     }
                  }
               }
            } // End Shared Ring loop
         }

         ringcount   = 0;
         sectorcount = 0;
         for(auto&& i : UsedRing) {
            if(!i) {
               ringcount++;
            }
         }

         for(auto&& i : UsedSector) {
            if(!i) {
               sectorcount++;
            }
         }

         if(ringcount > 1 || sectorcount > 1) {
            // Shared Sector loop
            for(size_t i = 0; i < fS3SectorHits.size(); ++i) {
               if(UsedSector.at(i)) {
                  continue;
               }
               for(size_t j = 0; j < fS3RingHits.size(); ++j) {
                  if(UsedRing.at(j)) {
                     continue;
                  }
                  if(fS3SectorHits[i].GetArrayPosition()!=fS3RingHits[j].GetArrayPosition())continue;
		  
                  for(size_t k = j + 1; k < fS3RingHits.size(); ++k) {
                     if(UsedRing.at(k)) {
                        continue;
                     }
                     if(fS3RingHits[j].GetArrayPosition()!=fS3RingHits[k].GetArrayPosition())continue;

                     if(abs(fS3SectorHits[i].GetTime() - fS3RingHits[j].GetTime())*1.6 < fFrontBackTime &&
                        abs(fS3SectorHits[i].GetTime() - fS3RingHits[k].GetTime())*1.6 < fFrontBackTime) { // first check time
                        if((EneS[i] - fFrontBackOffset) * fFrontBackEnergy < (EneR[j] + EneR[k]) &&
                           (EneR[j] + EneR[k] - fFrontBackOffset) * fFrontBackEnergy < EneS[i]) { // if time is good check energy

                           if(abs(fS3RingHits[j].GetRing() - fS3RingHits[k].GetRing()) == 1) {
                              // Same sector and neighbour rings, almost certainly charge sharing
                              // Experiments with breakup might get real mult2 events like this but most
                              // will be charge
                              // sharing

                              if(KeepShared()) {
                                 TS3Hit dethit = fS3SectorHits[i]; // Sector defines all data ring just gives position
                                 // Selecting one of the sectors is currently the best class allows, some
                                 // loss of
                                 // position information
                                 if(fS3RingHits[k].GetEnergy() < fS3RingHits[j].GetEnergy()) {
                                    dethit.SetRingNumber(fS3RingHits[j].GetRing());
                                 } else {
                                    dethit.SetRingNumber(fS3RingHits[k].GetRing());
                                 }
                                 fS3Hits.push_back(dethit);
                              }
                           } else {
                              // 2 separate hits with shared sector

                              // Now we have accepted a good event, build it
                              TS3Hit dethit = fS3RingHits[j]; // Ring defines all data sector just gives position
                              dethit.SetSectorNumber(fS3SectorHits[i].GetSector());
                              fS3Hits.push_back(dethit);

                              // Now we have accepted a good event, build it
                              TS3Hit dethitB = fS3RingHits[k]; // Ring defines all data sector just gives position
                              dethitB.SetSectorNumber(fS3SectorHits[i].GetSector());
                              fS3Hits.push_back(dethitB);
                           }

                           UsedSector[i] = true;
                           UsedRing[j]   = true;
                           UsedRing[k]   = true;
                        }
                     }
                  }
               }
            } // End Shared Sector loop
         }
      }

      SetBitNumber(ES3Bits::kPixelsSet, true);
   }
}

TVector3 TS3::GetPosition(int ring, int sector, bool smear)
{
   return GetPosition(ring, sector, fOffsetPhiSet, fTargetDistance, true, smear);
}

TVector3 TS3::GetPosition(int ring, int sector, double offsetphi, double offsetZ, bool sectorsdownstream, bool smear)
{

   double ring_width   = (fOuterDiameter - fInnerDiameter) * 0.5 / fRingNumber; // 24 rings   radial width!
   double inner_radius = fInnerDiameter / 2.0;
   double phi_width    = 2. * TMath::Pi() / fSectorNumber;
   double radius       = inner_radius + ring_width * (ring + 0.5);
   double phi          = phi_width * sector; // the phi angle....
   phi += fOffsetPhiCon;
   // The above calculates the position on the S3

   // This orients the detector relative to the beam
   if(sectorsdownstream) {
      phi = -phi;
   }
   phi += offsetphi;

   if(smear) {
      double sep = ring_width * 0.025;
      double r1 = radius - ring_width * 0.5 + sep, r2 = radius + ring_width * 0.5 - sep;
      radius        = sqrt(gRandom->Uniform(r1 * r1, r2 * r2));
      double sepphi = sep / radius;
      phi           = gRandom->Uniform(phi - phi_width * 0.5 + sepphi, phi + phi_width * 0.5 - sepphi);
   }

   return TVector3(cos(phi) * radius, sin(phi) * radius, offsetZ);
}

void TS3::ResetRingsSectors(){
	// This is necessary if you want mnemonics in a cal file to override those used during frag sort.
	for(size_t i = 0; i < fS3SectorHits.size(); ++i) {
		fS3SectorHits.at(i).SetSectorNumber();
	}
	for(size_t i = 0; i < fS3RingHits.size(); ++i) {
		fS3RingHits.at(i).SetRingNumber();
	}
}


TGRSIDetectorHit* TS3::GetHit(const int& idx)
{
   return GetS3Hit(idx);
}

TS3Hit* TS3::GetS3Hit(const int& i)
{
   if(i < GetPixelMultiplicity()) {
      return &fS3Hits.at(i);
   }
   std::cerr<<"S3 pixel hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

TS3Hit* TS3::GetRingHit(const int& i)
{
   if(i < GetRingMultiplicity()) {
      return &fS3RingHits.at(i);
   }
   std::cerr<<"S3 ring hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

TS3Hit* TS3::GetSectorHit(const int& i)
{
   if(i < GetSectorMultiplicity()) {
      return &fS3SectorHits.at(i);
   }
   std::cerr<<"S3 sector hits are out of range"<<std::endl;
   throw grsi::exit_exception(1);
   return nullptr;
}

/*void TS3::PushBackHit(TGRSIDetectorHit *deshit) {
  fS3Hits.push_back(*((TS3Hit*)deshit));
  return;
  }
  */

void TS3::Print(Option_t*) const
{
   printf("%s\tnot yet written.\n", __PRETTY_FUNCTION__);
}

void TS3::Clear(Option_t* opt)
{
   TGRSIDetector::Clear(opt);
   fS3Hits.clear();
   fS3RingHits.clear();
   fS3SectorHits.clear();

   SetPixels(false);
   SetMultiHit(false);
}
