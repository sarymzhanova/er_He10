#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TCanvas.h"

int DrawPID_test(
        TString simFile = "../simulations/sim_digi_1test.root", 
        TString recoFile = "../reco/sim_digi_1test.target.root"
        )
{
    TFile* sim = new TFile(simFile, "read");
    TFile* reco = new TFile(recoFile, "read");
    TTree* simTree = (TTree*)sim->Get("er");
    TTree* recoTree = (TTree*)reco->Get("er");
    TH2F* hSim = new TH2F(
            "hSim", 
            "Simulation;Edep Thick station, MeV; Edep Thin station, MeV",
            400,
            0,
            200,
            400,
            0,
            100
            );
    TH2F* hReco = new TH2F(
            "hReco", 
            "Reconstruction;Edep All stations, MeV; Edep One station, MeV",
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
    simTree->Draw(
"TelescopeDigi_Telescope_he8_SingleSi_SSD20_X.fEdep:TelescopeDigi_Telescope_he8_SingleSi_SSD20_1_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_2_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_3_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_4_Y.fEdep+\
TelescopeDigi_Telescope_he8_SingleSi_SSD20_5_Y.fEdep>>hSim", 
"", 
"colz");
    aCanvas->cd(2);
    recoTree->Draw(
"TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThinStation:\
TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.\
fEdepInThickStation>>hReco",
"",
"colz"
);
    aCanvas->SaveAs("./Image/pid_1test.png");

    return 0;
}
