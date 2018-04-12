#pragma once

class SIM_PBSolverRelationships : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_S(SIM_RELATTRIB_NAME, RelAttrib);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverRelationships(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverRelationships() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverRelationships, SIM_SingleSolver, "PB Solver Apply Relationships", getDopDescription());

	// THREADING PART
	SIM_Pebble* m_pebble;

	THREADED_METHOD(SIM_PBSolverRelationships, true, solve);

	void solvePartial(const UT_JobInfo &info);

	UT_String m_relAttr;

	vector<GU_RayIntersect*> m_colliders;
	vector<GU_RayIntersect*> m_sources;
};
