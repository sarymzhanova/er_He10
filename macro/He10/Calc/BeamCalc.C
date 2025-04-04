#if !defined(__CLING__)

//standard ROOT includes
#include "TString.h"
#include "TSystem.h"
#include "TStopwatch.h"

//FairRoot includes
#include "FairPrimaryGenerator.h"

//ExpertRoot includes
#include "ERRunSim.h"
#include "ER10Heto8HeEventHeader.h"
#include "ERBeamDetSetup.h"
#include "ERTelescopeSetup.h"
#include "ERRTelescopeGeoComponentDoubleSi.h"
#include "ERTelescopeDigitizer.h"
#include "ERBeamDetDigitizer.h"
#include "ERCave.h"
#include "ERTarget.h"
#include "ERQTelescopeGeoComponentSingleSi.h"
#include "ERTelescope.h"
#include "ERBeamDet.h"
#include "ERND.h"
#include "ERIonMixGenerator.h"
#include "ERDecay10Heto8He.h"
#include "ERNDDigitizer.h"
#include "FairParRootFileIo.h"

#endif

//Beam parameters
//Point beam along z-axis, zero dispersion 
Double_t beamKinE_AMeV = 500;  // [AMeV]

//Beam type

Int_t Z = 1, A = 1, Q = 1;
TString ionName = "proton";

// Int_t Z = 2, A = 3, Q = 2;
// TString ionName = "3He";

// Int_t Z = 2, A = 4, Q = 2;
// TString ionName = "4He";

// Int_t Z = 6, A = 9, Q = 6;
// TString ionName = "9C";



void BeamCalc(Int_t nEvents = 10000){
    TString outFile;
    outFile.Form("BeamCalc_%s_%.0fAMeV.root",ionName.Data(),beamKinE_AMeV);
    TString parFile= "par.root";
    TString workDirPath = gSystem->Getenv("VMCWORKDIR");
    TString paramFileQTelescope = workDirPath 
             + "/db/QTelescope/QTelescopeParts_beamcalc.xml";
    TString targetGeoFileName = workDirPath + "/geometry/target.be_beamcalc.geo.root";
    TString interactionVol = "targetBeVol";
    Double_t beamStartPosition = -1600.;  // [cm]


    // -----   Timer   --------------------------------------------------------
    TStopwatch timer; 
    timer.Start();
    // -----   Create simulation run   ----------------------------------------
    ERRunSim* run = new ERRunSim();
    run->SetName("TGeant4");              // Transport engine
    run->SetOutputFile(outFile.Data());   // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    run->SetMaterials("media.geo");       // Materials
    //-------- Set MC event header --------------------------------------------
    ERMCEventHeader* header = new ERMCEventHeader();
	run->SetMCEventHeader(header);

    // -----   Create detectors  ----------------------------------------------
    FairModule* cave= new ERCave("CAVE");
	cave->SetGeometryFileName("cave.geo");
	run->AddModule(cave);
    //-------------------------------------------------------------------------

    // -----   Create target  -------------------------------------------------
    FairModule* target = new ERTarget("target", kTRUE, 1);
    target->SetGeometryFileName(targetGeoFileName);
    run->AddModule(target); 
    //-------------------------------------------------------------------------

    // -----  QTelescope Setup ------------------------------------------------
    ERTelescopeSetup* setupQTelescope = ERTelescopeSetup::Instance();
    setupQTelescope->SetXMLParametersFile(paramFileQTelescope);
    setupQTelescope->SetGeoName("QTelescopeTmp");

    Double_t posx_Si = 0.;
    Double_t posy_Si = 0.;
    Double_t posz_Si = 7.;
    TVector3 fZeroRotation(0., 0., 0.);

    ERGeoSubAssembly* assembly_Si = new ERGeoSubAssembly("Telescope_Si", TVector3(posx_Si, posy_Si, posz_Si), fZeroRotation);
    ERQTelescopeGeoComponentSingleSi* det_Si = new ERQTelescopeGeoComponentSingleSi("SingleSi", "SingleSi_beamCalc", 
        TVector3(0., 0., 0.), TVector3(), "X");
    assembly_Si->AddComponent(det_Si); 
    setupQTelescope->AddSubAssembly(assembly_Si);   

    // ------QTelescope -------------------------------------------------------
    Int_t verbose = 0;
    ERTelescope* qtelescope= new ERTelescope("ERTelescope", kTRUE,verbose);
    run->AddModule(qtelescope);

    // -----   Create PrimaryGenerator   --------------------------------------

    if(ionName.Contains("proton")){
        printf("proton is beam\n");
    }
    FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
    
    Double_t kin_energy = beamKinE_AMeV * 1e-3 * A; //GeV

        ERIonMixGenerator* generator = new ERIonMixGenerator(ionName, Z, A, Q, 1);    
        generator->SetKinE(kin_energy);
        generator->SetPSigmaOverP(0.);
        generator->SetThetaSigma(0, 0);
        generator->SetPhiRange(0, 0);
        primGen->AddGenerator(generator);
    
    run->SetGenerator(primGen);
    //-------------------------------------------------------------------------

    //-------Set visualisation flag to true------------------------------------
    // run->SetStoreTraj(kTRUE);   //enables storage of GeoTracks
                                  //to turn on MCTracks go to data/ERStack.cxx
                                  //l.336  FairRootManager::Instance()->Register("MCTrack", "Stack", fTracks,kTRUE);


    //-------Set LOG verbosity  ----------------------------------------------- 
	// FairLogger::GetLogger()->SetLogScreenLevel("DEBUG");
    FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    // Geant tracking: set /tracking/verbose 1 instead of 0
    // in gconfig/g4config.in



    // -----   Initialize simulation run   ------------------------------------
	run->Init();
	Int_t nSteps = -15000;
    // -----   Runtime database   ---------------------------------------------
	Bool_t kParameterMerged = kTRUE;
	FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
	parOut->open(parFile.Data());
	rtdb->setOutput(parOut);
	rtdb->saveOutput();
	rtdb->print();
    TString setup_name = outFile;
	setup_name.Prepend("setup_");
	run->CreateGeometryFile(setup_name);

    // -----   Run simulation  ------------------------------------------------
	run->Run(nEvents);

	// -----   Finish   -------------------------------------------------------
	timer.Stop();
	Double_t rtime = timer.RealTime();
	Double_t ctimer = timer.CpuTime();
	cout << endl << endl;
	cout << "Macro finished succesfully." << endl;
	cout << "Output file is " << outFile << endl;
	cout << "Parameter file is " << parFile << endl;
	cout << "Real time " << rtime << " s, CPU time " << ctimer
             << "s" << endl << endl;
	TDatabasePDG *tdb = TDatabasePDG::Instance();

}