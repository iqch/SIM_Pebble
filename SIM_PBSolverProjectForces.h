#pragma once

class SIM_PBSolverProjectForces : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverProjectForces(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverProjectForces() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverProjectForces, SIM_SingleSolver, "PB Solver Project Forces", getDopDescription());

	// THREADING PART
	SIM_Pebble* m_pebble;

	THREADED_METHOD(SIM_PBSolverProjectForces, true, solve)

	void solvePartial(const UT_JobInfo &info);

	vector<const SIM_Force*> m_forces;

	SIM_Object* m_object;

	void solve(int start, int end);

};
