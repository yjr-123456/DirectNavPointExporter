# DirectNavPointExporter

`DirectNavPointExporter` is a standalone Unreal Engine 5 runtime plugin that samples reachable points directly from the current world `NavMesh`, without exporting intermediate navmesh files first.

## Overview

The plugin is designed to:

- sample reachable points directly from runtime `RecastNavMesh`
- avoid the `navmesh -> file export -> post-processing` workflow
- expose Blueprint-callable runtime APIs
- cache query results by sampling parameters
- rebuild sampling context automatically when the map changes
- optionally integrate with `UnrealCV`
- provide reachable-point debug visualization

## Features

- Runtime sampling from the current world `RecastNavMesh`
- Parameterized reachable-point queries
- Query result cache keyed by sampling and filter parameters
- Automatic `WorldSubsystem` initialization
- Automatic cache invalidation on world change
- Blueprint function library for gameplay and runtime use
- Optional `UnrealCV` command integration
- Radius-based reachable-point debug visualization

## Requirements

- Unreal Engine 5.6
- A valid `RecastNavMesh` in the target map

If a map has no usable `NavMesh`, sampling functions return `false` and output empty results.

## Installation

1. Copy the plugin into your project:

```text
YourProject/Plugins/DirectNavPointExporter
```

2. Open the project.
3. Enable the plugin if needed.
4. Build the project.

## Runtime Model

The plugin uses a `WorldSubsystem` to manage sampling state automatically.

This means:

- no manager actor is required in every map
- the subsystem is created automatically for game worlds
- the sampling context is rebuilt when the map changes
- repeated queries with the same parameters hit cache

## Main Blueprint Functions

Blueprint category:

- `Direct Nav Point Exporter`

Core functions:

- `Get Reachable Points Cached`
- `Get Default Cached Free Points From World NavMesh`
- `Get Reachable Points In Radius Cached`
- `Refresh Free Point Cache`
- `Invalidate Free Point Cache`
- `Get Free Point Cache Status`
- `Has Valid World NavMesh`
- `Get World NavMesh Bounds`

Area debug functions:

- `Build Reachable Area Cached`
- `Show Reachable Area Cached`
- `Show Default Cached Reachable Area`
- `Clear Reachable Area Debug`

## Query Parameters

Main query struct:

- `FDirectNavReachablePointQueryConfig`

Important fields:

- `GridSpacing`
- `ValidationExtent`
- `bApplyWorldFilter`
- `bFilterUnderOverhangs`
- `MinClearHeightAbove`
- `OverhangTraceDistance`
- `bFilterOnObstacles`

Recommended starting values:

- `GridSpacing = 100`
- `ValidationExtent = (10, 10, 10)`
- `bApplyWorldFilter = true`
- `bFilterUnderOverhangs = true`
- `MinClearHeightAbove = 200`
- `OverhangTraceDistance = 500`
- `bFilterOnObstacles = true`

## Reachability Strategy

The current sampling strategy is deterministic and grid-based.

In short:

1. Read the current world `RecastNavMesh`
2. Compute navmesh bounds
3. Keep only the largest connected ground region
4. Sample a 2D grid over the region
5. Project each grid point to navmesh
6. Optionally apply world collision filters
7. Cache the final valid points

This is not random sampling.

## Reachable Area Visualization

`Show Reachable Area Cached` and `Show Default Cached Reachable Area` now draw actual reachable points inside a radius.

This is useful when:

- tuning `GridSpacing`
- tuning `MinClearHeightAbove`
- checking the effect of world filtering
- limiting debug render cost to a local area

Current area visualization is:

- radius-limited point rendering
- not a direct render of native UE navmesh polygons
- capped by `MaxCellsToDraw` in the display config

## Optional UnrealCV Integration

If your project integrates this plugin with `UnrealCV`, the following commands can be exposed:

- `vget /reachablepoints`
- `vget /reachablepoints [grid_spacing]`
- `vget /reachablepoints [grid_spacing] [min_clear_height]`
- `vget /reachablepoints/inradius center_x center_y center_z radius [grid_spacing] [min_clear_height]`
- `vget /reachablepoints/count`
- `vget /reachablepoints/status`
- `vset /reachablepoints/refresh`
- `vset /reachablepoints/invalidate`
- `vset /reachablepoints/invalidate context`
- `vset /reachablearea/show center_x center_y center_z radius [grid_spacing] [min_clear_height]`
- `vset /reachablearea/clear`

## API Documentation

See [API.md](API.md) for the full interface reference.

Note:

- `UnrealCV` integration is optional and project-side
- this repository contains the standalone plugin itself

## Behavior on Maps Without NavMesh

If a map has no valid `NavMesh`:

- sampling functions return `false`
- point arrays are empty
- cache status shows no ready context or default result
- debug area rendering does not draw anything

## Repository Layout

```text
DirectNavPointExporter/
├─ DirectNavPointExporter.uplugin
├─ Source/
│  └─ DirectNavPointExporterRuntime/
│     ├─ Public/
│     └─ Private/
└─ README.md
```

## Notes

- `Binaries/` and `Intermediate/` are generated and should not be versioned
- the plugin is intended to stay independent from legacy nav export plugins
- for large maps, prefer cached queries and area visualization over repeated full point dumps
- for large maps, prefer cached queries and radius-limited point visualization over repeated full point dumps
