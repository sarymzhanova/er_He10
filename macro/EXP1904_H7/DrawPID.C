#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH2F.h"

int DrawPID(TString inFile = "input/h7_ct_18_0001.lmd.root.reco.root")
{
    TFile* aFile = new TFile(inFile, "read");
    TTree* aTree = (TTree*)aFile->Get("er");
    TH2F* aHisto = new TH2F(
            "aHisto", 
            "PID Plot;Edep Thick detector, MeV;Edep Thin detector, MeV",
            280, 0, 70, 280, 0, 8.75
            );
    TCanvas* aCanvas = new TCanvas("aCanvas", "aCanvas", 700, 700);
    aCanvas->cd();
    aTree->Draw(
"TelescopeParticle_Telescope_1_SingleSi_SSD20_1_XTelescope_1_SingleSi_SSD_1_Y_1000020040.fEdepInThinStation:\
TelescopeParticle_Telescope_1_SingleSi_SSD20_1_XTelescope_1_SingleSi_SSD_1_Y_1000020040.fEdepInThickStation\
>>aHisto",
"",
"colz"
        );
    aCanvas->SaveAs("pid.png");

    return 0;
}
