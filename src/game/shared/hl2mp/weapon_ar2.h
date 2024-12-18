//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONAR2_H
#define	WEAPONAR2_H

#include "basegrenade_shared.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
#define CWeaponAR2 C_WeaponAR2
#endif

class CWeaponAR2 : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS( CWeaponAR2, CHL2MPMachineGun );

	CWeaponAR2();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	ItemPostFrame( void );
	void	Precache( void );
	
	void	SecondaryAttack( void );
	void	DelayedAttack( void );

	const char *GetTracerType( void ) { return "AR2Tracer"; }

	void	AddViewKick( void );
	
	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles ); //AI Patch Addition.
	void	FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles ); //AI Patch Addition.
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary ); //AI Patch Addition.
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator ); //AI Patch Addition.

	int		GetMinBurst( void ) { return 2; }
	int		GetMaxBurst( void ) { return 5; }
	float	GetFireRate( void ) { return 0.1f; }

	bool	CanHolster( void );
	bool	Reload( void );
	
	#ifndef CLIENT_DLL //AI Patch Addition.
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; } //AI Patch Addition.

	Activity	GetPrimaryAttackActivity( void );
	
	#endif //AI Patch Addition.
	
	void	DoImpactEffect( trace_t &tr, int nDamageType );
	
	virtual bool Deploy( void );


	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		
		cone = VECTOR_CONE_3DEGREES;

		return cone;
	}
	
	const WeaponProficiencyInfo_t *GetProficiencyValues();

private:
	CWeaponAR2( const CWeaponAR2 & );

protected:

	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	int						m_nVentPose;
	
	DECLARE_ACTTABLE();
};


#endif	//WEAPONAR2_H
