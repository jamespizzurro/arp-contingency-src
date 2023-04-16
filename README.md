The complete game source code of Contingency, a Half-Life 2 modification built using the Source engine. See [the mod's page on the Valve Developer Community wiki](https://developer.valvesoftware.com/wiki/Contingency) for more information about it. Contingency's game files are also available [here](https://github.com/jamespizzurro/arp-contingency).

## Changelog

### v 0.1.8d

* Fixed an oversight in ghost prop prevention, now it works when the prop-holding gravity gun is taken away at the end of any game phase.

**Known issues**

* To start a server visible through Hamachi/Internet the hosting player has to type 'sv_lan 0' and 'map map_name' in the console after starting a server.
See discussion here: https://steamcommunity.com/app/211/discussions/0/522730700940732111/

### v 0.1.8c Feb 7, 2023

* Fixed an oversight in viewrender.cpp

### v 0.1.8b

* Headcrabs have joined the zombie wave, looking for some victims to zombify.

### v 0.1.8

**Balance improvements in binaries**

* Fast headcrab now has 10 hp to die from one AR2 shot or two shots from other automatic weapons. Didn't notice before that in code it had read the classic headcrab health entry from skill.cfg.
* Fast zombie health has been de-hardcoded and brought to skill.cfg.

### v 0.1.6

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

### v 0.1.5 (these and older changes by @jamespizzurro)

* All NPCs are now much more aware of spawnable props so that enemy NPCs will attack them upon encountering them, including if they are blocking the way to players
* Players can no longer collide with each other, thereby fixing any issues where players could get stuck inside other players when spawning and whatnot
* Support NPCs and deployed turrets no longer damage players' spawned props
* The RPG is now an equipment item rather than a primary weapon
* S.L.A.M. tripmines can no longer be set off by players, support NPCs, deployed turrets, or spawnable props
* Zombie and headcrab waves have now been combined into one zombie wave, where headcrabs have been re-added to all zombie types and have the possibility of releasing from their hosts
* NPCs that wield weapons (e.g. Combine soldiers and armed citizens) can no longer fire from insanely far distances, making them less effective overall
* Reworked the number of NPCs that spawn per wave based on more logical factors and additional play-testing (we used maths!), where players are now expected to progress further in the game overall (i.e. players will likely clear more waves per game now compared to the old system)
* Added a new weapon for the equipment slot: the vortex hopwire grenade
* Added challenge waves, where only one random type of NPC is spawned during the entire duration of the wave, that occur every 5 waves by default (this can be changed via ConVar by servers though)
* Fixed all known issues pertaining to contingency_bunker
* Fixed all known issues pertaining to contingency_factory
* Fixed all known issues pertaining to contingency_overpass
* The maximum health of spawnable props is now shown on the prop spawning menu and the current health of a particular spawned prop is now shown when you put your crosshair over it
* There is now a 'teleport back to spawn' function that can be bound to a specific key ('m' by default), but it can only be used when you are truly stuck on something
* Players can no longer spawn props in ways that get them, other players or support NPCs directly stuck inside said props
* Completely revamped contingency_ravine from scratch
* Added a concrete barrier to the prop spawning menu (costs 10 credits, has 200HP)

### v 0.1.0

Initial release.