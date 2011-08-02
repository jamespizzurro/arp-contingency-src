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

#include <vgui_controls/ImagePanel.h>

#include "tier0/memdbgon.h"

#define RADAR_SCALE 20	// this seems to be an appropriate base scale for the radar

using namespace vgui;

class CHudRadarDisplay : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE( CHudRadarDisplay, Panel );

public:
	CHudRadarDisplay( const char *pElementName );

	void DrawRadarDot( CBaseEntity *pTarget, Color dotColor );
	void Paint( void );
	void OnThink( void );

private:
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

	ImagePanel *m_pRadarImagePanel;
};

DECLARE_HUDELEMENT( CHudRadarDisplay );

CHudRadarDisplay::CHudRadarDisplay( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudRadarDisplay" )
{
	// Hide the radar if the player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	// Initialize variables
	panelHeight = 0;
	panelWidth = 0;
	distanceOffsetX = 0.0f;
	distanceOffsetY = 0.0f;
	m_fRadarScaleAdjust = 0.0f;
	drawX = 0.0f;
	drawY = 0.0f;

	// Draw our radar image and show it
	m_pRadarImagePanel = new ImagePanel( this, "RadarImage" );
	m_pRadarImagePanel->SetImage( scheme()->GetImage("HUDicons/radar_display", false) );
	m_pRadarImagePanel->SetPos( 0, 0 );
	m_pRadarImagePanel->SetSize( GetWide(), GetTall() );
	m_pRadarImagePanel->SetProportional( true );
	m_pRadarImagePanel->SetShouldScaleImage( true );
	m_pRadarImagePanel->SetAlpha( 255 );
	m_pRadarImagePanel->SetVisible( true );

	SetProportional( true );
	SetPaintBorderEnabled( false );
	SetVisible( true );
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
	distanceOffsetY = pLocalPlayer->EyePosition().y - pTarget->EyePosition().y;

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

void CHudRadarDisplay::Paint()
{
	pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pLocalPlayer )
		return;

	int i;

	// Update any players on radar
	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		pTargetPlayer = ToContingencyPlayer( UTIL_PlayerByIndex(i) );
		if ( !pTargetPlayer )
			continue;

		if ( !ContingencyRules()->IsPlayerPlaying(pTargetPlayer) )
			continue;	// don't show players who aren't playing

		// Don't show ourselves (that would be silly!)
		if ( pTargetPlayer == pLocalPlayer )
			continue;

		DrawRadarDot( pTargetPlayer, GameResources()->GetTeamColor(pTargetPlayer->GetTeamNumber()) );
	}

	// Update any NPCs on radar
	for ( i = 0; i < m_NPCList.Count(); i++ )
	{
		pTargetCitizen = dynamic_cast<C_NPC_Citizen*>( m_NPCList[i] );
		if ( pTargetCitizen )
		{
			DrawRadarDot( pTargetCitizen, COLOR_GREEN );
			continue;
		}

		pTargetTurret = dynamic_cast<C_NPC_FloorTurret*>( m_NPCList[i] );
		if ( pTargetTurret )
		{
			DrawRadarDot( pTargetTurret, COLOR_YELLOW );
			continue;
		}

		pTargetNPC = dynamic_cast<C_AI_BaseNPC*>( m_NPCList[i] );
		if ( pTargetNPC )
			DrawRadarDot( pTargetNPC, COLOR_RED );	// all other NPCs are assumed to be EVIL (hence the red)
	}
}

void CHudRadarDisplay::OnThink()
{
	BaseClass::OnThink();
}
