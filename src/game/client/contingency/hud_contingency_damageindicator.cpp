#include "cbase.h"

#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include "hudelement.h"

#include "contingency_gamerules.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_BLOOD_SPLATER_NUM -1
#define NUM_STAGES 4	// remember to change this number if we add more stages

extern ConVar contingency_client_heartbeatsounds;

class CHudContingencyDamageIndicator : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE( CHudContingencyDamageIndicator, Panel );

public:
	CHudContingencyDamageIndicator( const char *pElementName );

	void Init( void );
	void VidInit( void );
	void Reset( void );
	bool ShouldDraw( void );

private:
	void Paint();
	void DrawFullscreenDamageIndicator();
	void HandleHeartbeatSounds();
	void ApplySchemeSettings( IScheme *pScheme );

private:
	CPanelAnimationVar( Color, m_DmgFullscreenColor, "DmgFullscreenColor", "255 255 255 255" );

	CMaterialReference m_BloodSplatterMaterials[NUM_STAGES];

	void UpdateBloodSplatterToDisplay( void );
	int m_iBloodSplatter, m_iPlayerHealth;
	bool m_bHealthHasChangedRecently;
};

DECLARE_HUDELEMENT( CHudContingencyDamageIndicator );

static const char* kBloodSplatter[NUM_STAGES] =	// more stages can be added here (remember to change NUM_STAGES too)
{
	"hud/bloodsplatter_stage1",
	"hud/bloodsplatter_stage2",
	"hud/bloodsplatter_stage3",
	"hud/bloodsplatter_stage4"
};

CHudContingencyDamageIndicator::CHudContingencyDamageIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudContingencyDamageIndicator" )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	for ( int i = 0; i < NUM_STAGES; i++ )
		m_BloodSplatterMaterials[i].Init( kBloodSplatter[i], TEXTURE_GROUP_VGUI );
}

void CHudContingencyDamageIndicator::Init()
{
	Reset();
}

void CHudContingencyDamageIndicator::VidInit()
{
	Reset();
}

void CHudContingencyDamageIndicator::Reset( void )
{
	m_iBloodSplatter = INIT_BLOOD_SPLATER_NUM;
	m_iPlayerHealth = 0;
	m_bHealthHasChangedRecently = false;
}

bool CHudContingencyDamageIndicator::ShouldDraw( void )
{
	// Don't draw if the local player isn't playing
	C_Contingency_Player *pPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pPlayer )
		return false;

	// Refresh some stuff before we check whether or not to draw (kind of a hack, but oh well)
	UpdateBloodSplatterToDisplay();
	HandleHeartbeatSounds();

	if ( (m_iBloodSplatter <= INIT_BLOOD_SPLATER_NUM) || (m_iBloodSplatter >= NUM_STAGES) )
		return false;	// out of bounds; will crash the game; do not show any blood splatter

	return CHudElement::ShouldDraw();
}

// Determines what stage of blood splatter to use based on the player's current health
void CHudContingencyDamageIndicator::UpdateBloodSplatterToDisplay( void )
{
	C_Contingency_Player *pPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pPlayer )
	{
		m_iBloodSplatter = INIT_BLOOD_SPLATER_NUM;	// can't find local player; don't show any blood splatter
		return;
	}

	int savedPlayerHealth = pPlayer->GetHealth();
	if ( savedPlayerHealth == m_iPlayerHealth )
		return;	// the player's health hasn't changed since we last checked, so there's nothing else that needs to be done here

	m_bHealthHasChangedRecently = true;	// remember the fact that our health has changed recently
	m_iPlayerHealth = savedPlayerHealth;	// save the player's new health value to our class variable

	int maxHealth = pPlayer->GetMaxHealth();

	if ( m_iPlayerHealth <= 0 )
		m_iBloodSplatter = 3;
	else if ( m_iPlayerHealth < (maxHealth/4) )
		m_iBloodSplatter = 2;
	else if ( m_iPlayerHealth < (maxHealth/2) )
		m_iBloodSplatter = 1;
	else if ( m_iPlayerHealth < ((3*maxHealth)/4) )
		m_iBloodSplatter = 0;
	else
		m_iBloodSplatter = INIT_BLOOD_SPLATER_NUM;	// we must be pretty healthy, so don't show any blood splatter
}

void CHudContingencyDamageIndicator::HandleHeartbeatSounds( void )
{
	C_Contingency_Player *pPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
	if ( !pPlayer )
		return;

	if ( contingency_client_heartbeatsounds.GetBool() &&
		ContingencyRules()->IsPlayerPlaying(pPlayer) )
	{
		if ( m_iPlayerHealth > 0 )
		{
			CLocalPlayerFilter filter;

			int maxHealth = pPlayer->GetMaxHealth();
			if ( m_iPlayerHealth < (maxHealth/4) )
			{
				pPlayer->StopSound( "Heartbeat.Slow" );
				pPlayer->EmitSound( filter, -1, "Heartbeat.Fast" );
			}
			else if ( m_iPlayerHealth < (maxHealth/2) )
			{
				pPlayer->StopSound( "Heartbeat.Fast" );
				pPlayer->EmitSound( filter, -1, "Heartbeat.Slow" );
			}
			else
			{
				pPlayer->StopSound( "Heartbeat.Fast" );
				pPlayer->StopSound( "Heartbeat.Slow" );
			}
		}
		else
		{
			pPlayer->StopSound( "Heartbeat.Fast" );
			pPlayer->StopSound( "Heartbeat.Slow" );
		}
	}
	else
	{
		pPlayer->StopSound( "Heartbeat.Fast" );
		pPlayer->StopSound( "Heartbeat.Slow" );
	}
}

void CHudContingencyDamageIndicator::DrawFullscreenDamageIndicator()
{
	CMatRenderContextPtr pRenderContext( materials );
	IMesh *pMesh = pRenderContext->GetDynamicMesh( true, NULL, NULL, m_BloodSplatterMaterials[m_iBloodSplatter] );
	if ( !pMesh )
		return;	// don't let bad things happen...

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	int r = m_DmgFullscreenColor[0], g = m_DmgFullscreenColor[1], b = m_DmgFullscreenColor[2], a = m_DmgFullscreenColor[3];
	float wide = GetWide(), tall = GetTall();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0, 0, 0 );
	meshBuilder.Position3f( 0.0f, 0.0f, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0, 1, 0 );
	meshBuilder.Position3f( wide, 0.0f, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0, 1, 1 );
	meshBuilder.Position3f( wide, tall, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub( r, g, b, a );
	meshBuilder.TexCoord2f( 0, 0, 1 );
	meshBuilder.Position3f( 0.0f, tall, 0 );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	if ( m_bHealthHasChangedRecently )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudContingencyDamageIndicatorBlink" );	// blink (quick fade out and in)
		m_bHealthHasChangedRecently = false;	// don't blink our indicator until our health has changed again
	}
}

void CHudContingencyDamageIndicator::Paint()
{
	if ( ShouldDraw() )
		DrawFullscreenDamageIndicator();	// draw fullscreen damage indicators
}

void CHudContingencyDamageIndicator::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetPaintBackgroundEnabled( false );

	int wide, tall;
	GetHudSize( wide, tall );
	SetSize( wide, tall );
}
