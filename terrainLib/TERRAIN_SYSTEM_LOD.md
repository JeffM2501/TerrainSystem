# Terrain System & Index-Based Level of Detail (LOD) Architecture

## 1. Overview

The `terrainLib` library implements a tile-based heightfield terrain engine built on top of **raylib** and **rlgl** (OpenGL). It divides terrain into discrete square tiles (`TerrainTile`), each defined by a regular grid of elevation values, multi-layer texturing via splatmaps, and dynamic Level of Detail (LOD) mesh rendering.

The system features an **index-driven Level of Detail (LOD)** mechanism. Rather than generating, maintaining, or swapping separate vertex buffers for different detail levels, all tiles share a single, full-resolution vertex buffer while an optimized, globally shared index buffer (Element Array Buffer / EBO) subsamples the vertex grid at progressively lower resolutions.

---

## 2. Core Components in `terrainLib`

The library consists of three primary modules:

| Component | Files | Description |
|---|---|---|
| **Terrain Tile & Config** | [`TerrainTile.h`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/include/TerrainTile.h), [`TerrainTile.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/src/TerrainTile.cpp) | Defines configuration parameters (`TerrainInfo`), tile coordinates (`TerrainPosition`), heightmap storage, materials, and GPU buffer handles (`VaoId`, `VboId`). |
| **Mesh Generation** | [`TerrainBuilder.h`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/include/TerrainBuilder.h), [`TerrainBuilder.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/src/TerrainBuilder.cpp) | Constructs vertex attributes (positions, normals, UVs, colors) and generates the multi-level index buffer. |
| **Renderer** | [`TerrainRender.h`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/include/TerrainRender.h), [`TerrainRender.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/src/TerrainRender.cpp) | Binds shader uniforms, material diffuse maps, and splatmap masks, then dispatches draw calls for a tile at a requested LOD index. |

---

## 3. Terrain Representation & Geometry

### 3.1 Terrain Configuration (`TerrainInfo`)
```cpp
struct TerrainInfo
{
    uint8_t TerrainGridSize = 128; // Quads per tile edge
    float TerrainTileSize = 128;   // World-space size of a tile (meters/units)
    float TerrainMinZ = -50;       // Minimum elevation mapped from heightmap
    float TerrainMaxZ = 100;       // Maximum elevation mapped from heightmap
};
```
- A tile consists of $128 \times 128$ quads by default.
- The vertex grid required to represent $128 \times 128$ quads has $(128 + 1) \times (128 + 1) = 129 \times 129 = 16,641$ vertices.

### 3.2 Heightmap Storage & 1-Ring Border Margin
When heights are loaded from a source image (`TerrainTile::SetHeightsFromImage`), the heightmap buffer is allocated with extra padding:
$$\text{Allocated Grid Dimension} = (\text{TerrainGridSize} + 3) \times (\text{TerrainGridSize} + 3)$$

```cpp
float TerrainTile::GetLocalHeight(int x, int y) const
{
    size_t index = (y + 1) * (Info.TerrainGridSize + 3) + x + 1;
    return TerrainHeightMap[index];
}
```

#### Why the $(N + 3)$ padding exists:
To compute smooth normals across the entire tile—including its boundary vertices ($x=0$ or $x=\text{TerrainGridSize}$)—the normal calculation queries neighbors at $(x-1)$ and $(x+1)$. The 1-vertex apron/skirt around all four sides allows `GetLocalHeight(x - 1, y)` and `GetLocalHeight(x + 1, y)` to access valid height values without branching or bounds violations.

### 3.3 Normal Calculation
`TileMeshBuilder::ComputeNormalForLocation` evaluates central difference cross-products using four orthogonal neighbor samples:
- $A = (-1, 0, Z_{x-1, y} - Z_{x, y})$
- $B = (0, 1, Z_{x, y+1} - Z_{x, y})$
- $C = (1, 0, Z_{x+1, y} - Z_{x, y})$
- $D = (0, -1, Z_{x, y-1} - Z_{x, y})$

Four triangular facet normals around the center point are computed via cross products:
$$\vec{N}_1 = \vec{PB} \times \vec{PA}, \quad \vec{N}_2 = \vec{PC} \times \vec{PB}, \quad \vec{N}_3 = \vec{PD} \times \vec{PC}, \quad \vec{N}_4 = \vec{PA} \times \vec{PD}$$
These four vectors are summed and normalized to produce a smooth per-vertex surface normal.

### 3.4 Vertex Attribute Buffers (VBOs)
Each tile owns a dedicated Vertex Array Object (`tile.VaoId`) and individual VBOs:
- **VBO 0**: Vertex positions $(X, Y, Z)$ where $X, Y \in [0, \text{TerrainTileSize}]$.
- **VBO 1**: Base UV coordinates normalized to $[0.0, 1.0]$.
- **VBO 2**: Smoothed vertex normals $(N_x, N_y, N_z)$.
- **VBO 3**: Vertex colors (default RGBA `255, 255, 255, 255`).
- **VBO 4**: Tangents (optional / default).
- **VBO 5**: Secondary tiled detail UV coordinates scaled by $\frac{\text{TerrainTileSize}}{\text{TerrainGridSize} \times 4}$.
- **VBO 6**: Element index buffer (points to the **shared global index buffer**).

---

## 4. Index-Based Level of Detail (LOD) Management

The core efficiency of `terrainLib` lies in how it handles levels of detail: **vertices are never modified or re-uploaded for LOD changes; only the vertex indices passed to the draw call vary.**

### 4.1 Global Shared Index Buffer
Because every terrain tile has an identical grid topology ($128 \times 128$ quads), the index sequence for any LOD is identical for all tiles.

In [`TerrainBuilder.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/terrainLib/src/TerrainBuilder.cpp):
```cpp
int IndexList = -1; // OpenGL Buffer ID for shared EBO
TerrainLODTriangleInfo LODInfos[MaxLODLevels];
```
- `SetupIndexes(tile)` runs once. If `IndexList >= 0`, it immediately returns.
- Every tile sets its element buffer handle to `tile.VboId[6] = IndexList` and points its LOD metadata pointer to `tile.LODs = LODInfos`.
- This means GPU memory for indices is allocated **only once for the entire application**, regardless of how many tiles are loaded into the world.

### 4.2 Seamless LOD Transitions & The 16 Edge Permutations
When adjacent tiles render at different LOD levels (e.g., Tile A at LOD 0 and Tile B at LOD 1), the higher-detail tile contains boundary vertices that the lower-detail neighbor skips. Without edge stitching, this mismatch causes visible cracks (T-junction tears).

To eliminate cracks with **zero CPU draw overhead and single-draw-call performance**, the system implements a **finer-adapts-to-coarser** index permutation strategy:

```cpp
enum TerrainTransitionEdge : uint8_t
{
    TRANSITION_NONE  = 0,
    TRANSITION_NORTH = 1 << 0, // +Y edge (y = GridSize)
    TRANSITION_SOUTH = 1 << 1, // -Y edge (y = 0)
    TRANSITION_EAST  = 1 << 2, // +X edge (x = GridSize)
    TRANSITION_WEST  = 1 << 3, // -X edge (x = 0)
};

static constexpr uint8_t MaxTransitionMasks = 16;
```

For each of LODs 0, 1, and 2, all $2^4 = 16$ edge combinations are precomputed and stored sequentially in the shared index buffer. For the coarsest level (LOD 3), no coarser transition is possible, so all 16 masks alias to the base LOD 3 index range without taking extra VRAM.

### 4.3 Macro-Block Triangulation Architecture ($2S \times 2S$ Cells)
To seamlessly handle arbitrary combinations of the 4 transition edges without special corner cases, the grid is partitioned into macro-blocks of size $2S \times 2S$ (where $S = 2^{\text{LOD}}$):
- LOD 0 ($S = 1$, $2S = 2$): $64 \times 64$ blocks of size $2 \times 2$
- LOD 1 ($S = 2$, $2S = 4$): $32 \times 32$ blocks of size $4 \times 4$
- LOD 2 ($S = 4$, $2S = 8$): $16 \times 16$ blocks of size $8 \times 8$

Each macro-block consists of 9 vertices arranged in a $3 \times 3$ grid:
```text
(y2) v02 -------- v12 -------- v22
      |       \    |    /       |
      |        \   |   /        |
(y1) v01 -------- v11 -------- v21  <-- v11 is Center (C)
      |        /   |   \        |
      |       /    |    \       |
(y0) v00 -------- v10 -------- v20
     (x0)         (x1)         (x2)
```

Each of the 4 quadrants (South, East, North, West) independently adapts to boundary transitions:
1. **South Quadrant ($y = 0$)**:
   - If `TRANSITION_SOUTH` is set and $y = 0$: Midpoint $v_{10}$ is dropped. A single triangle $(v_{00}, v_{20}, v_{11})$ spans the boundary of length $2S$.
   - Otherwise: Standard 2 triangles $(v_{00}, v_{10}, v_{11})$ and $(v_{10}, v_{20}, v_{11})$.
2. **East Quadrant ($x_2 = \text{GridSize}$)**:
   - If `TRANSITION_EAST` is set and $x_2 = \text{GridSize}$: Midpoint $v_{21}$ is dropped $\to (v_{20}, v_{22}, v_{11})$.
   - Otherwise: 2 triangles $(v_{20}, v_{21}, v_{11})$ and $(v_{21}, v_{22}, v_{11})$.
3. **North Quadrant ($y_2 = \text{GridSize}$)**:
   - If `TRANSITION_NORTH` is set and $y_2 = \text{GridSize}$: Midpoint $v_{12}$ is dropped $\to (v_{22}, v_{02}, v_{11})$.
   - Otherwise: 2 triangles $(v_{22}, v_{12}, v_{11})$ and $(v_{12}, v_{02}, v_{11})$.
4. **West Quadrant ($x = 0$)**:
   - If `TRANSITION_WEST` is set and $x = 0$: Midpoint $v_{01}$ is dropped $\to (v_{02}, v_{00}, v_{11})$.
   - Otherwise: 2 triangles $(v_{02}, v_{01}, v_{11})$ and $(v_{01}, v_{00}, v_{11})$.

All triangles maintain strictly counter-clockwise (CCW) winding and outward normal orientation.

#### Mathematical Guarantee of Watertightness:
When Tile A (at LOD $L$) borders Tile B (at coarser LOD $L+1$):
- Tile A detects that its neighbor is coarser, so it activates the transition bit for that edge.
- Tile A's boundary along that edge drops intermediate vertices and connects directly between step-$2S$ vertices.
- Tile B is at step $2S$, so its boundary naturally connects between those exact same step-$2S$ vertices.
- Both meshes share identical boundary line segments with identical $(X, Y, Z)$ coordinates $\implies$ **100% watertight, zero gaps, zero T-junctions**.

---

## 5. Rendering Pipeline & LOD Dispatch

### 5.1 Metadata Storage: `TerrainLODTriangleInfo`
```cpp
struct TerrainLODTriangleInfo
{
    size_t IndexStart = -1; // Starting triangle offset in the index buffer
    size_t IndexCount = 0;  // Number of triangles for this LOD + mask
};
```
The global array `LODInfos` contains $\text{MaxLODLevels} \times \text{MaxTransitionMasks} = 4 \times 16 = 64$ entries.

### 5.2 Draw Execution in `TerainRenderer::Draw`
```cpp
void TerainRenderer::Draw(TerrainTile& tile, size_t lod = 0, uint8_t transitionMask = 0);
```

The rendering sequence is:
1. **Transform & Matrices**: Uploads model, view, projection, and normal matrices.
2. **Materials & Splatmap**: Binds diffuse textures and splatmap masks.
3. **VAO Binding**: `rlEnableVertexArray(tile.VaoId);` binds all vertex attributes and the shared EBO.
4. **Single-Draw-Call Dispatch**:
   ```cpp
   size_t lodIndex = (lod * MaxTransitionMasks) + (transitionMask & 0x0F);
   rlDrawVertexArrayElements(
       (int)tile.LODs[lodIndex].IndexStart * 3, // Start index offset (elements)
       (int)tile.LODs[lodIndex].IndexCount * 3, // Number of index elements to draw
       0
   );
   ```

### 5.3 Neighbor LOD Determination at Runtime
In both the editor ([`TerrainDocument.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/editor/src/TerrainDocument.cpp)) and runtime game loop ([`main.cpp`](file:///c:/Users/jeffm/Desktop/Projects/TerrainSystem/game/src/main.cpp)), transition masks are determined dynamically by comparing neighbor tile LODs:

```cpp
uint8_t transitionMask = TRANSITION_NONE;
int northLOD = GetNeighborLOD(tile.Origin.X, tile.Origin.Y + 1);
int southLOD = GetNeighborLOD(tile.Origin.X, tile.Origin.Y - 1);
int eastLOD  = GetNeighborLOD(tile.Origin.X + 1, tile.Origin.Y);
int westLOD  = GetNeighborLOD(tile.Origin.X - 1, tile.Origin.Y);

if (northLOD > lod) transitionMask |= TRANSITION_NORTH;
if (southLOD > lod) transitionMask |= TRANSITION_SOUTH;
if (eastLOD  > lod) transitionMask |= TRANSITION_EAST;
if (westLOD  > lod) transitionMask |= TRANSITION_WEST;

Renderer.Draw(tile, lod, transitionMask);
```

---

## 6. Architecture Advantages & Performance Characteristics

### Advantages
1. **Watertight Mesh**: Completely resolves T-junction cracks and gaps between adjacent tiles of different LODs.
2. **Single Draw Call Per Tile**: Unlike modular approaches that require 5 draw calls per tile (interior + 4 edges), every tile is rendered in **exactly 1 draw call** (`glDrawElements` / `rlDrawVertexArrayElements`).
3. **Zero Runtime GPU Allocation**: All 16 permutations per LOD are precomputed once at startup in the shared 3.9 MB index buffer. Switching transitions involves only changing the integer index offset into the EBO.
4. **Backward Compatible**: Calling `Renderer.Draw(tile, lod)` without a transition mask defaults to `TRANSITION_NONE` (mask 0), maintaining full compatibility with non-stitched rendering.
5. **Cache-Friendly 16-Bit Indices**: Maximum vertex index on a $128 \times 128$ quad tile is $16,641$, comfortably fitting in `uint16_t` ($0 \dots 65,535$).
