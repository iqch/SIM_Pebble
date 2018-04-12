#pragma once

class SIM_Pebble;

class SIM_PBBoundaryCondition : public SIM_Data, public SIM_OptionsUser
{
public:
	virtual	bool	applyBoundaryConditions(SIM_Pebble&, Patch&, UT_String, Page&) const { return true; };

protected:
	explicit SIM_PBBoundaryCondition(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual ~SIM_PBBoundaryCondition() {};

private:
	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBBoundaryCondition, SIM_Data, "Pebble Boundary Condition Data", getDopDescription());
};
