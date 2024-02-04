link_internal void
GrowGrass( world_chunk *Chunk, v3i P, r32 NoiseValue, r32 MaskValue, v3i SrcToDest, v3i WorldChunkDim, r32 WorldZSubZMin, u16 *ThisColor, b32 *IsFilled )
{
  s32 x = P.x;
  s32 y = P.y;
  s32 z = P.z;


  // NOTE(Jesse): This is pretty henious .. the math to compute GrassAreaX here
  // (and, actually, maybe most of these calculations to turn positions into floats)
  // is generating a fucked up index for the voxels along the exterior edge.
  //
  // The thing that's wrong is that the 0th still gets the Chunk->WorldP position,
  // when it should get the previous chunks worldp.  Similar story with the max-edge
  //
  // I think this was actually also manifesting when doing mip-meshing, but I
  // wasn't aware of this tom-fuckery then, and didn't catch it.
  //
  // TODO(Jesse): Should go audit the terrain-gen functions and find a better
  // way of computing these float values.
  //
  if (x == 0 || y == 0 || x == Chunk->Dim.x-1 || y == Chunk->Dim.y-1) { return; }

  f32 GrassAreaX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / 16.f;
  f32 GrassAreaY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / 16.f;

  // TODO(Jesse): Does this actually help reduce stuff 'growing' in places
  // it shouldn't?
  f32 GrassAreaZ = 1.f; //(z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / 32.f;

  r32 GrassyAreaValue = PerlinNoise(GrassAreaX, GrassAreaY, GrassAreaZ);
  if (*ThisColor == GRASS_GREEN)
  {
    if (GrassyAreaValue > 0.4f)
    {
      *ThisColor = GRASS_GREEN-1;

      if (GrassyAreaValue > 0.6f)
      {
        if ((NoiseValue+(MaskValue*GrassyAreaValue*3.f)) > WorldZSubZMin)
        {
          f32 HashX = hash_f32(f32(x));
          f32 HashY = hash_f32(f32(y));

          s32 iHashX = *(s32*)&HashX;
          s32 iHashY = *(s32*)&HashY;

          random_series S0 { u64(x + y) | u64( (iHashX | (iHashY << 31)) ^ (iHashY << 16)) };
          random_series S1 { RandomU32(&S0) | RandomU32(&S0)<<31 };

          if (RandomUnilateral(&S1) > 0.92f)
          {
            *IsFilled = True;

            r32 GrassColor = RandomUnilateral(&S1);
            if (GrassColor > 0.8f)
              *ThisColor = LONG_GREEN_GRASS0;

            if (GrassColor > 0.88f)
              *ThisColor = LONG_GREEN_GRASS1;

            if (GrassColor > 0.99f)
              *ThisColor = LONG_YELLOW_GRASS0;

            if (GrassColor > 0.993f)
              *ThisColor = LONG_YELLOW_GRASS1;

            if (GrassColor > 0.997f)
              *ThisColor = PINK;
          }
        }
      }
    }
  }
}


link_internal void
ComputeNormalsForChunkFromFilledFlag(world_chunk *Chunk, v3i WorldChunkDim, v3 *Normals)
{
  TIMED_FUNCTION();

  v3i ChunkDim = Chunk->Dim;

  for ( s32 z = 0; z < ChunkDim.z; ++ z)
  {
    for ( s32 y = 0; y < ChunkDim.y; ++ y)
    {
      for ( s32 x = 0; x < ChunkDim.x; ++ x)
      {
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), ChunkDim);

        if ( Chunk->Voxels[VoxIndex].Flags & Voxel_Filled )
        {
          v3 Normal = {};
          for ( s32 dz = -1; dz < 2; ++ dz)
          {
            for ( s32 dy = -1; dy < 2; ++ dy)
            {
              for ( s32 dx = -1; dx < 2; ++ dx)
              {
                if (dz == 0 && dy == 0 && dx == 0) continue; // Skip the middle-most voxel

                s32 dP = TryGetIndex(Voxel_Position(x+dx,y+dy,z+dz), ChunkDim);
                if (dP > -1)
                {
                  if ( Chunk->Voxels[dP].Flags & Voxel_Filled )
                  {
                    Normal += V3(dx,dy,dz);
                  }
                }
              }
            }
          }

          Normals[VoxIndex] = Normalize(Normal) * -1.f; // Invert because we tally'd up all filled voxels
        }
      }
    }
  }

#if 0
  // TODO(Jesse): Parameterize this iteratin in terms of the min offset in case
  // we change it
  for ( s32 z = 1; z < WorldChunkDim.z+1; ++ z)
  {
    for ( s32 y = 1; y < WorldChunkDim.y+1; ++ y)
    {
      for ( s32 x = 1; x < WorldChunkDim.x+1; ++ x)
      {
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), ChunkDim);

        if ( Chunk->Voxels[VoxIndex].Flags & Voxel_Filled )
        {
          v3 Normal = Normals[VoxIndex];
          for ( s32 dz = -1; dz < 2; ++ dz)
          {
            for ( s32 dy = -1; dy < 2; ++ dy)
            {
              for ( s32 dx = -1; dx < 2; ++ dx)
              {
                if (dz == 0 && dy == 0 && dx == 0) continue; // Skip the middle-most voxel

                s32 dP = GetIndex(V3i(x+dx,y+dy,z+dz), ChunkDim);
                Normal += 0.05f*Normals[dP];
              }
            }
          }
          Normals[VoxIndex] = Normalize(Normal);
        }

      }
    }
  }
#endif

}

#if 1
link_internal void
ComputeNormalsForChunkFromNoiseValues(v3i Dim, r32 ChunkWorldZ, r32 *NoiseValues, v3 *Normals)
{
  TIMED_FUNCTION();

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s32 VoxIndex = GetIndex(V3i(x,y,z), Dim);

        r32 CurrentToThresh = (z+ChunkWorldZ)-NoiseValues[VoxIndex];
        r32 CurrentNoiseValue = NoiseValues[VoxIndex];
        /* r32 CurrentNoiseValue = z+dz+ChunkWorldZ; */
        /* r32 CurrentNoiseValue = z+dz+ChunkWorldZ; */
        /* r32 CurrentNoiseValue = NoiseValues[VoxIndex] + z; */
        /* r32 CurrentNoiseValue = NoiseValues[VoxIndex] + z + ChunkWorldZ; */

        /* if ( NoiseValues[VoxIndex] < (z+ChunkWorldZ) ) */
        {
          v3 Normal = {};
          for ( s32 dz = -1; dz < 2; ++ dz)
          {
            for ( s32 dy = -1; dy < 2; ++ dy)
            {
              for ( s32 dx = -1; dx < 2; ++ dx)
              {
                if (dz == 0 && dy == 0 && dx == 0) continue; // Skip the middle-most voxel

                s32 dPIndex = TryGetIndex(V3i(x+dx,y+dy,z+dz), Dim);
                if (dPIndex > -1)
                {
                  r32 NoiseDelta = 0.f;
                  /* r32 NoiseDelta = CurrentNoiseValue-NoiseValues[dPIndex]; */
                  /* r32 NoiseDelta = NoiseValues[dPIndex]-CurrentNoiseValue; */

                  r32 AbsThresh = (z+dz+ChunkWorldZ);
                  r32 CurrentToAbsThres = AbsThresh - NoiseValues[dPIndex];

                  // works
                  /* if ( NoiseValues[dPIndex] > (z+dz+ChunkWorldZ) ) */
                  /* if ( NoiseValues[dPIndex]-CurrentToAbsThres > AbsThresh ) */

                  /* r32 CurrentToThresh = (z+dz+ChunkWorldZ)-NoiseValues[dPIndex]; */
                  /* if ( NoiseValues[dPIndex]+CurrentToThresh > AbsThresh ) */

                  /* if ( NoiseValues[dPIndex]+CurrentToThresh+NoiseDelta > (z+dz+ChunkWorldZ) ) */
                  /* if ( NoiseValues[dPIndex]+CurrentToThresh    > (z+ChunkWorldZ) ) */
                  /* if ( NoiseValues[dPIndex]+CurrentToThresh+dz > (z+dz+ChunkWorldZ) ) */
                  /* if ( NoiseValues[dPIndex]+CurrentToThresh    > (z+ChunkWorldZ) ) */

                  /* if (  CurrentNoiseValue < NoiseValues[dPIndex]+z+dz  ) */
                  /* if (  NoiseValues[dPIndex]+z+dz < CurrentNoiseValue ) */
                  /* if ( NoiseValues[dPIndex] > CurrentNoiseValue ) */
                  if ( NoiseValues[dPIndex] > Truncate(CurrentNoiseValue)+dz )
                  {
                    Normal += V3(dx,dy,dz);
                  }
                }
              }
            }
          }
          Normals[VoxIndex] = Normalize(Normal) * -1.f;
        }
      }
    }
  }

#if 0
  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);

        if ( NoiseValues[VoxIndex] > WorldZSubZMin)
        {
          v3 Normal = Normals[VoxIndex];
          for ( s32 dz = -1; dz < 2; ++ dz)
          {
            for ( s32 dy = -1; dy < 2; ++ dy)
            {
              for ( s32 dx = -1; dx < 2; ++ dx)
              {
                if (dz == 0 && dy == 0 && dx == 0) continue; // Skip the middle-most voxel

                s32 dPIndex = TryGetIndex(Voxel_Position(x+dx,y+dy,z+dz), Dim);
                if (dPIndex > -1)
                {
                  Normal += 0.05f*Normals[dPIndex];
                }
              }
            }
          }
          Normals[VoxIndex] = Normalize(Normal);
        }

      }
    }
  }
#endif
}
#endif


link_internal u32
HoodooTerrain( perlin_noise *Noise,
               world_chunk *Chunk,
               chunk_dimension Dim,
               chunk_dimension SrcToDest,
               u16 ColorIndex,
               s32 Frequency,
               s32 Amplitude,
               s64 zMin,
               chunk_dimension WorldChunkDim,
               void *BaseNoise )
{
  TIMED_FUNCTION();

  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)BaseNoise;
  u32 OctaveCount = OctaveBuf->Count;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        r32 NoiseValue = 0.f;
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );

        /* s32 Amp2 = Amplitude*2; */
        r32 HighestNoise = 0.f;
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          v3 OctaveFrequency = Octave->Freq;
          r32 OctaveAmplitude = Octave->Amp;

          /* r32 WarpFactor = 3.f; */

          f32 InX = SafeDivide0((x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)), OctaveFrequency.x);
          f32 InY = SafeDivide0((y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)), OctaveFrequency.y);
          f32 InZ = SafeDivide0((z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)), OctaveFrequency.z);
          /* f32 InZ = 1.0; */


          r32 Warp = PerlinNoise(InX, InY, InZ);
          v3 WarpFactor = Octave->WarpStrength*Warp;
          /* Warp = PerlinNoise(InX, InY, InZ) * r32(1.f/(OctaveIndex+1)); */
          /* Warp = PerlinNoise(InX, InY, InZ); */

          /* r32 N = PerlinNoise(InX, InY, InZ); */
          /* r32 N = PerlinNoise(InX, InY+Warp, InZ); */
          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);
          /* r32 N = PerlinNoise(Warp, Warp, Warp); */

          /* r32 N2 = PerlinNoise(InX, InY, InZ); */

          Assert(N <= 1.05f);
          Assert(N > -1.05f);

          /* Assert(N2 <= 1.05f); */
          /* Assert(N2 > -1.05f); */

          /* b32 IsFilled = IsUnderground && NoiseValue > 0.20f; */
          /* s32 zValue = (s32)Abs( (N*OctaveAmplitude) ); */
          /* s32 zValue = (s32)Abs( (N*OctaveAmplitude) + (N2*Amp2) ); */
          /* s32 zValue =  s32(N*OctaveAmplitude) + s32(N2*Amplitude*Amplitude); */
          /* s32 zValue = s32(N*OctaveAmplitude); */

          r32 NoiseContrib = (N*OctaveAmplitude) / 1+OctaveIndex;
          NoiseValue += NoiseContrib;

          HighestNoise = Max(HighestNoise, N);

          /* if (OctaveIndex == 1) */

          /* b32 IsUnderground =  zValue < WorldZSubZMin; */
          /* b32 IsFilled = IsUnderground && N > 0.5f; */
          /* b32 IsFilled = IsUnderground; */

          /* if (IsFilled) */
          {
            /* NoiseValue += (N*OctaveAmplitude) / OctaveCount; */
            /* NoiseValue += (N*OctaveAmplitude) / 1+OctaveIndex; */
          }

          /* OctaveAmplitude = Max(1, OctaveAmplitude/2); */
          /* OctaveFrequency = Max(1, OctaveFrequency/2); */
        }

        {
           octave HoodooOctaves[2] = {
             {V3(15, 15, 300), 30, V3(1.f)},
             {V3(50, 50, 3),   9, V3(1.f)},
           };


           RangeIterator(HoodooOctaveIndex, (s32)ArrayCount(HoodooOctaves))
           {
             octave HoodooOctave = HoodooOctaves[HoodooOctaveIndex];
             f32 OctaveX = SafeDivide0((x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)), HoodooOctave.Freq.x);
             f32 OctaveY = SafeDivide0((y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)), HoodooOctave.Freq.y);
             f32 OctaveZ = SafeDivide0((z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)), HoodooOctave.Freq.z);

             r32 N2 = PerlinNoise(OctaveX, OctaveY, OctaveZ);

             /* NoiseValue += N2*HoodooOctave.Amp * ((Sin(InX)+Cos(InY))/2.f); */
             /* NoiseValue += N2*HoodooOctave.Amp * Pow((Sin(InX) + Cos(InY))/20, 3); */

             // Pretty good hoodoos
             /* NoiseValue += Pow(ClampMin(N2 * Sin(InX)+Cos(InY)-0.95f, 0.f), 3) * HoodooOctave.Amp; */

             // Better Hoodoos
             /* NoiseValue += Pow(ClampMin(N2 * Sin(InX*17)+Cos(InY*17)-0.9f, 0.f), 4) * HoodooOctave.Amp; */
             /* NoiseValue += Pow(ClampMin(N2 * ((Sin(InX*17)+Cos(InY*3))-0.9f), 0.f), 4) * HoodooOctave.Amp; */

             // Hoodoos along ridges
             /* NoiseValue += Pow(ClampMin(N2 * N, 0.f), 4) * HoodooOctave.Amp; */

             NoiseValue += Pow(ClampMin(N2*HighestNoise, 0.f), 4) * HoodooOctave.Amp;
           }
        }



        /* NoiseValue /= r32(Octaves+Octaves); */
        /* Assert(NoiseValue <= 1.10f); */

        /* b32 IsFilled = NoiseValue > 0.5f;; */
        /* b32 IsFilled = NoiseValue > r32(Amplitude); //0.5f;; */
        b32 IsFilled = r64(NoiseValue) > r64(WorldZSubZMin);

        u16 ThisColor = ColorIndex;

        s32 SnowThreshold = 100;
        if (IsFilled == True && WorldZ > SnowThreshold)
        {
          ThisColor = WHITE;
        }

        s32 SandThreshold = 3;
        if (IsFilled == True && WorldZ < SandThreshold)
        {
          ThisColor = YELLOW;
        }

        /* s32 WaterThreshold = 0; */
        /* if (IsFilled == False && WorldZ < WaterThreshold) */
        /* { */
        /*   IsFilled = True; */
        /*   ThisColor = BLUE; */
        /* } */

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u16(IsFilled);
        ChunkSum += IsFilled;

        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

  return ChunkSum;
}

link_internal u32
TerracedTerrain( perlin_noise *Noise,
                 world_chunk *Chunk,
                 v3i Dim,
                 v3i SrcToDest,
                 u16 ColorIndex,

                 s32 IgnoredFrequency,
                 s32 IgnoredAmplitude,

                 s64 zMin,
                 v3i WorldChunkDim,
                 void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    /* s64 WorldZSubZMin = zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      /* s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y); */
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        /* s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x); */
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        /* Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) ); */

        r32 NoiseValue = 0.f;
        r32 BaseNoiseValue = 0.f;
        /* v3 Normal = V3(0); */
        v3 Derivs = V3(0);
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;
#if 1

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

/* #define MAP_WIDTH 200000000 */
/* #define MAP_WIDTH 200000 */
/*           f32 InX = ((x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) * Octave->Freq.x) / MAP_WIDTH; */
/*           f32 InY = ((y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) * Octave->Freq.y) / MAP_WIDTH; */
/*           f32 InZ = ((z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) * Octave->Freq.z) / MAP_WIDTH; */

          /* f32 InX = SafeDivide0((x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)), Octave->Freq.x); */
          /* f32 InY = SafeDivide0((y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)), Octave->Freq.y); */
          /* f32 InZ = SafeDivide0((z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)), Octave->Freq.z); */
          /* f32 InZ = 1.f; */
#else
          /* f32 InX = f32(WorldX); */
          /* f32 InY = f32(WorldY); */
          /* f32 InZ = f32(WorldZ); */

          /* f32 InX = f32(r64(WorldX)/100.0); */
          /* f32 InY = f32(r64(WorldY)/100.0); */
          /* f32 InZ = f32(r64(WorldZ)/100.0); */

          f32 InX = f32(x/32.f + (WorldChunkDim.x*Chunk->WorldP.x)/32.f) / 10.f;
          f32 InY = f32(y/32.f + (WorldChunkDim.y*Chunk->WorldP.y)/32.f) / 10.f;
          f32 InZ = f32(z/32.f + (WorldChunkDim.z*Chunk->WorldP.z)/32.f) / 10.f;

          /* f32 InX = f32(x/32.f + (WorldChunkDim.x*Chunk->WorldP.x)/32.f); */
          /* f32 InY = f32(y/32.f + (WorldChunkDim.y*Chunk->WorldP.y)/32.f); */
          /* f32 InZ = f32(z/32.f + (WorldChunkDim.z*Chunk->WorldP.z)/32.f); */

          /* InX = 1.f; */
          /* InY = 1.f; */
          /* InZ = 1.f; */

/*           f32 InX = f32(x/32.f + (WorldChunkDim.x*Chunk->WorldP.x)/32.f)/2.f; */
/*           f32 InY = f32(y/32.f + (WorldChunkDim.y*Chunk->WorldP.y)/32.f)/2.f; */
/*           f32 InZ = f32(z/32.f + (WorldChunkDim.z*Chunk->WorldP.z)/32.f)/2.f; */
#endif

          /* r32 Warp = PerlinNoise(InX, InY, InZ); */
          /* v3 WarpFactor = Warp*Octave->WarpStrength; */
          v3 WarpFactor = {};

          /* r32 N = IQ_QuinticGradientNoise(V3(InX+WarpFactor.x, InZ+WarpFactor.z, InY+WarpFactor.y)); */
          /* r32 N = IQ_ValueNoise_AnalyticNormals(InX+WarpFactor.x, InZ+WarpFactor.z, InY+WarpFactor.y,  &Derivs); */
          /* r32 N = IQ_ValueNoise_AnalyticNormals(InX+WarpFactor.x, InZ+WarpFactor.z, InY+WarpFactor.y,  &Derivs); */
          /* r32 N = PerlinNoise_Derivitives0(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z, &Derivs); */
          /* r32 N = PerlinNoise_Derivitives1(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z, &Derivs); */
          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);

          /* Assert(N <= 1.05f); */
          /* Assert(N > -1.05f); */

          BaseNoiseValue += (N*Octave->Amp);
          /* BaseNoiseValue = N; */

          /* Normal += CalcNormal(V3(InX, InY, InZ), N, PerlinNoise ); */
        }

#if 1
        {
          octave TerraceOctaves[] =
          {
            {V3(250, 250, 25), 15.f, V3(2.f)},
          };


          /* RangeIterator(HoodooOctaveIndex, (s32)ArrayCount(TerraceOctaves)) */
          {
            r32 Mask = 0.f;
            {
              octave O = TerraceOctaves[0];
              f32 OctaveX = (x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)) / O.Freq.x;
              f32 OctaveY = (y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)) / O.Freq.y;
              f32 OctaveZ = (z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)) / O.Freq.z;
              /* f32 OctaveZ = 1.f; */
              Mask = PerlinNoise(OctaveX, OctaveY, OctaveZ) * O.Amp;
            }

            /* NoiseValue = Pow(BaseNoiseValue, Mask); */
            /* NoiseValue = BaseNoiseValue * Mask; */
            NoiseValue = BaseNoiseValue / Mask;
          }
        }
#endif

        /* Normal /= OctaveCount; */

        /* Chunk->Voxels[VoxIndex].Derivs = Derivs; */

        /* v3 Tangent, Bitangent, Normal; */
        /* CalculateTBN(Derivs, &Tangent, &Bitangent, &Normal); */

        /* Chunk->Voxels[VoxIndex].DebugColor = Derivs; */
        /* Chunk->Voxels[VoxIndex].DebugColor = Normalize(Normal); */

        /* Info("%f", r64(NoiseValue)); */
        /* b32 IsFilled = r32(NoiseValue) > 100.f; */
        /* b32 IsFilled = r32(NoiseValue) > 0.5f; */
        /* b32 IsFilled = r32(NoiseValue) > 0.f; */
        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        /* u16 StartColorMin = GREY_4; */
        /* u16 StartColorMax = GREY_6; */
        /* u16 STONE = GREY_5; */
        /* u16 ThisColor = SafeTruncateToU16(umm(RandomBetween(StartColorMin, &GenColorEntropy, StartColorMax))); */
        u16 ThisColor = DIRT;

        u8 ThisTransparency = 0;

        s32 SandThreshold   = 3;
        s32 GravelThreshold = 1;
        s32 WaterThreshold  = 0;

        r32 StoneThresh = r32(WorldZSubZMin) + 2.f;
        r32 DirtThresh = r32(WorldZSubZMin) + 1.f;

        if (IsFilled)
        {
          ThisColor = GRASS_GREEN;

          /* if (NoiseValue > DirtThresh) */
          /* { */
          /*   ThisColor = DIRT; */
          /* } */

          if (NoiseValue > StoneThresh)
          {
            ThisColor = DIRT;
          }


          /* if (WorldZ < SandThreshold) */
          /* { */
          /*   ThisColor = SAND; */
          /* } */

          /* if (WorldZ < GravelThreshold) */
          /* { */
          /*   ThisColor = GRAVEL; */
          /* } */
        }

        if (IsFilled == False && WorldZ < WaterThreshold)
        {
          /* IsFilled = True; */
          /* ThisColor = BLUE; */
          /* ThisTransparency = 255; */
        }

        /* if (IsFilled && ThisColor == StartColorMin) */
#if 0
        if (IsFilled)
        {
          /* if (Dot(TerrainNormal, V3(0, 0, 1)) > 0.5f) */
          if (Dot(Normal, V3(0, 0, 1)) > 0.65f)
          {
            ThisColor = DIRT;
          }
        }
#endif

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        Chunk->Voxels[VoxIndex].Transparency = ThisTransparency;
        ChunkSum += IsFilled;



        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  ComputeNormalsForChunk(Chunk, Dim);

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s32 VoxIndex = GetIndex(x, y, z, Dim);
        voxel *V = Chunk->Voxels+VoxIndex;
        if (V->Flags & Voxel_Filled)
        {
          v3 Normal = V->DebugColor * -1.f;
          /* V->DebugColor = V3(Dot(Normal, V3(0, 0, 1)), 0.f, 0.f); */

          if (V->Color == DIRT &&
              Dot(Normal, V3(0, 0, 1)) > 0.7f)
          {
            V->Color = GRASS_GREEN;
          }
        }
      }
    }
  }
#endif

  return ChunkSum;
}

link_internal u32
GrassyTerracedTerrain2( perlin_noise *Noise,
                       world_chunk *Chunk,
                       v3i Dim,
                       v3i SrcToDest,
                       u16 ColorIndex,

                       s32 IgnoredFrequency,
                       s32 IgnoredAmplitude,

                       s64 zMin,
                       v3i WorldChunkDim,
                       void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;


  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    /* s64 WorldZSubZMin = zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      /* s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y); */
      for ( s32 x = 0; x < Dim.x; ++ x)
      {

        r32 TerraceMask = 0.f;
        {
          octave TerraceOctaves[] =
          {
            {V3(250, 250, 25), 15.f, V3(2.f)},
          };
          /* RangeIterator(HoodooOctaveIndex, (s32)ArrayCount(TerraceOctaves)) */
          {
            {
              octave O = TerraceOctaves[0];
              f32 OctaveX = (x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)) / O.Freq.x;
              f32 OctaveY = (y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)) / O.Freq.y;
              f32 OctaveZ = (z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)) / O.Freq.z;
              /* f32 OctaveZ = 1.f; */
              TerraceMask = PerlinNoise(OctaveX, OctaveY, OctaveZ) * O.Amp;
            }
          }
        }

        /* s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x); */
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        /* Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) ); */

        r32 NoiseValue = 0.f;
        r32 BaseNoiseValue = 0.f;
        /* v3 Normal = V3(0); */
        v3 Derivs = V3(0);
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

          /* r32 Warp = PerlinNoise(InX, InY, InZ); */
          /* v3 WarpFactor = Warp*Octave->WarpStrength; */
          v3 WarpFactor = {};

          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);

          /* Assert(N <= 1.05f); */
          /* Assert(N > -1.05f); */

          BaseNoiseValue += (N*Octave->Amp);
        }

        NoiseValue = BaseNoiseValue / TerraceMask;
        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        u16 ThisColor = DIRT;

        u8 ThisTransparency = 0;

        s32 SandThreshold   = 3;
        s32 GravelThreshold = 1;
        s32 WaterThreshold  = 0;

        r32 StoneThresh = r32(WorldZSubZMin) + 2.f;
        r32 DirtThresh = r32(WorldZSubZMin) + 1.f;

        /* if (IsFilled) */
        {
          ThisColor = GRASS_GREEN;

          if (NoiseValue > StoneThresh)
          {
            ThisColor = DIRT;
          }
        }

        if (!IsFilled)
        {
          f32 GrassAreaX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / 32.f;
          f32 GrassAreaY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / 32.f;
          f32 GrassAreaZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / 32.f;

          r32 GrassyAreaValue = PerlinNoise(GrassAreaX, GrassAreaY, GrassAreaZ);
          if (ThisColor == GRASS_GREEN)
          {
            if (GrassyAreaValue > 0.5f && NoiseValue+(GrassyAreaValue*5.f) > WorldZSubZMin)
            {
              f32 GrassX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / 30.1f;
              f32 GrassY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / 30.1f;
              f32 GrassZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / 450.f;

              r32 GrassValue = PerlinNoise(GrassX, GrassY, GrassZ);

              /* if (NoiseValue+(GrassValue*2.5f) > WorldZSubZMin) */
              {
                IsFilled = True;
              }
            }
          }
        }

        GrowGrass( Chunk, V3i(x,y,z), NoiseValue, 1.f-TerraceMask, SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled );

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        Chunk->Voxels[VoxIndex].Transparency = ThisTransparency;
        ChunkSum += IsFilled;



        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  RangeIterator(z, (s32)Chunk->Dim.z)
  RangeIterator(y, (s32)Chunk->Dim.y)
  RangeIterator(x, (s32)Chunk->Dim.x)
  {
    voxel *V   = GetVoxel(Chunk, V3i(x,y,z));
    voxel *VUp = TryGetVoxel(Chunk, V3i(x,y,z+1));

    if (VUp && (VUp->Flags & Voxel_Filled) == 0)
    {
      if (V->Flags & Voxel_Filled)
      {
      }
    }
  }
#endif


  return ChunkSum;
}

link_internal u32
GrassyTerracedTerrain3( perlin_noise *Noise,
                       world_chunk *Chunk,
                       v3i Dim,
                       v3i SrcToDest,
                       u16 ColorIndex,

                       s32 IgnoredFrequency,
                       s32 IgnoredAmplitude,

                       s64 zMin,
                       v3i WorldChunkDim,
                       void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;


  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    /* s64 WorldZSubZMin = zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {

        r32 TerraceMask = 0.f;
#if 0
        {
          octave TerraceOctaves[] =
          {
            {V3(250, 250, 25), 15.f, V3(2.f)},
          };
          /* RangeIterator(HoodooOctaveIndex, (s32)ArrayCount(TerraceOctaves)) */
          {
            {
              octave O = TerraceOctaves[0];
              f32 OctaveX = (x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)) / O.Freq.x;
              f32 OctaveY = (y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)) / O.Freq.y;
              f32 OctaveZ = (z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)) / O.Freq.z;
              /* f32 OctaveZ = 1.f; */
              TerraceMask = PerlinNoise(OctaveX, OctaveY, OctaveZ) * O.Amp;
            }
          }
        }
#endif

        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        /* Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) ); */

        r32 NoiseValue = 0.f;
        r32 BaseNoiseValue = 0.f;
        /* v3 Normal = V3(0); */
        v3 Derivs = V3(0);

        f32 MaxValue = 0.f;
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

          /* r32 Warp = PerlinNoise(InX, InY, InZ); */
          /* v3 WarpFactor = Warp*Octave->WarpStrength; */
          v3 WarpFactor = {};

          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);
#if 1
          if (OctaveIndex == 0)
          {
            BaseNoiseValue += MapNoiseValueToFinal(N) * Octave->Amp;
          }
          else
          {
            BaseNoiseValue += N * Octave->Amp;
          }
#else
            BaseNoiseValue += MapNoiseValueToFinal(N) * Octave->Amp;
#endif

          MaxValue += Octave->Amp;
        }

        /* NoiseValue = BaseNoiseValue / TerraceMask; */
        NoiseValue = BaseNoiseValue;

        u16 ThisColor = MapNoiseValueToMaterial(BaseNoiseValue/MaxValue);

        u8 ThisTransparency = 0;

        if (ThisColor == STONE)
        {
          r32 Voronoi = VoronoiNoise3D(V3(s32(WorldX), s32(WorldY), s32(WorldZ)) * 0.04f) * 25.f;
          if (Voronoi < 2.f)
          {
            NoiseValue -= 2.f;
          }

          if (Voronoi < 5.f)
          {
            ThisColor = DARK_STONE;
          }
        }

        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin);

        GrowGrass( Chunk, V3i(x,y,z), NoiseValue, 1.f-TerraceMask, SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled );

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        Chunk->Voxels[VoxIndex].Transparency = ThisTransparency;
        ChunkSum += IsFilled;



        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  RangeIterator(z, (s32)Chunk->Dim.z)
  RangeIterator(y, (s32)Chunk->Dim.y)
  RangeIterator(x, (s32)Chunk->Dim.x)
  {
    voxel *V   = GetVoxel(Chunk, V3i(x,y,z));
    voxel *VUp = TryGetVoxel(Chunk, V3i(x,y,z+1));

    if (VUp && (VUp->Flags & Voxel_Filled) == 0)
    {
      if (V->Flags & Voxel_Filled)
      {
      }
    }
  }
#endif


  return ChunkSum;
}

link_internal u32
GrassyTerracedTerrain4( perlin_noise *Noise,
                       world_chunk *Chunk,
                       v3i Dim,
                       v3i SrcToDest,
                       u16 ColorIndex,

                       s32 IgnoredFrequency,
                       s32 IgnoredAmplitude,

                       s64 zMin,
                       v3i WorldChunkDim,
                       void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  Assert(Dim == Chunk->Dim);

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;

  f32 MaxNoiseValue = 0.f;
  for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
  {
    octave *Octave = OctaveBuf->Octaves+OctaveIndex;
    MaxNoiseValue += Octave->Amp;
  }

  r32 *NoiseValues = Allocate(r32, GetTranArena(), Volume(Dim));

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);

        r32 *NoiseValue = NoiseValues + VoxIndex;
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

          r32 N = PerlinNoise(InX, InY, InZ);
          if (OctaveIndex == 0)
          {
            *NoiseValue += MapNoiseValueToFinal(N) * Octave->Amp;
          }
          else
          {
            *NoiseValue += N * Octave->Amp;
          }
        }
      }
    }
  }

  s64 ChunkWorldZ = SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z) - zMin;

  v3 *Normals = Allocate(v3, GetTranArena(), Volume(Dim));
  ComputeNormalsForChunkFromNoiseValues(Dim, ChunkWorldZ, NoiseValues, Normals);

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    /* s64 WorldZSubZMin = WorldZ - zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);

        r32 *NoiseValue = NoiseValues + VoxIndex;
        v3  *Normal = Normals + VoxIndex;
        /* Chunk->Voxels[VoxIndex].DebugColor.x = *NoiseValue / MaxNoiseValue;; */

        u16 ThisColor = STONE;

#if 1
        if (Dot(*Normal, V3(0,0,1)) < 0.65f)
        {
          r32 Squareness = 0.15f;
          r32 Voronoi = VoronoiNoise3D(V3(s32(WorldX), s32(WorldY), s32(WorldZ)) * 0.04f, Squareness) * 25.f;
          if (Voronoi < 2.f)
          {
            *NoiseValue -= 1.6f;
          }

          if (Voronoi < 2.5f)
          {
            ThisColor = DARK_STONE;
          }
        }
        else
        {
          ThisColor = GRASS_GREEN;
        }

#endif
        b32 IsFilled = *NoiseValue > r32(z+ChunkWorldZ);

        /* GrowGrass( Chunk, V3i(x,y,z), *NoiseValue, 1.f, SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled ); */

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        /* Chunk->Voxels[VoxIndex].Transparency = ThisTransparency; */
        ChunkSum += IsFilled;

        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  v3 *Normals = Allocate(v3, GetTranArena(), Volume(Dim));
  ComputeNormalsForChunkFromFilledFlag(Chunk, WorldChunkDim, Normals);
#endif

#if VOXEL_DEBUG_COLOR
#if 1
  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].DebugColor = Normals[VoxIndex];
      }
    }
  }
#endif
#endif

  return ChunkSum;
}

link_internal u32
GrassyTerracedTerrain( perlin_noise *Noise,
                       world_chunk *Chunk,
                       v3i Dim,
                       v3i SrcToDest,
                       u16 ColorIndex,

                       s32 IgnoredFrequency,
                       s32 IgnoredAmplitude,

                       s64 zMin,
                       v3i WorldChunkDim,
                       void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    /* s64 WorldZSubZMin = zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      /* s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y); */
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        /* s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x); */
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        /* Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) ); */

        r32 NoiseValue = 0.f;
        r32 BaseNoiseValue = 0.f;
        /* v3 Normal = V3(0); */
        v3 Derivs = V3(0);
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

          /* r32 Warp = PerlinNoise(InX, InY, InZ); */
          /* v3 WarpFactor = Warp*Octave->WarpStrength; */
          v3 WarpFactor = {};

          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);

          /* Assert(N <= 1.05f); */
          /* Assert(N > -1.05f); */

          BaseNoiseValue += (N*Octave->Amp);
          /* BaseNoiseValue = N; */

          /* Normal += CalcNormal(V3(InX, InY, InZ), N, PerlinNoise ); */
        }

#if 1
        {
          octave TerraceOctaves[] =
          {
            {V3(250, 250, 25), 15.f, V3(2.f)},
          };


          /* RangeIterator(HoodooOctaveIndex, (s32)ArrayCount(TerraceOctaves)) */
          {
            r32 Mask = 0.f;
            {
              octave O = TerraceOctaves[0];
              f32 OctaveX = (x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)) / O.Freq.x;
              f32 OctaveY = (y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)) / O.Freq.y;
              f32 OctaveZ = (z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)) / O.Freq.z;
              /* f32 OctaveZ = 1.f; */
              Mask = PerlinNoise(OctaveX, OctaveY, OctaveZ) * O.Amp;
            }

            /* NoiseValue = Pow(BaseNoiseValue, Mask); */
            /* NoiseValue = BaseNoiseValue * Mask; */
            NoiseValue = BaseNoiseValue / Mask;
          }
        }
#endif

        /* Normal /= OctaveCount; */

        /* Chunk->Voxels[VoxIndex].Derivs = Derivs; */

        /* v3 Tangent, Bitangent, Normal; */
        /* CalculateTBN(Derivs, &Tangent, &Bitangent, &Normal); */

        /* Chunk->Voxels[VoxIndex].DebugColor = Derivs; */
        /* Chunk->Voxels[VoxIndex].DebugColor = Normalize(Normal); */

        /* Info("%f", r64(NoiseValue)); */
        /* b32 IsFilled = r32(NoiseValue) > 100.f; */
        /* b32 IsFilled = r32(NoiseValue) > 0.5f; */
        /* b32 IsFilled = r32(NoiseValue) > 0.f; */
        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        /* u16 StartColorMin = GREY_4; */
        /* u16 StartColorMax = GREY_6; */
        /* u16 STONE = GREY_5; */
        /* u16 ThisColor = SafeTruncateToU16(umm(RandomBetween(StartColorMin, &GenColorEntropy, StartColorMax))); */
        u16 ThisColor = DIRT;

        u8 ThisTransparency = 0;

        s32 SandThreshold   = 3;
        s32 GravelThreshold = 1;
        s32 WaterThreshold  = 0;

        r32 StoneThresh = r32(WorldZSubZMin) + 2.f;
        /* r32 DirtThresh = r32(WorldZSubZMin) + 1.f; */

        /* if (IsFilled) */
        {
          ThisColor = GRASS_GREEN;

          if (NoiseValue > StoneThresh)
          {
            ThisColor = DIRT;
          }
        }

        GrowGrass( Chunk, V3i(x,y,z), NoiseValue, 1.f, SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled );

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        Chunk->Voxels[VoxIndex].Transparency = ThisTransparency;
        ChunkSum += IsFilled;



        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  RangeIterator(z, (s32)Chunk->Dim.z)
  RangeIterator(y, (s32)Chunk->Dim.y)
  RangeIterator(x, (s32)Chunk->Dim.x)
  {
    voxel *V   = GetVoxel(Chunk, V3i(x,y,z));
    voxel *VUp = TryGetVoxel(Chunk, V3i(x,y,z+1));

    if (VUp && (VUp->Flags & Voxel_Filled) == 0)
    {
      if (V->Flags & Voxel_Filled)
      {
      }
    }
  }
#endif


  return ChunkSum;
}

link_internal u32
GrassyLargeTerracedTerrain( perlin_noise *Noise,
                            world_chunk *Chunk,
                            v3i Dim,
                            v3i SrcToDest,
                            u16 ColorIndex,

                            s32 IgnoredFrequency,
                            s32 IgnoredAmplitude,

                            s64 zMin,
                            v3i WorldChunkDim,
                            void *OctavesIn )
{
  TIMED_FUNCTION();
  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  octave_buffer *OctaveBuf = (octave_buffer*)OctavesIn;
  u32 OctaveCount = OctaveBuf->Count;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    /* s64 WorldZSubZMin = zMin; */
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      /* s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y); */
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        r32 TerraceMask = 0.f;
        r32 TerraceNoiseValue = 0.f;
        {
          octave TerraceOctaves[] =
          {
            {V3(500, 500, 100), 15.f, V3(2.f)},
          };

          {
            octave O = TerraceOctaves[0];
            f32 OctaveX = (x + SrcToDest.x + ( WorldChunkDim.x*Chunk->WorldP.x)) / O.Freq.x;
            f32 OctaveY = (y + SrcToDest.y + ( WorldChunkDim.y*Chunk->WorldP.y)) / O.Freq.y;
            f32 OctaveZ = (z + SrcToDest.z + ( WorldChunkDim.z*Chunk->WorldP.z)) / O.Freq.z;
            /* f32 OctaveZ = 1.f; */
            TerraceMask = PerlinNoise(OctaveX, OctaveY, OctaveZ);
            TerraceNoiseValue = TerraceMask * O.Amp;
          }

        }

        /* s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x); */
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        /* Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) ); */

        r32 NoiseValue = 0.f;
        r32 BaseNoiseValue = 0.f;
        /* v3 Normal = V3(0); */
        v3 Derivs = V3(0);
        for (u32 OctaveIndex = 0; OctaveIndex < OctaveCount; ++OctaveIndex)
        {
          octave *Octave = OctaveBuf->Octaves+OctaveIndex;

          f32 InX = (x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x)) / Octave->Freq.x;
          f32 InY = (y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y)) / Octave->Freq.y;
          f32 InZ = (z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z)) / Octave->Freq.z;

          /* r32 Warp = PerlinNoise(InX, InY, InZ); */
          /* v3 WarpFactor = Warp*Octave->WarpStrength; */
          v3 WarpFactor = {};

          r32 N = PerlinNoise(InX+WarpFactor.x, InY+WarpFactor.y, InZ+WarpFactor.z);

          /* Assert(N <= 1.05f); */
          /* Assert(N > -1.05f); */

          /* BaseNoiseValue = N; */

          /* Normal += CalcNormal(V3(InX, InY, InZ), N, PerlinNoise ); */
          if (OctaveIndex < 2)
          {
            BaseNoiseValue += (N*Octave->Amp) / TerraceNoiseValue;
            /* BaseNoiseValue += (N*Octave->Amp); */
          }
          else
          {
            BaseNoiseValue += (N*Octave->Amp);
          }
        }

        NoiseValue = BaseNoiseValue;

        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        u16 ThisColor = DIRT;

        u8 ThisTransparency = 0;

        s32 SandThreshold   = 3;
        s32 GravelThreshold = 1;
        s32 WaterThreshold  = 0;

        r32 StoneThresh = r32(WorldZSubZMin) + 2.f;
        /* r32 DirtThresh = r32(WorldZSubZMin) + 1.f; */

        /* if (IsFilled) */
        {
          ThisColor = GRASS_GREEN;

          if (NoiseValue > StoneThresh)
          {
            ThisColor = DIRT;
          }
        }

        Assert(TerraceMask >= 0.f);
        Assert(TerraceMask <= 1.1f);
        /* GrowGrass( Chunk, V3i(x,y,z), NoiseValue, 0.7f-TerraceMask, SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled ); */
        GrowGrass( Chunk, V3i(x,y,z), NoiseValue, 1.f-Clamp01(TerraceMask), SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled );
        /* GrowGrass( Chunk, V3i(x,y,z), NoiseValue, Clamp01(TerraceMask), SrcToDest, WorldChunkDim, WorldZSubZMin, &ThisColor, &IsFilled ); */

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = ThisColor*u8(IsFilled);
        Chunk->Voxels[VoxIndex].Transparency = ThisTransparency;
        ChunkSum += IsFilled;



        Assert( (Chunk->Voxels[VoxIndex].Flags&VoxelFaceMask) == 0);

        if (IsFilled)
        {
          Assert( IsSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
        else
        {
          Assert( NotSet(&Chunk->Voxels[VoxIndex], Voxel_Filled) );
        }
      }
    }
  }

#if 0
  RangeIterator(z, (s32)Chunk->Dim.z)
  RangeIterator(y, (s32)Chunk->Dim.y)
  RangeIterator(x, (s32)Chunk->Dim.x)
  {
    voxel *V   = GetVoxel(Chunk, V3i(x,y,z));
    voxel *VUp = TryGetVoxel(Chunk, V3i(x,y,z+1));

    if (VUp && (VUp->Flags & Voxel_Filled) == 0)
    {
      if (V->Flags & Voxel_Filled)
      {
      }
    }
  }
#endif


  return ChunkSum;
}

link_internal u32
SinCosTerrain( perlin_noise *Noise,
               world_chunk *Chunk,
               v3i Dim,
               v3i SrcToDest,
               u16 ColorIndex,

               s32 Frequency,
               s32 Amplitude,

               s64 zMin,
               v3i WorldChunkDim,
               void *OctavesIn )
{
  TIMED_FUNCTION();
  Assert(OctavesIn == 0);

  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;
        r32 NoiseValue = ((Sin(r32(WorldX)/r32(Frequency))+1.f)/4.f) + ((Cos(r32(WorldY)/r32(Frequency))+1.f)/4.f);

        NoiseValue = MapNoiseValueToFinal(NoiseValue);
        NoiseValue *= Amplitude;

        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = GRASS_GREEN*u8(IsFilled);
        ChunkSum += IsFilled;
      }
    }
  }
  return ChunkSum;
}

link_internal u32
VoronoiTerrain( perlin_noise *Noise,
                world_chunk *Chunk,
                v3i Dim,
                v3i SrcToDest,
                u16 ColorIndex,

                s32 Frequency,
                s32 Amplitude,
 
                s64 zMin,
                v3i WorldChunkDim,
                void *OctavesIn )
{
  TIMED_FUNCTION();
  Assert(OctavesIn == 0);

  u32 ChunkSum = 0;

  s32 MinZ = Chunk->WorldP.z*WorldChunkDim.z;
  s32 MaxZ = MinZ+WorldChunkDim.z ;

  for ( s32 z = 0; z < Dim.z; ++ z)
  {
    s64 WorldZ = z + SrcToDest.z + (WorldChunkDim.z*Chunk->WorldP.z);
    s64 WorldZSubZMin = WorldZ - zMin;
    for ( s32 y = 0; y < Dim.y; ++ y)
    {
      s64 WorldY = y + SrcToDest.y + (WorldChunkDim.y*Chunk->WorldP.y);
      for ( s32 x = 0; x < Dim.x; ++ x)
      {
        s64 WorldX = x + SrcToDest.x + (WorldChunkDim.x*Chunk->WorldP.x);
        s32 VoxIndex = GetIndex(Voxel_Position(x,y,z), Dim);
        Chunk->Voxels[VoxIndex].Flags = Voxel_Empty;

        r32 NoiseValue = VoronoiNoise3D(V3(s32(WorldX), s32(WorldY), s32(WorldZ)) * 0.01f);
        NoiseValue = Clamp01(NoiseValue);
        /* NoiseValue = MapNoiseValueToFinal(NoiseValue); */ // Crazyballs
        NoiseValue *= Amplitude;

        b32 IsFilled = r32(NoiseValue) > r32(WorldZSubZMin) ;

        SetFlag(&Chunk->Voxels[VoxIndex], (voxel_flag)(Voxel_Filled*IsFilled));
        Chunk->Voxels[VoxIndex].Color = GRASS_GREEN*u8(IsFilled);
        ChunkSum += IsFilled;
      }
    }
  }
  return ChunkSum;
}
