
// TODO(Jesse): rename this to just thumbnail and move to renderer?
struct asset_thumbnail
{
  texture Texture;
  camera  Camera;
};

struct render_debug
{
  u32 BytesSolidGeoLastFrame;
  u32 BytesTransGeoLastFrame;

  u32 DrawCallsLastFrame;
};

enum asset_window_view_mode
{
  AssetWindowViewMode_AssetFiles,
  AssetWindowViewMode_AssetTable,
};

struct engine_debug
{
  memory_arena *Memory;

  picked_world_chunk_static_buffer PickedChunks;
  texture_ptr_block_array Textures;

  ui_debug UiDebug;

  render_debug Render;

  b8 DrawEntityCollisionVolumes;
  b8 DrawWorldAxies;
  b8 TriggerRuntimeBreak;
  b8 ResetAssetNodeView;

  u8 PickedChunkState;
  world_chunk *PickedChunk;

  asset_window_view_mode AssetWindowViewMode;

  asset_id SelectedAsset;
  u64 ModelIndex;

  entity_id SelectedEntity;

  // World Edit debug
  //
  untextured_3d_geometry_buffer WorldEditDebugMesh;
  asset_thumbnail WorldEditDebugThumb;
};

enum engine_debug_view_mode
{
  EngineDebugViewMode_Level,
  EngineDebugViewMode_WorldEdit,
  EngineDebugViewMode_Entities,
  EngineDebugViewMode_Assets,
  EngineDebugViewMode_WorldChunks,
  EngineDebugViewMode_Textures,
  EngineDebugViewMode_RenderSettings,
  EngineDebugViewMode_EngineDebug,

  // TODO(Jesse): Make these values bitfields?
  /* EngineDebugViewMode_Level          = (1 << 0), */
  /* EngineDebugViewMode_WorldEdit      = (1 << 1), */
  /* EngineDebugViewMode_Entities       = (1 << 2), */
  /* EngineDebugViewMode_Assets         = (1 << 3), */
  /* EngineDebugViewMode_WorldChunks    = (1 << 4), */
  /* EngineDebugViewMode_Textures       = (1 << 5), */
  /* EngineDebugViewMode_RenderSettings = (1 << 6), */
  /* EngineDebugViewMode_EngineDebug    = (1 << 7), */
};

link_internal engine_debug * GetEngineDebug();
