#ifndef _DAnalysisUtilities_
#define _DAnalysisUtilities_

#include <deque>
#include <set>

#include "TSystem.h"

#include "DKinematicData.h"
#include "DParticleCombo.h"

using namespace std;

class DAnalysisUtilities
{
	public:
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_MissingP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, int locUpToStepIndex, set<size_t> locUpThroughIndices, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, bool locUseKinFitDataFlag) const;
		TLorentzVector Calc_FinalStateP4(const DParticleCombo* locParticleComboWrapper, size_t locStepIndex, set<size_t> locToIncludeIndices, map<unsigned int, set<Int_t> >& locSourceObjects, bool locUseKinFitDataFlag) const;

		bool Get_IsPolarizedBeam(int locRunNumber, bool& locIsPARAFlag) const; //RCDB environment must be setup!!
		bool Get_PolarizationAngle(int locRunNumber, int& locPolarizationAngle) const; //RCDB environment must be setup!!
		bool Get_CoherentPeak(int locRunNumber, double& locCoherentPeak, bool locIsPolarizedFlag) const; //RCDB environment must be setup!!
		double Get_BeamBunchPeriod(int locRunNumber); //CCDB environment must be setup!!
		double Get_AccidentalScalingFactor(int locRunNumber, double locBeamEnergy, bool locIsMC = false); //CCDB environment must be setup!!
		double Get_AccidentalScalingFactorError(int locRunNumber, double locBeamEnergy, bool locIsMC = false); //CCDB environment must be setup!!
		double Get_DeltaT_RF(int locRunNumber, const TLorentzVector locBeamX4_Measured, const DParticleCombo* locParticleComboWrapper); //CCDB environment must be setup!!
		int Get_RelativeBeamBucket(int locRunNumber, const TLorentzVector locBeamX4_Measured, const DParticleCombo* locParticleComboWrapper); //CCDB environment must be setup!!
		
		double Get_BeamEndpoint(int locRunNumber);
		vector< pair<double,double>> Get_EnergyTAGM(int locRunNumber);
		vector< pair<double,double>> Get_EnergyTAGH(int locRunNumber);
		int Get_ColumnTAGM(int locRunNumber, double locBeamEnergy);
		int Get_CounterTAGH(int locRunNumber, double locBeamEnergy);

		double Calc_ProdPlanePhi_Pseudoscalar(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locMesonP4) const;
		double Calc_DecayPlanePsi_Vector_2BodyDecay(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locBaryonP4, const TLorentzVector& locMesonP4, const TLorentzVector& locMesonProduct1P4, double& locDecayPlaneTheta) const;
		double Calc_DecayPlanePsi_Vector_3BodyDecay(double locBeamEnergy, Particle_t locTargetPID, const TLorentzVector& locBaryonP4, const TLorentzVector& locMesonP4, const TLorentzVector& locMesonProduct1P4, const TLorentzVector& locMesonProduct2P4, double& locDecayPlaneTheta) const;
		
		std::tuple<double,double> Calc_vanHoveCoord(TLorentzVector locXP4, TLorentzVector locYP4, TLorentzVector locZP4);
		std::tuple<double,double,double> Calc_vanHoveCoordFour(TLorentzVector locVec1P4, TLorentzVector locVec2P4, TLorentzVector locVec3P4, TLorentzVector locVec4P4);		

		set<set<size_t> > Build_IndexCombos(const DParticleComboStep* locParticleComboStepWrapper, deque<Particle_t> locToIncludePIDs) const;
		double* Generate_LogBinning(int locLowest10Power, int locHighest10Power, unsigned int locNumBinsPerPower, int& locNumBins) const;


	private:

		bool Handle_Decursion(int& locParticleIndex, deque<size_t>& locComboDeque, deque<int>& locResumeAtIndices, deque<deque<size_t> >& locPossibilities) const;

		//Cache so we only have to look up from CCDB once per run number. CCDB environment must be setup!!
		map< int, vector<double> > dAccidentalScalingFactor_Cache;
		map< int, double >         dBeamBunchPeriod_Cache; 
		map< int, double >         dBeamEndpoint_Cache; 
		map< int, vector< pair<double,double>> > dEnergyTAGM_Cache;
		map< int, vector< pair<double,double>> > dEnergyTAGH_Cache;
		
};

#endif // _DAnalysisUtilities_
