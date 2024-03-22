/********************************************************************************
 *              Copyright (C) Joint Institute for Nuclear Research              *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#include "ERNDTrackFinder.h"

#include "FairLogger.h"

#include "ERBeamDetTrack.h"
#include "ERNDDigi.h"

ERNDTrackFinder::ERNDTrackFinder()
  : ERTask("ER ND track finder") 
{
    fAvailibleRunManagers.push_back("ERRunAna");
    fAvailibleRunManagers.push_back("FairRunAna");
}

InitStatus ERNDTrackFinder::Init() {
  if (ERTask::Init() != kSUCCESS)
    return kFATAL;
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) Fatal("Init", "No FairRootManager");
  fNDDigis = (TClonesArray*) ioman->GetObject("NDDigi");
  if (!fNDDigis) Fatal("Init", "Can`t find collection NDDigi!"); 
  fNDTracks = new TClonesArray("ERNDTrack",1000);
  ioman->Register("NDTrack", "ND track", fNDTracks, kTRUE);
  fSetup = ERNDSetup::Instance();
  if (!fUserTargetVertexIsSet) {
    fBeamDetTrack = (TClonesArray*) ioman->GetObject("BeamDetTrack");   
    if (!fBeamDetTrack) {
    LOG(DEBUG) << "ERNDTrackFinder: target point not initialized by user " 
                << "(by means of SetTargetVertex()) and there is no ERBeamDetTrack branch" 
                <<FairLogger::endl;
    return kFATAL;
    } 
  }
  fSetup->ReadGeoParamsFromParContainer();
  return kSUCCESS;
}

void ERNDTrackFinder::Exec(Option_t* opt) {
  Reset();
  Bool_t isN1 = kFALSE;
  Bool_t isN2 = kFALSE;
  float edep_n1 = 0;
  float edep_n2 = 0;
  if (!fUserTargetVertexIsSet) {
    ERBeamDetTrack* trackFromMWPC = (ERBeamDetTrack*)fBeamDetTrack->At(0);
    if (!trackFromMWPC) {
        //fRun->MarkFill(kFALSE);
        return ;
    }
    fTargetVertex = TVector3(trackFromMWPC->GetTargetX(), trackFromMWPC->GetTargetY(),
                             trackFromMWPC->GetTargetZ());
  }
  for (Int_t iDigi(0); iDigi < fNDDigis->GetEntriesFast(); iDigi++) {
    const auto* digi = static_cast<ERNDDigi*>(fNDDigis->At(iDigi));
    edep_n1 = digi->EdepN1();
    edep_n2 = digi->EdepN2();
    if(edep_n1>0) isN1 = kTRUE;
    if(edep_n2>0) isN2 = kTRUE;
    AddTrack(fSetup->Pos(digi->Channel()), isN1, isN2, fTargetVertex, digi->Edep(), edep_n1, edep_n2, digi->Time(), digi->TimeN1(), digi->TimeN2(), digi->TAC());
   isN1 = kFALSE;
   isN2 = kFALSE;
  }
}

void ERNDTrackFinder::Reset() {
    fNDTracks->Clear();
}

void ERNDTrackFinder::SetTargetVertex(const TVector3& vertex) {
  fTargetVertex = vertex;
  fUserTargetVertexIsSet = true;
}

ERNDTrack* ERNDTrackFinder::AddTrack(const TVector3& detectorVertex, Bool_t isN1, Bool_t isN2, 
                                     const TVector3& targetVertex,
                                     float edep, float edep_n1, float edep_n2, float time, float time_n1, float time_n2, float tac) {
    return new((*fNDTracks)[fNDTracks->GetEntriesFast()])
                            ERNDTrack(detectorVertex, isN1, isN2, targetVertex, edep, edep_n1, edep_n2, time, time_n1, time_n2, tac);
}
