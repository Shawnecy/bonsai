struct world;
struct platform;
struct entity;
struct memory_arena;
struct random_series;

struct game_state
{
  world         *World;
  platform      *Plat;
  memory_arena  *Memory;
  graphics      *Graphics;

  random_series Entropy;

  perlin_noise Noise;

  game_mode Mode;
  model   *Models;
  noise_3d *Turb;
  event_queue EventQueue;

  entity *EntityTable[TOTAL_ENTITY_COUNT];

  entity *Players[MAX_CLIENTS];
};
