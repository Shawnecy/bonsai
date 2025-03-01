// src/engine/editor.cpp:259:0

link_internal void
DoEditorUi(renderer_2d *Ui, window_layout *Window, lod_element_buffer *Element, cs Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
{
  if (Element)
  {
    if (ToggleButton(Ui, FSz("v %S", Name), FSz("> %S", Name), UiId(Window, "toggle lod_element_buffer", Element), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      PushNewRow(Ui);

      PushTableStart(Ui);
      PushForceUpdateBasis(Ui, V2(20.f, 0.f));
      DoEditorUi(Ui, Window, &Element->MeshMask, CSz("u32 MeshMask"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      if (ToggleButton(Ui, CSz("v gpu_element_buffer_handles GpuBufferHandles[MeshIndex_Count]"), CSz("> gpu_element_buffer_handles GpuBufferHandles[MeshIndex_Count]"), UiId(Window, "toggle lod_element_buffer gpu_element_buffer_handles GpuBufferHandles", Element->GpuBufferHandles), EDITOR_UI_FUNCTION_INSTANCE_NAMES ))
      {
        PushForceUpdateBasis(Ui, V2(20.f, 0.f));
        PushNewRow(Ui);
        RangeIterator(ArrayIndex, MeshIndex_Count)
        {
          DoEditorUi(Ui, Window, Element->GpuBufferHandles+ArrayIndex, FSz("gpu_element_buffer_handles GpuBufferHandles[%d]", ArrayIndex), EDITOR_UI_FUNCTION_INSTANCE_NAMES);
          
        }
        PushForceUpdateBasis(Ui, V2(-20.f, 0.f));
      }
      PushNewRow(Ui);



      
      if (ToggleButton(Ui, CSz("v geo_u3d_ptr E[MeshIndex_Count]"), CSz("> geo_u3d_ptr E[MeshIndex_Count]"), UiId(Window, "toggle lod_element_buffer geo_u3d_ptr E", Element->E), EDITOR_UI_FUNCTION_INSTANCE_NAMES ))
      {
        PushForceUpdateBasis(Ui, V2(20.f, 0.f));
        PushNewRow(Ui);
        RangeIterator(ArrayIndex, MeshIndex_Count)
        {
          DoEditorUi(Ui, Window, Element->E+ArrayIndex, FSz("geo_u3d_ptr E[%d]", ArrayIndex), EDITOR_UI_FUNCTION_INSTANCE_NAMES);
          
        }
        PushForceUpdateBasis(Ui, V2(-20.f, 0.f));
      }
      PushNewRow(Ui);



      
      if (ToggleButton(Ui, CSz("v bonsai_futex Locks[MeshIndex_Count]"), CSz("> bonsai_futex Locks[MeshIndex_Count]"), UiId(Window, "toggle lod_element_buffer bonsai_futex Locks", Element->Locks), EDITOR_UI_FUNCTION_INSTANCE_NAMES ))
      {
        PushForceUpdateBasis(Ui, V2(20.f, 0.f));
        PushNewRow(Ui);
        RangeIterator(ArrayIndex, MeshIndex_Count)
        {
          DoEditorUi(Ui, Window, Element->Locks+ArrayIndex, FSz("bonsai_futex Locks[%d]", ArrayIndex), EDITOR_UI_FUNCTION_INSTANCE_NAMES);
          
        }
        PushForceUpdateBasis(Ui, V2(-20.f, 0.f));
      }
      PushNewRow(Ui);
      PushForceUpdateBasis(Ui, V2(-20.f, 0.f));
      PushTableEnd(Ui);
    }

    PushNewRow(Ui);
  }
  else
  {
    PushColumn(Ui, Name, EDITOR_UI_FUNCTION_INSTANCE_NAMES);
    PushColumn(Ui, CSz("(null)"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);
    PushNewRow(Ui);
  }

}

