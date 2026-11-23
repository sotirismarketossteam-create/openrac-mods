# NG++ — Ratchet & Clank (RAC1)

This is the separate RAC1 edition. The existing `ng-plus-plus/` directory remains the Going Commando (RAC2) edition; RAC1 work stays on the `ng-plus-plus-rac1` branch.

## Agreed progression design

- Weapon levels run from **1 to 10**.
- Each weapon tracks and saves its own experience and level.
- Only the weapon responsible for an enemy's kill receives that kill's XP.
- Each weapon has its own setting for the XP needed to reach its next level.
- Launcher settings expose the level cap, linear/increasing XP curve, level-up notices, damage scaling and ammo-capacity scaling.

## Implementation status

- [x] RAC1-only manifest and launcher settings.
- [x] Independent per-weapon progression state (14 weapon slots, levels 1–10).
- [x] Versioned save-payload serialization for OpenRAC's per-save mod data API.
- [x] Per-weapon XP requirements and linear/increasing requirement calculation.
- [ ] Verified RAC1 item-ID mapping.
- [ ] Reliable weapon-to-kill attribution in RAC1.
- [ ] Connect progression state to OpenRAC save callbacks.
- [ ] Damage/ammo effects and level-up notices.
- [ ] Native plugin entry point, packaging and in-game testing on NTSC and PAL.

The progression core is deliberately separate from game hooks. A kill award must not be called until RAC1 has positively identified the weapon responsible for that kill. This avoids giving XP to the wrong weapon. The XP defaults in the manifest (100) are provisional until enemy XP values and kill attribution are tested.

## Build note

The current CMake target builds the standalone progression core only. It is not yet an installable OpenRAC mod and must not be enabled as one.
