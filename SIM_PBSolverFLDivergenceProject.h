#pragma once

class SIM_PBBoundaryCondition;

class SIM_PBSolverFLDivergenceProject : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_I(SIM_NAME_ITERATIONS, Iterations);
	//GETSET_DATA_FUNCS_S(SIM_NAME_CHANNELS, Channels);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverFLDivergenceProject(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverFLDivergenceProject() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverFLDivergenceProject, SIM_SingleSolver, "PBFL Solver Diffuse", getDopDescription());

	THREADED_METHOD(SIM_PBSolverFLDivergenceProject, true, solve);

	void solvePartial(const UT_JobInfo &info);

	SIM_Pebble* m_pebble;

	//UT_StringList m_channels;

	int m_iterations;

	vector<SIM_PBBoundaryCondition*> m_bcs;
};
