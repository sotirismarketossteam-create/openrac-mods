# NG++ — Ratchet & Clank (RAC1)

This directory is the separate RAC1 edition. The existing `ng-plus-plus/` directory remains the Going Commando (RAC2) edition; do not merge RAC1 changes into the RAC2 project.

## Agreed progression design

- Weapon levels run from **1 to 10**.
- Each weapon tracks and saves its own experience and level.
- Only the weapon responsible for an enemy's kill receives that kill's XP.
- Each weapon has its own launcher setting for the XP needed to reach its next level.
- The launcher settings also expose the level cap, linear/increasing XP curve, level-up notices, damage scaling and ammo-capacity scaling.

## Current state

`openrac-mod.json` defines the RAC1-only configuration and declares the RAC1 hook points that the gameplay implementation will use. It is a configuration/porting scaffold, **not yet a playable binary**. In particular, the RAC1 weapon item identifiers, hit/kill attribution path, save format and per-weapon effects still need to be implemented and tested against the RAC1 SDK before this edition can be built and enabled safely.

The XP defaults are deliberately conservative placeholders (100 XP per level) until the kill attribution and enemy XP values are verified in-game. Changing an individual weapon's setting does not change another weapon's requirement.
