// Added player status HUD element
// Helps players keep track of other players on the map

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "contingency_gamerules.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/AnimationController.h>

#include "vgui_avatarimage.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class PlayerSlot
{
public:
	bool IsOccupied( void ) { return m_bOccupied; }
	void IsOccupied( bool boolean ) { m_bOccupied = boolean; }

	C_Contingency_Player* GetPlayer( void ) { return pPlayer; }
	void SetPlayer( C_Contingency_Player* pNewPlayer ) { pPlayer = pNewPlayer; }

	ImagePanel* GetPlayerAvatar( void ) { return pPlayerAvatar; }
	void SetPlayerAvatar( ImagePanel* pPlayerNewAvatar ) { pPlayerAvatar = pPlayerNewAvatar; }

	Label* GetPlayerName( void ) { return pPlayerName; }
	void SetPlayerName( Label* pPlayerNewName ) { pPlayerName = pPlayerNewName; }

	Label* GetPlayerCondition( void ) { return pPlayerCondition; }
	void SetPlayerCondition( Label* pPlayerNewCondition ) { pPlayerCondition = pPlayerNewCondition; }

private:
	bool m_bOccupied;
	C_Contingency_Player* pPlayer;
	ImagePanel* pPlayerAvatar;
	Label* pPlayerName;
	Label* pPlayerCondition;
};

class CHudContingencyPlayerStatus : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudContingencyPlayerStatus, Panel );

public:
	CHudContingencyPlayerStatus( const char *pElementName );

public:
	void InitializePlayerSlot( PlayerSlot* pPlayerSlot, int number );
	void Init( void );
	void Reset();

protected:
	void OnThink();

private:
	void UpdatePlayerSlots( C_Contingency_Player *pLocalPlayer );

private:
	CUtlVector<PlayerSlot*> m_PlayerSlotList;
	CUtlVector<C_Contingency_Player*> m_PlayerSlotPlayersList;
	PlayerSlot* pPlayerSlot1;
	PlayerSlot* pPlayerSlot2;
	PlayerSlot* pPlayerSlot3;
	PlayerSlot* pPlayerSlot4;

	float m_flUpdateDelay;
	int m_iNumSlotsLastOccupied;
};

DECLARE_HUDELEMENT( CHudContingencyPlayerStatus );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudContingencyPlayerStatus::CHudContingencyPlayerStatus( const char *pElementName ) : BaseClass(NULL, "HudContingencyPlayerStatus"), CHudElement( pElementName )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	SetAlpha( 255 );

	InitializePlayerSlot( pPlayerSlot1, 1 );
	InitializePlayerSlot( pPlayerSlot2, 2 );
	InitializePlayerSlot( pPlayerSlot3, 3 );
	InitializePlayerSlot( pPlayerSlot4, 4 );

	m_flUpdateDelay = 0.0f;
	m_iNumSlotsLastOccupied = 0;
}

void CHudContingencyPlayerStatus::InitializePlayerSlot( PlayerSlot* pPlayerSlot, int number )
{
	// Actually create the new player slot
	pPlayerSlot = new PlayerSlot();

	// Get this new player slot set up correctly
	pPlayerSlot->IsOccupied( false );
	pPlayerSlot->SetPlayer( NULL );
	pPlayerSlot->SetPlayerAvatar( new ImagePanel(this, VarArgs("Player%iAvatar", number)) );
	pPlayerSlot->GetPlayerAvatar()->SetPos( 16 + (130 * (number-1)), 16 );
	pPlayerSlot->GetPlayerAvatar()->SetVisible( false );
	pPlayerSlot->GetPlayerAvatar()->SetAlpha( 0 );
	pPlayerSlot->SetPlayerName( new Label(this, VarArgs("Player%iName", number), "") );
	pPlayerSlot->GetPlayerName()->SetContentAlignment( Label::a_west );
	pPlayerSlot->GetPlayerName()->SetFgColor( Color(255, 255, 255) );
	pPlayerSlot->GetPlayerName()->SetPos( 16 + (130 * (number-1)), 64 );
	pPlayerSlot->GetPlayerName()->SetVisible( false );
	pPlayerSlot->GetPlayerName()->SetAlpha( 0 );
	pPlayerSlot->SetPlayerCondition( new Label(this, VarArgs("Player%iCondition", number), "") );
	pPlayerSlot->GetPlayerCondition()->SetContentAlignment( Label::a_west );
	pPlayerSlot->GetPlayerCondition()->SetFgColor( Color(255, 255, 255) );
	pPlayerSlot->GetPlayerCondition()->SetPos( 16 + (130 * (number-1)), 80 );
	pPlayerSlot->GetPlayerCondition()->SetVisible( false );
	pPlayerSlot->GetPlayerCondition()->SetAlpha( 0 );
	
	// Add this new player slot to our player slot list
	m_PlayerSlotList.AddToTail( pPlayerSlot );
}

void CHudContingencyPlayerStatus::Init()
{
	Reset();
}

void CHudContingencyPlayerStatus::Reset( void )
{
	for ( int i = 0; i < m_PlayerSlotList.Count(); i++ )
	{
		PlayerSlot *pPlayerSlot = m_PlayerSlotList.Element(i);
		if ( !pPlayerSlot )
			continue;

		pPlayerSlot->IsOccupied( false );
		pPlayerSlot->SetPlayer( NULL );
	}

	m_flUpdateDelay = 0.0f;
}

void CHudContingencyPlayerStatus::OnThink()
{
	if ( gpGlobals->curtime >= m_flUpdateDelay )
	{
		C_Contingency_Player *pLocalPlayer = C_Contingency_Player::GetLocalContingencyPlayer();
		if ( pLocalPlayer )
		{
			int numSlotsNowOccupied = 0;	// keep track of how many slots are now occupied this check

			// Make sure our player slots are up to date before we evaluate them
			UpdatePlayerSlots( pLocalPlayer );

			// All slots have been updated, which means we should go through
			// our list of slots again and see which ones are worthy of being
			// displayed on the local player's HUD

			for ( int i = 0; i < m_PlayerSlotList.Count(); i++ )
			{
				PlayerSlot *pPlayerSlot = m_PlayerSlotList.Element(i);
				if ( !pPlayerSlot )
					continue;

				// Do not display slots that aren't occupied
				if ( !pPlayerSlot->IsOccupied() )
				{
					pPlayerSlot->SetPlayer( NULL );
					pPlayerSlot->GetPlayerAvatar()->SetVisible( false );
					pPlayerSlot->GetPlayerAvatar()->SetAlpha( 0 );
					pPlayerSlot->GetPlayerName()->SetVisible( false );
					pPlayerSlot->GetPlayerName()->SetAlpha( 0 );
					pPlayerSlot->GetPlayerCondition()->SetVisible( false );
					pPlayerSlot->GetPlayerCondition()->SetAlpha( 0 );
					continue;
				}

				// This slot appears to qualify, so display it
				pPlayerSlot->GetPlayerAvatar()->SetVisible( true );
				pPlayerSlot->GetPlayerAvatar()->SetAlpha( 255 );
				pPlayerSlot->GetPlayerName()->SetVisible( true );
				pPlayerSlot->GetPlayerName()->SetAlpha( 255 );
				pPlayerSlot->GetPlayerCondition()->SetVisible( true );
				pPlayerSlot->GetPlayerCondition()->SetAlpha( 255 );

				numSlotsNowOccupied++;	// this slot is now occupied, so count it
			}

			// If the number of spots that are occupied has changed since we last checked,
			// then play the proper animation according to the new number of slots that are occupied
			//if ( numSlotsNowOccupied != m_iNumSlotsLastOccupied )	// commented for now to fix this shit not updating when we first join as observer or something
			{
				m_iNumSlotsLastOccupied = numSlotsNowOccupied;

				const char *animationSequenceName = "PlayerStatusZeroSlotsOccupied";
				switch ( m_iNumSlotsLastOccupied )
				{
				case 1:
					animationSequenceName = "PlayerStatusOneSlotOccupied";
					break;
				case 2:
					animationSequenceName = "PlayerStatusTwoSlotsOccupied";
					break;
				case 3:
					animationSequenceName = "PlayerStatusThreeSlotsOccupied";
					break;
				case 4:
					animationSequenceName = "PlayerStatusFourSlotsOccupied";
					break;
				}

				// Actually play the proper animation sequence
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( animationSequenceName );
			}

		}

		m_flUpdateDelay = gpGlobals->curtime + 1.0f;	// update every second
	}
}

void CHudContingencyPlayerStatus::UpdatePlayerSlots( C_Contingency_Player *pLocalPlayer )
{
	// First, go through each player slot and make it not occupied
	// so that we can check which slots are occupied during our
	// update to avoid potential headaches
	for ( int i = 0; i < m_PlayerSlotList.Count(); i++ )
	{
		PlayerSlot *pPlayerSlot = m_PlayerSlotList.Element(i);
		if ( !pPlayerSlot )
			continue;

		pPlayerSlot->IsOccupied( false );
	}

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		C_Contingency_Player *pPlayer = ToContingencyPlayer( UTIL_PlayerByIndex(i) );
		if ( !pPlayer )
			continue;

		// Don't display the local player in a slot
		// since this whole system is for the local player to begin with!
		if ( pPlayer == pLocalPlayer )
			continue;

		// We have a player who qualifies to be displayed
		// in a slot, so let's see if we can do that

		for ( int i = 0; i < m_PlayerSlotList.Count(); i++ )
		{
			PlayerSlot *pPlayerSlot = m_PlayerSlotList.Element(i);
			if ( !pPlayerSlot )
				continue;

			// This player slot has been used this update, which means
			// it should not be updated again, so skip it
			if ( pPlayerSlot->IsOccupied() )
				continue;

			// We have a player slot for this player, so fill it with him/her

			pPlayerSlot->SetPlayer( pPlayer );
			pPlayerSlot->GetPlayerName()->SetText( pPlayerSlot->GetPlayer()->GetPlayerName() );
			pPlayerSlot->GetPlayerName()->SizeToContents();
			pPlayerSlot->GetPlayerCondition()->SetText( pPlayerSlot->GetPlayer()->GetHealthCondition() );
			pPlayerSlot->GetPlayerCondition()->SizeToContents();
			pPlayerSlot->GetPlayerCondition()->SetFgColor( pPlayerSlot->GetPlayer()->GetHealthConditionColor() );

			// This next block of code deals with the player's avatar
			// (pulled from somewhere and edited, but I forget where)

			if ( pPlayerSlot->GetPlayerAvatar()->GetImage() )
				((CAvatarImage*)pPlayerSlot->GetPlayerAvatar()->GetImage())->ClearAvatarSteamID();

			if ( pPlayer && steamapicontext->SteamUtils() )
			{
				int iIndex = pPlayer->entindex();
				player_info_t pi;
				if ( engine->GetPlayerInfo(iIndex, &pi) )
				{
					if ( pi.friendsID )
					{
						CSteamID steamIDForPlayer( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );

						if ( !pPlayerSlot->GetPlayerAvatar()->GetImage() )
						{
							CAvatarImage *pImage = new CAvatarImage();
							pPlayerSlot->GetPlayerAvatar()->SetImage( pImage );
						}

						CAvatarImage *pAvImage = ((CAvatarImage*)pPlayerSlot->GetPlayerAvatar()->GetImage());
						pAvImage->SetAvatarSteamID( steamIDForPlayer );

						// Indent the image. These are deliberately non-resolution-scaling.
						pAvImage->SetAvatarSize( 32, 32 );	// Deliberately non scaling

						pPlayerSlot->GetPlayerAvatar()->SetSize( pAvImage->GetWide(), GetTall() );
						pPlayerSlot->GetPlayerAvatar()->GetImage()->Paint();
						pPlayerSlot->GetPlayerAvatar()->SetVisible( true );
						pPlayerSlot->GetPlayerAvatar()->SetAlpha( 255 );
					}
					else
					{
						pPlayerSlot->GetPlayerAvatar()->SetVisible( false );
						pPlayerSlot->GetPlayerAvatar()->SetAlpha( 0 );
					}
				}
				else
				{
					pPlayerSlot->GetPlayerAvatar()->SetVisible( false );
					pPlayerSlot->GetPlayerAvatar()->SetAlpha( 0 );
				}
			}
			else
			{
				pPlayerSlot->GetPlayerAvatar()->SetVisible( false );
				pPlayerSlot->GetPlayerAvatar()->SetAlpha( 0 );
			}

			// END player avatar stuff

			pPlayerSlot->IsOccupied( true );	// this slot is occupied with a player now
			break;	// we're done here!
		}
	}
}
