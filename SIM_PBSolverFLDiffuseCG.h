#pragma once

class SIM_PBBoundaryCondition;

class SIM_PBSolverFLDiffuseCG : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_F(SIM_NAME_DIFFUSE, Diffuse);
	GETSET_DATA_FUNCS_I(SIM_NAME_ITERATIONS, Iterations);
	GETSET_DATA_FUNCS_S(SIM_NAME_CHANNELS, Channels);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverFLDiffuseCG(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverFLDiffuseCG() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverFLDiffuseCG, SIM_SingleSolver, "PBFL Solver Diffuse v2", getDopDescription());

	THREADED_METHOD(SIM_PBSolverFLDiffuseCG, true, solve);

	void solvePartial(const UT_JobInfo &info);

	SIM_Pebble* m_pebble;

	float m_timestep;

	float m_diffuse;

	UT_StringList m_channels;

	int m_iterations;

	vector<SIM_PBBoundaryCondition*> m_bcs;
};
