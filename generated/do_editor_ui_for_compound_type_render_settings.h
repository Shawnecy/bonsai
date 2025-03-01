// src/engine/editor.cpp:190:0

link_internal void
DoEditorUi(renderer_2d *Ui, window_layout *Window, render_settings *Element, cs Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
{
  if (Element)
  {
    if (ToggleButton(Ui, FSz("v %S", Name), FSz("> %S", Name), UiId(Window, "toggle render_settings", Element), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      PushNewRow(Ui);

      PushTableStart(Ui);
      PushForceUpdateBasis(Ui, V2(20.f, 0.f));
      DoEditorUi(Ui, Window, Cast(b8*, &Element->UseSsao), CSz("b32 UseSsao"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->UseShadowMapping), CSz("b32 UseShadowMapping"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->UseLightingBloom), CSz("b32 UseLightingBloom"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->BravoilMyersOIT), CSz("b32 BravoilMyersOIT"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->BravoilMcGuireOIT), CSz("b32 BravoilMcGuireOIT"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->DrawMajorGrid), CSz("b32 DrawMajorGrid"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, Cast(b8*, &Element->DrawMinorGrid), CSz("b32 DrawMinorGrid"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      PushNewRow(Ui);

      DoEditorUi(Ui, Window, &Element->MajorGridDim, CSz("r32 MajorGridDim"), EDITOR_UI_FUNCTION_INSTANCE_NAMES,0, 32);





      PushNewRow(Ui);

      

      PushNewRow(Ui);

      DoEditorUi(Ui, Window, &Element->ToneMappingType, CSz("tone_mapping_type ToneMappingType"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->Lighting, CSz("lighting_settings Lighting"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->ApplicationResolution, CSz("v2 ApplicationResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->ShadowMapResolution, CSz("v2 ShadowMapResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->LuminanceMapResolution, CSz("v2 LuminanceMapResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->iApplicationResolution, CSz("v2i iApplicationResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->iShadowMapResolution, CSz("v2i iShadowMapResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);







      
      DoEditorUi(Ui, Window, &Element->iLuminanceMapResolution, CSz("v2i iLuminanceMapResolution"), EDITOR_UI_FUNCTION_INSTANCE_NAMES);
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

