link_internal b32
Serialize(native_file *File, entity_1 *Element)
{
  u64 PointerTrue = True; 
  u64 PointerFalse = False; 

  b32 Result = True;

  Result &= Serialize(File, &Element->Version);




  Result &= Serialize(File, &Element->Id);




  Result &= Serialize(File, &Element->P);




  Result &= Serialize(File, &Element->EulerAngles);




  Result &= Serialize(File, &Element->Scale);




  Result &= Serialize(File, &Element->_CollisionVolumeRadius);




  Result &= Serialize(File, &Element->Physics);




  Result &= Serialize(File, &Element->AssetId);




  if (Element->Emitter) { Result &= WriteToFile(File, Cast(u8*, &PointerTrue), sizeof(PointerTrue)); }
  else                        { Result &= WriteToFile(File, Cast(u8*, &PointerFalse), sizeof(PointerFalse)); }


  Result &= Serialize(File, (u32*)&Element->State);



  Result &= Serialize(File, (u32*)&Element->Behavior);



  Result &= Serialize(File, &Element->UserType);




  Result &= Serialize(File, &Element->UserData);

  if (Element->Emitter) { Result &= Serialize(File, Element->Emitter); }




  MAYBE_WRITE_DEBUG_OBJECT_DELIM();
  return Result;
}

link_internal b32
Deserialize(u8_stream *Bytes, entity_1 *Element, memory_arena *Memory)
{
  b32 Result = True;
  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->Version, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->Id, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->P, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->EulerAngles, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->Scale, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->_CollisionVolumeRadius, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->Physics, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->AssetId, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  b64 HadEmitterPointer = Read_u64(Bytes);
  Assert(HadEmitterPointer < 2); // Should be 0 or 1


  Element->State = Cast(entity_state, Read_u32(Bytes));



  Element->Behavior = Cast(entity_behavior_flags, Read_u32(Bytes));



  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->UserType, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */




  // NOTE(Jesse): Unfortunately we can't check for primitives because
  // strings are considered primitive, but need memory to deserialize
  Result &= Deserialize(Bytes, &Element->UserData, Memory);

  /* member.is_primitive? */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name)); */
  /* } */
  /* { */
  /*   Result &= Deserialize(Bytes, &Element->(member.name), Memory); */
  /* } */

  if (HadEmitterPointer)
  {
    if (Element->Emitter == 0) { Element->Emitter = Allocate(particle_system, Memory, 1); }
    Result &= Deserialize(Bytes, Element->Emitter, Memory);
  }




  MAYBE_READ_DEBUG_OBJECT_DELIM();
  return Result;
}


