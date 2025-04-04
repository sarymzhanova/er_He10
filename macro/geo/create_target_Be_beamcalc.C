#if !defined(__CLING__)

//standard ROOT includes
#include "TSystem.h"
#include "TGeoManager.h"
#include "TFile.h"
#include "TBrowser.h"
#include "TROOT.h"
#include "TGeoTube.h"

//FairRoot includes
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairGeoMedia.h"
#include "FairGeoMedium.h"
#include "FairGeoBuilder.h"

#endif

void create_target_Be_beamcalc()
{
  TString erPath = gSystem->Getenv("VMCWORKDIR");

  // Output paths
  // TString outGeoFilenameRoot = erPath + "/geometry/target.3h_thin.geo.root";
  // TString outGeoFilenameGdml = erPath + "/geometry/target.3h_thin.gdml";
  // TString outGeoFilenameRoot = erPath + "/geometry/target.3h.geo.root";
  // TString outGeoFilenameGdml = erPath + "/geometry/target.3h.gdml";
//  TString outGeoFilenameRoot = erPath + "/geometry/target.3h_steel.geo.root";
//  TString outGeoFilenameGdml = erPath + "/geometry/target.3h_steel.gdml";
  TString outGeoFilenameRoot = erPath + "/geometry/target.be_beamcalc.geo.root";
  TString outGeoFilenameGdml = erPath + "/geometry/target.be_beamcalc.gdml";

  // Input paths
  TString medFile = erPath + "/geometry/media.geo";

  // Materials and media
  FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  geoFace->setMediaFile(medFile);
  geoFace->readMedia();
  FairGeoMedia* geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

  // Geometry manager
  TGeoManager* geoM = (TGeoManager*)gROOT->FindObject("FAIRGeom");

  TString mediumName;

  
  mediumName = "beryllium";
    FairGeoMedium* mberyllium = geoMedia->getMedium(mediumName);
    if (!mberyllium) Fatal("BeamCalc", "FairMedium %s not found", mediumName.Data());
    geoBuild->createMedium(mberyllium);
    TGeoMedium* pberyllium = geoM->GetMedium(mediumName);
    if (!pberyllium) Fatal("BeamCalc", "Medium %s not found", mediumName.Data());

    mediumName = "silicon";
    FairGeoMedium* msilicon = geoMedia->getMedium(mediumName);
    if (!msilicon) Fatal("BeamCalc", "FairMedium %s not found", mediumName.Data());
    geoBuild->createMedium(msilicon);
    TGeoMedium* psilicon = geoM->GetMedium(mediumName);
    if (!psilicon) Fatal("BeamCalc", "Medium %s not found", mediumName.Data());

  // General dimensions
  Double_t transX = 0.; // cm
    Double_t transY = 0.; // cm
    Double_t transZ = 0.; // cm
    Double_t targetBeR = 3.; // cm
    Double_t targetBeZ = 1.35; // cm

  // Shapes
  TGeoTube* targetBeShape = new TGeoTube("targetBeShape",0.,targetBeR, targetBeZ/2.);
    // Volumes
  TGeoVolume* targetBeVol = new TGeoVolume("targetBeVol", targetBeShape, pberyllium);  

  // Matrices
  TGeoRotation* rotNoRot = new TGeoRotation("rotNoRot", 0., 0., 0.);
  rotNoRot->RegisterYourself();

  // // Structure
  // targetShellVol->AddNode(target3HVol, 1);

  // This is the one but last level in the hierarchy
  // This volume-assembly is the only volume to be inserted into TOP
  TGeoVolumeAssembly* subdetectorVolAss = new TGeoVolumeAssembly("target_Be");
  subdetectorVolAss->AddNode(targetBeVol, 1,
    new TGeoCombiTrans("mTargetBeVolInTarget", transX, transY, transZ, rotNoRot));

  // World ------------------------------------
  TGeoVolumeAssembly* topVolAss = new TGeoVolumeAssembly("TOP");
  topVolAss->AddNode(subdetectorVolAss, 1);

  // Finalize
  geoM->SetTopVolume(topVolAss);
  geoM->CloseGeometry();
  geoM->CheckOverlaps();
  geoM->PrintOverlaps();
  //geoM->CheckGeometry();
  //geoM->CheckGeometryFull();
  //geoM->Test();

  // Export
  //geoM->Export(outGeoFilenameGdml);
  TFile* outGeoFileRoot = new TFile(outGeoFilenameRoot, "RECREATE");
  geoM->GetTopVolume()->Write();
  outGeoFileRoot->Close();

  // Draw
  TBrowser* bro = new TBrowser("bro", "bro");
  // geoM->GetTopVolume()->Draw("ogl");
}
