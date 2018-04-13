#pragma once

// PRE-DEINES

#ifdef LINUX
#define DLLEXPORT
#else
#define WIN32
#define DLLEXPORT __declspec(dllexport)
#endif

#define MAKING_DSO

#define SIZEOF_VOID_P 8

#define _USE_MATH_DEFINES

#define SESI_LITTLE_ENDIAN 1

#define HBOOST_ALL_NO_LIB

#define UT_DSO_TAGINFO "\"PEBBLE CORE SYSTEM\""

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#define _SCL_SECURE_NO_WARNINGS

//#define EIGEN_MALLOC_ALREADY_ALIGNED 0
//#define FBX_ENABLED 1
//#define OPENCL_ENABLED 1
//#define OPENVDB_ENABLED 1


// CRT
#include <vector>
//#include <sstream>

#include <map>
#include <string>

using namespace std;

// HDK
#include <OP/OP_OperatorTable.h>

#include <PRM/PRM_Include.h>

#include <DOP/DOP_PRMShared.h>
#include <DOP/DOP_InOutInfo.h>
#include <DOP/DOP_Operator.h>
#include <DOP/DOP_Engine.h>
#include <DOP/DOP_Node.h>
#include <DOP/DOP_SubNet.h>

#include <SIM/SIM_DataFilter.h>
#include <SIM/SIM_Relationship.h>
#include <SIM/SIM_RelationshipGroup.h>
//#include <SIM/SIM_Force.h>
#include <SIM/SIM_OptionsUser.h>
#include <SIM/SIM_PRMShared.h>
#include <SIM/SIM_DopDescription.h>
#include <SIM/SIM_OptionsUser.h>
#include <SIM/SIM_SingleSolver.h>
#include <SIM/SIM_Geometry.h>
#include <SIM/SIM_GeometryCopy.h>
#include <SIM/SIM_Solver.h>
//#include <SIM/SIM_MultipleSolver.h>
#include <SIM/SIM_Engine.h>
#include <SIM/SIM_Options.h>
#include <SIM/SIM_Object.h>
#include <SIM/SIM_ObjectArray.h>
#include <SIM/SIM_DopDescription.h>
//#include <SIM/SIM_Random.h>
//#include <SIM/SIM_RandomTwister.h>
#include <SIM/SIM_Position.h>
//#include <SIM/SIM_Result.h>
#include <SIM/SIM_GuideShared.h>
#include <SIM/SIM_Force.h>
#include <SIM/SIM_RelationshipSink.h>
#include <SIM/SIM_RelationshipSource.h>

#include <UT/UT_HashTable.h>
#include <UT/UT_Hash.h>
#include <UT/UT_IStream.h>
#include <UT/UT_PtrArray.h>
#include <UT/UT_Lock.h>

//#include <SYS/SYS_Floor.h>
#include <SHOP/SHOP_VopShaderAdapter.h>
#include <VOP/VOP_Node.h>

#include <UT/UT_Vector3.h>
#include <UT/UT_WorkBuffer.h>
#include <UT/UT_ThreadedAlgorithm.h>
#include <UT/UT_Quaternion.h>

#include <GU/GU_DetailHandle.h>
#include <GU/GU_Detail.h>
#include <GU/GU_RayIntersect.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimPolySoup.h>
#include <GU/GU_PrimMesh.h>

//#include <GEO/GEO_PrimVolume.h>
//#include <GU/GU_PrimVolume.h>

#include <VEX/VEX_Error.h>
#include <CVEX/CVEX_Context.h>
#include <CVEX/CVEX_Value.h>

#include <SHOP/SHOP_Node.h>

#include <GU/GU_Detail.h>
#include <GU/GU_PrimVolume.h>
#include <GA/GA_SaveOptions.h>
#include <OBJ/OBJ_Node.h>
#include <SOP/SOP_Node.h>

#include <PRM/PRM_Include.h>
#include <PRM/PRM_Template.h>
#include <PRM/PRM_SpareData.h>
#include <PRM/PRM_ChoiceList.h>

#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Caller.h>

#include <UT/UT_Vector3.h>
#include <UT/UT_Ramp.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_FastRandom.h>

#include <UT/UT_SparseMatrix.h>

#include <VEX/VEX_Error.h>
#include <CVEX/CVEX_Context.h>
#include <CVEX/CVEX_Value.h>

#include <SHOP/SHOP_Node.h>

#include<OP/OP_Director.h>

class Page : public vector<UT_Vector3> //Array
{
	bool m_extended;
	
public:
	UT_String m_name;
	int dim[2];
	bool m_projected = false;
	const UT_Vector3& get(int i, int j) const
	{
		int I = i;
		int J = j;

		while (I < 0) I += dim[0];
		while (J < 0) J += dim[1];

		I %= dim[0];
		J %= dim[1];

		return (*this)[I*dim[0] + J];
	};

	UT_Vector3& get(int i, int j)
	{
		int I = i;
		int J = j;

		while (I < 0) I += dim[0];
		while (J < 0) J += dim[1];

		I %= dim[0];
		J %= dim[1];

		return (*this)[I*dim[0] + J];
	};

	UT_Vector3 get(UT_Vector3 C) const
	{
		float x = C[0];
		float y = C[1];

		int DX = dim[0];
		int DY = dim[1];

		if (m_extended)
		{
			DX -= 2;
			DY -= 2;
		};

		float dx = 1.0 / DX;
		float dy = 1.0 / DY;

		if (m_extended)
		{
			if (x > 1 + 0.5 * dx) x = 1 + 0.5 * dx;
			if (y > 1 + 0.5 * dy) y = 1 + 0.5 * dy;

			if (x < -0.5 * dx) x = -0.5 * dx;
			if (y < -0.5 * dy) y = -0.5 * dy;
		}
		else
		{
			if (x > 1 - 0.5 * dx) x = 1 - 0.5 * dx;
			if (y > 1 - 0.5 * dy) y = 1 - 0.5 * dy;

			if (x < 0.5 * dx) x = 0.5 * dx;
			if (y < 0.5 * dy) y = 0.5 * dy;
		};

		int DDX = floor(x / dx + 0.5);
		int DDY = floor(y / dy + 0.5);

		float QX = DDX*dx - 0.5*dx;
		float QY = DDY*dy - 0.5*dy;

		float ddx = x - QX; ddx /= dx;
		float ddy = y - QY; ddy /= dy;

		UT_Vector3 U0 = get(DDX, DDY)*(1 - ddx);
		if (ddx != 0) U0 += get(DDX + 1, DDY)*ddx;
		UT_Vector3 U1 = get(DDX, DDY + 1)*(1 - ddx);
		if (ddx != 0) U1 += get(DDX + 1, DDY + 1)*ddx;

		UT_Vector3 res = U0*(1 - ddy) + U1*ddy;

		return res;
	};

	UT_Vector3 d() const
	{
		return UT_Vector3(
			1.0 / (dim[0] - (m_extended ? 2 : 0)),
			1.0 / (dim[1] - (m_extended ? 2 : 0)),
			0.0
		);
	};

	void cross(UT_Vector3& C, const UT_Vector2& D, UT_Vector3& CLR) const
	{
		float x = C[0];
		float y = C[1];

		int DX = dim[0];
		int DY = dim[1];

		if (m_extended)
		{
			DX -= 2;
			DY -= 2;
		};

		float dx = 1.0 / DX;
		float dy = 1.0 / DY;

		int DDX = floor(x / dx + 0.5);
		int DDY = floor(y / dy + 0.5);

		float QX = DDX*dx - 0.5*dx;
		float QY = DDY*dy - 0.5*dy;

		float ddx = x - QX; ddx /= dx;
		float ddy = y - QY; ddy /= dy;

		float tgx = D[0] < 0 ? 0.0 : 1.0;
		float tgy = D[1] < 0 ? 0.0 : 1.0;

		float X = ddx;
		float Y = ddy;

		//QQ[0] = QY; QQ[1] = QY;
		//dd[0] = ddx; dd[1] = ddy;
		//DD[0] = DDX; DD[1] = DDY;

		if (ddx*ddy == 0) // SIDE/CORNER CASE
		{
			if (ddx == 0 && D[0] < 0) { DDX--; QX -= dx; X = 1; };

			if (ddy == 0 && D[1] < 0) { DDY--; QY -= dy; Y = 1; };

			CLR = UT_Vector3(0, 0, 1);
		}
		else
		{
			CLR = UT_Vector3(1, 0, 1);
		};

		if (DDX == 0 && D[0] < 0) tgx = 0.5;
		if (DDX == DX && D[0] > 0) tgx = 0.5;
		if (DDY == 0 && D[1] < 0) tgy = 0.5;
		if (DDY == DY && D[1] > 0) tgy = 0.5;

		if (D[0] == 0)
		{
			C[0] = QX;
			C[1] = QY + tgy*dy;
			return;
		};

		if (D[1] == 0)
		{
			C[0] = QX + tgx*dx;
			C[1] = QY;
			return;
		};

		float tx = (tgx - X) / D[0];
		float ty = (tgy - Y) / D[1];

		float t = min(tx, ty);

		C[0] = QX + (X + D[0] * t)*dx;
		C[1] = QY + (Y + D[1] * t)*dy;

		return;
	};

	explicit Page(int i, int j, bool extended = false) : m_extended(extended)
	{
		dim[0] = i; dim[1] = j;
		resize(capacity());
		zero();
	};

	void zero()
	{
		for (int k = 0; k < capacity(); k++) (*this)[k] = UT_Vector3(0, 0, 0);
	};

	int capacity() const { return dim[0] * dim[1]; };

	operator UT_Vector3*() { return data(); };
	const UT_Vector3* getConstData() const { return data(); };

	void apply(const Page& src, int x0 = 0, int y0 = 0)
	{
		for (int i = 0; i < dim[0]; i++)
			for (int j = 0; j < dim[1]; j++)
			{
				get(i, j) = src.get(i + x0, j + y0);
			};
	};

	Page(const Page& src) : vector<UT_Vector3>(src)
	{
		dim[0] = src.dim[0]; dim[1] = src.dim[1];
		m_name = src.m_name;
		m_extended = src.m_extended;
		m_projected = src.m_projected;
	};

	Page& operator=(const Page& src)
	{
		dim[0] = src.dim[0]; dim[1] = src.dim[1];
		resize(capacity());
		apply(src);

		m_name = src.m_name;

		m_extended = src.m_extended;
		m_projected = src.m_projected;

		return *this;
	};

private :
	Page()
	{
		// SHOULD NOT BE!
		const char* MSG = "SHOULD NOT TO HAPPEN!";

		throw(MSG);

	};
};

class Patch : public vector<Page*>
{
public:

	int32 id;

	int32 dim[2]; // dimensions

	int32 f[4]; // adjacents

	UT_StringArray chs; // vars names

	Patch(int x, int y, UT_StringArray names = UT_StringArray())
	{
		dim[0] = x; dim[1] = y;
		declarePages(names);
	};

	virtual ~Patch() { for (Page* p : *this) delete p; };

	Patch(const Patch& src) : vector<Page*>(src)
	{
		id = src.id;
		dim[0] = src.dim[0]; dim[1] = src.dim[1];
		memcpy(&f, &src.f, 4 * sizeof(int32));

		clear();

		for (Page* p : src)
		{
			Page* _P = new Page(*p);
			push_back(_P);
		};

		chs += src.chs;
	};

	void declarePages(const UT_StringArray& names)
	{
		for (int i = 0; i < names.size(); i++)
		{
			UT_String name;
			name = names[i];
			declarePage(name);
		};
	};

	void declarePage(const UT_String& name)
	{
		if (chs.find(name) != -1) return;

		// NEW
		Page* _p = new Page(dim[0], dim[1]);
		Page& p = *_p;
		p.m_name = name;
		p.zero();

		push_back(_p);

		chs.append(name);
	};

	Page& getPrimVar(const UT_String& name)
	{
		// ALREADY DEFINED
		int index = chs.find(name);
		if (index != -1)
		{
			Page& found = *(*this)[index];
			return found;
		};

		const char* MSG = "SHOULD NOT HAPPEN!";
		throw(MSG);
	};

	const Page& getPrimVar(const UT_String& name) const
	{
		// ALREADY DEFINED
		int index = chs.find(name);
		if (index != -1)
		{
			const Page& found = *(*this)[index];
			return found;
		};

		const char* MSG = "SHOULD NOT HAPPEN!";
		throw(MSG);
	};

	const UT_Vector3 centroid() const
	{
		const Page& P = getPrimVar(UT_String("P"));

		int x0 = 0, x1 = 0;
		if (dim[0] % 2 == 1)
		{
			x0 = (dim[0] - 1) / 2;
			x1 = x0;
		}
		else
		{
			x0 = dim[0] / 2 - 1;
			x1 = x0 + 1;
		};

		int y0 = 0, y1 = 0;
		if (dim[1] % 2 == 1)
		{
			y0 = (dim[1] - 1) / 2;
			y1 = y0;
		}
		else
		{
			y0 = dim[1] / 2 - 1;
			y1 = y0 + 1;
		};

		UT_Vector3 C0 = P.get(x0, y0);
		UT_Vector3 C1 = P.get(x1, y1);

		return (C0 + C1) / 2;
	};

	int64 memsize() const
	{
		int64 m = sizeof(Patch);
		for (const Page* p : *this) m += p->capacity() * 3 * sizeof(float);
		return m;
	};

	void kill(int n)
	{
		Page* l = (*this)[n];
		delete l;

		(*this)[n] = (*this).back();
		resize(chs.size() - 1);

		chs[n] = chs.last();
		chs.setSize(chs.size() - 1);
	};

	void killLast()
	{
		Page* l = (*this).back();
		delete l;

		int n = chs.size() - 1;
		chs.setSize(n);
		resize(n);
	};

	int reflect(int id) const
	{
		for (int i = 0; i < 4; i++)
		{
			if (id != f[i]) continue;
			return i;
		};

		return -1;
	};

private :
	Patch()
	{
		// SHOULD NOT HAPPEN!
		const char* MSG = "SHOULD NOT HAPPEN!";

		throw(MSG);
	};
};

typedef vector<Page*> Proxy;

////////////////////////////////////
// NAMES

#define SIM_NAME_ACTIVATE "activation"
#define SIM_NAME_DETAIL	"detail"
#define SIM_NAME_CHANNELS	"channels"
#define SIM_NAME_METRICS	"metrics"
#define SIM_NAME_DATAGEOMETRY	"pbdataname"

#define SIM_NAME_VOP "vop_path"

#define SIM_NAME_UPPER "upper"

#define SIM_NAME_TIMESTEP "timestep"
#define SIM_NAME_SEED "seed"
#define SIM_NAME_AMOUNT "amount"
#define SIM_NAME_SHOWV "showvguide"
#define SIM_NAME_MARK "mark"

#define SIM_NAME_PATCH "patch"
#define SIM_NAME_ALL "all"

#define SIM_RELATTRIB_NAME "rel_attrib"

#define SIM_NAME_BOUNDARYMODE "pbbc_mode"
#define SIM_NAME_BOUNDARYVAL "pbbc_values"
#define SIM_NAME_BOUNDARYMASK "pbbc_mask"

#define SIM_NAME_EDGEGROUP "edge_group"

#define SIM_NAME_DISSIPATE "dissipate"
#define SIM_NAME_DIFFUSE "diffuse"

#define SIM_NAME_ITERATIONS "solver_iterations"

#define SIM_NAME_GEODATA "geometry"

// PRMS

extern PRM_Name	 theActivateName;

extern PRM_Name	 theDataNameName;
extern PRM_Default theDataNameDef;

extern PRM_Name	 theGeoDataNameName;
extern PRM_Default	theGeoDataNameDef;

extern PRM_Name	 theDetailName;
extern PRM_Range theDetailRng;

extern PRM_Name	 theUpperName;
extern PRM_Range theUpperRng;

extern PRM_Name	 theChannelsName;
extern PRM_Default theChannelsDef;

extern PRM_Name	 theTimestepName;
extern PRM_Default theTimestepDef;
extern PRM_Name	 theAmountName;
extern PRM_Name	 theSeedName;

extern PRM_Name	 theMarkName;
extern PRM_Default theMarkDef;
extern PRM_Range theMarkRng;

extern PRM_Name	 theShowVGuideName;

extern PRM_Name	 thePatchName;
extern PRM_Range thePatchRng;
extern PRM_Name	 theAllName;

extern PRM_Name	 theVOPPathName;

extern PRM_Name	 theRelAttribName;
extern PRM_Default  theRelAttribDef;

extern PRM_Name	 theModeName;
extern PRM_Name	 theValuesName;
extern PRM_Name	 theMaskName;

extern PRM_Name modeNames[];

extern PRM_ChoiceList modeMenu;

extern PRM_Name	 theDissipateName;
extern PRM_Name	 theDiffuseName;
extern PRM_Name	 theIterationsName;

extern PRM_Name	 theEdgeGroupName;
extern PRM_Default  theEdgeGroupDef;

extern const char* ch_base_names[];


/////////////////////////////////

#define EDGE_CLAUSE "__edge__"
#define CLOSURE_GROUP "__closure__"
#define CLOSURE_CORNERS_GROUP "__closure_corners__"

void pb2geo(GU_Detail* gdp, const Patch& pb);
void geo2pb(vector<Patch*>& p, const GU_Detail* gdp);

void edges2geo(GU_Detail* gdp, const map<string, vector< UT_Vector2i> >&);
void geo2edges(map<string, vector< UT_Vector2i> >&, const GU_Detail* gdp);

void addClosure(GU_Detail* gdp, const vector<Patch*>& PB);

struct traceStat
{
	int max_deep = 8;
	int max_path = 50;
	int state = 0;
	bool back = true;

	UT_Matrix2F w = UT_Matrix2F(1);
};

bool trace(const vector<Patch*>& PEBBLE, UT_Vector3 coords, /*UT_Vector2 d,*/ float & length,
	UT_Vector3Array & path, UT_Vector3Array & colors, //UT_Vector3Array & normals,
	vector<Page*>& Ps, vector<Page*>& Vs,
	//vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns, 
	vector<Page*>& Rels, UT_Lock& lock, traceStat& res);

Page* getExpandedPrimVar(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name);
bool getExpandedPrimVarDiv(const vector<Patch*>& PEBBLE, const Patch & pb, Page* DPDU, Page* DPDV);
Page* getExpandedPrimVarProjected(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name);

bool declareEntity(vector<Patch*>& PEBBLE, UT_String name);
bool applyProxy(vector<Patch*>& PEBBLE, UT_String name);
