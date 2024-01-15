link_internal void
DoEditorUi(renderer_2d *Ui, entity_behavior_flags *Element, cs Name, EDITOR_UI_FUNCTION_PROTO_DEFAULTS)
{
  if (Name) { PushColumn(Ui, CS(Name), EDITOR_UI_FUNCTION_INSTANCE_NAMES); }

  cs ElementName = ToString(*Element);
  if (ToggleButton(Ui, ElementName, ElementName, UiId(Element, "enum value.type value.name"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
  {
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_None"), UiId(Element, "enum EntityBehaviorFlags_None"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_None) == EntityBehaviorFlags_None)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_None);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_None);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_Gravity"), UiId(Element, "enum EntityBehaviorFlags_Gravity"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_Gravity) == EntityBehaviorFlags_Gravity)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_Gravity);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_Gravity);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_WorldCollision"), UiId(Element, "enum EntityBehaviorFlags_WorldCollision"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_WorldCollision) == EntityBehaviorFlags_WorldCollision)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_WorldCollision);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_WorldCollision);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_EntityCollision"), UiId(Element, "enum EntityBehaviorFlags_EntityCollision"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_EntityCollision) == EntityBehaviorFlags_EntityCollision)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_EntityCollision);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_EntityCollision);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_UnspawnOnParticleSystemTerminate"), UiId(Element, "enum EntityBehaviorFlags_UnspawnOnParticleSystemTerminate"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_UnspawnOnParticleSystemTerminate) == EntityBehaviorFlags_UnspawnOnParticleSystemTerminate)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_UnspawnOnParticleSystemTerminate);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_UnspawnOnParticleSystemTerminate);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_CameraGhost"), UiId(Element, "enum EntityBehaviorFlags_CameraGhost"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_CameraGhost) == EntityBehaviorFlags_CameraGhost)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_CameraGhost);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_CameraGhost);
      }

    }
    PushNewRow(Ui);
    PushColumn(Ui, CSz("")); // Skip the first Name column
    if (Button(Ui, CSz("EntityBehaviorFlags_Default"), UiId(Element, "enum EntityBehaviorFlags_Default"), EDITOR_UI_FUNCTION_INSTANCE_NAMES))
    {
      if ((*Element & EntityBehaviorFlags_Default) == EntityBehaviorFlags_Default)
      {
        *Element = entity_behavior_flags(*Element&~EntityBehaviorFlags_Default);
      }
      else
      {
        *Element = entity_behavior_flags(*Element|EntityBehaviorFlags_Default);
      }

    }
    PushNewRow(Ui);
  }
  else
  {
    PushNewRow(Ui);
  }
}

