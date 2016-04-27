#include "DHistogramActions.h"

void DHistogramAction_ParticleComboKinematics::Initialize(void)
{
	string locHistName, locHistTitle, locStepName, locStepROOTName, locParticleName, locParticleROOTName;

	//CREATE MAIN FOLDER
	string locDirName = "Hist_ParticleComboKinematics";
	if(dActionUniqueString != "")
		locDirName += string("_") + dActionUniqueString;
	(new TDirectoryFile(locDirName.c_str(), locDirName.c_str()))->cd();

	//Steps
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		ostringstream locStepName;
		locStepName << "Step" << loc_i << "__" << locStep->Get_StepName();
		string locStepROOTName = locStep->Get_StepROOTName();
		Particle_t locInitialPID = locStep->Get_InitialPID();
		bool locStepDirectoryCreatedFlag = false;

		//parent //reconstructed &: is gamma, or don't need measured
		bool locIsBeamFlag = (locInitialPID == Gamma);
		if((locStep->Get_InitialParticle() != NULL) && (!dUseMeasuredFlag || locIsBeamFlag))
		{
			(new TDirectoryFile(locStepName.str().c_str(), locStepName.str().c_str()))->cd();
			locStepDirectoryCreatedFlag = true;

			locParticleName = ParticleType(locInitialPID);
			locDirName = locIsBeamFlag ? "Beam" : locParticleName;
			(new TDirectoryFile(locDirName.c_str(), locDirName.c_str()))->cd();
			Create_Hists(loc_i, locStepROOTName, locInitialPID, locIsBeamFlag);
			gDirectory->cd("..");
		}

		//kids
		for(size_t loc_j = 0; loc_j < locStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(locStep->Get_FinalParticle(loc_j) == NULL)
				continue; //not reconstructed at all

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locStep->Get_DecayStepIndex(loc_j);
			if(dUseMeasuredFlag && (locDecayStepIndex != -2))
				continue; //not measured

			Particle_t locPID = locStep->Get_FinalPID(loc_j);
			if(dHistMap_P[loc_i].find(locPID) != dHistMap_P[loc_i].end())
				continue; //pid already done

			if(!locStepDirectoryCreatedFlag)
			{
				(new TDirectoryFile(locStepName.str().c_str(), locStepName.str().c_str()))->cd();
				locStepDirectoryCreatedFlag = true;
			}

			locParticleName = ParticleType(locPID);
			(new TDirectoryFile(locParticleName.c_str(), locParticleName.c_str()))->cd();

			Create_Hists(loc_i, locStepROOTName, locPID);
			gDirectory->cd("..");
		} //end of particle loop

		// Vertex
		string locInitParticleROOTName = ParticleName_ROOT(locInitialPID);
		string locInitParticleName = ParticleType(locInitialPID);
		if((loc_i == 0) || IsDetachedVertex(locInitialPID))
		{
			if(!locStepDirectoryCreatedFlag)
			{
				(new TDirectoryFile(locStepName.str().c_str(), locStepName.str().c_str()))->cd();
				locStepDirectoryCreatedFlag = true;
			}

			locHistName = "StepVertexZ";
			locHistTitle = (loc_i == 0) ? ";Production Vertex-Z (cm)" : string(";") + locInitParticleROOTName + string(" Decay Vertex-Z (cm)");
			dHistMap_StepVertexZ[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

			locHistName = "StepVertexYVsX";
			locHistTitle = (loc_i == 0) ? "Production Vertex" : locInitParticleROOTName + string(" Decay Vertex");
			locHistTitle += string(";Vertex-X (cm);Vertex-Y (cm)");
			dHistMap_StepVertexYVsX[loc_i] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);

			locHistName = "StepVertexT";
			locHistTitle = (loc_i == 0) ? ";Production Vertex-T (ns)" : string(";") + locInitParticleROOTName + string(" Decay Vertex-T (ns)");
			dHistMap_StepVertexT[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumTBins, dMinT, dMaxT);
		}

		if((loc_i != 0) && IsDetachedVertex(locInitialPID))
		{
			if(!locStepDirectoryCreatedFlag)
			{
				(new TDirectoryFile(locStepName.str().c_str(), locStepName.str().c_str()))->cd();
				locStepDirectoryCreatedFlag = true;
			}

			locHistName = locInitParticleName + string("PathLength");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Path Length (cm)");
			dHistMap_DetachedPathLength[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPathLengthBins, 0.0, dMaxPathLength);

			locHistName = locInitParticleName + string("Lifetime");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Lifetime (ns)");
			dHistMap_DetachedLifetime[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumLifetimeBins, 0.0, dMaxLifetime);

			locHistName = locInitParticleName + string("Lifetime_RestFrame");
			locHistTitle = string(";") + locInitParticleROOTName + string(" Rest Frame Lifetime (ns)");
			dHistMap_DetachedLifetime_RestFrame[loc_i] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumLifetimeBins, 0.0, dMaxLifetime);
		}

		if(locStepDirectoryCreatedFlag)
			gDirectory->cd("..");
	} //end of step loop

	//Return to the base directory
	gDirectory->cd("..");
}

void DHistogramAction_ParticleComboKinematics::Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID, bool locIsBeamFlag)
{
	string locParticleROOTName = ParticleName_ROOT(locPID);
	string locHistName, locHistTitle;
	Particle_t locFillPID = locIsBeamFlag ? Unknown : locPID;

	// Momentum
	double locMaxP = locIsBeamFlag ? dMaxBeamE : dMaxP;
	locHistName = "Momentum";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";p (GeV/c)");
	dHistMap_P[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPBins, dMinP, locMaxP);

	if(!locIsBeamFlag)
	{
		// Theta
		locHistName = "Theta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ");
		dHistMap_Theta[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumThetaBins, dMinTheta, dMaxTheta);

		// Phi
		locHistName = "Phi";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#phi#circ");
		dHistMap_Phi[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumPhiBins, dMinPhi, dMaxPhi);

		// P Vs Theta
		locHistName = "PVsTheta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;p (GeV/c)");
		dHistMap_PVsTheta[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPBins, dMinP, dMaxP);

		// Phi Vs Theta
		locHistName = "PhiVsTheta";
		locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";#theta#circ;#phi#circ");
		dHistMap_PhiVsTheta[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DThetaBins, dMinTheta, dMaxTheta, dNum2DPhiBins, dMinPhi, dMaxPhi);
	}

	// Vertex-Z
	locHistName = "VertexZ";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-Z (cm)");
	dHistMap_VertexZ[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexZBins, dMinVertexZ, dMaxVertexZ);

	// Vertex-Y Vs Vertex-X
	locHistName = "VertexYVsX";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-X (cm);Vertex-Y (cm)");
	dHistMap_VertexYVsX[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNumVertexXYBins, dMinVertexXY, dMaxVertexXY, dNumVertexXYBins, dMinVertexXY, dMaxVertexXY);

	if(!locIsBeamFlag)
	{
		//beta vs p
		locHistName = "BetaVsP";
		locHistTitle = locParticleROOTName + string(";p (GeV/c);#beta");
		dHistMap_BetaVsP[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

		//delta-beta vs p
		locHistName = "DeltaBetaVsP";
		locHistTitle = locParticleROOTName + string(";p (GeV/c);#Delta#beta");
		dHistMap_DeltaBetaVsP[locStepIndex][locFillPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaBetaBins, dMinDeltaBeta, dMaxDeltaBeta);
	}

	// Vertex-T
	locHistName = "VertexT";
	locHistTitle = locParticleROOTName + string(", ") + locStepROOTName + string(";Vertex-T (ns)");
	dHistMap_VertexT[locStepIndex][locFillPID] = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumTBins, dMinT, dMaxT);

	if(locIsBeamFlag)
	{
		// Delta-T (Beam, RF)
		locHistName = "DeltaTRF";
		locHistTitle = string("Beam ") + locParticleROOTName + string(";#Deltat_{Beam - RF} (ns)");
		dBeamParticleHist_DeltaTRF = new TH1I(locHistName.c_str(), locHistTitle.c_str(), dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);

		// Delta-T (Beam, RF) Vs Beam E
		locHistName = "DeltaTRFVsBeamE";
		locHistTitle = string("Beam ") + locParticleROOTName + string(";E (GeV);#Deltat_{Beam - RF} (ns)");
		dBeamParticleHist_DeltaTRFVsBeamE = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTRFBins, dMinDeltaTRF, dMaxDeltaTRF);
	}
}

bool DHistogramAction_ParticleComboKinematics::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locParticleComboStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//initial particle
		Particle_t locInitialPID = locParticleComboStep->Get_InitialPID();
		DKinematicData* locKinematicData = locParticleComboStep->Get_InitialParticle();
		if(locKinematicData != NULL)
		{
			if(locInitialPID == Gamma)
			{
				//check if will be duplicate
				set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][Unknown];
				if(locParticleSet.find(locKinematicData->Get_ID()) == locParticleSet.end())
				{
					double locRFTime = dUseMeasuredFlag ? dParticleComboWrapper->Get_RFTime_Measured() : dParticleComboWrapper->Get_RFTime();
					Fill_BeamHists(locKinematicData, locRFTime);
					locParticleSet.insert(locKinematicData->Get_ID());
				}
			}
			else if(!dUseMeasuredFlag) //decaying particle, but reconstructed so can hist
				Fill_Hists(locKinematicData, loc_i); //derived from everything in the combo: is technically unique
		}

		//VERTEX INFORMATION
		//other than first, skipped if not detached vertex
		TLorentzVector locStepSpacetimeVertex = locParticleComboStep->Get_X4();
		if((loc_i == 0) || IsDetachedVertex(locInitialPID))
		{
			dHistMap_StepVertexZ[loc_i]->Fill(locStepSpacetimeVertex.Z());
			dHistMap_StepVertexYVsX[loc_i]->Fill(locStepSpacetimeVertex.X(), locStepSpacetimeVertex.Y());
			dHistMap_StepVertexT[loc_i]->Fill(locStepSpacetimeVertex.T());
		}

		//DETACHED VERTEX INFORMATION
		if((loc_i != 0) && IsDetachedVertex(locInitialPID))
		{
			int locFromStepIndex = locParticleComboStep->Get_InitDecayFromIndices().first;
			TLorentzVector locFromSpacetimeVertex = dParticleComboWrapper->Get_ParticleComboStep(locFromStepIndex)->Get_X4();
			TLorentzVector locDeltaSpacetime = locStepSpacetimeVertex - locFromSpacetimeVertex;

			double locPathLength = locDeltaSpacetime.Vect().Mag();
			dHistMap_DetachedPathLength[loc_i]->Fill(locPathLength);
			dHistMap_DetachedLifetime[loc_i]->Fill(locDeltaSpacetime.T());

			if(locKinematicData != NULL)
			{
				TLorentzVector locInitialP4 = locKinematicData->Get_P4();
				//below, t, x, and tau are really delta-t, delta-x, and delta-tau
				//tau = gamma*(t - beta*x/c)  //plug in: t = x/(beta*c), gamma = 1/sqrt(1 - beta^2)
				//tau = (x/(beta*c) - beta*x/c)/sqrt(1 - beta^2)  //plug in beta = p*c/E
				//tau = (x*E/(p*c^2) - p*x/E)/sqrt(1 - p^2*c^2/E^2)  //multiply num & denom by E*P, factor x/c^2 out of num
				//tau = (x/c^2)*(E^2 - p^2*c^2)/(p*sqrt(E^2 - p^2*c^2))  //plug in m^2*c^4 = E^2 - p^2*c^2
				//tau = (x/c^2)*m^2*c^4/(p*m*c^2)  //cancel c's & m's
				//tau = x*m/p
				//however, in data, p & m are in units with c = 1, so need an extra 1/c
				double locRestFrameLifetime = locPathLength*ParticleMass(locInitialPID)/(29.9792458*locInitialP4.P()); //tau
				dHistMap_DetachedLifetime_RestFrame[loc_i]->Fill(locRestFrameLifetime);
				//note that tau = hbar / Gamma, hbar = 6.582119E-22 MeV*s, Gamma = Resonance FWHM
			}
		}

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locParticleComboStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//check if measured if not using Kin Fit
			if(dUseMeasuredFlag && locParticleComboStep->Get_DecayStepIndex(loc_j) != -2)
				continue; //not measured

			//check if duplicate
			set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][locKinematicData->Get_PID()];
			if(locParticleSet.find(locKinematicData->Get_ID()) != locParticleSet.end())
				continue;

			Fill_Hists(locKinematicData, loc_i);
			locParticleSet.insert(locKinematicData->Get_ID());
		} //end of particle loop
	} //end of step loop

	return true;
}

void DHistogramAction_ParticleComboKinematics::Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex)
{
	Particle_t locPID = locKinematicData->Get_PID();
	TLorentzVector locP4 = dUseMeasuredFlag ? locKinematicData->Get_P4_Measured() : locKinematicData->Get_P4();
	TLorentzVector locX4 = dUseMeasuredFlag ? locKinematicData->Get_X4_Measured() : locKinematicData->Get_X4();

	double locPhi = locP4.Phi()*180.0/TMath::Pi();
	double locTheta = locP4.Theta()*180.0/TMath::Pi();
	double locP = locP4.P();

	double locBeta_Timing = 0.0;
	if(ParticleCharge(locPID) != 0)
	{
		const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
		if(locChargedTrackHypothesis != NULL)
			locBeta_Timing = dUseMeasuredFlag ? locChargedTrackHypothesis->Get_Beta_Timing_Measured() : locChargedTrackHypothesis->Get_Beta_Timing();
	}
	else
	{
		const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
		if(locNeutralParticleHypothesis != NULL)
			locBeta_Timing = dUseMeasuredFlag ? locNeutralParticleHypothesis->Get_Beta_Timing_Measured() : locNeutralParticleHypothesis->Get_Beta_Timing();
	}
	double locDeltaBeta = locP4.P()/locP4.E() - locBeta_Timing;

	dHistMap_P[locStepIndex][locPID]->Fill(locP);
	dHistMap_Phi[locStepIndex][locPID]->Fill(locPhi);
	dHistMap_Theta[locStepIndex][locPID]->Fill(locTheta);
	dHistMap_PVsTheta[locStepIndex][locPID]->Fill(locTheta, locP);
	dHistMap_PhiVsTheta[locStepIndex][locPID]->Fill(locTheta, locPhi);
	dHistMap_BetaVsP[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
	dHistMap_DeltaBetaVsP[locStepIndex][locPID]->Fill(locP, locDeltaBeta);
	dHistMap_VertexZ[locStepIndex][locPID]->Fill(locX4.Z());
	dHistMap_VertexYVsX[locStepIndex][locPID]->Fill(locX4.X(), locX4.Y());
	dHistMap_VertexT[locStepIndex][locPID]->Fill(locX4.T());
}

void DHistogramAction_ParticleComboKinematics::Fill_BeamHists(const DKinematicData* locKinematicData, double locRFTime)
{
	TLorentzVector locP4 = dUseMeasuredFlag ? locKinematicData->Get_P4_Measured() : locKinematicData->Get_P4();
	TLorentzVector locX4 = dUseMeasuredFlag ? locKinematicData->Get_X4_Measured() : locKinematicData->Get_X4();

	double locP = locP4.P();
	double locDeltaTRF = locX4.T() - (locRFTime + (locX4.Z() - dTargetCenterZ)/29.9792458);

	dHistMap_P[0][Unknown]->Fill(locP);
	dHistMap_VertexZ[0][Unknown]->Fill(locX4.Z());
	dHistMap_VertexYVsX[0][Unknown]->Fill(locX4.X(), locX4.Y());
	dHistMap_VertexT[0][Unknown]->Fill(locX4.T());
	dBeamParticleHist_DeltaTRF->Fill(locDeltaTRF);
	dBeamParticleHist_DeltaTRFVsBeamE->Fill(locP4.E(), locDeltaTRF);
}

void DHistogramAction_ParticleID::Initialize(void)
{
	string locHistName, locHistTitle, locStepName, locStepROOTName, locParticleName, locParticleROOTName;

	//CREATE MAIN FOLDER
	string locDirName = "Hist_ParticleID";
	if(dActionUniqueString != "")
		locDirName += string("_") + dActionUniqueString;
	(new TDirectoryFile(locDirName.c_str(), locDirName.c_str()))->cd();

	//Steps
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		ostringstream locStepName;
		locStepName << "Step" << loc_i << "__" << locStep->Get_StepName();
		string locStepROOTName = locStep->Get_StepROOTName();
		bool locStepDirectoryCreatedFlag = false;

		// final state particles
		for(size_t loc_j = 0; loc_j < locStep->Get_NumFinalParticles(); ++loc_j)
		{
			if(locStep->Get_FinalParticle(loc_j) == NULL)
				continue; //not reconstructed at all

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			Particle_t locPID = locStep->Get_FinalPID(loc_j);
			if(dHistMap_BetaVsP_BCAL[loc_i].find(locPID) != dHistMap_BetaVsP_BCAL[loc_i].end())
				continue; //pid already done

			if(!locStepDirectoryCreatedFlag)
			{
				(new TDirectoryFile(locStepName.str().c_str(), locStepName.str().c_str()))->cd();
				locStepDirectoryCreatedFlag = true;
			}

			locParticleName = ParticleType(locPID);
			(new TDirectoryFile(locParticleName.c_str(), locParticleName.c_str()))->cd();

			Create_Hists(loc_i, locStepROOTName, locPID);
			gDirectory->cd("..");
		} //end of particle loop

		gDirectory->cd("..");
	} //end of step loop

	//Return to the base directory
	gDirectory->cd("..");
}

void DHistogramAction_ParticleID::Create_Hists(int locStepIndex, string locStepROOTName, Particle_t locPID)
{
	string locParticleROOTName = ParticleName_ROOT(locPID);
	string locHistName, locHistTitle;
	
	// deltaT and beta vs p
	locHistName = "DeltaTVsP_BCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL #Delta T (ns)");
	dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
	locHistName = "BetaVsP_BCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL #beta");
	dHistMap_BetaVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

	locHistName = "DeltaTVsP_FCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL #Delta T (ns)");
	dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
	locHistName = "BetaVsP_FCAL";
	locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL #beta");
	dHistMap_BetaVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

	if(ParticleCharge(locPID) != 0) {
		
		locHistName = "DeltaTVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF #Delta T (ns)");
		dHistMap_DeltaTVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumDeltaTBins, dMinDeltaT, dMaxDeltaT);
		locHistName = "BetaVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF #beta");
		dHistMap_BetaVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumBetaBins, dMinBeta, dMaxBeta);

		// dE/dx vs P
		locHistName = "dEdxVsP_CDC";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); CDC dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_CDC[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
	
		locHistName = "dEdxVsP_FDC";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); FDC dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_FDC[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		locHistName = "dEdxVsP_ST";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); ST dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_ST[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		locHistName = "dEdxVsP_TOF";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); TOF dE/dx (MeV/cm) ");
		dHistMap_dEdxVsP_TOF[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNum2DdEdxBins, dMindEdx, dMaxdEdx);
		
		// E/p vs p, theta
		locHistName = "EoverPVsP_BCAL";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); BCAL E/p");
		dHistMap_EoverPVsP_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumEoverPBins, dMinEoverP, dMaxEoverP);
		locHistName = "EoverPVsTheta_BCAL";
		locHistTitle = locParticleROOTName + string(";#theta (degrees); BCAL E/p");
		dHistMap_EoverPVsTheta_BCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 260, 10., 140., dNumEoverPBins, dMinEoverP, dMaxEoverP);
		
		locHistName = "EoverPVsP_FCAL";
		locHistTitle = locParticleROOTName + string(";p (GeV/c); FCAL E/p");
		dHistMap_EoverPVsP_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), dNum2DPBins, dMinP, dMaxP, dNumEoverPBins, dMinEoverP, dMaxEoverP);
		locHistName = "EoverPVsTheta_FCAL";
		locHistTitle = locParticleROOTName + string(";#theta (degrees); FCAL E/p");
		dHistMap_EoverPVsTheta_FCAL[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 120, 0., 12., dNumEoverPBins, dMinEoverP, dMaxEoverP);	
	}
	else {
		locHistName = "ShowerZVsParticleZ";
		locHistTitle = locParticleROOTName + string(";Particle Vertex Z (cm); Shower Vertex Z (cm)");
		dHistMap_ShowerZVsParticleZ[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 200, 0., 200., 200, 0., 200);
		locHistName = "ShowerTVsParticleT";
		locHistTitle = locParticleROOTName + string(";Particle Vertex T (ns); Shower Vertex T (ns)");
		dHistMap_ShowerTVsParticleT[locStepIndex][locPID] = new TH2I(locHistName.c_str(), locHistTitle.c_str(), 200, -100., 100., 200, -100., 100);
	}

}
	
bool DHistogramAction_ParticleID::Perform_Action(void)
{
	for(size_t loc_i = 0; loc_i < dParticleComboWrapper->Get_NumParticleComboSteps(); ++loc_i)
	{
		DParticleComboStep* locParticleComboStep = dParticleComboWrapper->Get_ParticleComboStep(loc_i);

		//final particles
		for(size_t loc_j = 0; loc_j < locParticleComboStep->Get_NumFinalParticles(); ++loc_j)
		{
			DKinematicData* locKinematicData = locParticleComboStep->Get_FinalParticle(loc_j);
			if(locKinematicData == NULL)
				continue; //e.g. a decaying or missing particle whose params aren't set yet

			//-2 if detected, -1 if missing, > 0 if decaying (step where it is the parent)
			int locDecayStepIndex = locParticleComboStep->Get_DecayStepIndex(loc_j);
			if(locDecayStepIndex != -2)
				continue; //not measured

			//check if duplicate
			set<Int_t>& locParticleSet = dPreviouslyHistogrammed[loc_i][locKinematicData->Get_PID()];
			if(locParticleSet.find(locKinematicData->Get_ID()) != locParticleSet.end())
				continue;

			Fill_Hists(locKinematicData, loc_i);
			locParticleSet.insert(locKinematicData->Get_ID());
		} //end of particle loop
	} //end of step loop

	return true;
}

void DHistogramAction_ParticleID::Fill_Hists(const DKinematicData* locKinematicData, size_t locStepIndex)
{
	double locRFTime = dParticleComboWrapper->Get_RFTime_Measured();

	Particle_t locPID = locKinematicData->Get_PID();
	TLorentzVector locP4 = dUseMeasuredFlag ? locKinematicData->Get_P4_Measured() : locKinematicData->Get_P4();
	TLorentzVector locX4 = dUseMeasuredFlag ? locKinematicData->Get_X4_Measured() : locKinematicData->Get_X4();

	double locTheta = locP4.Theta()*180.0/TMath::Pi();
	double locP = locP4.P();

	double locBeta_Timing = 0.0;
	if(ParticleCharge(locPID) != 0)
	{
		const DChargedTrackHypothesis* locChargedTrackHypothesis = dynamic_cast<const DChargedTrackHypothesis*>(locKinematicData);
		if(locChargedTrackHypothesis != NULL) {
			locBeta_Timing = dUseMeasuredFlag ? locChargedTrackHypothesis->Get_Beta_Timing_Measured() : locChargedTrackHypothesis->Get_Beta_Timing();
			double locDeltaT = locX4.T() - locRFTime;

			if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_BCAL) {
				dHistMap_BetaVsP_BCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_TOF) {
				dHistMap_BetaVsP_TOF[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_TOF[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locChargedTrackHypothesis->Get_Detector_System_Timing() == SYS_FCAL) {
				dHistMap_BetaVsP_FCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			
			// dE/dx vs p
			if(locChargedTrackHypothesis->Get_dEdx_CDC() > 0.) 
				dHistMap_dEdxVsP_CDC[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_CDC()*1e6);
			if(locChargedTrackHypothesis->Get_dEdx_FDC() > 0.) 
				dHistMap_dEdxVsP_FDC[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_FDC()*1e6);
			if(locChargedTrackHypothesis->Get_dEdx_ST() > 0.) 
				dHistMap_dEdxVsP_ST[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_ST()*1e3);
			if(locChargedTrackHypothesis->Get_dEdx_TOF() > 0.) 
				dHistMap_dEdxVsP_TOF[locStepIndex][locPID]->Fill(locP,locChargedTrackHypothesis->Get_dEdx_TOF()*1e3);

			// E over P
			if(locChargedTrackHypothesis->Get_Energy_BCAL() > 0.) {
				double locEoverP = locChargedTrackHypothesis->Get_Energy_BCAL()/locP;
				dHistMap_EoverPVsP_BCAL[locStepIndex][locPID]->Fill(locP, locEoverP);
				dHistMap_EoverPVsTheta_BCAL[locStepIndex][locPID]->Fill(locTheta, locEoverP);
			}
			if(locChargedTrackHypothesis->Get_Energy_FCAL() > 0.) {
				double locEoverP = locChargedTrackHypothesis->Get_Energy_FCAL()/locP;
				dHistMap_EoverPVsP_FCAL[locStepIndex][locPID]->Fill(locP, locEoverP);
				dHistMap_EoverPVsTheta_FCAL[locStepIndex][locPID]->Fill(locTheta, locEoverP);
			}
		}
	}
	else
	{
		const DNeutralParticleHypothesis* locNeutralParticleHypothesis = dynamic_cast<const DNeutralParticleHypothesis*>(locKinematicData);
		if(locNeutralParticleHypothesis != NULL) {
			locBeta_Timing = dUseMeasuredFlag ? locNeutralParticleHypothesis->Get_Beta_Timing_Measured() : locNeutralParticleHypothesis->Get_Beta_Timing();
			
			//TLorentzVector locX4_Neutral = dUseMeasuredFlag ? locNeutralParticleHypothesis->Get_X4_Measured() : locKinematicData->Get_X4();
			TLorentzVector locX4_Neutral = locNeutralParticleHypothesis->Get_X4_Measured();
			double locDeltaT = locX4_Neutral.T() - locRFTime;

			TLorentzVector locX4_Shower = locNeutralParticleHypothesis->Get_X4_Shower();
			//TLorentzVector locX4_Measured = locNeutralParticleHypothesis->Get_X4_Measured();
			dHistMap_ShowerZVsParticleZ[locStepIndex][locPID]->Fill(locX4_Neutral.Z(), locX4_Shower.Z());
			dHistMap_ShowerTVsParticleT[locStepIndex][locPID]->Fill(locX4_Neutral.T(), locX4_Shower.T());
			
			if(locNeutralParticleHypothesis->Get_Energy_BCAL() > 0.) {
				dHistMap_BetaVsP_BCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_BCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
			else if(locNeutralParticleHypothesis->Get_Energy_FCAL() > 0.) {
				dHistMap_BetaVsP_FCAL[locStepIndex][locPID]->Fill(locP, locBeta_Timing);
				dHistMap_DeltaTVsP_FCAL[locStepIndex][locPID]->Fill(locP, locDeltaT);
			}
		}
	}
}
