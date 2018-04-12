#pragma once

class SIM_PBApplyBoundaryCondition : public SIM_PBBoundaryCondition
{
public:
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	
	//GETSET_DATA_FUNCS_S(SIM_NAME_CHANNELS, Channels);
	//GETSET_DATA_FUNCS_I(SIM_NAME_BOUNDARYMODE, Mode);
	//GETSET_DATA_FUNCS_V3(SIM_NAME_BOUNDARYVAL, Values);
	
	//GETSET_DATA_FUNCS_V3(SIM_BOUNDARYMASK_NAME, Mask);
	
	//GETSET_DATA_FUNCS_S(SIM_NAME_EDGEGROUP, Group);

	virtual	bool	applyBoundaryConditions(SIM_Pebble&, Patch&, UT_String, Page&) const;

protected:
	explicit SIM_PBApplyBoundaryCondition(const SIM_DataFactory *factory) : BaseClass(factory) {};
	virtual ~SIM_PBApplyBoundaryCondition() {};

private:
	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBApplyBoundaryCondition, SIM_PBBoundaryCondition, "Pebble Apply Boundary Condition Data", getDopDescription());
};
