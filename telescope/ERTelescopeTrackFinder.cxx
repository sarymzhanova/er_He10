/********************************************************************************
 *              Copyright (C) Joint Institute for Nuclear Research              *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#include "ERTelescopeTrackFinder.h"

#include <cassert>

#include "TVector3.h"
#include "TMath.h"

#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"
#include "FairLink.h"

#include "ERBeamDetTrack.h"
#include "ERRunAna.h"
#include "ERDigi.h"

//--------------------------------------------------------------------------------------------------
ERTelescopeTrackFinder::ERTelescopeTrackFinder()
  : ERTask("ER telescope track finding scheme")
{
  fAvailibleRunManagers.push_back("ERRunAna");
  fQTelescopeSetup = ERTelescopeSetup::Instance();
}
//--------------------------------------------------------------------------------------------------
ERTelescopeTrackFinder::ERTelescopeTrackFinder(Int_t verbose)
  : ERTask("ER telescope track finding scheme ", verbose)
{
  fAvailibleRunManagers.push_back("ERRunAna");
  fQTelescopeSetup = ERTelescopeSetup::Instance();
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::SetHitStation(TString subassemblyName, TString componentId) {
  TString xStripArrayName = componentId;
  TString yStripArrayName = componentId;
  fSiHitStationsPair[subassemblyName].emplace(
      std::make_pair(componentId, std::pair<TString, TString>(xStripArrayName.Append("_X"), 
                                                              yStripArrayName.Append("_Y"))));
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::SetHitStation(TString subassemblyName, TString componentIdX,
                                                                     TString componentIdY) 
{
  fSiHitStationsPair[subassemblyName].emplace(
      std::make_pair(componentIdX + componentIdY, std::pair<TString, TString>(componentIdX, componentIdY)));
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::
SetTrackPositionCorrection(const TString& station_name, ERChannel channel, float strip_fraction) {
  if (strip_fraction < -0.5 || strip_fraction > 0.5) {
    LOG(FATAL) << "Correction of track position in strip should be in range [-0.5, 0.5]"
              << FairLogger::endl;
  }
  track_position_corrections_[station_name][channel] = strip_fraction;
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::SetStripEdepRange(Double_t edepMin, Double_t edepMax) {
  fSiDigiEdepMin = edepMin; 
  fSiDigiEdepMax = edepMax;
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::SetInteractionPosition(double x, double y, double z) {
  interaction_position_is_set_ = true;
  interaction_x_ = x;
  interaction_y_ = y;
  interaction_z_ = z;
}
//--------------------------------------------------------------------------------------------------
TVector3 ERTelescopeTrackFinder::
GetGlobalTrackPositionByStrip(const TString& branch_name, const ERChannel channel) const {
  // Local position of strip center
  auto local_position = fQTelescopeSetup->GetStripLocalPosition(branch_name, channel);
  const auto strip_width = fQTelescopeSetup->GetStripWidth(branch_name, channel);
  // Apply user coorections                                                     
  for (const auto& station_to_channels : track_position_corrections_) {
    const auto station_name = station_to_channels.first;
    if (!branch_name.Contains(station_name))
      continue;
    const auto channel_to_position_correction = station_to_channels.second;
    const auto channel_and_correction = channel_to_position_correction.find(channel);
    if (channel_and_correction == channel_to_position_correction.end())
      continue;
    const auto correction = channel_and_correction->second;
    const auto current_position = local_position[0];
    local_position[0] = current_position + strip_width * correction;
    LOG(DEBUG) << "[ERQTelescopeTrackFinder] Local position of strip " << channel << " of " 
              << station_name << " corrected from " << current_position << " to " 
              << local_position[0] << FairLogger::endl;
  }
  return fQTelescopeSetup->ToGlobalCoordinateSystem(branch_name, local_position);                                              
}
//--------------------------------------------------------------------------------------------------
InitStatus ERTelescopeTrackFinder::Init() {
  if (ERTask::Init() != kSUCCESS)
    return kFATAL;

  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) Fatal("Init", "No FairRootManager");
  TList* allbrNames = ioman->GetBranchNameList();
  TIter nextBranch(allbrNames);
  TObjString* bName;
  std::vector<TString> pointBranches;
  while (bName = (TObjString*)nextBranch()) {
    TString bFullName = bName->GetString();
    if (bFullName.Contains("Digi") && bFullName.Contains("Telescope")) {
      Int_t bPrefixNameLength = bFullName.First('_'); 
      TString brName(bFullName(bPrefixNameLength + 1, bFullName.Length()));
      fQTelescopeDigi[brName] = (TClonesArray*) ioman->GetObject(bFullName);
    }
  }
  // Register output track branches only for stations that are setted by interface SetStation(){
  for (const auto itSubassemblies : fSiHitStationsPair) {
    for (const auto itComponent : itSubassemblies.second) {
      fQTelescopeTrack[itComponent.first] = new TClonesArray("ERTelescopeTrack", 
                                                             consts::approx_telescope_track_number);
      ioman->Register("TelescopeTrack_" + itComponent.first, "Telescope", 
                      fQTelescopeTrack[itComponent.first], kTRUE);
    }
  }

  fBeamDetTrack = (TClonesArray*) ioman->GetObject("BeamDetTrack");   
  if (!interaction_position_is_set_) {
    if (!fBeamDetTrack) {
      LOG(DEBUG) << "ERTelescopeTrackFinder: target point not initialized by user " 
                 << "(by means of SetTargetPoint()) and there is no ERBeamDetTrack branch" 
                 <<FairLogger::endl;
      return kFATAL;
    } 
  }

  fQTelescopeSetup->ERTelescopeSetup::ReadGeoParamsFromParContainer();

  //@TODO check setup and digi branch names
  
  return kSUCCESS;
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::Exec(Option_t* opt) { 
  LOG(DEBUG) << "[ERTelescopeTrackFinder]------------Started--------------------------------------"
             << FairLogger::endl;
  Reset();
  if (!interaction_position_is_set_) {
      ERBeamDetTrack* trackFromMWPC = (ERBeamDetTrack*)fBeamDetTrack->At(0);
      if (!trackFromMWPC) {
        //fRun->MarkFill(kFALSE);
        return ;
      }
      interaction_x_ = trackFromMWPC->GetTargetX();
      interaction_y_ = trackFromMWPC->GetTargetY();
      interaction_z_ = trackFromMWPC->GetTargetZ();
  }
  for (const auto& itSubassemblies : fSiHitStationsPair) {
    for (const auto& itComponent : itSubassemblies.second) {
      // pairs of X and Y strips that have difference between edep less than fEdepDiffXY 
      std::vector<std::pair<Int_t, Int_t>>  hitTelescopePoint;
      // strips with edep in correct interval (fSiDigiEdepMin, fSiDigiEdepMax)     
      std::vector<Int_t> correctStripsX;
      // strips with edep in correct interval (fSiDigiEdepMin, fSiDigiEdepMax)          
      std::vector<Int_t> correctStripsY;
      const TString xDigiBranchName = itComponent.second.first;
      const TString yDigiBranchName = itComponent.second.second;
      if (fQTelescopeSetup->GetStationType(xDigiBranchName) != fQTelescopeSetup->GetStationType(yDigiBranchName)) {
        LOG(FATAL) << "Do not mix R and Q telescopes for track finding" << FairLogger::endl;
      }
      const TClonesArray* xDigi = fQTelescopeDigi[xDigiBranchName];
      const TClonesArray* yDigi = fQTelescopeDigi[yDigiBranchName];
      if ( !xDigi || !yDigi) {
        continue;
      }
      if (xDigi->GetEntriesFast() == 0 || yDigi->GetEntriesFast()==0) {
        continue;
      }
      for (Int_t iXDigi  = 0; iXDigi < xDigi->GetEntriesFast(); iXDigi++) {
        const Double_t xStripEdep = ((ERDigi*)xDigi->At(iXDigi))->Edep();
        if (xStripEdep > fSiDigiEdepMin && xStripEdep < fSiDigiEdepMax) {
          correctStripsX.push_back(iXDigi);
        }
      }
      for (Int_t iYDigi  = 0; iYDigi < yDigi->GetEntriesFast(); iYDigi++) {
        const Double_t yStripEdep = ((ERDigi*)yDigi->At(iYDigi))->Edep();
        if (yStripEdep > fSiDigiEdepMin && yStripEdep < fSiDigiEdepMax) {
          correctStripsY.push_back(iYDigi);
        }
      }
      for (const auto itCorrectStripsX : correctStripsX) {
        const Double_t xStripEdep = ((ERDigi*)xDigi->At(itCorrectStripsX))->Edep();
        for (const auto itCorrectStripsY : correctStripsY) {
          const Double_t yStripEdep = ((ERDigi*)yDigi->At(itCorrectStripsY))->Edep();
          if (TMath::Abs(xStripEdep - yStripEdep) < fEdepDiffXY) {
            hitTelescopePoint.push_back(std::pair<Int_t, Int_t>(itCorrectStripsX, itCorrectStripsY));
          }
        }
      }
      LOG(DEBUG) << "[ERTelescopeTrackFinder] Strips array pair " << itComponent.second.first << " " 
                                         << itComponent.second.second << FairLogger::endl;
      LOG(DEBUG) << "[ERTelescopeTrackFinder] Hits count on pair " << hitTelescopePoint.size() << FairLogger::endl;
      for (const auto& itHitPoint : hitTelescopePoint) {
        const auto xChannelIndex = itHitPoint.first;
        const auto yChannelIndex = itHitPoint.second;
        const auto* xStrip = dynamic_cast<ERDigi*>(xDigi->At(xChannelIndex));
        const auto* yStrip = dynamic_cast<ERDigi*>(yDigi->At(yChannelIndex));
        if (!xStrip || !yStrip)
          continue;
        const auto xChannel = xStrip->Channel();
        const auto yChannel = yStrip->Channel();
        if (fQTelescopeSetup->GetStationType(xDigiBranchName) == ERTelescopeSetup::StationType::QStation) {
          CreateTrackInQTelescope(xChannelIndex, yChannelIndex, xChannel, yChannel, 
                                  xStrip->Edep(), yStrip->Edep(), xDigiBranchName, yDigiBranchName,
                                  itComponent.first);
        } else {
          CreateTrackInRTelescope(xChannelIndex, yChannelIndex, xChannel, yChannel, 
                                  xStrip->Edep(), yStrip->Edep(), xDigiBranchName, yDigiBranchName,
                                  itComponent.first);
        }
      }
    }
  }
  LOG(DEBUG) << "[ERTelescopeTrackFinder]------------Finished--------------------------------------"
             << FairLogger::endl;
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::CreateTrackInQTelescope(
    const Int_t xChannelIndex, const Int_t yChannelIndex, const Int_t xChannel, const Int_t yChannel,
    const Double_t xEdep, const Double_t yEdep, const TString& xDigiBranchName, const TString& yDigiBranchName,
    const TString& trackBranchName) {
  LOG(DEBUG) << "[ERTelescopeTrackFinder] Branch names X:" << xDigiBranchName 
              << " Y: " << yDigiBranchName << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] Strips pair numbers " << xChannel << " " 
              << yChannel << FairLogger::endl;
  // Calc unknown coordinated using condition: target, hit on first station(closest) and
  // hit on second station lie on line :
  // {x2, y2, z2} = {interaction_x_, interaction_y_, interaction_z_} + k * ({x1, y1, z1} - {interaction_x_, interaction_y_, interaction_z_}).
  const bool xStationIsClosest = fQTelescopeSetup->GetStripGlobalZ(xDigiBranchName, xChannel)
                                  < fQTelescopeSetup->GetStripGlobalZ(yDigiBranchName, yChannel);
  // We know all about z coordinate, so 
  const double z1 = xStationIsClosest 
                    ? fQTelescopeSetup->GetStripGlobalZ(xDigiBranchName, xChannel)
                    : fQTelescopeSetup->GetStripGlobalZ(yDigiBranchName, yChannel);
  const double z2 =  xStationIsClosest 
                    ? fQTelescopeSetup->GetStripGlobalZ(yDigiBranchName, yChannel)
                    : fQTelescopeSetup->GetStripGlobalZ(xDigiBranchName, xChannel);
  assert(z1 != interaction_z_);
  const double k = (z2 - interaction_z_) / (z1 - interaction_z_);
  double x1 = 0., x2 = 0., y1 = 0., y2 = 0.;
  if (xStationIsClosest) { // find y1, x2 from equation
    x1 = GetGlobalTrackPositionByStrip(xDigiBranchName, xChannel)[0];
    y2 = GetGlobalTrackPositionByStrip(yDigiBranchName, yChannel)[1];
    LOG(DEBUG) << "[ERTelescopeTrackFinder] Coordinates from strips. x1 = " << x1 
                << " y2 = " << y2 << " z1 = " << z1 << " z2 = " << z2 << FairLogger::endl;
    y1 = (-1./k)*((1. - k)*interaction_y_ - y2);
    x2 = (1. - k)*interaction_x_ + k*x1;
  } else { // find x1, y2 from equation
    x2 = GetGlobalTrackPositionByStrip(xDigiBranchName, xChannel)[0];
    y1 = GetGlobalTrackPositionByStrip(yDigiBranchName, yChannel)[1];
    LOG(DEBUG) << "[ERTelescopeTrackFinder] Coordinates from strips. x2 = " << x2 
                << " y1 = " << y1 << " z1 = " << z1 << " z2 = " << z2 << FairLogger::endl;
    x1 = (-1./k)*((1. - k)*interaction_x_ - x2);
    y2 = (1. - k)*interaction_y_ + k*y1;
  }
  const auto& xStationVertex = xStationIsClosest ? TVector3(x1, y1, z1) : TVector3(x2, y2, z2);
  const auto& yStationVertex = xStationIsClosest ? TVector3(x2, y2, z2) : TVector3(x1, y1, z1);
  const auto& xStationLocalVertex = fQTelescopeSetup->ToStationCoordinateSystem(xDigiBranchName, xStationVertex);
  const auto& yStationLocalVertex = fQTelescopeSetup->ToStationCoordinateSystem(yDigiBranchName, yStationVertex);
  LOG(DEBUG) << "[ERTelescopeTrackFinder] X Station Vertex (" << xStationVertex.x() << " " << xStationVertex.y() 
              << " " << xStationVertex.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] Y Station Vertex (" << yStationVertex.x() << " " << yStationVertex.y() 
              << " " << yStationVertex.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] X Station Vertex in station CS (" << xStationLocalVertex.x() << " " << xStationLocalVertex.y() 
              << " " << xStationLocalVertex.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] Y Station Vertex in station CS (" << yStationLocalVertex.x() << " " << yStationLocalVertex.y() 
              << " " << yStationLocalVertex.z() << ")" << FairLogger::endl;
  auto* track = AddTrack(TVector3(interaction_x_, interaction_y_, interaction_z_), xStationVertex, yStationVertex,
                          xStationLocalVertex, yStationLocalVertex, xChannel, yChannel, xEdep, yEdep,
                          trackBranchName);
  //track->AddLink(FairLink(xDigiBranchName, xChannelIndex));
  //track->AddLink(FairLink(yDigiBranchName, yChannelIndex));
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::CreateTrackInRTelescope(
    const Int_t phiChannelIndex, const Int_t rChannelIndex, const Int_t phiChannel, const Int_t rChannel,
    const Double_t phiEdep, const Double_t rEdep, const TString& phiDigiBranchName, const TString& rDigiBranchName,
    const TString& trackBranchName) {
  LOG(DEBUG) << "[ERTelescopeTrackFinder] Branch names phi:" << phiDigiBranchName 
              << " R: " << rDigiBranchName << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] phi channel = " << phiChannel << " r channel = " 
              << rChannel << FairLogger::endl;
  // Calc unknown coordinated using condition: target, hit on first station(closest) and
  // hit on second station lie on line :
  // {x2, y2, z2} = {interaction_x_, interaction_y_, interaction_z_} + k * ({x1, y1, z1} - {interaction_x_, interaction_y_, interaction_z_}).
  // x = x_station + r * cos(phi); y = y_station + r * sin(phi)
  // Lets 1 - phi station(we know phi1), 2 - r station (we know r2)
  // r2 cos(phi2) = interaction_x_ - x_station2 + k(x_station1 + r1 cos(phi1) - interaction_x_)
  // r2 sin(phi2) = interaction_y_ - y_station2 + k(y_station1 + r1 sin(phi1) - interaction_x_)
  // k = (z2 - interaction_z_) / (z1 - interaction_z_)
  // ----
  // r1: r2^2 = (interaction_x_ - x_station2 + k(x_station1 + r1 cos(phi1) - interaction_x_))^2 
  //            + (interaction_y_ - y_station2 + k(y_station1 + r1 sin(phi1) - interaction_y_))^2
  // A = interaction_x_ - x_station2 + k(x_station1 - interaction_x_)
  // B = k cos(phi1)
  // C = interaction_y_ - y_station2 + k(y_station1 - interaction_y_)
  // D = k sin(phi1)
  // r2^2 = (A + Br1)^2 + (C + Dr1)^2
  // r1 = -/+(sqrt(D^2(r2^2 - A^2)+2ABCD +B^2(r2^2 - C^2)) +- AB +/- CD) / (B^2 +D^2)
  // r1 = -/+(sqrt(D^2(r2^2 - A^2)+2ABCD +B^2(r2^2 - C^2)) +- AB +/- CD) / k^2
  const TVector3 station1 = fQTelescopeSetup->GetStationTranslation(phiDigiBranchName);
  const TVector3 station2 = fQTelescopeSetup->GetStationTranslation(rDigiBranchName);
  const TVector3 target(interaction_x_, interaction_y_, interaction_z_);
  const Double_t phi1 = fQTelescopeSetup->GetStripPhi(phiDigiBranchName, phiChannel);
  const Double_t r2 = fQTelescopeSetup->GetStripR(rDigiBranchName, rChannel);
  const Double_t k = (station2.Z() - interaction_z_) / (station1.Z() - interaction_z_);
  const Double_t A = interaction_x_ - station2.X() + k * (station1.X() - interaction_x_); 
  const Double_t B = k * TMath::Cos(phi1*TMath::RadToDeg());
  const Double_t C = interaction_y_ - station2.Y() + k * (station1.Y() - interaction_y_);
  const Double_t D = k * TMath::Sin(phi1*TMath::RadToDeg());
  const Double_t r1 = (TMath::Sqrt(D*D*(r2*r2 - A*A) + 2*A*B*C*D +B*B*(r2*r2 - C*C)) - A*B - C*D) / (k*k);
  const TVector3 local_vertex1(r1 * TMath::Cos(phi1*TMath::DegToRad()), r1 * TMath::Sin(phi1*TMath::DegToRad()), 0.);
  const TVector3 global_vertex1 = station1 + local_vertex1;
  const TVector3 global_vertex2 = target + k * (global_vertex1 - target);
  const TVector3 local_vertex2 = global_vertex2 - station2;
  const Double_t phi2 = TMath::ACos(local_vertex2.X() / r2) * TMath::RadToDeg();
  LOG(DEBUG) << "[ERTelescopeTrackFinder] phi station: phi = " << phi1 << " r = " << r1 << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] r station: phi = " << phi2 << " r = " << r2 << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] phi station: local vertex = (" << local_vertex1.x() << " " << local_vertex1.y() 
             << " " << local_vertex1.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] r station: local vertex = (" << local_vertex2.x() << " " << local_vertex2.y() 
             << " " << local_vertex2.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] phi station: global vertex = (" << global_vertex1.x() << " " << global_vertex1.y() 
             << " " << global_vertex1.z() << ")" << FairLogger::endl;
  LOG(DEBUG) << "[ERTelescopeTrackFinder] r station: global vertex = (" << global_vertex2.x() << " " << global_vertex2.y() 
             << " " << global_vertex2.z() << ")" << FairLogger::endl;
  auto* track = AddTrack(TVector3(interaction_x_, interaction_y_, interaction_z_), global_vertex1, global_vertex2,
                          local_vertex1, local_vertex2, phiChannel, rChannel, phiEdep, rEdep,
                          trackBranchName);
  //track->AddLink(FairLink(phiDigiBranchName, phiChannelIndex));
  //track->AddLink(FairLink(rDigiBranchName, rChannelIndex));
}
//--------------------------------------------------------------------------------------------------
void ERTelescopeTrackFinder::Reset() {
  for (const auto itTrackBranches : fQTelescopeTrack) {
    if (itTrackBranches.second) {
      itTrackBranches.second->Delete();
    }
  }
}
//--------------------------------------------------------------------------------------------------
ERTelescopeTrack* ERTelescopeTrackFinder::AddTrack(
    const TVector3& targetVertex, const TVector3& xStationVertex, const TVector3& yStationVertex,
    const TVector3& xStationLocalVertex, const TVector3& yStationLocalVertex, 
    const Int_t xChannel, const Int_t yChannel, const Double_t xEdep, const Double_t yEdep,
    const TString& digiBranchName) {
  return new((*fQTelescopeTrack[digiBranchName]) [fQTelescopeTrack[digiBranchName]->GetEntriesFast()])
      ERTelescopeTrack(targetVertex, xStationVertex, yStationVertex, xStationLocalVertex,
                        yStationLocalVertex, xChannel, yChannel, xEdep, yEdep);
}
//--------------------------------------------------------------------------------------------------
ClassImp(ERTelescopeTrackFinder)
