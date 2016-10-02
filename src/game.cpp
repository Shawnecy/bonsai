#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <constants.hpp>

#include <shader.cpp>
#include <objloader.cpp>
#include <controls.cpp>

#include <simplex.cpp>
#include <perlin.cpp>

#include <math.h>
#include <game.h>

// #include <common/controls.hpp>

void initWindow( int width, int height )
{
  // Initialise GLFW
  if( !glfwInit() )
  {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    getchar();
    return;
  }

  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow( width, height, "Playground", NULL, NULL);
  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    getchar();
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glClearColor(0.35f, 0.0f, 0.5f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);

  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
}

void GenChunk( v4* Buffer, int numVoxels, v3 Offset )
{
  srand(time(NULL));
  PerlinNoise pn(rand());

  for ( int i = 0; i < numVoxels; ++i )
  {
    Buffer[i].x = i%CHUNK_WIDTH + Offset.x;
    Buffer[i].y = (i/CHUNK_WIDTH) % CHUNK_HEIGHT + Offset.y;
    Buffer[i].z = i/(CHUNK_WIDTH*CHUNK_HEIGHT) + Offset.z;

    double InX = (double)Buffer[i].x/(double)CHUNK_WIDTH;
    double InY = (double)Buffer[i].y/(double)CHUNK_HEIGHT;
    double InZ = (double)Buffer[i].z/(double)CHUNK_DEPTH;

    double l = pn.noise(InX, InY, InZ);
    Buffer[i].w = floor(l + 0.5);

    /* Buffer[i].w = 1; */
  }

  return;
}

bool IsFilled( v4* VoxelBuffer, int chunkVol, int chunkWidth, int chunkHeight, int idx )
{
  bool isFilled = false;

  if ( idx < 0 ) return isFilled;
  if ( idx > chunkVol ) return isFilled;

  if ( VoxelBuffer[idx].w == 1 )
  {
    isFilled = true;
  }

  return isFilled;
}

void BufferFace( GLfloat* worldVertexData, float* Verts, int sizeofVerts, int numVerts, int *worldVertCount)
{
  triCount += 2;

  memcpy( &worldVertexData[*worldVertCount], Verts, sizeofVerts );
  *worldVertCount += numVerts;
}

void BufferRightFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] =
  {
    // Right
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,

    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

void BufferLeftFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] = {
    // Left
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,

    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

void BufferTopFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] = {
    // Top
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,

    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

void BufferBottomFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] = {
    // Bottom
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,

    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

void BufferFrontFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] = {
    // Front
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,

    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z +  VOXEL_RADIUS,
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

void BufferBackFace( glm::vec3 worldP, GLfloat* worldVertexData, int *worldVertCount)
{
  float localVertexData[] = {
    // Back
    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,

    worldP.x +  VOXEL_RADIUS, worldP.y +  VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x +  VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
    worldP.x + -VOXEL_RADIUS, worldP.y + -VOXEL_RADIUS, worldP.z + -VOXEL_RADIUS,
  };

  BufferFace( worldVertexData,
      localVertexData,
      sizeof(localVertexData),
      ArrayCount(localVertexData),
      worldVertCount);
}

bool IsRightChunkBoundary( v3 ChunkDim, int idx )
{
  return (idx+1) % ChunkDim.x == 0;
}

bool IsLeftChunkBoundary( v3 ChunkDim, int idx )
{
  return (idx) % ChunkDim.x == 0;
}

bool IsTopChunkBoundary( v3 ChunkDim, int idx )
{
  return ((idx/ChunkDim.x)+1) % ChunkDim.y == 0;
}

bool IsBottomChunkBoundary( v3 ChunkDim, int idx )
{
  return (idx/ChunkDim.x) % ChunkDim.y == 0;
}

void DrawChunk(
    v4* VoxelBuffer,
    v3 ChunkDim,

    GLfloat* worldVertexData,
    int sizeofVertexData,

    GLuint &colorbuffer,
    GLuint &vertexbuffer)
{
  triCount=0;
  int worldVertCount = 0;
  int numVoxels = ChunkDim.x * ChunkDim.y * ChunkDim.z;

  // Clear out render from last frame
  memset( worldVertexData, 0, sizeofVertexData );

  for ( int i = 0; i < numVoxels; ++i )
  {
    if ( VoxelBuffer[i].w == 1 )
    {
      int nextIdx  = i+1;
      int prevIdx  = i-1;
      int botIdx   = i-CHUNK_WIDTH;
      int topIdx   = i+CHUNK_WIDTH;
      int backIdx  = i + (CHUNK_WIDTH*CHUNK_HEIGHT);
      int frontIdx = i - (CHUNK_WIDTH*CHUNK_HEIGHT);

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, nextIdx) ||
           IsRightChunkBoundary(ChunkDim, i) )
      {
        BufferRightFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, prevIdx) ||
           IsLeftChunkBoundary(ChunkDim, i) )
      {
        BufferLeftFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, botIdx) ||
           IsBottomChunkBoundary(ChunkDim, i) )
      {
        BufferBottomFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, topIdx) ||
           IsTopChunkBoundary(ChunkDim, i) )
      {
        BufferTopFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, backIdx) )
      {
        BufferFrontFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }

      if ( ! IsFilled(VoxelBuffer, CHUNK_VOL, CHUNK_WIDTH, CHUNK_HEIGHT, frontIdx) )
      {
        BufferBackFace(
          glm::vec3(VoxelBuffer[i].x, VoxelBuffer[i].y, VoxelBuffer[i].z),
          worldVertexData,
          &worldVertCount
        );
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeofVertexData, worldVertexData, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_WorldVertexColorData), g_WorldVertexColorData, GL_STATIC_DRAW);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // 2nd attribute buffer : colors
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(
    1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  glDrawArrays(GL_TRIANGLES, 0, WORLD_VERTEX_COUNT);
  /* printf("Draw Complete\n"); */
}

int main( void )
{
  static int count = 0;

  int width, height;

  width = 1920;
  height = 1080;

  initWindow(width, height);

  v3 ChunkDim = V3( CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 Projection = glm::perspective(
      glm::radians(45.0f),  // FOV
      (float)width / (float)height,  // display ratio
      0.1f,     // near margin
      500.0f);  // far margin

  // CameraP in world space
  glm::vec3 CameraP = glm::vec3(
      CHUNK_WIDTH+CHUNK_HEIGHT+CHUNK_DEPTH*0.3,
      CHUNK_WIDTH+CHUNK_HEIGHT+CHUNK_DEPTH*0.3,
      CHUNK_WIDTH+CHUNK_HEIGHT+CHUNK_DEPTH*0.3
    );

  GLuint vertexbuffer;
  GLuint normalbuffer;
  GLuint colorbuffer;

  glGenBuffers(1, &vertexbuffer);
  glGenBuffers(1, &colorbuffer);

  double lastPrintTime = glfwGetTime();
  double lastFrameTime = lastPrintTime;
  int nbFrames = 0;

  glfwWindowHint(GLFW_SAMPLES, 4);

  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;

  bool res = loadOBJ("cube.obj", vertices, uvs, normals);

  v4* VoxelBuffer = (v4*)malloc(CHUNK_VOL*sizeof(v4));
  GLfloat *worldVertexData = (GLfloat *)malloc(WORLD_VERTEX_BUFFER_SIZE);

  for ( int i = 0; i < CHUNK_VOL; ++i )
  {
    memcpy( &g_WorldVertexColorData[i*VERT_PER_VOXEL*3], g_VoxelColorBuffer, sizeof(g_VoxelColorBuffer) );
  }

  GenChunk( VoxelBuffer, CHUNK_VOL, V3(0,0,0) );

  /*
   *  Main Render loop
   */
 do {

    computeMatricesFromInputs(&CameraP);

    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Measure speed
    double currentTime = glfwGetTime();
    double dtForFrame = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    nbFrames++;

    if ( currentTime - lastPrintTime >= 1.0 )
    {
      printf("%f ms/frame\n", 1000.0/(double)nbFrames );
      printf("%d triangles/frame\n", triCount );
      nbFrames = 0;
      lastPrintTime += 1.0;
    }

    if ( glfwGetKey(window, GLFW_KEY_ENTER ) == GLFW_PRESS )
    {
       GenChunk( VoxelBuffer, CHUNK_VOL, V3(0,0,0) );
    }

    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawChunk(
      VoxelBuffer,
      ChunkDim,

      worldVertexData,
      WORLD_VERTEX_BUFFER_SIZE,

      vertexbuffer,
      colorbuffer
    );

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

    // Use our shader
    glUseProgram(programID);

    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
         glfwGetKey(window, GLFW_KEY_Q )      != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0 );

  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &colorbuffer);
  glDeleteVertexArrays(1, &VertexArrayID);
  glDeleteProgram(programID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  free(worldVertexData);
  free(VoxelBuffer);

  return 0;
}

