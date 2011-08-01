#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char* NODE_STARTING_ENTITY = "info_player_deathmatch";
static const float NODE_HEIGHT_BUFFER = 5.0f;

static enum CHILDREN_NODE_DIRECTIONS
{
	// ordered clockwise from forward
	CHILDREN_NODE_NO_DIRECTION = -1,	// usually indicates some sort of error occured
	CHILDREN_NODE_DIRECTION_NORTH = 0,
	CHILDREN_NODE_DIRECTION_EAST,
	CHILDREN_NODE_DIRECTION_SOUTH,
	CHILDREN_NODE_DIRECTION_WEST,

	NUM_CHILDREN_NODE_DIRECTIONS
};

static const float CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS_AMOUNT = 100.0f;
static const Vector CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS[NUM_CHILDREN_NODE_DIRECTIONS] =
{
	// order corresponds to CHILDREN_NODE_DIRECTIONS enumerator
	Vector( CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS_AMOUNT, 0.0f, 0.0f ),
	Vector( 0.0f, CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS_AMOUNT, 0.0f ),
	Vector( -CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS_AMOUNT, 0.0f, 0.0f ),
	Vector( 0.0f, -CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS_AMOUNT, 0.0f )
};

static int GetOppositeDirection( int direction )
{
	switch ( direction )
	{
	case CHILDREN_NODE_DIRECTION_NORTH:
		return CHILDREN_NODE_DIRECTION_SOUTH;
		break;
	case CHILDREN_NODE_DIRECTION_EAST:
		return CHILDREN_NODE_DIRECTION_WEST;
		break;
	case CHILDREN_NODE_DIRECTION_SOUTH:
		return CHILDREN_NODE_DIRECTION_NORTH;
		break;
	case CHILDREN_NODE_DIRECTION_WEST:
		return CHILDREN_NODE_DIRECTION_EAST;
		break;
	}

	return CHILDREN_NODE_NO_DIRECTION;
}

class Node
{
public:
	Node::Node();
	Node::~Node();

	void AddChildNode( Node* pChildNode, int posInList ) { m_ChildrenNodeList[posInList] = pChildNode; }

	Vector GetPosition( void ){ return m_vecPosition; }
	void SetPosition( Vector newPosition ) { m_vecPosition = newPosition; }

	Node *GetParentNode( void ){ return m_ParentNode; }
	void SetParentNode( Node *newParentNode ) { m_ParentNode = newParentNode; }

	int GetDirectionToParent( void ){ return m_iDirToParent; }
	void SetDirectionToParent( int newDirToParent ) { m_iDirToParent = newDirToParent; }

protected:
	Node* m_ChildrenNodeList[NUM_CHILDREN_NODE_DIRECTIONS];

private:
	Vector m_vecPosition;
	Node *m_ParentNode;
	int m_iDirToParent;
};

Node::Node()
{
	m_vecPosition = Vector( 0, 0, 0 );
	m_ParentNode = NULL;
	m_iDirToParent = CHILDREN_NODE_NO_DIRECTION;
}

Node::~Node()
{
}

class NodeGenerator
{
public:
	NodeGenerator::NodeGenerator();
	NodeGenerator::~NodeGenerator();

	bool GetStartingPosition( Vector &vecPlacementPos );
	bool GetPlacementPosition( const Vector vecSamplingPos, Vector &vecPlacementPos );
	bool GenerateChildNodes( Node *pNode );
	void InitiateNodeGeneration( void );

//protected:
	CUtlVector<Node*> m_CompleteNodeList;

private:
	Node *m_pOriginNode;
};

NodeGenerator::NodeGenerator()
{
	m_pOriginNode = NULL;
}

NodeGenerator::~NodeGenerator()
{
	m_CompleteNodeList.PurgeAndDeleteElements();	// if we're going down, so are all of our nodes!
}

bool NodeGenerator::GetStartingPosition( Vector &vecPlacementPos )
{
	CBaseEntity *pStartingEntity = gEntList.FindEntityByClassname( NULL, NODE_STARTING_ENTITY );
	if ( !pStartingEntity )
		return false;

	if ( GetPlacementPosition(pStartingEntity->GetAbsOrigin(), vecPlacementPos) )
		return true;

	return false;
}

bool NodeGenerator::GetPlacementPosition( const Vector vecSamplingPos, Vector &vecPlacementPos )
{
	Vector vecToGround = Vector( vecSamplingPos.x,
		vecSamplingPos.y,
		vecSamplingPos.z - 99999.0f );

	trace_t result;
	UTIL_TraceLine( vecSamplingPos, vecToGround, MASK_PLAYERSOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &result );
	if ( result.startsolid || !result.DidHit() )
		return false;

	vecPlacementPos = Vector( result.endpos.x, result.endpos.y, result.endpos.z + NODE_HEIGHT_BUFFER );

	return true;
}

bool NodeGenerator::GenerateChildNodes( Node *pParentNode )
{
	if ( !pParentNode )
		return false;	// no null pointers!

	for ( int dir = CHILDREN_NODE_DIRECTION_NORTH; dir < NUM_CHILDREN_NODE_DIRECTIONS; dir++ )
	{
		if ( dir == pParentNode->GetDirectionToParent() )
			continue;	// do not go back from where we came

		Node *pChildNode = new Node();
		if ( !pChildNode )
			continue;

		Vector vecNodePos = pParentNode->GetPosition();
		Vector vecChildNodeSamplingPos = Vector( vecNodePos.x + CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS[dir].x,
			vecNodePos.y + CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS[dir].y,
			vecNodePos.z + CHILDREN_NODE_DIRECTIONS_VECTOROFFSETS[dir].z );

		Vector vecChildNodePos;
		if ( !GetPlacementPosition(vecChildNodeSamplingPos, vecChildNodePos) )
		{
			delete pChildNode;	// unable to obtain a valid placement position, so the entire node is invalid; remove it
			continue;
		}
		
		pChildNode->SetPosition( vecChildNodePos );
		pChildNode->SetDirectionToParent( GetOppositeDirection(dir) );
		pChildNode->SetParentNode( pParentNode );
		pParentNode->AddChildNode( pChildNode, dir );
		m_CompleteNodeList.AddToTail( pChildNode );

		NDebugOverlay::Line( pParentNode->GetPosition(), pChildNode->GetPosition(), 0, 0, 255, true, 99999.0f );
		NDebugOverlay::Box( pChildNode->GetPosition(), Vector(-5, -5, -5), Vector(5, 5, 5), 0, 255, 0, 255, 99999.0f );
		
		//GenerateChildNodes( pChildNode );
	}

	return true;
}

void NodeGenerator::InitiateNodeGeneration( void )
{
	Vector vecOriginPos;
	if ( !GetStartingPosition(vecOriginPos) )
		return;

	m_pOriginNode = new Node();
	if ( !m_pOriginNode )
		return;

	m_pOriginNode->SetPosition( vecOriginPos );
	// it has no parent, it has no direction to parent...it's the origin node!

	m_CompleteNodeList.AddToTail( m_pOriginNode );

	NDebugOverlay::Box( m_pOriginNode->GetPosition(), Vector(-5, -5, -5), Vector(5, 5, 5), 255, 0, 0, 255, 99999.0f );

	GenerateChildNodes( m_pOriginNode );	// well, here goes everything
}

void CC_InitiateNodeGeneration( void )
{
	NodeGenerator *pNodeGenerator = new NodeGenerator();
	if ( !pNodeGenerator )
		return;

	pNodeGenerator->InitiateNodeGeneration();
}
static ConCommand initiatenodegeneration( "initiatenodegeneration", CC_InitiateNodeGeneration, "Initiates node generation", 0 );

void CC_NodeStep( void )
{
	NodeGenerator *pNodeGenerator = new NodeGenerator();
	if ( !pNodeGenerator )
		return;

	for ( int i = 0; i < pNodeGenerator->m_CompleteNodeList.Count(); i++ )
		pNodeGenerator->GenerateChildNodes( pNodeGenerator->m_CompleteNodeList[i] );
}
static ConCommand nodestep( "nodestep", CC_NodeStep, "nodestep", 0 );
