#include "TFragmentMap.h"

#include <iterator>

bool TFragmentMap::fDebug = false;

TFragmentMap::TFragmentMap(
   std::vector<std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TFragment>>>>& goodOutputQueue,
   std::shared_ptr<ThreadsafeQueue<std::shared_ptr<const TBadFragment>>>&           badOutputQueue)
   : fGoodOutputQueue(goodOutputQueue), fBadOutputQueue(badOutputQueue)
{
}

bool TFragmentMap::Add(const std::shared_ptr<TFragment>& frag, const std::vector<Int_t>& charge,
                       const std::vector<Short_t>& integrationLength)
{
   if(fDebug) {
      std::cout << "Adding fragment " << frag << " (address " << frag->GetAddress() << " , # pileups "
                << frag->GetNumberOfPileups() << ") with " << charge.size() << " charges and "
                << integrationLength.size() << " k-values:" << std::endl;
      for(size_t i = 0; i < charge.size() && i < integrationLength.size(); ++i) {
         std::cout << "\t" << charge[i] << ",\t" << integrationLength[i] << std::endl;
      }
   }
   // a single fragment with just one charge/integration length can be directly put into the queue
   if(charge.size() == 1 && fMap.count(frag->GetAddress()) == 0) {
      frag->SetCharge(charge[0]);
      frag->SetKValue(integrationLength[0]);
      if(fDebug) {
         std::cout << "address " << frag->GetAddress() << ": added single fragment " << frag << std::endl;
      }
      if(fDebug && integrationLength[0] != 700) {
         std::cout << "single fragment (address " << hex(frag->GetAddress(), 4) << ") with integration length "
                   << integrationLength[0] << ", # pileups " << frag->GetNumberOfPileups() << std::endl;
         if(frag->GetNumberOfPileups() > 1) {
            std::cout << "have fragments:" << std::endl;
            for(auto& iter : fMap) {
               std::cout << "\t" << hex(std::get<0>(iter.second)->GetAddress(), 4) << ": "
                         << std::get<0>(iter.second)->GetNumberOfPileups() << std::endl;
            }
         }
      }
      frag->SetEntryNumber();
      for(const auto& outputQueue : fGoodOutputQueue) {
         outputQueue->Push(frag);
      }
      return true;
   }
   // check if this is the last fragment needed
   size_t nofFrags   = 1;
   size_t nofCharges = charge.size();
   // equal_range returns a pair of iterators:
   // the first points to the first fragment with this address,
   // the second to the first fragment of the next address
   // if no fragment with this address exists, both point to the first fragment of the next address
   auto range = fMap.equal_range(frag->GetAddress());
   for(auto it = range.first; it != range.second; ++it) {
      ++nofFrags;
      nofCharges += std::get<1>((*it).second).size();
   }
   // not the last fragment:
   if(nofCharges != 2 * nofFrags - 1) {
      // we need to insert this element into the map, and thanks to equal_range we already know where
      if(fDebug) {
         std::cout << "address " << frag->GetAddress() << ": inserting fragment " << frag << " with " << charge.size() << " charges" << std::endl;
      }
      if(range.first != range.second) {
         fMap.insert(
            range.second,
            std::pair<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>(
               frag->GetAddress(), std::make_tuple(frag, charge, integrationLength)));
      } else {
         fMap.insert(
            std::pair<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>(
               frag->GetAddress(), std::make_tuple(frag, charge, integrationLength)));
      }
      if(fDebug) {
         std::cout << "done" << std::endl;
      }
      return true;
   }
   if(fDebug) {
      std::cout << "address " << frag->GetAddress() << ": last fragment found, calculating charges for " << nofFrags
                << " fragments" << std::endl;
   }
   // last fragment:
   // now we can loop over the stored fragments and the current fragment and calculate all charges
   std::vector<std::shared_ptr<TFragment>> frags;            // all fragments
   std::vector<Long_t>                     kValues;          // all integration lengths
   std::vector<Float_t>                    charges;          // all charges (not integrated charges, but integrated charge divided by integration length!)
   int                                     situation = -1;   // flag to select different scenarios for the time sequence of multiple hits
   switch(nofFrags) {
   case 2:   // only one option: (2, 1)
   {
      if(charge.size() != 1) {
         DropFragments(range);
         if(fDebug) {
            std::cout << "2 w/o single charge" << std::endl;
         }
         return false;
      }
      // create and fill the vector of fragments
      frags.push_back(std::get<0>((*(range.first)).second));
      frags.push_back(frag);
      // create and fill the vector of all integration lengths
      if(fDebug) {
         std::cout << "inserting ... " << std::endl;
      }
      kValues.insert(kValues.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
      if(fDebug) {
         std::cout << "done" << std::endl;
      }
      kValues.push_back(integrationLength[0]);
      // create and fill the vector of all charges
      // we need the actual charges, not the integrated ones, so we calculate them now
      int dropped = -1;
      for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) {
         if(kValues[i] > 0) {
            charges.push_back((static_cast<float>(std::get<1>((*(range.first)).second)[i]) + static_cast<float>(gRandom->Uniform())) / static_cast<float>(kValues[i]));
            if(fDebug) {
               std::cout << "2, " << i << ": " << hex(std::get<1>((*(range.first)).second)[i]) << "/" << hex(kValues[i])
                         << " = " << (std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())
                         << "/" << kValues[i] << " = " << charges.back() << std::endl;
            }
         } else {
            // drop this charge, it's no good
            if(dropped >= 0) {   // we've already dropped one, so we don't have enough left
               DropFragments(range);
               if(fDebug) {
                  std::cout << "2 too much dropped" << std::endl;
               }
               return false;
            }
            if(fDebug) {
               std::cout << "2, dropping " << i << std::endl;
            }
            dropped = static_cast<int>(i);
         }
      }
      if(dropped >= 0 && integrationLength[0] <= 0) {   // we've already dropped one, so we don't have enough left
         DropFragments(range);
         if(fDebug) {
            std::cout << "2 too much dropped (end)" << std::endl;
         }
         return false;
      }
      if(integrationLength[0] <= 0) {
         dropped = 2;
      }
      // we've dropped one, so we use the other two charges to set the fragment charges directly
      // this should never happen, if the two hits are too close to get an integration of their individual charges, we
      // don't see them as two hits (and we would miss both of them)
      // if they are too far apart to get an integration of their sum, they're not piled up
      if(fDebug) {
         std::cout << "dropped = " << dropped << ", charges.size() = " << charges.size() << std::endl;
      }
      switch(dropped) {
      case 0:   // dropped e0, so only e0+e1 and e1 are left
         frags[0]->SetCharge(charges[0] * static_cast<float>(integrationLength[0] - charge[0]));
         frags[1]->SetCharge(charge[0]);
         frags[0]->SetKValue(integrationLength[0]);
         frags[1]->SetKValue(integrationLength[1]);
         frags[0]->SetNumberOfPileups(-200);
         frags[1]->SetNumberOfPileups(-201);
         break;
      case 1:   // dropped e0+e1, so only e0 and e1 are left
         frags[0]->SetCharge(charges[0] * static_cast<float>(integrationLength[0]));
         frags[1]->SetCharge(charge[0]);
         frags[0]->SetKValue(integrationLength[0]);
         frags[1]->SetKValue(integrationLength[1]);
         frags[0]->SetNumberOfPileups(-200);
         frags[1]->SetNumberOfPileups(-201);
         break;
      case 2:   // dropped e1, so only e0 and e0+e1 are left
         frags[0]->SetCharge(charges[0] * static_cast<float>(integrationLength[0]));
         frags[1]->SetCharge((charges[1] - charges[0]) * static_cast<float>(integrationLength[0]));
         frags[0]->SetKValue(integrationLength[0]);
         frags[1]->SetKValue(integrationLength[1]);
         frags[0]->SetNumberOfPileups(-200);
         frags[1]->SetNumberOfPileups(-201);
         break;
      default:   // dropped none
         charges.push_back(static_cast<float>(charge[0] + gRandom->Uniform()) / static_cast<float>(integrationLength[0]));
         if(fDebug) {
            std::cout << "2, -: " << hex(charge[0]) << "/" << hex(integrationLength[0]) << " = "
                      << (charge[0] + gRandom->Uniform()) << "/" << integrationLength[0] << " = " << charges.back()
                      << std::endl;
         }
         Solve(frags, charges, kValues);
         break;
      }
   } break;
   case 3:   // two options: (3, 1, 1), (2, 2, 1)
   {
      if(charge.size() != 1) {
         DropFragments(range);
         if(fDebug) {
            std::cout << "3 w/o single charge" << std::endl;
         }
         return false;
      }
      // create and fill the vector of fragments
      frags.push_back(std::get<0>((*(range.first)).second));
      frags.push_back(std::get<0>((*std::next(range.first)).second));
      frags.push_back(frag);
      // create and fill the vector of all integration lengths
      if(fDebug) {
         std::cout << "inserting first ... " << std::endl;
      }
      kValues.insert(kValues.begin(), std::get<2>((*(range.first)).second).begin(), std::get<2>((*(range.first)).second).end());
      if(fDebug) {
         std::cout << "done" << std::endl;
      }
      situation = kValues.size();
      if(fDebug) {
         std::cout << "inserting second ... " << std::endl;
      }
      kValues.insert(kValues.end(), std::get<2>((*std::next(range.first)).second).begin(),
                     std::get<2>((*std::next(range.first)).second).end());
      if(fDebug) {
         std::cout << "done" << std::endl;
      }
      kValues.push_back(integrationLength[0]);
      // create and fill the vector of all charges
      // we need the actual charges, not the integrated ones, so we calculate them now
      std::vector<int> dropped;
      for(size_t i = 0; i < std::get<1>((*(range.first)).second).size(); ++i) {
         if(kValues[i] > 0) {
            charges.push_back((static_cast<float>(std::get<1>((*(range.first)).second)[i]) + static_cast<float>(gRandom->Uniform())) / static_cast<float>(kValues[i]));
            if(fDebug) {
               std::cout << "3, " << i << ": " << hex(std::get<1>((*(range.first)).second)[i]) << "/"
                         << hex(kValues[i]) << " = " << (std::get<1>((*(range.first)).second)[i] + gRandom->Uniform())
                         << "/" << kValues[i] << " = " << charges.back() << std::endl;
            }
         } else {
            dropped.push_back(i);
            if(fDebug) {
               std::cout << "3, dropping " << i << std::endl;
            }
         }
      }
      for(size_t i = 0; i < std::get<1>((*std::next(range.first)).second).size(); ++i) {
         if(kValues[i + situation] > 0) {
            charges.push_back((static_cast<float>(std::get<1>((*std::next(range.first)).second)[i]) + static_cast<float>(gRandom->Uniform())) / static_cast<float>(kValues[i + situation]));
            if(fDebug) {
               std::cout << "3, " << i + situation << ": "
                         << hex(std::get<1>((*std::next(range.first)).second)[i]) << "/" << hex(kValues[i + situation])
                         << " = " << (std::get<1>((*std::next(range.first)).second)[i] + gRandom->Uniform()) << "/"
                         << kValues[i + situation] << " = " << charges.back() << std::endl;
            }
         } else {
            dropped.push_back(i + situation);
            if(fDebug) {
               std::cout << "dropping " << i + situation << std::endl;
            }
         }
      }
      if(integrationLength[0] <= 0) {
         dropped.push_back(4);
      }
      switch(dropped.size()) {
      case 0:   // dropped none
         charges.push_back((static_cast<float>(charge[0]) + static_cast<float>(gRandom->Uniform())) / static_cast<float>(integrationLength[0]));
         if(fDebug) {
            std::cout << "3, -: " << hex(charge[0]) << "/" << hex(integrationLength[0]) << " = "
                      << (charge[0] + gRandom->Uniform()) << "/" << integrationLength[0] << " = " << charges.back()
                      << std::endl;
         }
         Solve(frags, charges, kValues, situation);
         break;
      case 1:   // dropped one
         // don't know how to handle these right now
         DropFragments(range);
         if(fDebug) {
            std::cout << "3, single drop" << std::endl;
         }
         return false;
         break;
      case 2:   // dropped two => as many left as there are fragments
         DropFragments(range);
         if(fDebug) {
            std::cout << "3, double drop" << std::endl;
         }
         return false;
         switch(dropped[0]) {
         case 0:
            switch(dropped[1]) {
            case 1:   // e0+e1+e2, e2, e3
               break;
            case 2: break;
            case 3: break;
            case 4: break;
            }
            break;
         }
         break;
      default:   // dropped too many
         DropFragments(range);
         if(fDebug) {
            std::cout << "3, dropped too many" << std::endl;
         }
         return false;
      }
   } break;
   case 4:   // five options: (4, 1, 1, 1), (3, 2, 1, 1), (3, 1, 2, 1), (2, 3, 1, 1), (2, 2, 2, 1)
             // break;
   default:
      // std::cerr<<"address "<<frag->GetAddress()<<": deconvolution of "<<nofCharges<<" charges for "<<nofFrags<<"
      // fragments not implemented yet!"<<std::endl;
      DropFragments(range);
      if(fDebug) {
         std::cout << "unknown number of fragments " << nofFrags << std::endl;
      }
      return false;
      break;
   }   // switch(nofFrags)
   // add all fragments to queue
   int index = 0;
   for(auto it = range.first; it != range.second; ++it) {
      frag->SetEntryNumber();
      for(const auto& outputQueue : fGoodOutputQueue) {
         outputQueue->Push(std::get<0>((*it).second));
      }
      if(fDebug) {
         std::cout << "Added " << ++index << ". fragment " << std::get<0>((*it).second) << std::endl;
      }
   }
   frag->SetEntryNumber();
   for(const auto& outputQueue : fGoodOutputQueue) {
      outputQueue->Push(frag);
   }
   if(fDebug) {
      std::cout << "address " << frag->GetAddress() << ": added last fragment " << frag << std::endl;
   }
   // remove these fragments from the map
   fMap.erase(range.first, range.second);

   return true;
}

void TFragmentMap::Solve(std::vector<std::shared_ptr<TFragment>> frag, std::vector<Float_t> charges,
                         std::vector<Long_t> kValues, int situation)
{
   /// Solves minimization of charges for given integrated charges (charges) and integration lengths (kValues).
   /// Resulting charges are stored in the provided fragments with a k-value of 1.
   /// The situation parameter distinguishes between the two different ways 3 hits can pile up with each other:
   /// 3 - both later hits pile up with the first, any other value - the third hit only piles up with the second hit not the first one.

   // all k's are needed squared so we square all elements of k, cast to float here, because that is what we use later on
   std::vector<float> kSquared(kValues.size());
   for(size_t i = 0; i < kValues.size(); ++i) {
      kSquared[i] = static_cast<float>(kValues[i] * kValues[i]);
   }

   switch(frag.size()) {
   case 2:
      frag[0]->SetCharge((charges[0] * (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2]) + (charges[1] - charges[2]) * kSquared[1] * kSquared[2]) /
                         (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2]) * static_cast<float>(kValues[0]));
      frag[1]->SetCharge((charges[2] * (kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2]) + (charges[1] - charges[0]) * kSquared[0] * kSquared[1]) /
                         (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2]) * static_cast<float>(kValues[1]));
      frag[0]->SetKValue(kValues[0]);
      frag[1]->SetKValue(kValues[1]);
      frag[0]->SetNumberOfPileups(-20);
      frag[1]->SetNumberOfPileups(-21);
      if(fDebug) {
         std::cout << "2: charges " << charges[0] << ", " << charges[1] << ", " << charges[2] << " and squared int. lengths " << kSquared[0]
                   << ", " << kSquared[1] << ", " << kSquared[2] << " => " << frag[0]->GetCharge() << ", " << frag[1]->GetCharge()
                   << std::endl
                   << "\t("
                   << (charges[0] * (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2]) + (charges[1] - charges[2]) * kSquared[1] * kSquared[2]) /
                         (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2])
                   << ", "
                   << (charges[2] * (kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2]) + (charges[1] - charges[0]) * kSquared[0] * kSquared[1]) /
                         (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2])
                   << " = " << (charges[0] * (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2]) + (charges[1] - charges[2]) * kSquared[1] * kSquared[2]) << "/"
                   << (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2] + kSquared[1] * kSquared[2]) << " = ("
                   << charges[0] * (kSquared[0] * kSquared[1] + kSquared[0] * kSquared[2]) << "+" << (charges[1] - charges[2]) * kSquared[1] * kSquared[2] << ")/("
                   << kSquared[0] * kSquared[1] << "+" << kSquared[0] * kSquared[2] << "+" << kSquared[1] * kSquared[2] << "))" << std::endl
                   << "pileups = " << frag[0]->GetNumberOfPileups() << ", " << frag[1]->GetNumberOfPileups()
                   << std::endl;
      }
      break;
   case 3:
      if(situation == 3) {   //(3, 1, 1)
         frag[0]->SetCharge(
            (charges[0] * (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[4] +
                           kSquared[0] * kSquared[3] * kSquared[4]) +
             (charges[1] + charges[4]) * kSquared[1] * kSquared[3] * kSquared[4] + charges[2] * (kSquared[1] * kSquared[2] * kSquared[3] + kSquared[2] * kSquared[3] * kSquared[4]) -
             charges[3] * (kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4])) /
            (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[4] +
             kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4]) *
            static_cast<float>(kValues[0]));
         frag[1]->SetCharge(
            (charges[0] * (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[4]) -
             charges[1] * (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3]) +
             charges[2] * (kSquared[1] * kSquared[2] * kSquared[3] - kSquared[0] * kSquared[2] * kSquared[4]) -
             charges[3] * (kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4]) +
             charges[4] * (kSquared[0] * kSquared[2] * kSquared[4] + kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4])) /
            (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[4] +
             kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4]) *
            static_cast<float>(kValues[1]));
         frag[2]->SetCharge(((charges[0] + charges[3]) * kSquared[0] * kSquared[1] * kSquared[3] +
                             charges[1] * (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[3]) +
                             charges[2] * (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[1] * kSquared[2] * kSquared[3]) +
                             charges[4] * (kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[4] + kSquared[0] * kSquared[3] * kSquared[4] +
                                           kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4])) /
                            (kSquared[0] * kSquared[1] * kSquared[2] + kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] +
                             kSquared[0] * kSquared[2] * kSquared[4] + kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] +
                             kSquared[1] * kSquared[3] * kSquared[4] + kSquared[2] * kSquared[3] * kSquared[4]) *
                            static_cast<float>(kValues[2]));
      } else {   //(2, 2, 1)
         frag[0]->SetCharge(
            (charges[0] * (kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[3] + kSquared[0] * kSquared[2] * kSquared[4] +
                           kSquared[0] * kSquared[3] * kSquared[4]) +
             charges[1] * (kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[4] + kSquared[1] * kSquared[3] * kSquared[4]) -
             charges[2] * (kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[4]) - (charges[3] - charges[4]) * kSquared[1] * kSquared[3] * kSquared[4]) /
            (kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[3] + kSquared[0] * kSquared[2] * kSquared[4] +
             kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[4] + kSquared[1] * kSquared[3] * kSquared[4]) *
            static_cast<float>(kValues[0]));
         frag[1]->SetCharge(
            -(charges[0] * kSquared[0] * kSquared[1] * kSquared[3] + charges[0] * kSquared[0] * kSquared[1] * kSquared[4] - charges[1] * kSquared[0] * kSquared[1] * kSquared[3] -
              charges[1] * kSquared[0] * kSquared[1] * kSquared[4] - charges[2] * kSquared[0] * kSquared[2] * kSquared[3] - charges[2] * kSquared[0] * kSquared[2] * kSquared[4] -
              charges[2] * kSquared[1] * kSquared[2] * kSquared[3] - charges[2] * kSquared[1] * kSquared[2] * kSquared[4] - charges[3] * kSquared[0] * kSquared[3] * kSquared[4] -
              charges[3] * kSquared[1] * kSquared[3] * kSquared[4] + charges[4] * kSquared[0] * kSquared[3] * kSquared[4] + charges[4] * kSquared[1] * kSquared[3] * kSquared[4]) /
            (kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[3] + kSquared[0] * kSquared[2] * kSquared[4] +
             kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[4] + kSquared[1] * kSquared[3] * kSquared[4]) *
            static_cast<float>(kValues[1]));
         frag[2]->SetCharge(
            (charges[0] * kSquared[0] * kSquared[1] * kSquared[3] - charges[1] * kSquared[0] * kSquared[1] * kSquared[3] - charges[2] * kSquared[0] * kSquared[2] * kSquared[3] -
             charges[2] * kSquared[1] * kSquared[2] * kSquared[3] + charges[3] * kSquared[0] * kSquared[1] * kSquared[3] + charges[3] * kSquared[0] * kSquared[2] * kSquared[3] +
             charges[3] * kSquared[1] * kSquared[2] * kSquared[3] + charges[4] * kSquared[0] * kSquared[1] * kSquared[4] + charges[4] * kSquared[0] * kSquared[2] * kSquared[4] +
             charges[4] * kSquared[0] * kSquared[3] * kSquared[4] + charges[4] * kSquared[1] * kSquared[2] * kSquared[4] + charges[4] * kSquared[1] * kSquared[3] * kSquared[4]) /
            (kSquared[0] * kSquared[1] * kSquared[3] + kSquared[0] * kSquared[1] * kSquared[4] + kSquared[0] * kSquared[2] * kSquared[3] + kSquared[0] * kSquared[2] * kSquared[4] +
             kSquared[0] * kSquared[3] * kSquared[4] + kSquared[1] * kSquared[2] * kSquared[3] + kSquared[1] * kSquared[2] * kSquared[4] + kSquared[1] * kSquared[3] * kSquared[4]) *
            static_cast<float>(kValues[2]));
      }
      frag[0]->SetKValue(kValues[0]);
      frag[1]->SetKValue(kValues[1]);
      frag[2]->SetKValue(kValues[2]);
      frag[0]->SetNumberOfPileups(-30);
      frag[1]->SetNumberOfPileups(-31);
      frag[2]->SetNumberOfPileups(-32);
      if(fDebug) {
         std::cout << "3, situation " << situation << ": charges " << charges[0] << ", " << charges[1] << ", " << charges[2] << ", "
                   << charges[3] << ", " << charges[4] << " and squared int. lengths " << kSquared[0] << ", " << kSquared[1] << ", " << kSquared[2]
                   << ", " << kSquared[3] << ", " << kSquared[4] << " => " << frag[0]->GetCharge() << ", " << frag[1]->GetCharge()
                   << ", " << frag[2]->GetCharge() << std::endl;
      }
      break;
   }
}

void TFragmentMap::DropFragments(std::pair<
                                 std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>::iterator,
                                 std::multimap<UInt_t, std::tuple<std::shared_ptr<TFragment>, std::vector<Int_t>, std::vector<Short_t>>>::iterator>& range)
{
   /// put the fragments within the range of the two iterators into the bad output queue
   for(auto it = range.first; it != range.second; ++it) {
      //(*it).second is a tuple, with the first element being a shared_ptr<TFragment>
      // we need to convert this to a shared_ptr<TBadFragment>
      fBadOutputQueue->Push(std::make_shared<TBadFragment>(*(std::get<0>((*it).second).get())));
      if(fDebug) {
         std::cout << "Added bad fragment " << std::get<0>((*it).second) << std::endl;
      }
   }
   fMap.erase(range.first, range.second);
}
