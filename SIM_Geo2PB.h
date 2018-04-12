#pragma once

class SIM_Geo2PB : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);
	GETSET_DATA_FUNCS_S(SIM_NAME_GEODATA, GeoDataName);


protected:
	explicit	SIM_Geo2PB(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_Geo2PB() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_Geo2PB, SIM_SingleSolver, "Geo2PB", getDopDescription());
};
