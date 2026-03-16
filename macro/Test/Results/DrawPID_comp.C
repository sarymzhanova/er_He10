#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TCanvas.h"

int DrawPID_comp(
        TString recoFile = "sim_digi_8_1nNDSteel.target.root", 
        TString testFile = "sim_digi_1test.target.root"
        )
{
    TFile* reco = new TFile(recoFile, "read");
    TFile* test = new TFile(testFile, "read");
    TTree* recoTree = (TTree*)reco->Get("er");
    TTree* testTree = (TTree*)test->Get("er");
    TH2F* hReco = new TH2F(
            "hReco", 
            "Reconstruction;Edep Thick station, MeV; Edep Thin station, MeV",
            400,
            0,
            200,
            200,
            0,
            100
            );
    TH2F* hTest = new TH2F(
            "hTest", 
            "Reconstruction_Test;Edep All stations, MeV; Edep One station, MeV",
            400,
            0,
            200,
            200,
            0,
            100
            );
    TCanvas* aCanvas = new TCanvas("aCanvas", "aCanvas", 1400, 700);
    aCanvas->Divide(2, 1);
    aCanvas->cd(1);
    recoTree->Draw(
"TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThinStation:\
TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThickStation>>hReco",
"",
"colz"
);
    aCanvas->cd(2);
    testTree->Draw(
"TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThinStation:\
TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThickStation>>hTest",
"",
"colz"
);
    aCanvas->SaveAs("./Image/pid_comp_reco.png");

    return 0;
}
