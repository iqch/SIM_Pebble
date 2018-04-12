#pragma once

class SIM_PBSolverVOPProcess : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_S(SIM_NAME_VOP, VopPath);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverVOPProcess(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverVOPProcess() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverVOPProcess, SIM_SingleSolver, "PB Solver VOP Process", getDopDescription());

	int m_primcnt;

	THREADED_METHOD(SIM_PBSolverVOPProcess, m_primcnt > 15, solve);

	void solvePartial(const UT_JobInfo &info);

	SIM_Pebble* m_pebble;

	UT_String m_script;

	UT_StringArray m_chouts;
};
