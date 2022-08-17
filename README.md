The complete game files of Contingency, a Half-Life 2 modification built using the Source engine. See [the mod's page on the Valve Developer Community wiki](https://developer.valvesoftware.com/wiki/Contingency) for more information about it. Contingency's game files are also available [here](https://github.com/jamespizzurro/arp-contingency).

## Changelog

### HEVcrab's community patch changes

**Fixes in binaries**

* Poison headcrabs reducing spawnable barricade props' health to 1 hp has been fixed, now poison headcrabs deal fixed 10 damage to spawnable props.
* If the player was carrying a physics prop with a gravity gun at the moment when interim phase ended (so when combat phase started the player was deprived of the grav gun and the prop was dropped), this prop became collision-less with bullets, players and NPCs. This is no longer the case, and props dropped because of game phase change behave correctly.
* Now the player [can detonate the SLAM](https://youtu.be/pvfdsDBldqI) when holding a detonator (i.e. having a satchel in the world) and ready tripmine-mode satchel in hands.
* Prop fading in the distance is now controlled by convar cl_propfade saved to config.cfg (0 to disable - default value, 1 to enable).
* Darkening of the screen at the start of the combat phase has been disabled as it blinded the players in darker areas/maps.
* "PlayerName's Prop" string displayed when aiming at props spawned by other players getting trimmed to first 8 characters of PlayerName has been fixed.

**Balance improvements in binaries**

* Fast zombie's health is now 40, to get dropped from one .357 Magnum shot.
* Players' weapons damage values now are 5 for SMG1, 8 for 9mm pistol(s) and Alyx's gun, 10 for AR2, 15 for the turret.
* SMG1, 9mm pistol(s) and AR2s deal the same damage when wielded by enemies and players.
* Both players and NPCs take 4x damage from headshots.