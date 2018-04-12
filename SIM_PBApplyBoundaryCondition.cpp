/////////////////////////////////////////////////////
// +--BOUNDARY CONDITION DATA : GEOMETRIC

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBBoundaryCondition.h"

#include "SIM_PBApplyBoundaryCondition.h"

const SIM_DopDescription *SIM_PBApplyBoundaryCondition::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {
		//PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_STRING,		1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_ORD,   PRM_Template::PRM_EXPORT_MAX, 1,&theModeName, 0, &modeMenu),
		PRM_Template(PRM_FLT_J,			3, &theValuesName, PRMzeroDefaults),
		//PRM_Template(PRM_FLT_J,			3, &theMaskName, PRMoneDefaults),
		PRM_Template(PRM_STRING,	1, &theEdgeGroupName, &theEdgeGroupDef),
		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pebble_apply_bc", "Pebble Apply Boundary Condition ", "PebbleApplyBC", classname(), theTemplates);
	return &theDopDescription;
};

// +--
bool SIM_PBApplyBoundaryCondition::applyBoundaryConditions(SIM_Pebble& ps, Patch & pb, UT_String name, Page& p) const
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return true;

	UT_StringArray channels;
	int mode;
	UT_Vector3 values;
	//UT_Vector3 mask;
	UT_StringArray groups;
	bool activate;


	const SIM_Options& opts = getOptions();
	UT_Options::iterator it = opts.begin();
	while (it != opts.end())
	{
		UT_String name = it.name().c_str();
		const UT_OptionEntry* e = it.entry();

		if (name == SIM_NAME_ACTIVATE)
		{
			activate = e->getOptionB();
			if (!activate) return false;
			++it; continue;
		};

		if (name == SIM_NAME_BOUNDARYMODE)
		{
			mode = e->getOptionI();
			++it; continue;
		};

		if (name == SIM_NAME_BOUNDARYVAL)
		{
			values = e->getOptionV3();
			++it; continue;
		};

		if (name == SIM_NAME_EDGEGROUP)
		{
			UT_String str = e->getOptionS().c_str();
			str.tokenize(groups);
			++it; continue;
		};

		if (name == SIM_NAME_CHANNELS)
		{
			UT_String str = e->getOptionS().c_str();
			str.tokenize(channels);
			++it; continue;
		};

		++it;
	};

	bool all_chan = (channels.find("*") != -1);

	if (!all_chan && channels.find(name) == -1) return -1;

	bool all_grp = (groups.find("*") != -1);

	for (const pair<string, vector<UT_Vector2i>> val : ps.m_edges)
	{
		if (!all_grp)
		{
			if (groups.find(val.first.c_str()) == -1) continue;
		};

		const vector<UT_Vector2i>& E = val.second;

		for (const UT_Vector2i& e : E)
		{
			if (e[0] != pb.id) continue;

			const Patch& pb = *ps.m_P[e[0]];
			const Page& P = pb.getPrimVar("P");

			const Page& dPdu = pb.getPrimVar("dPdu");
			const Page& dPdv = pb.getPrimVar("dPdv");
			const Page& N = pb.getPrimVar("N");

			int _e = e[1];

			int vtcnt = pb.dim[_e % 2]-2;

			int dx[] = { 1,0,1,0 };

			int x[] = { 1,	pb.dim[0],	1,			1 };
			int y[] = { 1,	1,			pb.dim[1],	1 };

			int X = x[_e];
			int Y = y[_e];

			int rx = (1 - dx[_e])*(x[_e] == 1 ? -1 : 1);
			int ry = (dx[_e])*(y[_e] == 1 ? -1 : 1);
			
			if (mode == 0) //PRM_Name("const", "GEO:Constant")
			{
				for (int i = 0; i < vtcnt+2; i++)
				{
					p.get(X, Y) = values;
					p.get(X+rx, Y+ry) = values;

					X += dx[_e];
					Y += 1 - dx[_e];
				};
				continue;
			};

			if (mode == 1) //PRM_Name("comp", "GEO:Components")
			{
				for (int i = 1; i < vtcnt; i++)
				{
					UT_Vector3 _DPDU = dPdu.get(X - 1, Y - 1);
					UT_Vector3 _DPDV = dPdv.get(X - 1, Y - 1);
					//UT_Vector3 NN = N.get(X - 1, Y - 1);

					UT_Vector3 DPDU, DPDV;

					switch (_e)
					{
					case 0:
						DPDU = _DPDU;
						DPDV = _DPDV;
						break;
					case 1:
						DPDU = _DPDV;
						DPDV = -_DPDU;
						break;
					case 2:
						DPDU = -_DPDU;
						DPDV = -_DPDV;
						break;
					case 3:
						DPDU = -_DPDV;
						DPDV = _DPDU;
						break;
					};

					DPDU.normalize();
					DPDV.normalize();

					DPDV -= DPDV.dot(DPDU)*DPDU;

					DPDV.normalize();

					//NN.normalize();

					UT_Vector3 V = p.get(X, Y);

					V = DPDU*V.dot(DPDU)*values[0];

					float VPRJ = V.dot(DPDV);
					if(values[1] > 0) V += DPDV*VPRJ*values[1]; // +NN*V.dot(NN)*values[2];
					else
					{
						if (VPRJ > 0) V -= DPDV*VPRJ*values[1];
						else V += DPDV*VPRJ*values[1];
					};


					p.get(X, Y) = V; 
					p.get(X + rx, Y + ry) = values;
					
					if (i == 1)
					{
						p.get(X - dx[_e] + rx, Y - (1 - dx[_e]) + ry) = V;
						p.get(X - dx[_e] + rx, Y - (1 - dx[_e]) + ry) = V;
					};
					
					if (i == vtcnt - 1)
					{
						p.get(X + dx[_e] + rx, Y + (1 - dx[_e]) + ry) = V;
						p.get(X + dx[_e] + rx, Y + (1 - dx[_e]) + ry) = V;
					};

					X += dx[_e];
					Y += 1 - dx[_e];
				};
				continue;
			};

			//PRM_Name("srcg", "Gradient:Source Constant"),
			//PRM_Name("srcg", "Gradient:Source Components"),
			//PRM_Name("srcc", "Gradient:Collide Constant"),
			//PRM_Name("srcc", "Gradient:Collide Component"),

		};


	};


	return true;
};
