#define PLATFORM_LIBRARY_AND_WINDOW_IMPLEMENTATIONS 1
#define DEBUG_SYSTEM_API 1

#include <bonsai_debug/debug.cpp>
#include <bonsai_stdlib/test/utils.h>


link_internal void
TestTable(debug_ui_render_group* Group)
{
  PushTableStart(Group);
  for (u32 Index = 0;
      Index < 2;
      ++Index)
  {
    interactable_handle Interaction = PushButtonStart(Group, (umm)"TestButtonInteraction"^(umm)Index);
      PushColumn(Group, CS("String1"));
      PushColumn(Group, CS("String2"));
      PushColumn(Group, CS("String3"));
      PushColumn(Group, CS("String4"));
      PushColumn(Group, CS("String5"));
      PushColumn(Group, CS("String6"));
      PushColumn(Group, CS("String7"));
      PushColumn(Group, CS("String8"));
      PushNewRow(Group);
    PushButtonEnd(Group);

    TestThat(!Hover(Group, &Interaction));
    TestThat(!Clicked(Group, &Interaction));
    TestThat(!Pressed(Group, &Interaction));
  }
  PushTableEnd(Group);

  PushTableStart(Group);
    PushButtonStart(Group, (umm)"Ignored");
      PushColumn(Group, CS("String1"));
      PushColumn(Group, CS("String2"));
      PushColumn(Group, CS("String3"));
      PushUntexturedQuadAt(Group, V2(0), V2(100), zDepth_Background);
    PushButtonEnd(Group);
    PushNewRow(Group);
  PushTableEnd(Group);

  PushTableStart(Group);
    PushButtonStart(Group, (umm)"Ignored");
      PushColumn(Group, CS("String1"));
      PushColumn(Group, CS("String2"));
      PushColumn(Group, CS("String3"));
      PushUntexturedQuadAt(Group, V2(0), V2(100), zDepth_Background);
      PushNewRow(Group);
    PushButtonEnd(Group);
  PushTableEnd(Group);
}

s32
main(s32 ArgCount, const char** Args)
{
  TestSuiteBegin("ui_command_buffer", ArgCount, Args);

  memory_arena Arena = {};
  heap_allocator Heap = InitHeap(Megabytes(128));

  renderer_2d Renderer = {};

  /* v2 MouseP = {}, MouseDP = {}; */
  /* input Input = {}; */

  InitRenderer2D(&Renderer, &Heap, &Arena);

  local_persist window_layout Window = WindowLayout("TestWindow", V2(0));

  PushWindowStart(&Renderer, &Window);
    TestTable(&Renderer);
  PushWindowEnd(&Renderer, &Window);
  FlushCommandBuffer(&Renderer, Renderer.CommandBuffer);

  TestTable(&Renderer);
  FlushCommandBuffer(&Renderer, Renderer.CommandBuffer);

  TestSuiteEnd();
}

