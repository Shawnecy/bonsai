struct composite_render_group
{
  shader Shader;
};

struct gaussian_render_group
{
  framebuffer FBOs[2];
  texture *Textures[2];

  shader Shader;

  shader DebugTextureShader0;
  shader DebugTextureShader1;
};

struct render_settings
{
  b32 Headless;

  // NOTE(Jesse): These have to be 32bit because they get passed to shaders
  // and the shader system is too dumb to handle 8bit values
  b32 UseSsao;
  b32 UseShadowMapping;
  b32 UseLightingBloom;
};

struct graphics
{
  render_settings Settings;

  camera *Camera;
  r32 Exposure;

  // TODO(Jesse): None of these need to be pointers..
  g_buffer_render_group  *gBuffer;
  ao_render_group        *AoGroup;
  shadow_render_group    *SG;

  lighting_render_group  Lighting;
  gaussian_render_group  Gaussian;
  composite_render_group CompositeGroup;

  gpu_mapped_element_buffer GpuBuffers[2];
  u32 GpuBufferWriteIndex;

  memory_arena *Memory;
};
