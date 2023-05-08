
struct game_state
{
  random_series Entropy;
  voxel_synthesis_result BakeResult;

  entity *BakeEntity; // Entity that has the original mesh attached to it.

  entity *CameraTarget;

#if DEBUG_SYSTEM_API
  get_debug_state_proc GetDebugState;
#endif
};
