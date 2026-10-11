# Sniper Shooter Prototype

This repository is the foundation for a UE5 sniper shooter game prototype inspired by tactical military shooters and the visual ambition of large-scale battle royale games.

## Goal

Create a playable prototype that captures the feel of a sniper-focused tactical shooter in Unreal Engine 5, with a clear path toward more advanced graphics, environment detail, and gameplay polish over time.

## Important note

Achieving full PUBG-level visuals and production scope is not realistic for a solo developer in a single pass. This repo is intentionally scoped as a prototype-first pipeline that starts with core gameplay and gradually layers in visual fidelity, animation quality, lighting, effects, and world details.

## Core prototype priorities

- First-person shooter controls
- Sniper rifle aiming and scope behavior
- One or more enemy targets
- Basic environment with cover and depth
- Lighting pass and atmospheric mood
- Simple UI for health, weapon state, and mission status
- Iterative visual improvement without rewriting the project

## Recommended stack

- Unreal Engine 5
- Blueprints for fast iteration
- C++ for gameplay systems when needed
- Houdini or FBX assets later, but placeholder assets are acceptable in the prototype phase

## Development roadmap

### Phase 1: Prototype foundation
- Create UE5 project structure
- Set up basic player movement
- Add camera and aiming behavior
- Implement a sniper rifle with scope zoom
- Add simple enemy AI
- Build a small test arena

### Phase 2: Gameplay feel
- Recoil, sway, and weapon handling tuning
- Hit detection and damage logic
- UI overlays for HUD and crosshair
- Environment cover and line-of-sight checks
- Sound and muzzle flash polish

### Phase 3: Visual quality ramp
- Directional lighting, post-processing, and volumetric fog
- Better materials and textures
- Environment composition with depth and atmosphere
- More advanced foliage, terrain, and weather
- Screen-space reflections and cinematic color grading

### Phase 4: Production polish
- Multiple maps or biomes
- More enemy behaviors
- Better animation and character fidelity
- Sound design pass and weapon feel tuning
- Performance optimization for target hardware

## Suggested project structure

- `Content/` - game assets and maps
- `Config/` - project configuration
- `Source/` - C++ gameplay code
- `docs/` - design and roadmap notes

## Recommended first milestone

Build a single playable level with:
- a player character
- a scoped sniper rifle
- an enemy target or patrol system
- simple shooting feedback
- a basic HUD

This is the first objective before trying to chase a battle royale look.

## Current status

Roadmap Milestones 1 and 2 are implemented as C++ source. The project needs no content assets: the test arena, enemies and rifle are built from engine basic shapes at runtime, and input is bound in `Config/DefaultInput.ini`.

Implemented:
- First-person movement, sprint, crouch and jump
- Sniper rifle with scope zoom, sway, breath hold, recoil, ammo and reload
- Hitscan firing with headshot bonus, hit markers and a muzzle flash light
- Patrolling enemies that spot the player (view cone and line of sight) and shoot back
- Health, mission tracking (eliminate all targets), and win/lose screens with restart
- Canvas HUD: crosshair, scope overlay, health, ammo, breath meter, target count

Not yet verified: this code has not been compiled or run in Unreal Engine yet, so expect a few small fixes on first build.

## Getting started

1. Install Unreal Engine 5.5 (or change `EngineAssociation` in `SniperShooter.uproject` to your version).
2. Right-click `SniperShooter.uproject` and choose **Generate Visual Studio project files**.
3. Open the generated solution, build the `SniperShooter` target (Development Editor), then launch the editor.
4. Press Play. The game mode creates the arena and spawns the enemies.

## Controls

| Key | Action |
| --- | --- |
| W A S D | Move |
| Mouse | Look |
| Left mouse | Fire (bolt action, one shot per click) |
| Right mouse (hold) | Scope |
| Shift | Sprint, or hold breath while scoped |
| Ctrl (hold) | Crouch |
| Space | Jump |
| R | Reload |
| Enter | Restart after the mission ends |

## Code layout

- `Source/SniperShooter/SniperCharacter` - player pawn, input, health
- `Source/SniperShooter/SniperWeaponComponent` - rifle logic
- `Source/SniperShooter/SniperEnemy` - patrol, perception and attack
- `Source/SniperShooter/SniperGameMode` - arena generation, spawning, mission state
- `Source/SniperShooter/SniperHUD` - HUD and scope overlay

## Next step

Milestone 3: replace the generated arena with a hand-built test range (set `bBuildArena` to false in the game mode), then add the lighting pass and atmospheric fog.
