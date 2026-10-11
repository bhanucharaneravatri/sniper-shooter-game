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

## Next step

The next move is to initialize the Unreal Engine 5 project structure and begin implementing the core gameplay loop in the repository.
