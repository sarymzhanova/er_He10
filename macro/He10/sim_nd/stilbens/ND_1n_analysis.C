#if !defined(__CLING__)

//standard ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
// #include "TH1.h"
// #include "TH2.h"
// #include "TClonesArray.h"
// #include "TCut.h"

// #include "FairAnaSelector.h"

//ExpertRoot includes
// #include "ER10Heto8HeEventHeader.h"
// #include "ERNDDigi.h"

#include <iostream>

#endif

#include "../reco/CreateCut.C"

TFile *f_simV,*f_simS,*f_recoV,*f_recoS,*f_outV,*f_outS;
TTree *tree_simV, *tree_recoV;
TTree *tree_simS, *tree_recoS;


void OpenFilesAndTrees(TString inFilelabV, TString inFilelabS,
						TString inFileRecoV, TString inFileRecoS) {
	
	f_simV = new TFile(inFilelabV,"READ");
	tree_simV = (TTree*)f_simV -> Get("er");
	f_recoV = new TFile(inFileRecoV,"READ");
	tree_recoV = (TTree*)f_recoV -> Get("er");
	tree_simV->AddFriend(tree_recoV);
	
	// f_outV = new TFile(outFileStilbensV,"RECREATE");
	
	f_simS = new TFile(inFilelabS,"READ");
	tree_simS = (TTree*)f_simS -> Get("er");
	f_recoS = new TFile(inFileRecoS,"READ");
	tree_recoS = (TTree*)f_recoS -> Get("er");
	tree_simS->AddFriend(tree_recoS);
	
	// f_outS = new TFile(outFileStilbensS,"RECREATE");

}

void ND_1n_analysis() {

	// OpenFilesAndTrees("../sim_digi_8_1nNDVac.root", "../sim_digi_8_1nNDVac.root",
	// 		"../reco/reco_sim_digi_8_1nNDVac.root", "../reco/reco_sim_digi_8_1nNDVac.root");

	OpenFilesAndTrees("../sim_digi_8_1nNDVac.root", "../sim_digi_8_1nNDVac.root",
			"../reco/sim_digi_8_1nNDVac.target.root", "../reco/sim_digi_8_1nNDVac.target.root");


	CreateCut();		//cut_8he - события лежат на гистограмме dE-E, соответствующей событиям 8Не


	TCanvas *c1 = new TCanvas();
	c1->Divide(3,2);

	c1->cd(1);
	tree_simV->Draw("NDDigi.fChannel", "");

	c1->cd(2);
	tree_simV->Draw("NDDigi@.size()", "");

	c1->cd(3);
	// tree_simV->Draw("NDDigi.fEdep", "NDDigi.fEdep>0.");
	tree_simV->Draw("NDDigi.fEdep", "NDDigi.fEdep>1.5 && NDDigi@.size()==1");

	c1->cd(6);
	tree_simV->Draw("NDDigi.fEdep:NDTrack.fEdep", "NDDigi.fEdep>1.5", "text");

	c1->cd(4);
	tree_simV->Draw("NDDigi.fLightYield:NDDigi.fEdep", "NDDigi.fEdep>1.1");

	c1->cd(5);
	// tree_simV->Draw("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()", "");
	tree_simV->Draw("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()", "");
	// tree_recoV->Draw("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()", "same");
	// 

	Long64_t number; 
	TString condition;



	for (size_t i = 0; i < 5; i++)
	{
		// condition.Form("NDDigi.fEdep>1.5 && NDDigi@.size()==%zu", i+1);
		condition.Form("NDDigi.fEdep>1.5 && NDTrack@.size()==%zu", i+1);
		// std::cout << condition << std::endl;
		number = tree_simV->Draw("NDDigi.fEdep", condition, "goff");
		std::cout << "\t\t" << number << std::endl;
	}

	std::cout << endl;

	for (size_t i = 0; i < 5; i++)
	{
		condition.Form("NDDigi.fEdep>1.5"
					// " && NDDigi@.size()==%zu"
					" && NDTrack@.size()==%zu"
					" && TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()==1"
					" && TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()==1"
					" && cut_8he"
					,
				i+1);
		// std::cout << condition << std::endl;
		number = tree_simV->Draw("NDDigi.fEdep", condition, "goff");
		std::cout << "\t\t" << number << std::endl;
	}
	


	// cout << tree_simV->Draw("NDDigi.fLightYield:NDDigi.fEdep", "NDDigi.fEdep>1.1 && NDDigi@.size()==1") << endl;




}