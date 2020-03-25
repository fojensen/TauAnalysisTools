import FWCore.ParameterSet.Config as cms

process = cms.Process("writeGBRForests")

process.maxEvents = cms.untracked.PSet(
	input = cms.untracked.int32(1) # CV: needs to be set to 1 so that GBRForestWriter::analyze method gets called exactly once
)

process.source = cms.Source("EmptySource")

process.gbrForestWriter = cms.EDAnalyzer("GBRForestWriter",
	jobs=cms.VPSet(
		cms.PSet(
			categories=cms.VPSet(
				cms.PSet(
					inputFileName=cms.string('MVAAnalysis_BDT.weights.xml'),
               inputFileType=cms.string("XML"),
					inputVariables=cms.vstring(
                     'pt',
                     'TMath::Abs(eta)',
                     'chargedIsoPtSum',
                     'neutralIsoPtSum',
                     'puCorrPtSum',
                     'photonPtSumOutsideSignalCone',
                     'decayMode',
                     'signalGammaCands_size',
                     'isolationGammaCands_size',
                     'isoCands_deta',
                     'isoCands_dphi',
                     'isoCands_dr',
                     'sigCands_deta',
                     'sigCands_dphi',
                     'sigCands_dr',
                     'eRatio',
                     'TMath::Sign(+1., dxy)',
                     'TMath::Sqrt(TMath::Abs(dxy))',
                     'TMath::Abs(dxy_Sig)',
                     'TMath::Sign(+1., ip3d)',
                     'TMath::Sqrt(TMath::Abs(ip3d))',
                     'TMath::Abs(ip3d_Sig)',
                     'hasSecondaryVertex',
                     'flightLength',
                     'flightLengthSig',
                     'leadingTrackNormChi2',
                     'hasSecondaryVertex?thetaGJ-thetaGJmax:-5.',
                     'TMath::Sign(+1., leadChargedHadrCand_dxy)',
                     'TMath::Sqrt(TMath::Abs(leadChargedHadrCand_dxy))',
                     'TMath::Abs(leadChargedHadrCand_dxysig)'
						),
					spectatorVariables=cms.vstring(
					   'eta'
               ),
					#gbrForestName=cms.string("tauIdMVAIsoDBoldDMwLT2018")  # consistent with TauAnalysisTools/TauAnalysisTools/macros/plotTauIdMVAEfficiency_and_FakeRate.C
				   gbrForestName=cms.string("tauIdMVAIsoPhase2")
            )
			),
			outputFileType=cms.string("GBRForest"),
			outputFileName=cms.string("gbrDiscriminationByIsolationMVAPhase2.root")  #arbitrary
		),
	)
)

process.p=cms.Path(process.gbrForestWriter)

