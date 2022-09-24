
struct game_state;

struct thread_startup_params
{
  bonsai_worker_thread_init_callback InitProc;
  bonsai_worker_thread_callback GameWorkerThreadCallback;

  game_state *GameState;

  volatile u32 *HighPriorityWorkerCount;
  bonsai_futex *SuspendWorkerThreads;

  semaphore  *GlobalQueueSemaphore;
  work_queue *LowPriority;
  work_queue *HighPriority;
  thread Self;
};

struct mesh_freelist;
struct thread_local_state
{
  memory_arena  *PermMemory;
  memory_arena  *TempMemory;
  mesh_freelist *MeshFreelist;
  perlin_noise  *Noise;
};

