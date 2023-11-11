#define LEVEL_HEADER_MAGIC_NUMBER (0x6969696942042042)
#define LEVEL_FILE_OBJECT_DELIM   (0xFCACFCACFCACFCAC)

poof(
  func serialize_vector(type)
  {
    link_internal b32
    Serialize(native_file *File, (type.name) *Element)
    {
      b32 Result = WriteToFile(File, Cast(u8*, Element), sizeof((type.name)));

      u64 Tag = LEVEL_FILE_OBJECT_DELIM;
      Ensure( Serialize(File, &Tag) );
      return Result;
    }
  }
)

poof(
  func deserialize_vector(type)
  {
    link_internal b32
    Deserialize(u8_stream *Bytes, (type.name)* Element, memory_arena *Ignored)
    {
      *Element = *Cast((type.name)*, Bytes->At);
      Bytes->At += sizeof((type.name));
      Assert(Bytes->At <= Bytes->End);

      u64 Tag = Read_u64(Bytes);
      Ensure( Tag == LEVEL_FILE_OBJECT_DELIM );
      return True;
    }
  }
)

poof(
  func serialize_struct(type)
  {
    link_internal b32
    Serialize(native_file *File, (type.name) *Element)
    {
      u64 PointerTrue = True; 
      u64 PointerFalse = False; 

      b32 Result = True;

      type.map(member)
      {
        member.is_pointer?
        {
          if (Element->(member.name)) { Result &= WriteToFile(File, Cast(u8*, &PointerTrue), sizeof(PointerTrue)); }
          else                        { Result &= WriteToFile(File, Cast(u8*, &PointerFalse), sizeof(PointerFalse)); }
        }
        {
          member.is_enum?
          {
            Result &= Serialize(File, (u32*)&Element->(member.name));
          }
          {
            Result &= Serialize(File, &Element->(member.name));
          }
        }
      }

      type.map(member)
      {
        member.is_pointer?
        {
          if (Element->(member.name)) { Result &= Serialize(File, Element->(member.name)); }
        }
      }

      u64 Tag = LEVEL_FILE_OBJECT_DELIM;
      Ensure( Serialize(File, &Tag) );
      return Result;
    }
  }
)

poof(
  func deserialize_struct(type)
  {
    link_internal b32
    Deserialize(u8_stream *Bytes, (type.name) *Element, memory_arena *Memory)
    {
      b32 Result = True;
      type.map(member)
      {
        member.is_pointer?
        {
          b64 Had(member.name)Pointer = Read_u64(Bytes);
          Assert(Had(member.name)Pointer < 2); // Should be 0 or 1
        }
        {
          member.is_enum?
          {
            Element->(member.name) = Cast((member.type), Read_u32(Bytes));
          }
          {
            member.is_primitive?
            {
              Result &= Deserialize(Bytes, &Element->(member.name));
            }
            {
              Result &= Deserialize(Bytes, &Element->(member.name), Memory);
            }
          }
        }
      }

      type.map(member)
      {
        member.is_pointer?
        {
          if (Had(member.name)Pointer)
          {
            if (Element->member.name == 0) { Element->member.name = Allocate((member.type), Memory, 1); }
            Result &= Deserialize(Bytes, Element->(member.name), Memory);
          }
        }
      }

      u64 Tag = Read_u64(Bytes);
      Ensure( Tag == LEVEL_FILE_OBJECT_DELIM );
      return Result;
    }
  }
)


poof(
  func serialize_array(type)
  {
    link_internal b32
    SerializeArray(native_file *File, (type.name) *Element, umm Count)
    {
      Assert(Count);
      {
        RangeIterator_t(umm, ElementIndex, Count)
        {
          Serialize(File, Element+ElementIndex);
        }
      }
      return True;
    }
  }
)

poof(
  func deserialize_array(type)
  {
    link_internal b32
    DeserializeArray(u8_stream *Bytes, (type.name) **Dest, umm Count, memory_arena *Memory)
    {
      Assert(Count);
      if (*Dest == 0) { *Dest = Allocate((type.name), Memory, Count); }
      RangeIterator_t(umm, ElementIndex, Count)
      {
        Deserialize(Bytes, (*Dest)+ElementIndex, Memory);
      }
      return True;
    }
  }
)

poof(
  func serdes_array(type)
  {
    serialize_array(type)
    deserialize_array(type)
  }
)

poof(
  func serdes_struct(type)
  {
    serialize_struct(type)
    deserialize_struct(type)
  }
)

poof(
  func serdes_vector(type)
  {
    serialize_vector(type)
    deserialize_vector(type)
  }
)


poof(
  func serdes_primitive(type_poof_symbol list)
  {
    list.map(type)
    {
      link_internal b32
      Serialize(native_file *File, (type.name) *Element)
      {
        b32 Result = WriteToFile(File, Cast(u8*, Element), sizeof((type.name)));
        return Result;
      }

      link_internal b32
      Deserialize(u8_stream *Bytes, (type.name)* Element)
      {
        *Element = *Cast((type.name)*, Bytes->At);
        Bytes->At += sizeof((type.name));
        Assert(Bytes->At <= Bytes->End);
        return True;
      }
    }
  }
)


poof(serdes_primitive({u8 s8 u16 s16 u32 s32 u64 s64 b8 r32 r64}))
#include <generated/serdes_primitive_592771169.h>
