/* link_internal void */
/* DeepCopy( Type.name *Src, Type.name *Dest) */
/* { */
/*   Assert(TotalElements(Src) <= TotalElements(Dest)); */
/*   IterateOver(Src, Element, ElementIndex) */
/*   { */
/*     DeepCopy(Element, Dest->Start+ElementIndex); */
/*   } */
/* } */
link_internal void
DeepCopy(entropy_lists *Src, entropy_lists *Dest)
{
  umm SrcAt = AtElements(Src);
  Assert(SrcAt <= TotalElements(Dest));

  IterateOver(Src, Element, ElementIndex)
  {
    DeepCopy(Element, Dest->Start+ElementIndex);
  }

  Dest->At = Dest->Start+SrcAt;
  Assert(Dest->At <= Dest->End);
}

