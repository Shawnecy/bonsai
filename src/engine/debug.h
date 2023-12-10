#define MAX_PICKED_WORLD_CHUNKS (64)

#define EDITOR_UI_FUNCTION_PROTO_DEFAULTS  ui_style *Style = &DefaultStyle, v4 Padding = {{5, 2, 5, 2}}, column_render_params ColumnParams = ColumnRenderParam_LeftAlign
#define EDITOR_UI_FUNCTION_PROTO_ARGUMENTS ui_style *Style,                 v4 Padding,                  column_render_params ColumnParams
#define EDITOR_UI_FUNCTION_INSTANCE_NAMES            Style,                    Padding,                                       ColumnParams


struct world;
struct world_chunk;
struct heap_allocator;
struct entity;


struct picked_world_chunk
{
  world_chunk *Chunk;
  r32 tChunk;
};

enum picked_voxel_position
{
  PickedVoxel_LastEmpty,
  PickedVoxel_FirstFilled,
  PickedVoxel_Count,
};

struct picked_voxel
{
  picked_world_chunk Chunks[PickedVoxel_Count];
  canonical_position Picks[PickedVoxel_Count]; // Technically we can just store the v3 offset, but I'm being lazy
};

struct maybe_picked_voxel
{
  maybe_tag Tag;
  picked_voxel Value;
};

// TODO(Jesse)(metaprogramming, ptr): Once poof can accept pointer types we can generate this struct
/* poof(static_buffer(world_chunk*, 64)) */
/* #include <generated/buffer_world_chunk.h> */
struct picked_world_chunk_static_buffer
{
  picked_world_chunk E[MAX_PICKED_WORLD_CHUNKS];
  u64 At;
};

link_internal void
Push(picked_world_chunk_static_buffer *Buf, world_chunk *Chunk, r32 t)
{
  if (Buf->At < MAX_PICKED_WORLD_CHUNKS)
  {
    Buf->E[Buf->At].Chunk = Chunk;
    Buf->E[Buf->At].tChunk = t;

    ++Buf->At;
  }
}

enum pick_chunk_state
{
  PickedChunkState_None,
  PickedChunkState_Hover,
};

struct render_debug
{
  u32 BytesSolidGeoLastFrame;
  u32 BytesTransGeoLastFrame;
};

struct engine_debug
{
  picked_world_chunk_static_buffer PickedChunks;
  texture_cursor Textures;

  ui_debug UiDebug;

  render_debug Render;

  b8 DrawEntityCollisionVolumes;
  b8 DrawWorldAxies;

  b8 TriggerRuntimeBreak;
  b8 ResetAssetNodeView;

  u8 PickedChunkState;
  world_chunk *PickedChunk;

  file_traversal_node SelectedAsset;

  entity *SelectedEntity;
};




poof(
  func generic_button_group_for_enum(enum_t, type_poof_symbol NamePrefix, type_poof_symbol extra_poof_flags)
  {
    link_internal b32
    ToggledOn(ui_toggle_button_group *ButtonGroup, enum_t.name Enum)
    {
      b32 Result = ButtonGroup->ToggleBits & (1 << Enum);
      return Result;
    }

    // NOTE(Jesse): This could be implemented by reconstructing the button ID
    // but I'm very unsure that's worth it.  Seems like just
    link_internal b32
    Clicked(ui_toggle_button_group *ButtonGroup, enum_t.name Enum)
    {
      b32 Result = False;
      NotImplemented;
      return Result;
    }

    link_internal ui_toggle_button_group
    (NamePrefix)ButtonGroup_(enum_t.name)(renderer_2d *Ui, umm IdModifier, ui_toggle_button_group_flags ExtraFlags = ToggleButtonGroupFlags_None, UI_FUNCTION_PROTO_DEFAULTS)
    {
      cs ButtonNames[] =
      {
        enum_t.map(value)
        {
          CSz("value.name.strip_all_prefix"),
        }
      };

      u32 ButtonCount = ArrayCount(ButtonNames);

      ui_toggle_button_handle_buffer ButtonBuffer = UiToggleButtonHandleBuffer(ButtonCount, GetTranArena());
      IterateOver(&ButtonBuffer, Button, ButtonIndex)
      {
        *Button = UiToggle(ButtonNames[ButtonIndex], IdModifier+ButtonIndex);
      }

      ui_toggle_button_group Result = UiToggleButtonGroup(Ui, &ButtonBuffer, ui_toggle_button_group_flags(ExtraFlags(extra_poof_flags)), UI_FUNCTION_INSTANCE_NAMES);

      return Result;
    }
  }
)

poof(
  func toggle_button_group_for_enum(enum_t)
  {
    generic_button_group_for_enum(enum_t, {Toggle}, {|ToggleButtonGroupFlags_None})
  }
)

poof(
  func radio_button_group_for_enum(enum_t)
  {
    generic_button_group_for_enum(enum_t, {Radio}, {|ToggleButtonGroupFlags_RadioButtons})
  }
)

poof(
  func radio_button_group_for_bitfield_enum(enum_t)
  {
    link_internal void
    RadioSelect(ui_toggle_button_group *RadioGroup, enum_t.name Selection)
    {
      Assert(CountBitsSet_Kernighan(u32(Selection)) == 1);
      u32 Index = GetIndexOfNthSetBit(u32(Selection), 1);
      ui_toggle_button_handle *ToggleHandle = RadioGroup->Buttons.Start + Index;
      SetRadioButton(RadioGroup, ToggleHandle, True);
      /* Ensure( ToggleRadioButton(RadioGroup, ToggleHandle) ); */
    }

    link_internal void
    GetRadioEnum(ui_toggle_button_group *RadioGroup, enum_t.name *Result)
    {
      if (RadioGroup->ToggleBits)
      {
        Assert(CountBitsSet_Kernighan(RadioGroup->ToggleBits) == 1);
        // NOTE(Jesse): This is better; it asserts that we've actually got a bitfield
        Assert((((enum_t.map(value).sep(||) {RadioGroup->ToggleBits == value.name}))));
        /* Assert((((enum_t.map(value).sep(|) {value.name})) & RadioGroup->ToggleBits) != 0); */
      }

      *Result = Cast((enum_t.name), RadioGroup->ToggleBits);
    }

    radio_button_group_for_enum(enum_t)
  }
)

enum asset_spawn_mode
{
  AssetSpawnMode_BlitIntoWorld = (1 << 0),
  AssetSpawnMode_Entity        = (1 << 1),
};

poof(radio_button_group_for_bitfield_enum(asset_spawn_mode))
#include <generated/radio_button_group_for_bitfield_enum_asset_spawn_mode.h>


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
};

poof(toggle_button_group_for_enum(engine_debug_view_mode))
#include <generated/toggle_button_group_for_enum_engine_debug_view_mode.h>

poof(
  func do_editor_ui_for_vector_type(type_poof_symbol type_list)
  {
    type_list.map(type)
    {
      link_internal void
      DoEditorUi(renderer_2d *Ui, type.name *Value, const char* Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
      {
        type.member(0, (E) 
        {
          if (Name) { PushColumn(Ui, CS(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES); }

          if (Value)
          {
            u32 Start = StartColumn(Ui);
              PushTableStart(Ui);
                E.map_array(e_index)
                {
                  DoEditorUi(Ui, &Value->(E.name)[e_index], 0);
                  PushNewRow(Ui);
                }
              PushTableEnd(Ui);
            EndColumn(Ui, Start);
          }
        })
      }
    }
  }
)

poof(
  func do_editor_ui_for_scalar_type(type_poof_symbol type_list)
  {
    type_list.map(type)
    {
      link_internal void
      DoEditorUi(renderer_2d *Ui, volatile type.name *Value, const char* Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
      {
        if (Name) { PushColumn(Ui, CS(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES); }

        if (Value)
        {
          u32 Start = StartColumn(Ui);
            PushTableStart(Ui);
              if (Button(Ui, CSz("-"), (umm)Value + (umm)"decrement" )) { *Value = *Value - 1; }
              PushColumn(Ui, CS(*Value));
              if (Button(Ui, CSz("+"), (umm)Value + (umm)"increment" )) { *Value = *Value + 1; }
            PushTableEnd(Ui);
          EndColumn(Ui, Start);
        }
        else
        {
          PushColumn(Ui, CSz("(null)"));
        }
      }
      link_internal void
      DoEditorUi(renderer_2d *Ui, type.name *Value, const char* Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
      {
        if (Name) { PushColumn(Ui, CS(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES); }

        if (Value)
        {
          u32 Start = StartColumn(Ui);
            PushTableStart(Ui);
              if (Button(Ui, CSz("-"), (umm)Value + (umm)"decrement" )) { *Value = *Value - 1; }
              PushColumn(Ui, CS(*Value));
              if (Button(Ui, CSz("+"), (umm)Value + (umm)"increment" )) { *Value = *Value + 1; }
            PushTableEnd(Ui);
          EndColumn(Ui, Start);
        }
        else
        {
          PushColumn(Ui, CSz("(null)"));
        }
      }
    }
  }
)

poof(
  func do_editor_ui_for_compound_type(type)
  {
    link_internal void
    DoEditorUi(renderer_2d *Ui, type.name *Element, const char* Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
    {
      /* PushTableStart(Ui); */
      if (ToggleButton(Ui, FSz("v %s", Name), FSz("> %s", Name), umm(Element) ^ umm(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
      {
        PushForceUpdateBasis(Ui, V2(20.f, 0.f));
        /* Padding.x += 20.f; */
        PushNewRow(Ui);
        type.map(member)
        {
          member.is_array?
          {
            RangeIterator(ArrayIndex, member.array)
            {
              DoEditorUi(Ui, Element->(member.name)+ArrayIndex, "member.type member.name", EDITOR_UI_FUNCTION_INSTANCE_NAMES);
            }
          }
          {
            member.is_pointer?
            {
              DoEditorUi(Ui, Element->(member.name), "member.type member.name", EDITOR_UI_FUNCTION_INSTANCE_NAMES);
            }
            {
              member.is_union?
              {
                // Select first member in union ..?
              }
              {
                DoEditorUi(Ui, &Element->(member.name), "member.type member.name", EDITOR_UI_FUNCTION_INSTANCE_NAMES);
              }
            }
          }

          PushNewRow(Ui);
        }
        PushForceUpdateBasis(Ui, V2(-20.f, 0.f));
      }
      else
      {
        PushNewRow(Ui);
      }
    }
  }
)

poof(
  func do_editor_ui_for_enum(enum_t)
  {
    link_internal void
    DoEditorUi(renderer_2d *Ui, enum_t.name *Element, const char* Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
    {
      if (Name) { PushColumn(Ui, CS(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES); }

      cs ElementName = ToString(*Element);
      if (ToggleButton(Ui, ElementName, ElementName, umm(Element)^umm(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
      {
        PushNewRow(Ui);
        enum_t.map(value)
        {
          PushColumn(Ui, CSz("")); // Skip the first Name column
          if (Button(Ui, CSz("value.name"), umm(Element)^umm("value.name"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
          {
            *Element = value.name;
          }
          PushNewRow(Ui);
        }
      }
      else
      {
        PushNewRow(Ui);
      }
    }
  }
)

link_internal void
DoEditorUi(renderer_2d *Ui, aabb *Element, const char* Name, EDITOR_UI_FUNCTION_PROTO_ARGUMENTS);

link_internal void
DoEditorUi(renderer_2d *Ui, v3i *Element, const char* Name, EDITOR_UI_FUNCTION_PROTO_ARGUMENTS);

link_internal engine_debug * GetEngineDebug();
