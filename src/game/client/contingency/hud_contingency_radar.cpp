// Contingency - James
// Added radar display

#include "cbase.h"
#include "iclientmode.h"

#include "hudelement.h"

#include "vgui/ISurface.h"
#include <vgui_controls/Panel.h>

#include "c_playerresource.h"

#include "contingency_gamerules.h"
#include "c_npc_contingency_turret.h"
#include "c_npc_citizen17.h"

// Added prop spawning system
// Update any spawnable props on radar
#include "c_contingency_spawnableprop.h"

#include <vgui_controls/ImagePanel.h>

#include "in_buttons.h"
#include <vgui_controls/AnimationController.h>
#include "iinput.h"
#include "input.h"

#include "tier0/memdbgon.h"

using namespace vgui;

class IRadarDisplayPanel
{
public:
	virtual void Toggle( void ) = 0;
};

class CHudRadarDisplay : public CHudElement, public Panel, public IRadarDisplayPanel
{
	DECLARE_CLASS_SIMPLE( CHudRadarDisplay, Panel );

public:
	CHudRadarDisplay( const char *pElementName );

	void DrawRadarDot( CBaseEntity *pTarget, Color dotColor );
	bool ShouldDraw( void );
	void Paint( void );
	void Toggle( void );

private:
	int wide;
	int tall;
	bool m_bDisplayed;
	float m_flTimeToPreventDisplay;
	int i;
	int panelHeight;
	int panelWidth;
	float distanceOffsetX;
	float distanceOffsetY;
	Vector distanceOffset;
	float m_fRadarScaleAdjust;
	float drawX;
	float drawY;

	C_BasePlayer *pLocalPlayer;
	C_Contingency_Player *pTargetPlayer;
	C_AI_BaseNPC *pTargetNPC;
	C_NPC_Citizen *pTargetCitizen;
	C_NPC_FloorTurret *pTargetTurret;

	// Added prop spawning system
	// Update any spawnable props on radar
	C_Contingency_SpawnableProp *pTargetSpawnableProp;

	ImagePanel *m_pRadarImagePanel;
};

IRadarDisplayPanel *g_pRadarDisplay = NULL;

DECLARE_HUDELEMENT( CHudRadarDisplay );

CHudRadarDisplay::CHudRadarDisplay( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudRadarDisplay" )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	// Initialize variables
	wide = 0;
	tall = 0;
	m_bDisplayed = true;
	m_flTimeToPreventDisplay = 0.0f;
	i = 0;
	panelHeight = 0;
	panelWidth = 0;
	distanceOffsetX = 0.0f;
	distanceOffsetY = 0.0f;
	m_fRadarScaleAdjust = 0.0f;
	drawX = 0.0f;
	drawY = 0.0f;

	// Draw our radar image and show it
	m_pRadarImagePanel = new ImagePanel( this, "RadarImage" );
	if ( m_pRadarImagePanel )
	{
		m_pRadarImagePanel->SetProportional( true );
		m_pRadarImagePanel->SetPos( 0, 0 );
		m_pRadarImagePanel->SetImage( scheme()->GetImage("hud/radar_display", false) );
		m_pRadarImagePanel->SetSize( GetWide(), GetTall() );
		m_pRadarImagePanel->SetShouldScaleImage( true );
		m_pRadarImagePanel->SetVisible( true );
		m_pRadarImagePanel->SetAlpha( 100 );
	}

	SetProportional( true );
	SetPaintBorderEnabled( true );
	SetVisible( true );

	g_pRadarDisplay = this;	// for ConCommand access (assumes only one radar display)
}

void CHudRadarDisplay::DrawRadarDot( CBaseEntity *pTarget, Color dotColor )
{
	pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	if ( !pTarget )
		return;

	panelHeight = GetTall();
	panelWidth = GetWide();

	// Get the x and y distance between the local player and the other player
	distanceOffsetX = pLocalPlayer->EyePosition().x - pTarget->EyePosition().x;
	distanceOffsetY = pTarget->EyePosition().y - pLocalPlayer->EyePosition().y;

	// Turn these differences in distances into a vector
	distanceOffset = Vector( distanceOffsetX, distanceOffsetY, 0 );

	// Scale the vector according to the size of our radar
	m_fRadarScaleAdjust = 20 / (panelWidth / 100);	// after a lot of trial-and-error, this is really, really close to what we want
	distanceOffset /= m_fRadarScaleAdjust;

	// Rotate the radar display according to the local player's camera
	VectorYawRotate( distanceOffset, pLocalPlayer->EyeAngles()[YAW] + 90.0f, distanceOffset );

	// Determine whether or not the other player should be displayed on the radar according to the bounds of our radar
	drawX = distanceOffset.x + (panelWidth / 2);
	drawY = distanceOffset.y + (panelHeight / 2);

	if ( (drawX < 0) || (drawY < 0) || (drawX > panelWidth) || (drawY > panelHeight) )
		return;	// square clipping: less pretty, but easier to deal with than circles

	// Actually draw the colored square
	surface()->DrawSetColor( dotColor );
	surface()->DrawFilledRect( drawX - 2, drawY - 2, drawX + 2, drawY + 2 );
}

bool CHudRadarDisplay::ShouldDraw( void )
{
	if ( ContingencyRules() && !ContingencyRules()->DoesMapAllowRadars() )
		return false;

	return true;
}

void CHudRadarDisplay::Paint()
{
	pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	if ( m_pRadarImagePanel )
	{
		m_pRadarImagePanel->SetSize( wide, tall );
		m_pRadarImagePanel->SetShouldScaleImage( true );
		m_pRadarImagePanel->SetAlpha( 100 );
	}

	if ( m_bDisplayed )
	{
		if ( gpGlobals->curtime < m_flTimeToPreventDisplay )
			return;

		wide = GetWide();
		tall = GetTall();
	}
	else
		return;

	// Update any players on radar
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pTargetPlayer = ToContingencyPlayer( UTIL_PlayerByIndex(i) );
		if ( !pTargetPlayer )
			continue;

		if ( ContingencyRules() && !ContingencyRules()->IsPlayerPlaying(pTargetPlayer) )
			continue;	// don't show players who aren't playing

		// Don't show ourselves (that would be silly!)
		if ( pTargetPlayer == pLocalPlayer )
			continue;

		DrawRadarDot( pTargetPlayer, GameResources()->GetTeamColor(pTargetPlayer->GetTeamNumber()) );
	}

	// Update any NPCs on radar
	for ( i = 0; i < m_NPCList.Count(); i++ )
	{
		// Added prop spawning system
		// Update any spawnable props on radar
		pTargetSpawnableProp = dynamic_cast<C_Contingency_SpawnableProp*>( m_NPCList[i] );
		if ( pTargetSpawnableProp )
		{
			if ( pTargetSpawnableProp->GetSpawnerPlayer() != pLocalPlayer )
				continue;	// only show the local player's spawnable props on the radar

			DrawRadarDot( pTargetSpawnableProp, COLOR_BLUE );
			continue;
		}

		pTargetCitizen = dynamic_cast<C_NPC_Citizen*>( m_NPCList[i] );
		if ( pTargetCitizen )
		{
			if ( !pTargetCitizen->IsAlive() )
				continue;

			DrawRadarDot( pTargetCitizen, COLOR_GREEN );
			continue;
		}

		pTargetTurret = dynamic_cast<C_NPC_FloorTurret*>( m_NPCList[i] );
		if ( pTargetTurret )
		{
			if ( !pTargetTurret->IsAlive() )
				continue;

			DrawRadarDot( pTargetTurret, COLOR_YELLOW );
			continue;
		}

		pTargetNPC = dynamic_cast<C_AI_BaseNPC*>( m_NPCList[i] );
		if ( pTargetNPC )
		{
			if ( !pTargetNPC->IsAlive() )
				continue;

			DrawRadarDot( pTargetNPC, COLOR_RED );	// all other NPCs are assumed to be EVIL (hence the red)
			continue;
		}
	}
}

void CHudRadarDisplay::Toggle( void )
{
	pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	if ( m_bDisplayed )
	{
		m_bDisplayed = false;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RadarDisplayHide" );
	}
	else
	{
		m_bDisplayed = true;
		m_flTimeToPreventDisplay = gpGlobals->curtime + g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength( "RadarDisplayShow" ) + 0.25f;	// extra time buffer (just in case?)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "RadarDisplayShow" );
	}
}

// Added radar display
void CC_ToggleRadarDisplay( void )
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	if ( !g_pRadarDisplay )
		return;

	if ( ContingencyRules() && !ContingencyRules()->DoesMapAllowRadars() )
	{
		// ClientPrint doesn't seem to work client-side??
		//ClientPrint( pLocalPlayer, HUD_PRINTTALK, "This map does not allow the use of radars." );
		return;
	}

	g_pRadarDisplay->Toggle();
}
static ConCommand toggleradardisplay( "toggleradardisplay", CC_ToggleRadarDisplay, "Toggles radar display (if possible)", 0 );
