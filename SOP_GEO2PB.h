#pragma once

class SOP_GEO2PB : public SOP_Node
{
	static int	*myIndirect;
public:
	SOP_GEO2PB(OP_Network *net, const char *name, OP_Operator *op) : SOP_Node(net, name, op)
	{
		if (myIndirect == NULL) myIndirect = allocIndirect(1);
	};
	virtual ~SOP_GEO2PB(void) {};
	virtual OP_ERROR		 cookMySop(OP_Context &context);
private:

	int m_primcnt;

	THREADED_METHOD1(SOP_GEO2PB, m_primcnt > 15, build,
		int, nsamples)

		void buildPartial(int nsamples, const UT_JobInfo &info);

	UT_StringArray m_baseNames;

	Far::PatchMap* m_patchmap;
	Far::PatchTable const *m_patchTable;

	//vector<UT_IntArray> m_edgemap;
	vector<vector<int32> > m_edgemap;

	Page* m_uvf;

	vector<VertexPosition> m_vertsP;
	vector<VertexColor> m_vertsCd;

	vector<UT_Vector2IArray> m_edges;
	//UT_StringArray m_edgeNames;

	UT_Array<const GA_EdgeGroup *> m_glist;

	const GA_Primitive* m_prim;
	const GU_PrimPolySoup* m_ps;

	const GU_Detail* m_input;
};

class OP_GEO2PBOperator : public OP_Operator
{
public:
	OP_GEO2PBOperator();

	static OP_Node* creator(OP_Network *net, const char *name, OP_Operator *op) { return new SOP_GEO2PB(net, name, op); }

	virtual ~OP_GEO2PBOperator() {};
};
