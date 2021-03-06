#include "TauAnalysisTools/TauAnalysisTools/plugins/GBRForestWriter.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

#include "CondFormats/EgammaObjects/interface/GBRForest.h"

#include "TMVA/ClassifierFactory.h"
#include "TMVA/Event.h"
#include "TMVA/Factory.h"
#include "TMVA/MethodBase.h"
#include "TMVA/MethodBDT.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include <TFile.h>

GBRForestWriter::GBRForestWriter(const edm::ParameterSet& cfg)
  : moduleLabel_(cfg.getParameter<std::string>("@module_label"))
{
  edm::VParameterSet cfgJobs = cfg.getParameter<edm::VParameterSet>("jobs");
  for ( edm::VParameterSet::const_iterator cfgJob = cfgJobs.begin();
	cfgJob != cfgJobs.end(); ++cfgJob ) {
    jobEntryType* job = new jobEntryType(*cfgJob);
    jobs_.push_back(job);
  }
}

GBRForestWriter::~GBRForestWriter()
{
  for ( std::vector<jobEntryType*>::iterator it = jobs_.begin();
	it != jobs_.end(); ++it ) {
    delete (*it);
  }
}

void GBRForestWriter::analyze(const edm::Event&, const edm::EventSetup&)
{
  std::cout << "<GBRForestWriter::analyze (moduleLabel = " << moduleLabel_ << ")>:" << std::endl;
  
  for ( std::vector<jobEntryType*>::iterator job = jobs_.begin();
	job != jobs_.end(); ++job ) {   
    std::map<std::string, const GBRForest*> gbrForests; // key = name
    for ( std::vector<categoryEntryType*>::iterator category = (*job)->categories_.begin();
	  category != (*job)->categories_.end(); ++category ) {
      const GBRForest* gbrForest = 0;
      if ( (*category)->inputFileType_ == categoryEntryType::kInputXML ) {
	TMVA::Tools::Instance();
	TMVA::Reader* mvaReader = new TMVA::Reader("!V:!Silent");   
	std::vector<Float_t> dummyVariables;
	for ( vstring::const_iterator inputVariable = (*category)->inputVariables_.begin();
	      inputVariable != (*category)->inputVariables_.end(); ++inputVariable ) {
	  dummyVariables.push_back(0.);
	  mvaReader->AddVariable(inputVariable->data(), &dummyVariables.back());
	}
	for ( vstring::const_iterator spectatorVariable = (*category)->spectatorVariables_.begin();
	      spectatorVariable != (*category)->spectatorVariables_.end(); ++spectatorVariable ) {
	  dummyVariables.push_back(0.);
	  mvaReader->AddSpectator(spectatorVariable->data(), &dummyVariables.back());
	}
	mvaReader->BookMVA((*category)->gbrForestName_.data(), (*category)->inputFileName_.data());
	std::cout << "reading BDT = " << (*category)->gbrForestName_ << " from XML file = " << (*category)->inputFileName_ << "." << std::endl;
	TMVA::MethodBDT* bdt = dynamic_cast<TMVA::MethodBDT*>(mvaReader->FindMVA((*category)->gbrForestName_.data()));	
	if ( !bdt )
	  throw cms::Exception("GBRForestWriter") 
	    << "Failed to load MVA = " << (*category)->gbrForestName_.data() << " from file = " << (*category)->inputFileName_ << " !!\n";
	gbrForest = new GBRForest(bdt);  
	delete mvaReader;
	TMVA::Tools::DestroyInstance();
      } else if ( (*category)->inputFileType_ == categoryEntryType::kInputGBRForest ) {
	TFile* inputFile = new TFile((*category)->inputFileName_.data());
	//gbrForest = dynamic_cast<GBRForest*>(inputFile->Get((*category)->gbrForestName_.data())); // CV: dynamic_cast<GBRForest*> fails for some reason ?!
	std::cout << "reading GBRForest = " << (*category)->gbrForestName_ << " from ROOT file = " << (*category)->inputFileName_ << "." << std::endl;	
	gbrForest = (GBRForest*)inputFile->Get((*category)->gbrForestName_.data());
	delete inputFile;
      } else assert(0);
      if ( !gbrForest ) 
	throw cms::Exception("GBRForestWriter") 
	  << " Failed to load GBRForest = " << (*category)->gbrForestName_.data() << " from file = " << (*category)->inputFileName_ << " !!\n";
      gbrForests[(*category)->gbrForestName_] = gbrForest;
    }
    if ( (*job)->outputFileType_ == jobEntryType::kOutputGBRForest ) {
      TFile* outputFile = new TFile((*job)->outputFileName_.data(), "RECREATE");
      for ( std::map<std::string, const GBRForest*>::iterator gbrForest = gbrForests.begin();
	    gbrForest != gbrForests.end(); ++gbrForest ) {
	std::cout << " writing GBRForest = " << gbrForest->first << " to ROOT file = " << (*job)->outputFileName_ << "." << std::endl;
	outputFile->WriteObject(gbrForest->second, gbrForest->first.data());
      }
      delete outputFile;
    } else if ( (*job)->outputFileType_ == jobEntryType::kOutputSQLLite ) {
      edm::Service<cond::service::PoolDBOutputService> dbService;
      if ( !dbService.isAvailable() ) 
	throw cms::Exception("GBRForestWriter") 
	  << " Failed to access PoolDBOutputService !!\n";
      for ( std::map<std::string, const GBRForest*>::iterator gbrForest = gbrForests.begin();
	    gbrForest != gbrForests.end(); ++gbrForest ) {
	std::string outputRecord = (*job)->outputRecord_;
	if ( gbrForests.size() > 1 ) outputRecord.append("_").append(gbrForest->first);
	std::cout << " writing GBRForest = " << gbrForest->first << " to SQLlite file, record = " << outputRecord << "." << std::endl;
	dbService->writeOne(gbrForest->second, dbService->beginOfTime(), outputRecord);
      }
    } else assert(0);
  }

  std::cout << "done." << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(GBRForestWriter);
