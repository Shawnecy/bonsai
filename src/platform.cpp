#ifndef BONSAI_PLATFORM_CPP
#define BONSAI_PLATFORM_CPP

#include <iostream>

#include <platform_constants.h>
#include <bonsai_types.h>
#include <constants.hpp>

#if _WIN32
#include <win32_platform.cpp>
#else
#include <unix_platform.cpp>
#endif

#include <platform.h>

#include <sys/types.h>
#include <sys/stat.h>

GLOBAL_VARIABLE s64 LastGameLibTime = 0;
GLOBAL_VARIABLE input NullInput = {};

b32
GameLibIsNew(const char *LibPath)
{
  b32 Result = False;
  struct stat StatStruct;

  if (stat(LibPath, &StatStruct) == 0)
  {
    if (StatStruct.st_mtime > LastGameLibTime)
    {
      LastGameLibTime = (u64)StatStruct.st_mtime;
      Result = True;
    }
  }
  else
  {
    Error("Stat-ing Game library :( ");
  }

  return Result;
}

THREAD_MAIN_RETURN
ThreadMain(void *Input)
{
  thread_startup_params *ThreadParams = (thread_startup_params *)Input;

  work_queue *Queue = ThreadParams->Queue;

  for (;;)
  {
    u32 OriginalNext = Queue->NextEntry;

    if (OriginalNext != Queue->EntryCount)
    {
      if ( AtomicCompareExchange(&Queue->NextEntry,
                                 (OriginalNext+1)%WORK_QUEUE_SIZE,
                                 OriginalNext) )
      {
        work_queue_entry Entry = Queue->Entries[OriginalNext];
        Entry.Callback(&Entry);
      }

    }
    else
    {
      ThreadSleep( Queue->Semaphore );
    }
  }

  return 0;
}

void
PushWorkQueueEntry(work_queue *Queue, work_queue_entry *Entry)
{
  Queue->Entries[Queue->EntryCount] = *Entry;

  CompleteAllWrites;

  Queue->EntryCount = ++Queue->EntryCount % WORK_QUEUE_SIZE;

  // TODO(Jesse): Is this check correct?
  Assert(Queue->NextEntry != Queue->EntryCount);

  WakeThread( Queue->Semaphore );

  return;
}


#define DefGlProc(ProcType, ProcName) Gl->ProcName = (ProcType)bonsaiGlGetProcAddress(#ProcName); Assert(Gl->ProcName)
void
InitializeOpenGlExtensions(gl_extensions *Gl)
{
  Info("Initializing OpenGL Extensions");

  DefGlProc(PFNGLCREATESHADERPROC, glCreateShader);
  DefGlProc(PFNGLSHADERSOURCEPROC, glShaderSource);
  DefGlProc(PFNGLCOMPILESHADERPROC, glCompileShader);
  DefGlProc(PFNGLGETSHADERIVPROC, glGetShaderiv);
  DefGlProc(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
  DefGlProc(PFNGLATTACHSHADERPROC, glAttachShader);
  DefGlProc(PFNGLDETACHSHADERPROC, glDetachShader);
  DefGlProc(PFNGLDELETESHADERPROC, glDeleteShader);
  DefGlProc(PFNGLCREATEPROGRAMPROC, glCreateProgram);
  DefGlProc(PFNGLLINKPROGRAMPROC, glLinkProgram);
  DefGlProc(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
  DefGlProc(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
  DefGlProc(PFNGLUSEPROGRAMPROC, glUseProgram);
  DefGlProc(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
  DefGlProc(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
  DefGlProc(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
  DefGlProc(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);

  // TODO(Jesse): This function appears to not work on ES 3.1 ..??
  // DefGlProc(PFNGLFRAMEBUFFERTEXTUREPROC, glFramebufferTexture);

  DefGlProc(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
  DefGlProc(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
  DefGlProc(PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);
  DefGlProc(PFNGLGENBUFFERSPROC, glGenBuffers);
  DefGlProc(PFNGLBINDBUFFERPROC, glBindBuffer);
  DefGlProc(PFNGLBUFFERDATAPROC, glBufferData);
  DefGlProc(PFNGLDRAWBUFFERSPROC, glDrawBuffers);
  DefGlProc(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
  DefGlProc(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
  DefGlProc(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
  DefGlProc(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);
  DefGlProc(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
  DefGlProc(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
  DefGlProc(PFNGLUNIFORM3FVPROC, glUniform3fv);
  DefGlProc(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);
  DefGlProc(PFNGLUNIFORM1IPROC, glUniform1i);
  DefGlProc(PFNGLACTIVETEXTUREPROC, glActiveTexture);


  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  AssertNoGlErrors;

  // Platform specific (wgl / glX)
  Gl->glSwapInterval = (PFNSWAPINTERVALPROC)bonsaiGlGetProcAddress("wglSwapIntervalEXT");

  if ( Gl->glSwapInterval )
    Gl->glSwapInterval(1); // vsync

  return;
}

void
PlatformInit(platform *Plat)
{
  Assert(sizeof(u8)  == 1);

  Assert(sizeof(u32) == 4);
  Assert(sizeof(s32) == 4);
  Assert(sizeof(r32) == 4);

  Assert(sizeof(umm) == 8);
  Assert(sizeof(u64) == 8);
  Assert(sizeof(s64) == 8);

  u32 LogicalCoreCount = GetLogicalCoreCount();
  u32 ThreadCount = LogicalCoreCount -1; // -1 because we already have a main thread

  Info("Detected %d Logical cores, creating %d threads", LogicalCoreCount, ThreadCount);

  Plat->Queue.EntryCount = 0;
  Plat->Queue.NextEntry = 0;

  Plat->Queue.Entries = (work_queue_entry *)calloc(sizeof(work_queue_entry), WORK_QUEUE_SIZE);
  Plat->Threads = (thread_startup_params *)calloc(sizeof(thread_startup_params), ThreadCount);
  work_queue *Queue = &Plat->Queue;

  Queue->Semaphore = CreateSemaphore(ThreadCount);

  for (u32 ThreadIndex = 0;
      ThreadIndex < ThreadCount;
      ++ ThreadIndex )
  {
    thread_startup_params *Params = &Plat->Threads[ThreadIndex];
    Params->Self.ThreadIndex = ThreadIndex;
    Params->Queue = Queue;

    CreateThread( ThreadMain, Params );
  }

  Plat->GetHighPrecisionClock = GetHighPrecisionClock;
  Plat->PushWorkQueueEntry = PushWorkQueueEntry;

  return;
}

/*
 *  Poor mans vsync
 */
void
WaitForFrameTime(r64 frameStart, float FPS)
{
  r64 frameTime = GetHighPrecisionClock() - frameStart;

  while (frameTime < (1.0f/FPS))
  {
    frameTime = GetHighPrecisionClock() - frameStart;
  }

  return;
}


inline b32
FileExists(const char *Path)
{
  b32 Result = False;

  if (FILE *file = fopen(Path, "r"))
  {
    fclose(file);
    Result = True;
  }

  return Result;
}


b32
SearchForProjectRoot(void)
{
  b32 Result = False;

  if (FileExists(".root_marker"))
  {
    Result = True;
  }
  else
  {
    b32 ChdirSuceeded = (chdir("..") == 0);
    b32 NotAtFilesystemRoot = (!IsFilesystemRoot(GetCwd()));

    if (ChdirSuceeded && NotAtFilesystemRoot)
      Result = SearchForProjectRoot();

    else
      Result = False;

  }

  return Result;
}


int
main(s32 NumArgs, char ** Args)
{
  Info("Initializing Bonsai");

  if (!SearchForProjectRoot())
  {
    Error("Couldn't find root dir, exiting.");
    return False;
  }

  Info("Running out of : %s", GetCwd() );

  platform Plat = {};
  PlatformInit(&Plat);

  os Os = {};
  Os.ContinueRunning = True;

  Plat.WindowHeight = SCR_HEIGHT;
  Plat.WindowWidth = SCR_WIDTH;

  GameLibIsNew(GAME_LIB);  // Hack to initialize the LastGameLibTime static

  shared_lib GameLib = OpenLibrary(GAME_LIB);
  if (!GameLib) { Error("Loading GameLib :( "); return False; }

  game_init_proc GameInit = (game_init_proc)GetProcFromLib(GameLib, "GameInit");
  if (!GameInit) { Error("Retreiving GameInit from Game Lib :( "); return False; }

  game_main_proc GameUpdateAndRender = (game_main_proc)GetProcFromLib(GameLib, "GameUpdateAndRender");
  if (!GameUpdateAndRender) { Error("Retreiving GameUpdateAndRender from Game Lib :( "); return False; }

  b32 WindowSuccess = OpenAndInitializeWindow(&Os, &Plat);
  if (!WindowSuccess) { Error("Initializing Window :( "); return False; }

  Assert(Os.Window);

  InitializeOpenGlExtensions(&Plat.GL);

  char *glslVer = (char*)glGetString ( GL_SHADING_LANGUAGE_VERSION );
  Info(glslVer);
  Assert(glslVer);

  r64 GLSL_Version = atof(glslVer);
  Info("GLSL verison : %f", GLSL_Version );

  if (GLSL_Version >= 3.3)
    Plat.GlslVersion = "330";

  else
    Plat.GlslVersion = "310ES";


  game_state *GameState = GameInit(&Plat);
  if (!GameState) { Error("Initializing Game State :( "); return False; }

  /*
   *  Main Game loop
   */

  r64 lastTime = Plat.GetHighPrecisionClock();

  while ( Os.ContinueRunning )
  {
    Plat.dt = (r32)ComputeDtForFrame(&lastTime);

    // Debug("%f ", Plat.dt);

    // Flush Message Queue
    while ( ProcessOsMessages(&Os, &Plat) );

    if ( GameLibIsNew(GAME_LIB) )
    {
      CloseLibrary(GameLib);

      GameLib = OpenLibrary(GAME_LIB);
      GameUpdateAndRender = (game_main_proc)GetProcFromLib(GameLib, "GameUpdateAndRender");
    }

    GameUpdateAndRender(&Plat, GameState);
    BonsaiSwapBuffers(&Os);

    /* float FPS = 60.0f; */
    /* WaitForFrameTime(lastTime, FPS); */
  }

  Terminate(&Os);

  return True;
}

#endif
