# NG++ — Ratchet & Clank (RAC1)

This is the separate RAC1 edition. The existing `ng-plus-plus/` directory remains the Going Commando (RAC2) edition; RAC1 work stays on the `ng-plus-plus-rac1` branch.

## Agreed progression design

- Weapon levels run from **1 to 10**.
- Each weapon tracks and saves its own experience and level.
- Only the weapon responsible for an enemy's kill receives that kill's XP.
- Each weapon has its own setting for the XP needed to reach its next level.
- Launcher settings expose the level cap, linear/increasing XP curve, level-up notices, damage scaling and ammo-capacity scaling.

## RAC1 weapon item IDs

The progression slots cover all 15 weapons in the OpenRAC RAC1 catalogue:

| Weapon | RAC1 item ID |
|---|---:|
| Bomb Glove | `0x0A` |
| Devastator | `0x0B` |
| Blaster | `0x0F` |
| Pyrocitor | `0x10` |
| Mine Glove | `0x11` |
| Walloper | `0x12` |
| Tesla Claw | `0x13` |
| Visibomb Gun | `0x0D` |
| Glove of Doom | `0x14` |
| Morph-o-Ray | `0x15` |
| R.Y.N.O. | `0x17` |
| Suck Cannon | `0x09` |
| Decoy Glove | `0x19` |
| Taunter | `0x0E` |
| Drone Device | `0x18` |

## Implementation status

- [x] RAC1-only manifest and launcher settings.
- [x] Independent per-weapon progression state (15 weapon slots, levels 1–10).
- [x] Versioned save-payload serialization for OpenRAC's per-save mod data API.
- [x] Native C++ mod entry point and SDK CMake target.
- [x] Reads per-weapon XP settings and reloads progression when the active game save changes.
- [ ] Verified RAC1 item-ID mapping.
- [ ] Reliable weapon-to-kill attribution in RAC1.
- [ ] Award XP only after a confirmed kill and persist the updated progression.
- [ ] Damage/ammo effects and level-up notices.
- [ ] Build/package for each platform and in-game testing on NTSC and PAL.

The native plugin currently loads its progression core and keeps save data separate per game save. It does **not** award XP yet: kill credit must first be positively attributed to the weapon that caused the kill. The plugin is therefore an early development build, not a finished playable NG++ experience.

The XP defaults in the manifest (100) are provisional until enemy XP values and kill attribution are tested.
