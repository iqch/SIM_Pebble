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
	const UT_Vector3& get(int i, int j) const;

	UT_Vector3& get(int i, int j);

	UT_Vector3 get(UT_Vector3 C) const;

	UT_Vector3 d() const;

	void cross(UT_Vector3& C, const UT_Vector2& D, UT_Vector3& CLR) const;

	explicit Page(int i, int j, bool extended = false);

	void zero();

	int capacity() const;

	operator UT_Vector3*();
	const UT_Vector3* getConstData() const;

	void apply(const Page& src, int x0 = 0, int y0 = 0);

	Page(const Page& src);

	Page& operator=(const Page& src);

private :
	Page();
};

class Patch : public vector<Page*>
{
public:

	int32 id;

	int32 dim[2]; // dimensions

	int32 f[4]; // adjacents

	UT_StringArray chs; // vars names

	Page *m_N, *m_DPDU, *m_DPDV;

	Patch(int x, int y, UT_StringArray names = UT_StringArray());

	virtual ~Patch();

	Patch(const Patch& src);

	void declarePages(const UT_StringArray& names);

	void declarePage(const UT_String& name);

	Page& getPrimVar(const UT_String& name);

	const Page& getPrimVar(const UT_String& name) const;

	const UT_Vector3 centroid() const;

	int64 memsize() const;

	void kill(int n);

	void killLast();

	int reflect(int id) const;

	UT_Vector3 project(UT_Vector3 V, int i, int j) const;

	UT_Vector3 project(UT_Vector3 V, UT_Vector3 UV) const;

	UT_Vector3 composite(UT_Vector3 V, int i, int j) const;

	UT_Vector3 composite(UT_Vector3 V, UT_Vector3 UV) const;

	UT_Vector2 decompose(UT_Vector3 V, UT_Vector3 UV) const;

	UT_Vector2 decompose(UT_Vector3 V, int i, int j) const;

private :
	Patch();
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
#define SIM_NAME_MINMAGNITUDE "minmagnitude"
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
extern PRM_Name	 theMinMagnitudeName;
extern PRM_Default theMinMagnitudeDef;
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
	//UT_Matrix2F W = UT_Matrix2F(1);

	int id = -1;
};

bool trace(const vector<Patch*>& PEBBLE, UT_Vector3 coords, /*UT_Vector2 d,*/ float & length,
	UT_Vector3Array & path, UT_Vector3Array & colors, //UT_Vector3Array & normals,
	vector<Page*>& Ps, vector<Page*>& Vs,
	vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns, 
	vector<Page*>& Rels, vector<Page*>& Gs, UT_Lock& lock, traceStat& res);

//bool trace_orig(const vector<Patch*>& PEBBLE, UT_Vector3 coords, /*UT_Vector2 d,*/ float & length,
//		UT_Vector3Array & path, UT_Vector3Array & colors, //UT_Vector3Array & normals,
//		vector<Page*>& Ps, vector<Page*>& Vs,
//		//vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns, 
//		vector<Page*>& Rels, UT_Lock& lock, traceStat& res);

Page* getExpandedPrimVar(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name);
Page* getExtrapolatedPrimVar(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name);

//bool getExpandedPrimVarDiv(const vector<Patch*>& PEBBLE, const Patch & pb, Page* DPDU, Page* DPDV);
//Page* getExpandedPrimVarProjected(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name);

bool declareEntity(vector<Patch*>& PEBBLE, UT_String name);
bool applyProxy(vector<Patch*>& PEBBLE, UT_String name);
