// Added a fully-automated nodegraph generation system

#ifndef CONTINGENCY_NODEPLACER_H
#define CONTINGENCY_NODEPLACER_H
#ifdef _WIN32
#pragma once
#endif

enum PROGRESS_STEPS
{
	PROGRESS_STEP_NONE = -1,
	PROGRESS_STEP_START,
	PROGRESS_STEP_NODEBLOCK_1,
	PROGRESS_STEP_NODEBLOCK_2,
	PROGRESS_STEP_NODEBLOCK_3,
	PROGRESS_STEP_NODEBLOCK_4,
	PROGRESS_STEP_NODEBLOCK_5,
	PROGRESS_STEP_NODEBLOCK_6,
	PROGRESS_STEP_NODEBLOCK_7,
	PROGRESS_STEP_NODEBLOCK_8,
	PROGRESS_STEP_SAVENODES,
	PROGRESS_STEP_FINISH
};

enum TRACE_DIRECTIONS
{
	TRACE_NEG_X,
	TRACE_POS_X,
	TRACE_NEG_Y,
	TRACE_POS_Y,
	TRACE_NEG_Z,	// directly downwards
	TRACE_POS_Z,	// directly upwards

	NUM_TRACE_DIRECTIONS
};

class Node
{
public:
	Node( Vector vecPosition = Vector(0, 0, 0) );
	~Node();

	Vector GetPosition( void ){ return m_vecPosition; }
	void SetPosition( Vector newPosition ) { m_vecPosition = newPosition; }

private:
	Vector m_vecPosition;
};

class NodeGenerator
{
public:
	NodeGenerator();
	~NodeGenerator();

	int GetCurrentProgressStep( void ) { return m_iCurrentProgressStep; }
	void SetCurrentProgressStep( int newCurrentProgressStep ) { m_iCurrentProgressStep = newCurrentProgressStep; }

	void Update( void );

protected:
	void ReportCurrentProgress( void );
	static void StopReportingProgress( void );
	static void ShowViewPortPanelToAll( const char *name, bool bShow, KeyValues *data );
	const char *GetCurrentProgressStepMsg( void );

	void ProcessCurrentProgressStep( void );
	void ProcessNodeBlock( bool bNegativeX, bool bNegativeY, bool bNegativeZ );
	void ProcessNode( Node *pNode );
	void SaveNodes( void );

	CUtlVector<Node*> m_CompleteNodeList;

private:
	int m_iCurrentProgressStep;
	bool m_bShouldReportProgressToClient;
	float m_flReportProgressToClientBuffer;

	// These variables are used in our ProcessNodeBlock function
	float x;
	float y;
	float z;

	// These variables are used in our ProcessNode function
	int dir;
	Vector vecTestPos;
	trace_t result;
	int i;
	Node *pOtherNode;

	friend Node;	// added so nodes can easily access m_CompleteNodeList
};

extern NodeGenerator *g_pNodeGenerator;	// there can be only one!

#endif	// CONTINGENCY_NODEPLACER_H
