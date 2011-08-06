// Added a fully-automated nodegraph generation system

#include "cbase.h"

#include "contingency_nodeplacer.h"
#include "viewport_panel_names.h"
#include "tier0/vprof.h"
#include "utlbuffer.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const float PROGRESS_STEP_PROCESSING_BUFFER = 1.0f;
static const float NODE_STEP_DISTANCE = 100.0f;

NodeGenerator *g_pNodeGenerator = NULL;

Node::Node( Vector vecPosition )
{
	m_vecPosition = vecPosition;

	if ( g_pNodeGenerator && (g_pNodeGenerator->m_CompleteNodeList.Find(this) == -1) )
		g_pNodeGenerator->m_CompleteNodeList.AddToTail( this );
}

Node::~Node()
{
	if ( g_pNodeGenerator )
		g_pNodeGenerator->m_CompleteNodeList.FindAndRemove( this );
}

NodeGenerator::NodeGenerator()
{
	m_iCurrentProgressStep = PROGRESS_STEP_NONE;
	m_bShouldReportProgressToClient = true;
	m_flReportProgressToClientBuffer = 0.0f;
	
	// These variables are used in our ProcessNodeBlock function
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;

	// These variables are used in our ProcessNode function
	dir = 0;
	i = 0;
	pOtherNode = NULL;

	g_pNodeGenerator = this;	// assign to singleton
}

NodeGenerator::~NodeGenerator()
{
	m_CompleteNodeList.PurgeAndDeleteElements();	// if we're going down, so are all of our nodes!
}

void NodeGenerator::Update( void )	// called every game frame
{
	VPROF( "NodeGenerator::Update" );

	if ( m_iCurrentProgressStep != PROGRESS_STEP_NONE )
	{
		if ( m_bShouldReportProgressToClient )
		{
			ReportCurrentProgress();	// let all players know what we're up to

			// Delay any actual processing of our current step by one second to ensure
			// our last message has enough time to appear on every player's screen
			m_flReportProgressToClientBuffer = Plat_FloatTime() + PROGRESS_STEP_PROCESSING_BUFFER;
			m_bShouldReportProgressToClient = false;
			return;
		}

		if ( Plat_FloatTime() >= m_flReportProgressToClientBuffer )
		{
			ProcessCurrentProgressStep();

			if ( m_iCurrentProgressStep != PROGRESS_STEP_FINISH )
				m_iCurrentProgressStep++;	// this step has been processed, so move onto the next one
			else
			{
				// We're done processing, so there's no need to update players on our progress anymore
				//StopReportingProgress();

				m_iCurrentProgressStep = PROGRESS_STEP_NONE;
			}

			m_bShouldReportProgressToClient = true;
			return;
		}
	}
}

// Updates all players on our progress via a VGUI element
void NodeGenerator::ReportCurrentProgress( void )
{
	Warning( GetCurrentProgressStepMsg() );	// also update players via console warnings

	KeyValues *data = new KeyValues( "data" );
	data->SetString( "msg",	GetCurrentProgressStepMsg() );
	ShowViewPortPanelToAll( PANEL_NAVGEN_DISPLAY, true, data );
	data->deleteThis();
}

void NodeGenerator::StopReportingProgress( void )
{
	KeyValues *data = new KeyValues( "data" );
	ShowViewPortPanelToAll( PANEL_NAVGEN_DISPLAY, false, data );
	data->deleteThis();
}

void NodeGenerator::ShowViewPortPanelToAll( const char *name, bool bShow, KeyValues *data )
{
	CRecipientFilter filter;
	filter.AddAllPlayers();
	filter.MakeReliable();

	int count = 0;
	KeyValues *subkey = NULL;

	if ( data )
	{
		subkey = data->GetFirstSubKey();
		while ( subkey )
		{
			subkey = subkey->GetNextKey();
			count++;
		}

		subkey = data->GetFirstSubKey(); // reset 
	}

	UserMessageBegin( filter, "VGUIMenu" );
		WRITE_STRING( name ); // menu name
		WRITE_BYTE( bShow ? 1 : 0 );
		WRITE_BYTE( count );
		
		// write additional data (be careful not more than 192 bytes!)
		while ( subkey )
		{
			WRITE_STRING( subkey->GetName() );
			WRITE_STRING( subkey->GetString() );
			subkey = subkey->GetNextKey();
		}
	MessageEnd();
}

const char *NodeGenerator::GetCurrentProgressStepMsg( void )
{
	const char *szProgressMsg = "";

	switch ( m_iCurrentProgressStep )
	{
	case PROGRESS_STEP_START:
		szProgressMsg = "Loading, please wait...";
		break;
	case PROGRESS_STEP_NODEBLOCK_1:
		szProgressMsg = "Processing node block 1 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_2:
		szProgressMsg = "Processing node block 2 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_3:
		szProgressMsg = "Processing node block 3 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_4:
		szProgressMsg = "Processing node block 4 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_5:
		szProgressMsg = "Processing node block 5 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_6:
		szProgressMsg = "Processing node block 6 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_7:
		szProgressMsg = "Processing node block 7 of 8...";
		break;
	case PROGRESS_STEP_NODEBLOCK_8:
		szProgressMsg = "Processing node block 8 of 8...";
		break;
	case PROGRESS_STEP_SAVENODES:
		szProgressMsg = "Saving all processed nodes...";
		break;
	case PROGRESS_STEP_FINISH:
		szProgressMsg = "Complete! Please reload the current map.";
		break;
	}

	return szProgressMsg;
}

void NodeGenerator::ProcessCurrentProgressStep( void )
{
	switch ( m_iCurrentProgressStep )
	{
	// Warming up...
	case PROGRESS_STEP_START:
		// Nothing to process here
		break;

	// Create our nodes by "node blocks" -- 8 of them

	// BLOCK 1:
	// x is positive, y is positive, z is positive
	case PROGRESS_STEP_NODEBLOCK_1:
		ProcessNodeBlock( false, false, false );
		break;

	// BLOCK 2:
	// x is positive, y is positive, z is negative
	case PROGRESS_STEP_NODEBLOCK_2:
		ProcessNodeBlock( false, false, true );
		break;

	// BLOCK 3:
	// x is positive, y is negative, z is positive
	case PROGRESS_STEP_NODEBLOCK_3:
		ProcessNodeBlock( false, true, false );
		break;

	// BLOCK 4:
	// x is positive, y is negative, z is negative
	case PROGRESS_STEP_NODEBLOCK_4:
		ProcessNodeBlock( false, true, true );
		break;

	// BLOCK 5:
	// x is negative, y is positive, z is positive
	case PROGRESS_STEP_NODEBLOCK_5:
		ProcessNodeBlock( true, false, false );
		break;

	// BLOCK 6:
	// x is negative, y is positive, z is negative
	case PROGRESS_STEP_NODEBLOCK_6:
		ProcessNodeBlock( true, false, true );
		break;

	// BLOCK 7:
	// x is negative, y is negative, z is positive
	case PROGRESS_STEP_NODEBLOCK_7:
		ProcessNodeBlock( true, true, false );
		break;

	// BLOCK 8:
	// x is negative, y is negative, z is negative
	case PROGRESS_STEP_NODEBLOCK_8:
		ProcessNodeBlock( true, true, true );
		break;

	// All nodes generated; we're done!
	case PROGRESS_STEP_SAVENODES:
		SaveNodes();	// save our changes
		break;

	// All nodes generated; we're done!
	case PROGRESS_STEP_FINISH:
		// Nothing to process here
		break;
	}
}

void NodeGenerator::ProcessNodeBlock( bool bNegativeX, bool bNegativeY, bool bNegativeZ )
{
	for ( x = 0.0f;
		(bNegativeX) ? (x > MIN_COORD_FLOAT) : (x < MAX_COORD_FLOAT);
		(bNegativeX) ? (x = x - NODE_STEP_DISTANCE) : (x = x + NODE_STEP_DISTANCE) )
	{
		for ( y = 0.0f;
			(bNegativeY) ? (y > MIN_COORD_FLOAT) : (y < MAX_COORD_FLOAT);
			(bNegativeY) ? (y = y - NODE_STEP_DISTANCE) : (y = y + NODE_STEP_DISTANCE) )
		{
			for ( z = 0.0f;
				(bNegativeZ) ? (z > MIN_COORD_FLOAT) : (z < MAX_COORD_FLOAT);
				(bNegativeZ) ? (z = z - NODE_STEP_DISTANCE) : (z = z + NODE_STEP_DISTANCE) )
			{
				ProcessNode( new Node(Vector(x, y, z)) );
			}
		}
	}
}

void NodeGenerator::ProcessNode( Node *pNode )
{
	if ( !pNode )
		return;

	// First, try tracing in all directions to see if our node is outside the map

	for ( dir = TRACE_NEG_X; dir < NUM_TRACE_DIRECTIONS; dir++ )
	{
		switch ( dir )
		{
		case TRACE_NEG_X:
			vecTestPos = Vector( pNode->GetPosition().x - MAX_TRACE_LENGTH,
				pNode->GetPosition().y,
				pNode->GetPosition().z );
			break;
		case TRACE_POS_X:
			vecTestPos = Vector( pNode->GetPosition().x + MAX_TRACE_LENGTH,
				pNode->GetPosition().y,
				pNode->GetPosition().z );
			break;
		case TRACE_NEG_Y:
			vecTestPos = Vector( pNode->GetPosition().x,
				pNode->GetPosition().y - MAX_TRACE_LENGTH,
				pNode->GetPosition().z );
			break;
		case TRACE_POS_Y:
			vecTestPos = Vector( pNode->GetPosition().x,
				pNode->GetPosition().y + MAX_TRACE_LENGTH,
				pNode->GetPosition().z );
			break;
		case TRACE_NEG_Z:
			vecTestPos = Vector( pNode->GetPosition().x,
				pNode->GetPosition().y,
				pNode->GetPosition().z - MAX_TRACE_LENGTH );
			break;
		case TRACE_POS_Z:
			vecTestPos = Vector( pNode->GetPosition().x,
				pNode->GetPosition().y,
				pNode->GetPosition().z + MAX_TRACE_LENGTH );
			break;
		}

		UTIL_TraceLine( pNode->GetPosition(), vecTestPos, MASK_NPCSOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &result );
		if ( result.startsolid || result.allsolid || (result.fraction == 1.0) )
		{
			delete pNode;	// node appears to be outside the map, so delete it
			return;
		}

		if ( dir == TRACE_POS_Z )
		{
			// Next, try tracing downwards from the upward position of our last result

			vecTestPos = Vector( result.endpos.x,
				result.endpos.y,
				result.endpos.z - MAX_TRACE_LENGTH );

			UTIL_TraceLine( result.endpos, vecTestPos, MASK_NPCSOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &result );
			if ( result.startsolid || result.allsolid || (result.fraction == 1.0) )
			{
				delete pNode;	// node appears to be in an invalid location, so delete it
				return;
			}

			// This was successful, so set this node's position to a slightly modified version of the result
			pNode->SetPosition( Vector(result.endpos.x, result.endpos.y, result.endpos.z + 5.0f) );

			// Next, see if this node's new position is too close to any other nodes that already exist

			for ( i = 0; i < m_CompleteNodeList.Count(); i++ )
			{
				pOtherNode = m_CompleteNodeList.Element( i );
				if ( !pOtherNode )
					continue;

				if ( pOtherNode == pNode )
					continue;	// skip over this node in our check

				if ( pOtherNode->GetPosition() == pNode->GetPosition() )
				{
					delete pNode;	// this node appears to be too close to another node, so delete it
					return;
				}
			}

			// Finally, see if this node's position fits a standard AI hull
		
			/*UTIL_TraceHull( result.endpos, result.endpos, NAI_Hull::Mins(HULL_HUMAN), NAI_Hull::Maxs(HULL_HUMAN), MASK_NPCSOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &result );
			if ( result.startsolid || result.allsolid )
			{
				delete pNode;	// node appears to be in an invalid location, so delete it
				return;
			}*/
		}
	}

	// This node has passed all of our tests, which means we can assume it is valid, so it gets to stay (yay)

	// Draw a nice box so it's easy for us to identify valid, processed nodes and where they're located in the world
	NDebugOverlay::Box( pNode->GetPosition(), Vector(-5, -5, -5), Vector(5, 5, 5), 0, 255, 0, 255, 99999.0f );
}

// The following function is based on one found here:
// http://developer.valvesoftware.com/wiki/Node_graphs_for_deathmatch_maps
void NodeGenerator::SaveNodes( void )
{
	// Write the positions of all our nodes to a text file
	// so that they can be loaded and used to generate
	// info_nodes at those positions, which can then
	// be baked into a nodegraph

	char szNodeTextFilename[MAX_PATH];
	Q_snprintf( szNodeTextFilename,
		sizeof(szNodeTextFilename),
		"maps/graphs/%s%s.txt",
		STRING(gpGlobals->mapname),
		GetPlatformExt() );

	// If a file for this map already exists, remove it first
	// before we generate another one to prevent any issues
	// (is this really necessary?)
	if ( filesystem->FileExists(szNodeTextFilename) )
		filesystem->RemoveFile( szNodeTextFilename );

	Node *pNode;
	CUtlBuffer buf( 0, 0, CUtlBuffer::TEXT_BUFFER );
	for ( i = 0; i < m_CompleteNodeList.Count(); i++ )
	{
		pNode = m_CompleteNodeList.Element( i );
		if ( !pNode )
			continue;

		buf.PutString( UTIL_VarArgs("%f	%f	%f\n",
			pNode->GetPosition().x,
			pNode->GetPosition().y,
			pNode->GetPosition().z) );
	}
	filesystem->WriteFile( szNodeTextFilename, "game", buf );

	// We're done with these nodes, so delete them
	m_CompleteNodeList.PurgeAndDeleteElements();
}

void CC_GenerateNodes( void )
{
	if ( g_pNodeGenerator )
		delete g_pNodeGenerator;	// delete the old node generator first (if it exists)

	new NodeGenerator();	// create the new node generator
	if ( !g_pNodeGenerator )
		return;

	g_pNodeGenerator->SetCurrentProgressStep( PROGRESS_STEP_START );	// begin!
}
static ConCommand generatenodes( "generatenodes", CC_GenerateNodes, "Manually starts the node generation process", FCVAR_CHEAT );
