#if !defined(__CLING__)

//standard ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
// #include "TH1.h"
// #include "TH2.h"
// #include "TClonesArray.h"
#include "TCut.h"

// #include "FairAnaSelector.h"

//ExpertRoot includes
// #include "ER10Heto8HeEventHeader.h"
// #include "ERNDDigi.h"

#include <iostream>

#endif

#include "../reco/CreateCut.C"
#include "../reco/CreateCutGamma.C"
// #include "cut.C"

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

	OpenFilesAndTrees("../sim_digi_8_1nNDVac.root", "../sim_digi_8_1nNDSteel.root",
			"../reco/sim_digi_8_1nNDVac.target.root", "../reco/sim_digi_8_1nNDSteel.target.root");


	CreateCut();		//cut_8he - события лежат на гистограмме dE-E, соответствующей событиям 8Не
	CreateCutGamma();
	// cutGamma();

	//threshold of energy deposit in stilben; in MeV
	// Double_t energyThr = 1.5;
	Double_t energyThr = 0.11;

	TCut cutReaction = "cut_8he*"
					"("
					"TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()==1"
					" && TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()==1"
					")";

	TCut cutStilbenEdep = Form("NDDigi.fEdep>%f", energyThr);

	TCut cutNoGamma = "!cut_gamma";

	Long64_t numberReactionCut;
	Long64_t numberStilbenEdepCut;
	Long64_t numberStilbenNoGammaCut;
	Long64_t numberStilbenEdepNoGammaCut;
	
	
	Long64_t number; 
	TString condition;


	// TCanvas *c3 = new TCanvas();
	// c3->Divide(3,2);


	Info("ND_1n_analysis", "Number of events under different conditions");
	std::cout << "\tenergy deposit in stilben > " << energyThr << " MeV" << std::endl;
	std::cout << "\tnumber of particles reconstructed in annual detector equal to 1" << std::endl;
	std::cout << "\tnumber of particles reconstructed in square detector equal to 1" << std::endl;
	std::cout << "\tgraphical cut for 8He in square detector employed" << std::endl;

	Info("ND_1n_analysis", "Table:");

	std::cout << "\tMult\t&\tEvents" << std::endl;


	for (size_t i = 0; i < 5; i++)
	{
		// c3->cd(i+1);

		TCut cutStilbenMult = Form("NDTrack@.size()==%zu", i+1);

		condition.Form("cut_8he && NDDigi.fEdep>%f"
					// " && NDDigi@.size()==%zu"
					// " && NDTrack.fEdep>1.5"
					" && NDTrack@.size()==%zu"
					" && TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()==1"
					" && TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()==1"
					// " && cut_8he"
					, energyThr
					, i+1);
		// std::cout << condition << std::endl;
		// number = tree_simV->Draw("NDDigi.fEdep", condition, "goff");
		// number = tree_simV->Draw("NDTrack.fEdep", condition, "goff");
		// number = tree_simV->Draw("NDTrack@.size()", condition, "goff");
		number = tree_simV->Draw("NDDigi.fChannel", condition, "goff");
		numberReactionCut = tree_simV->Draw("NDDigi.fChannel",
								cutReaction + cutStilbenMult,
								"goff");
		numberStilbenEdepCut = tree_simV->Draw("NDDigi.fChannel",
								cutReaction + cutStilbenEdep + cutStilbenMult,
								"goff");
		//warning: due to bug in ROOT, order of cuts is important
		numberStilbenNoGammaCut = tree_simV->Draw("NDDigi.fChannel",
								cutNoGamma + cutReaction + cutStilbenMult,
								"goff");
		numberStilbenEdepNoGammaCut = tree_simV->Draw("NDDigi.fChannel",
								cutNoGamma + cutStilbenEdep + cutReaction + cutStilbenMult,
								"goff");
		// number = tree_simV->Draw("NDDigi.fChannel", condition, "");
		std::cout << "\t" << i+1 
				<< "\t&\t"  << number 
				<< "\t&\t"  << numberStilbenEdepCut 
				<< "\t&\t"  << numberReactionCut 
				<< "\t&\t"  << numberStilbenNoGammaCut 
				<< "\t&\t"  << numberStilbenEdepNoGammaCut 
				<< std::endl;

	}

	return;


	TCanvas *c1 = new TCanvas();
	c1->Divide(3,2);

	c1->cd(1);
	// tree_simV->Draw("NDDigi.fChannel", "");
	// tree_simV->Draw("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()", "");
	tree_simV->SetMarkerStyle(20);
	tree_simV->SetMarkerSize(1.);
	tree_simV->SetMarkerSize(.5);

	TCut cutP = "CUTG*(NDDigi.fEdep>1.5)";

	number = tree_simV->Draw("NDDigi.fLightYield:NDDigi.fEdep",
				// "NDDigi.fEdep>1.5"
				// "&&  cut_gamma1"
				// "&&  cut_g"
				// " && CUTG"
				// "cut_gamma1"
				// "CUTG"
				// " && NDDigi.fEdep>1.5"
				// "CUTG*(NDDigi.fEdep>1.5)"
				cutNoGamma + "NDDigi.fEdep>1.5"
				// "NDDigi.fEdep>1.5" + cutNoGamma
				// "(NDDigi.fEdep>1.5)*CUTG"
				// , "col"
	);
	// cut_gamma->Draw("same");
	// cutg->Draw("same");
	std::cout << "\t" << number << std::endl;
	// tree_simV->Scan("NDDigi.fLightYield:NDDigi.fEdep",
	// 			// "NDDigi.fEdep>1.5"
	// 			// " && cut_gamma"
	// 			" cut_gamma1"
	// );

	// return;

	c1->cd(2);
	tree_simV->Draw("NDDigi@.size()", "");

	c1->cd(3);
	// tree_simV->Draw("NDDigi.fEdep", "NDDigi.fEdep>0.");
	tree_simV->Draw("NDDigi.fEdep", "NDDigi.fEdep>1.5 && NDDigi@.size()==1");
	// tree_simV->Scan("NDDigi.fEdep", "NDDigi.fEdep>1.5 && NDDigi@.size()==1");

	c1->cd(6);
	tree_simV->Draw("NDTrack@.size()", 
				"NDTrack.fEdep>1.5"
				, "");
	
	tree_simV->SetLineColor(kRed);
	tree_simV->Draw("NDTrack@.size()",
				"NDTrack.fEdep>1.5"
				" && cut_8he"
				, "same");
	tree_simV->SetLineColor(kBlack);

	c1->cd(4);
	number = tree_simV->Draw("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThinStation"
   					":TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThickStation",
					""
					" NDTrack@.size()==2"
					// " && NDTrack.fEdep>1.5"
					" && NDDigi.fEdep>1.5"
					);

	std::cout << number << std::endl;

	tree_simV->SetMarkerColor(kRed);
	number = tree_simV->Draw("TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThinStation"
   					":TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080.fEdepInThickStation",
					"cut_8he"
					" && TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()==1"
					" && TelescopeParticle_Telescope_he8_SingleSi_SSD20_XTelescope_he8_SingleSi_SSD20_1_Y_1000020080@.size()==1"
					" && NDTrack@.size()==2"
					" && NDTrack.fEdep>1.5"
					, "same");

	std::cout << number << std::endl << std::endl;

	c1->cd(5);
	tree_simV->Draw("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()", "");
	// tree_recoV->Draw("TelescopeParticle_Telescope_proton_DoubleSi_R_XY_1000010010@.size()", "same");
	// 


	// TCanvas *c2 = new TCanvas();
	// c2->Divide(3,2);


	for (size_t i = 0; i < 5; i++)
	{
		// c2->cd(i+1);
		// condition.Form("NDDigi.fEdep>1.5 && NDDigi@.size()==%zu", i+1);
		condition.Form("NDDigi.fEdep>1.5"
					" && NDTrack.fEdep>1.5"
					" && NDTrack@.size()==%zu"
					, i+1);
		// std::cout << condition << std::endl;
		// number = tree_simV->Draw("NDDigi.fEdep", condition, "goff");
		// number = tree_simV->Draw("NDTrack.fEdep", condition, "goff");
		// number = tree_simV->Draw("NDTrack@.size()", condition, "goff");
		number = tree_simV->Draw("NDDigi.fChannel", condition, "goff");
		std::cout << "\t\t" << number << std::endl;

		if (i==3) {
			// tree_simV->Scan("NDTrack@.size():NDTrack.fEdep:NDDigi.fChannel", condition, "goff");
		}
	}

	std::cout << std::endl;
}