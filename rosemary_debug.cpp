/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

//
// NOTE: String Helpers
// 

inline void DebugSnprintf(char* Dst, u32 StringSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(Dst, StringSize, Text, Arguments);
    va_end(Arguments);
}

//
// NOTE: Rendering
//

inline void DebugRenderStateBeginFrame(render_state* RenderState)
{
#if ROSEMARY_DEBUG
    debug_render_state* State = &DebugState->RenderState;
    State->RenderState = RenderState;
    State->NumInteractiveModels = 0;
    
    State->CurrPoint = 0;
    State->PointCurrProj = 0;
    ZeroMem(State->PointNumVerts, sizeof(u32)*State->PointMaxNumProjs);
    
    State->CurrLineVert = 0;
    State->LineCurrProj = 0;
    ZeroMem(State->LineNumVerts, sizeof(u32)*State->LineMaxNumProjs);
#endif
}

inline void DebugPushPointProjection(m4 Projection)
{
#if ROSEMARY_DEBUG
    debug_render_state* State = &DebugState->RenderState;
    Assert(State->PointCurrProj < State->PointMaxNumProjs);

    State->PointProjs[State->PointCurrProj] = Projection;
    State->PointCurrProj += 1;
#endif
}

inline void DebugPushPointScreenProjection()
{
    m4 OrthoProj = RenderOrthoProjM4(0, 1, 1, 0, 0.001, 1000);
    DebugPushPointProjection(OrthoProj);
}

inline void DebugPushPoint(v3 Pos, v3 Color, f32 Size)
{
#if ROSEMARY_DEBUG
    debug_render_state* State = &DebugState->RenderState;
    Assert(State->CurrPoint < State->MaxNumPoints);
    
    State->PointPosArray[State->CurrPoint] = Pos;
    State->PointColorArray[State->CurrPoint] = Color;
    State->PointSizeArray[State->CurrPoint] = Size;
    State->CurrPoint += 1;

    Assert(State->PointCurrProj > 0);
    State->PointNumVerts[State->PointCurrProj - 1] += 1;
#endif
}

inline void DebugPushLineProjection(m4 Projection)
{
#if ROSEMARY_DEBUG
    debug_render_state* State = &DebugState->RenderState;
    Assert(State->LineCurrProj < State->LineMaxNumProjs);

    State->LineProjs[State->LineCurrProj] = Projection;
    State->LineCurrProj += 1;
#endif
}

inline void DebugPushLineScreenProjection()
{
    m4 OrthoProj = RenderOrthoProjM4(0, 1, 1, 0, 0.001, 1000);
    DebugPushLineProjection(OrthoProj);
}

inline void DebugPushLine(v3 StartPos, v3 EndPos, v3 StartColor, v3 EndColor)
{
#if ROSEMARY_DEBUG
    debug_render_state* State = &DebugState->RenderState;
    Assert(State->CurrLineVert + 2 <= State->MaxNumLineVerts);
    
    State->LinePosArray[State->CurrLineVert] = StartPos;
    State->LineColorArray[State->CurrLineVert] = StartColor;
    State->CurrLineVert += 1;

    State->LinePosArray[State->CurrLineVert] = EndPos;
    State->LineColorArray[State->CurrLineVert] = EndColor;
    State->CurrLineVert += 1;

    Assert(State->LineCurrProj > 0);
    State->LineNumVerts[State->LineCurrProj - 1] += 2;
#endif
}

inline void DebugPushLineBox(v3 Center, v3 Radius, v3 LineColor)
{
    // NOTE: Top square lines
    {
        v3 CornerTR = Center + V3(Radius.x, Radius.y, Radius.z);
        v3 CornerTL = CornerTR - 2.0f*V3(Radius.x, 0.0f, 0.0f);
        v3 CornerBL = CornerTL - 2.0f*V3(0.0f, Radius.y, 0.0f);
        v3 CornerBR = CornerBL + 2.0f*V3(Radius.x, 0.0f, 0.0f);
        DebugPushLine(CornerTR, CornerTL, LineColor, LineColor);
        DebugPushLine(CornerTL, CornerBL, LineColor, LineColor);
        DebugPushLine(CornerBL, CornerBR, LineColor, LineColor);
        DebugPushLine(CornerBR, CornerTR, LineColor, LineColor);
    }
                                
    // NOTE: Bottom square lines
    {
        v3 CornerBL = Center + V3(-Radius.x, -Radius.y, -Radius.z);
        v3 CornerBR = CornerBL + 2.0f*V3(Radius.x, 0.0f, 0.0f);
        v3 CornerTR = CornerBR + 2.0f*V3(0.0f, Radius.y, 0.0f);
        v3 CornerTL = CornerTR - 2.0f*V3(Radius.x, 0.0f, 0.0f);
        DebugPushLine(CornerTR, CornerTL, LineColor, LineColor);
        DebugPushLine(CornerTL, CornerBL, LineColor, LineColor);
        DebugPushLine(CornerBL, CornerBR, LineColor, LineColor);
        DebugPushLine(CornerBR, CornerTR, LineColor, LineColor);
    }
                                
    // NOTE: Side lines
    {
        v3 CornerTR = Center + V3(Radius.x, Radius.y, Radius.z);
        v3 CornerTL = CornerTR - 2.0f*V3(Radius.x, 0.0f, 0.0f);
        v3 CornerBL = CornerTL - 2.0f*V3(0.0f, Radius.y, 0.0f);
        v3 CornerBR = CornerBL + 2.0f*V3(Radius.x, 0.0f, 0.0f);
        DebugPushLine(CornerTR, CornerTR - V3(0.0f, 0.0f, 2.0f*Radius.z), LineColor, LineColor);
        DebugPushLine(CornerTL, CornerTL - V3(0.0f, 0.0f, 2.0f*Radius.z), LineColor, LineColor);
        DebugPushLine(CornerBL, CornerBL - V3(0.0f, 0.0f, 2.0f*Radius.z), LineColor, LineColor);
        DebugPushLine(CornerBR, CornerBR - V3(0.0f, 0.0f, 2.0f*Radius.z), LineColor, LineColor);
    }
}

inline void DebugPushModel(asset_model_id ModelId, v3 Pos, v3 Radius, q4 Rotation, asset_indexed_texture TextureId, v4 Color)
{
    debug_render_state* DebugRenderState = &DebugState->RenderState;
    render_state* RenderState = DebugRenderState->RenderState;

    if (TextureId == Texture_None)
    {
        RenderPushStaticModel(RenderState, ModelId, Pos, Radius, Rotation, Color, 0);
    }
    else
    {
        render_material Material = RenderMaterialGeometry(TextureId);
        RenderPushStaticModel(RenderState, ModelId, Pos, Radius, Rotation, Color, &Material);
    }
}

inline void DebugPushModel(asset_model_id ModelId, v3 Pos, v3 Radius, q4 Rotation, v4 Color)
{
    // TODO: Handle the custom materials
    DebugPushModel(ModelId, Pos, Radius, Rotation, Texture_None, Color);
}

inline entity_handle DebugPushInteractiveModel(asset_model_id ModelId, v3 Pos, v3 Radius,
                                               q4 Rotation, v4 Color)
{
    debug_render_state* DebugRenderState = &DebugState->RenderState;
    render_state* RenderState = DebugRenderState->RenderState;

    u32 Id = DebugRenderState->NumInteractiveModels++;
    entity_handle Result = EntityHandleToEditorObject(Id);

    RenderPushStaticModel(RenderState, ModelId, Pos, Radius, Rotation, Color, Result);
    
    return Result;
}

//
// NOTE: CPU Profiling
//

inline u32 DebugGetNextFrameId(u32 CurrFrameId)
{
    u32 Result = CurrFrameId + 1;
    if (Result >= DEBUG_MAX_NUM_FRAMES)
    {
        Result = 0;
    }

    return Result;
}

inline u32 DebugGetPrevFrameId(u32 CurrFrameId)
{
    u32 Result = CurrFrameId - 1;
    if (Result < 0)
    {
        Result = DEBUG_MAX_NUM_FRAMES - 1;
    }

    return Result;
}

inline void DebugRecordingPause()
{
    DebugState->PausedTimeStampId = DebugState->CurrTimeStampId;
}

inline void DebugRecordingUnpause()
{
    DebugState->CurrTimeStampId = DebugState->PausedTimeStampId;
}

inline debug_open_block_stack DebugOpenBlockStackInit(linear_arena* Arena, u32 MaxNumOpenBlocks)
{
    debug_open_block_stack Result = {};
    Result.MaxNumOpenBlocks = MaxNumOpenBlocks;
    Result.CurrNumOpenBlocks = 0;
    Result.Blocks = PushArray(Arena, debug_open_block, Result.MaxNumOpenBlocks);

    return Result;
}

inline debug_open_block* DebugOpenBlockStackGetTop(debug_open_block_stack* Stack)
{
    Assert(Stack->CurrNumOpenBlocks > 0);
    debug_open_block* Result = Stack->Blocks + Stack->CurrNumOpenBlocks - 1;
    return Result;
}

inline debug_open_block* DebugOpenBlockAlloc(debug_open_block_stack* Stack)
{
    Assert(Stack->CurrNumOpenBlocks < Stack->MaxNumOpenBlocks);
    
    debug_open_block* Result = {};
    Result = Stack->Blocks + Stack->CurrNumOpenBlocks++;
    Result->FrameIndex = DebugState->CurrFrameId;

    return Result;
}

inline void DebugOpenBlockDealloc(debug_open_block_stack* Stack)
{
    Assert(Stack->CurrNumOpenBlocks > 0);
    Stack->CurrNumOpenBlocks -= 1;
}

inline debug_function_info* DebugFunctionInfoGetOrCreate(debug_frame* Frame, char* GUID)
{
    debug_function_info* Result = 0;

    // NOTE: Loop through our linked list and check if we have this GUID
    b32 Found = false;
    block_list_block* CurrEntry = Frame->FunctionInfoSentinel.Next;
    if (BlockListIsEmpty(&Frame->FunctionInfoSentinel))
    {
        while (!Found)
        {
            for (u32 InfoId = 0; InfoId < CurrEntry->NumEntries; ++InfoId)
            {
                debug_function_info* CurrInfo = (debug_function_info*)CurrEntry->Data + InfoId;
                if (CurrInfo->GUID == GUID)
                {
                    Found = true;
                    Result = CurrInfo;
                    break;
                }
            }
        
            if (BlockListIsLast(&Frame->FunctionInfoSentinel, CurrEntry) || Found)
            {
                break;
            }
            else
            {
                CurrEntry = CurrEntry->Next;
            }
        }
    }
    
    if (!Found)
    {
        Result = BlockListAddEntry(&DebugState->BlockArena, &Frame->FunctionInfoSentinel, debug_function_info);
        *Result = {};
        Result->GUID = GUID;
    }
    
    return Result;
}

inline void DebugFrameInit(debug_frame* Frame)
{
    Frame->SecondsElapsed = 0.0;
    Frame->BeginCycle = 0;
    Frame->EndCycle = 0;
    BlockListInitSentinel(&Frame->TimeBlockSentinel);
    BlockListInitSentinel(&Frame->FunctionInfoSentinel);
    BlockListInitSentinel(&Frame->ArenaInfoSentinel);
}

internal void DebugStateInit(game_memory* GameMem, input_state* InputState)
{
    *DebugState = {};
    DebugState->Arena = LinearArenaInit(DebugState + 1, GameMem->DebugMemSize - sizeof(debug_state));
    DebugState->PauseMemoryProfiling = true;
    DebugState->BlockArena = BlockSubArena(&DebugState->Arena, DEBUG_BLOCK_SIZE, 5000);

#if DEBUG_MEMORY_PROFILING
    BlockListInitSentinel(&DebugState->ArenaInfoSentinel);
    DebugState->PauseMemoryProfiling = false;
#endif

#if DEBUG_PROFILE_DATA
    DebugState->CpuTimeStamps = PushArray(&DebugState->Arena, debug_cpu_timestamp, NUM_DEBUG_EVENTS_RECORDED);
    DebugState->PrevCpuTimeStamps = PushArray(&DebugState->Arena, debug_cpu_timestamp, NUM_DEBUG_EVENTS_RECORDED);
#endif

    // NOTE: Init debug render state
    {
        debug_render_state* RenderState = &DebugState->RenderState;

        // NOTE: Points
        {
            // NOTE: Create debug point projection data
            RenderState->PointMaxNumProjs = 10;
            RenderState->PointCurrProj = 0;
            RenderState->PointProjs = PushArray(&DebugState->Arena, m4, RenderState->PointMaxNumProjs);
            RenderState->PointNumVerts = PushArray(&DebugState->Arena, u32, RenderState->PointMaxNumProjs);
            ZeroMem(RenderState->PointNumVerts, sizeof(u32)*RenderState->PointMaxNumProjs);

            // NOTE: Create debug point vertex buffers
            RenderState->MaxNumPoints = 40000;
            RenderState->CurrPoint = 0;
            RenderState->PointPosArray = PushArray(&DebugState->Arena, v3, RenderState->MaxNumPoints);
            RenderState->PointColorArray = PushArray(&DebugState->Arena, v3, RenderState->MaxNumPoints);
            RenderState->PointSizeArray = PushArray(&DebugState->Arena, f32, RenderState->MaxNumPoints);
        }
    
        // NOTE: Lines
        {
            // NOTE: Create debug line projection data
            RenderState->LineMaxNumProjs = 10;
            RenderState->LineCurrProj = 0;
            RenderState->LineProjs = PushArray(&DebugState->Arena, m4, RenderState->LineMaxNumProjs);
            RenderState->LineNumVerts = PushArray(&DebugState->Arena, u32, RenderState->LineMaxNumProjs);
            ZeroMem(RenderState->LineNumVerts, sizeof(u32)*RenderState->LineMaxNumProjs);

            // NOTE: Create debug line vertex buffers
            RenderState->MaxNumLineVerts = 40000;
            RenderState->CurrLineVert = 0;
            RenderState->LinePosArray = PushArray(&DebugState->Arena, v3, RenderState->MaxNumLineVerts);
            RenderState->LineColorArray = PushArray(&DebugState->Arena, v3, RenderState->MaxNumLineVerts);
        }
    }
        
    // NOTE: Reserve memory for frames
    DebugState->FrameArray = PushArray(&DebugState->Arena, debug_frame, DEBUG_MAX_NUM_FRAMES);
    for (u32 FrameIndex = 0; FrameIndex < DEBUG_MAX_NUM_FRAMES; FrameIndex++)
    {
        debug_frame* CurrFrame = DebugState->FrameArray + FrameIndex;
        DebugFrameInit(CurrFrame);
    }
}

internal void DebugProcessData()
{
    if (DebugState->PauseCpuProfiling)
    {
        DebugState->CurrTimeStampId = 0;
        return;
    }
    
    // NOTE: Get the current frame
    debug_frame* CurrFrame = DebugState->FrameArray + DebugState->CurrFrameId;
    CurrFrame->FrameIndex = DebugState->CurrFrameId;
    CurrFrame->LibId = DebugState->CurrLibId - DebugState->DidRecompile;
    DebugState->DidRecompile = false;

#if DEBUG_PROFILE_DATA    
    // NOTE: Swap the pointers
    {
        debug_cpu_timestamp* Temp = DebugState->CpuTimeStamps;
        DebugState->CpuTimeStamps = DebugState->PrevCpuTimeStamps;
        DebugState->PrevCpuTimeStamps = Temp;
    }
    
    u32 NumRecords = DebugState->CurrTimeStampId;
    DebugState->CurrTimeStampId = 0;

    temp_mem TempMem = BeginTempMem(&DebugState->Arena);
    debug_open_block_stack OpenBlockStack = DebugOpenBlockStackInit(&DebugState->Arena, 20);
    
    for (u32 EventIndex = 0; EventIndex < NumRecords; ++EventIndex)
    {
        debug_cpu_timestamp* DebugCpuTimeStamp = DebugState->PrevCpuTimeStamps + EventIndex;

        switch (DebugCpuTimeStamp->Type)
        {
            case DebugTimeStamp_BeginBlock:
            {
                debug_time_block* Parent = 0;
                if (OpenBlockStack.CurrNumOpenBlocks != 0)
                {
                    Parent = DebugOpenBlockStackGetTop(&OpenBlockStack)->TimeBlock;
                }
                else if (BlockListIsEmpty(&CurrFrame->TimeBlockSentinel))
                {
                    Parent = BlockListAddEntry(&DebugState->BlockArena, &CurrFrame->TimeBlockSentinel, debug_time_block);
                    *Parent = {};
                }
                else
                {
                    Parent = (debug_time_block*)CurrFrame->TimeBlockSentinel.Next->Data;
                }
                
                debug_time_block* Block = BlockListAddEntry(&DebugState->BlockArena, &CurrFrame->TimeBlockSentinel, debug_time_block);
                *Block = {};
                Block->StartCycle = DebugCpuTimeStamp->Cycle;
                Block->ThreadId = DebugCpuTimeStamp->ThreadId;
                Block->Name = DebugCpuTimeStamp->Name;
                Block->GUID = DebugCpuTimeStamp->GUID;

                Block->Next = Parent->Child;
                Parent->Child = Block;

                debug_open_block* OpenBlock = DebugOpenBlockAlloc(&OpenBlockStack);
                OpenBlock->TimeBlock = Block;
            } break;
            
            case DebugTimeStamp_EndBlock:
            {
                if (OpenBlockStack.CurrNumOpenBlocks != 0)
                {
                    // NOTE: Add to our profile bars data
                    debug_time_block* CurrBlock = DebugOpenBlockStackGetTop(&OpenBlockStack)->TimeBlock;
                    CurrBlock->NumCycles = DebugCpuTimeStamp->Cycle - CurrBlock->StartCycle;

                    // NOTE: Add to our top clock list
                    debug_function_info* FunctionInfo = DebugFunctionInfoGetOrCreate(CurrFrame, CurrBlock->GUID);
                    FunctionInfo->HitCount += 1;
                    FunctionInfo->CycleCount += CurrBlock->NumCycles;
                    
                    DebugOpenBlockDealloc(&OpenBlockStack);
                }
                else
                {
                    // TODO: We sometime run into this case, shouldn't it be illegal?
                    //InvalidCodePath;
                }
            } break;

            case DebugTimeStamp_FrameTime:
            {
                CurrFrame->SecondsElapsed = DebugCpuTimeStamp->SecondsElapsed;
            } break;
            
            case DebugTimeStamp_FrameMarker:
            {
                CurrFrame->EndCycle = DebugCpuTimeStamp->Cycle;

                DebugState->CurrDisplayFrameId = DebugState->CurrFrameId;
                DebugState->CurrFrameId = DebugGetNextFrameId(DebugState->CurrFrameId);
                
                CurrFrame = DebugState->FrameArray + DebugState->CurrFrameId;
                CurrFrame->FrameIndex = DebugState->CurrFrameId;

                // NOTE: Clear out all profiling memory used by this frame
                BlockListClear(&DebugState->BlockArena, &CurrFrame->TimeBlockSentinel);
                BlockListClear(&DebugState->BlockArena, &CurrFrame->FunctionInfoSentinel);
                DebugFrameInit(CurrFrame);
                CurrFrame->BeginCycle = DebugCpuTimeStamp->Cycle;
            } break;
        }
    }

    EndTempMem(TempMem);
#endif

#if DEBUG_MEMORY_PROFILING

    // IMPORTANT: We start a new frame when we get a debug time stmap frame marker technically. But we don't treat memory in the same
    // way and so for now, we are clearing here. This might leak data from prev frame but its not a huge deal atm.
    // NOTE: Clear out debug frames previous memory profiling data
    if (!(BlockListIsEmpty(&CurrFrame->ArenaInfoSentinel)))
    {
        BlockListClear(&DebugState->BlockArena, &CurrFrame->ArenaInfoSentinel);
    }
    BlockListInitSentinel(&CurrFrame->ArenaInfoSentinel);

    // NOTE: Save current frame data to debug frame and clear out our recorded data
    {
        block_list_block* SourceBlock = DebugState->ArenaInfoSentinel.Next;
        while (SourceBlock != &DebugState->ArenaInfoSentinel)
        {
            block_list_block* DestBlock = BlockListAddBlock(&DebugState->BlockArena, &CurrFrame->ArenaInfoSentinel);
            Copy(SourceBlock->Data, DestBlock->Data, DEBUG_BLOCK_SIZE - sizeof(block_list_block));

            block_list_block* SavedSourceBlock = SourceBlock;
            SourceBlock = SourceBlock->Next;
            BlockListFreeBlock(&DebugState->BlockArena, SavedSourceBlock);
        }
    }
    
#endif
}

//
// NOTE: Debug UI
//

inline v4 DebugGetColor(u32 Index)
{
    v4 Colors[] =
    {
        V4(1, 0, 0, 1),
        V4(0, 1, 0, 1),
        V4(0, 0, 1, 1),
        V4(0.5f, 0.5f, 0.0f, 1.0f),
        V4(0.5f, 0.0f, 0.5f, 1.0f),
        V4(0.0f, 0.5f, 0.5f, 1.0f),
        V4(0.5f, 0.5f, 1.0f, 1.0f),
    };

    v4 Result = Colors[Index % ArrayCount(Colors)];
    return Result;
}

inline v2i DebugConvertToPixels(v2 Pos)
{
    // TODO: Eventually make the debug code store in 0-1 range not -1-1 range
    v2i Result = {};

    Pos = 0.5f*(Pos + V2(1, 1));
    v2 ScreenInPixels = V2(DebugState->RenderState.RenderState->Settings.Width,
                           DebugState->RenderState.RenderState->Settings.Height);
    Result = V2i(Pos*ScreenInPixels);

    return Result;
}

inline aabb2 DebugConvertToPixels(aabb2 Bounds)
{
    // TODO: Eventually make the debug code store in 0-1 range not -1-1 range
    aabb2 Result = {};

    Bounds.Min = 0.5f*(Bounds.Min + V2(1, 1));
    Bounds.Max = 0.5f*(Bounds.Max + V2(1, 1));

    v2 ScreenInPixels = V2(DebugState->RenderState.RenderState->Settings.Width,
                           DebugState->RenderState.RenderState->Settings.Height);
    Result.Min = Bounds.Min*ScreenInPixels;
    Result.Max = Bounds.Max*ScreenInPixels;

    return Result;
}

// NOTE: Use this so that perfectly placed UI can't have the mouse hit two elements at a time
inline b32 DebugIntersect(aabb2 A, v2 B)
{
    return (A.Min.x <= B.x && A.Max.x > B.x &&
            A.Min.y <= B.y && A.Max.y > B.y);
}

inline void DebugProfileBarInteraction(input_state* InputState, aabb2 Bounds, debug_time_block* Block)
{
    input_frame* CurrInput = &InputState->CurrInput;
    input_pointer* MainPointer = &CurrInput->MainPointer;

    if (InputIsMainPointerOnly(CurrInput) && DebugIntersect(Bounds, MainPointer->PixelPos))
    {
        interaction Interaction = {};
        Interaction.Type = Interaction_DebugProfileBar;
        Interaction.Ref.Id = u64(Block);
        Interaction.DebugProfileBar.Block = Block;
        InputAddInteraction(InputState, Interaction);
    }
}

inline void DebugFrameBarInteraction(input_state* InputState, aabb2 Bounds, u16 FrameId)
{
    input_frame* CurrInput = &InputState->CurrInput;
    input_pointer* MainPointer = &CurrInput->MainPointer;
    
    if (InputIsMainPointerOnly(CurrInput) && DebugIntersect(Bounds, MainPointer->PixelPos))
    {
        interaction Interaction = {};
        Interaction.Type = Interaction_DebugFrameBar;
        Interaction.Ref.Id = u64(FrameId);
        Interaction.DebugFrameBar.FrameId = FrameId;
        InputAddInteraction(InputState, Interaction);
    }
}

internal void DebugDrawFrameSlider(input_state* InputState, ui_state* UiState, aabb2 Bounds)
{
    f32 TotalX = Bounds.Max.x - Bounds.Min.x;
    f32 TotalY = Bounds.Max.y - Bounds.Min.y;
    
    f32 CurrX = Bounds.Min.x;
    f32 CurrY = Bounds.Min.y;
    f32 StepX = TotalX / (f32)DEBUG_MAX_NUM_FRAMES;
    // TODO: Top bar disappears when ThickY = 0.1f
    f32 ThickY = TotalY;

    for (u32 FrameId = 0; FrameId < DEBUG_MAX_NUM_FRAMES; ++FrameId)
    {
        aabb2 Block = AabbMinMax(V2(CurrX, CurrY), V2(CurrX + StepX, CurrY + ThickY));
        DebugFrameBarInteraction(InputState, DebugConvertToPixels(Block), u16(FrameId));

        b32 Highlighted = false;
        v4 Color = V4(1, 1, 1, 1);
        if (FrameId == DebugState->CurrFrameId)
        {
            Highlighted = true;
            Color = V4(1, 0, 0, 1);
        }
        else if (FrameId == DebugState->CurrDisplayFrameId)
        {
            Highlighted = true;
            Color = V4(0, 1, 0, 1);
        }

        if (Highlighted)
        {
            UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(Block), Color);
        }
        UiRectOutline(UiState, DebugConvertToPixels(Block), V4(1, 1, 1, 1), 2);

        CurrX += StepX;
    }
}

internal void DebugDrawFrameTimes(game_state* GameState)
{
    TIMED_FUNC();

    input_state* InputState = &GameState->InputState;
    ui_state* UiState = &GameState->UiState;

    aabb2 Bounds = AabbMinMax(V2(-0.9f, -0.9f), V2(0.9f, 0.9f));
    f32 PercentBoundsX = (Bounds.Max.x - Bounds.Min.x) / 100.0f;
    f32 PercentBoundsY = (Bounds.Max.y - Bounds.Min.y) / 100.0f;
    
    f32 OffsetX = 2.0f * PercentBoundsX;
    f32 OffsetY = 2.0f * PercentBoundsY;
    f32 FrameSliderMinY = 8.0f * PercentBoundsY;
    
    aabb2 FrameSliderBounds = AabbMinMax(V2(Bounds.Min.x + OffsetX, Bounds.Max.y - FrameSliderMinY),
                                         Bounds.Max - V2(OffsetX, OffsetY));
    UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(Bounds), V4(0.0f, 0.0f, 0.0f, 0.8f));
    DebugDrawFrameSlider(InputState, UiState, FrameSliderBounds);

    aabb2 DataBounds = AabbMinMax(Bounds.Min, Bounds.Max - V2(0.0f, AabbGetDim(FrameSliderBounds).y + 0.05f));
    UiSetTexturedRectClipRect(UiState, Aabbi(DebugConvertToPixels(DataBounds)));

    f32 FrameTimeFactor = 10.0f;
    f32 CurrX = DataBounds.Min.x;
    f32 BarWidth = AabbGetDim(DataBounds).x / f32(DEBUG_MAX_NUM_FRAMES);
    f32 MinY = DataBounds.Min.y;
    f32 MaxY = DataBounds.Max.y;
    for (u32 FrameId = 0; FrameId < DEBUG_MAX_NUM_FRAMES; ++FrameId)
    {
        debug_frame* Frame = DebugState->FrameArray + FrameId;

        f32 T = Min(1.0f, Max(0.0f, (f32(Frame->SecondsElapsed) / (1.0f / 60.0f)) - 1.0f));
        
        f32 Red = Lerp(0.0f, 1.0f, T);
        f32 Green = Lerp(1.0f, 0.0f, T);
        
        aabb2 BarBounds = AabbMinMax(V2(CurrX, MinY), V2(CurrX + BarWidth, Lerp(MinY, MaxY, f32(Frame->SecondsElapsed) *FrameTimeFactor)));
        UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(BarBounds), V4(Red, Green, 0.0f, 1.0f));
        UiRectOutline(UiState, DebugConvertToPixels(BarBounds), V4(0, 0, 0, 1), 2);
        CurrX += BarWidth;
    }

    UiResetTexturedRectClipRect(UiState);
}

internal void DebugDrawProfileBars(game_state* GameState)
{
    TIMED_FUNC();

    input_state* InputState = &GameState->InputState;
    ui_state* UiState = &GameState->UiState;

    aabb2 Bounds = AabbMinMax(V2(-0.9f, -0.9f), V2(0.9f, 0.9f));
    f32 PercentBoundsX = (Bounds.Max.x - Bounds.Min.x) / 100.0f;
    f32 PercentBoundsY = (Bounds.Max.y - Bounds.Min.y) / 100.0f;
    
    f32 OffsetX = 2.0f * PercentBoundsX;
    f32 OffsetY = 2.0f * PercentBoundsY;
    f32 FrameSliderMinY = 8.0f * PercentBoundsY;
    
    aabb2 FrameSliderBounds = AabbMinMax(V2(Bounds.Min.x + OffsetX, Bounds.Max.y - FrameSliderMinY),
                                         Bounds.Max - V2(OffsetX, OffsetY));
    f32 BarMaxY = Bounds.Max.y - 10.0f * PercentBoundsY;
    f32 BarMinY = Bounds.Max.y - 96.0f * PercentBoundsY;
    f32 BarHeight = (BarMaxY - BarMinY) / 10.0f; // NOTE: Divisor is # of descents we can display
    f32 BarStartX = Bounds.Min.x + 2.0f * PercentBoundsX;
    f32 BarScaleX = 96.0f * PercentBoundsX;

    UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(Bounds), V4(0.0f, 0.0f, 0.0f, 0.8f));
    DebugDrawFrameSlider(InputState, UiState, FrameSliderBounds);
    
    u32 ViewFrameIndex = DebugState->CurrDisplayFrameId;
    debug_frame* Frame = DebugState->FrameArray + ViewFrameIndex;
    if (BlockListIsEmpty(&Frame->TimeBlockSentinel))
    {
        return;
    }
        
    temp_mem TempMem = BeginTempMem(&DebugState->Arena);

    u64 BeginFrameCycle = Frame->BeginCycle;
    u64 EndFrameCycle = Frame->EndCycle;
    f64 CyclesForFrame = (f64)(EndFrameCycle - BeginFrameCycle);

    debug_open_block_stack OpenBlockStack = DebugOpenBlockStackInit(&DebugState->Arena, 10);
    
    u32 ColorIndex = 0;
    f32 PosY = BarMaxY - BarHeight;
    debug_time_block* Root = (debug_time_block*)Frame->TimeBlockSentinel.Next->Data + 0;
    debug_time_block* CurrBlock = Root->Child;
    while (CurrBlock)
    {
        f64 StartCycle = (f64)(CurrBlock->StartCycle - BeginFrameCycle);
        f64 EndCycle = 0.0f;
        if (CurrBlock->NumCycles != 0)
        {
            EndCycle = (f64)(StartCycle + CurrBlock->NumCycles);
        }
        else
        {
            EndCycle = (f64)(CyclesForFrame);
        }
        
        f32 StartPos = (BarScaleX*(f32)(StartCycle / CyclesForFrame)) + BarStartX;
        f32 EndPos = (BarScaleX*(f32)(EndCycle / CyclesForFrame)) + BarStartX;

        aabb2 Block = AabbMinMax(V2(StartPos, PosY), V2(EndPos, PosY + BarHeight));

        DebugProfileBarInteraction(InputState, DebugConvertToPixels(Block), CurrBlock);

        // NOTE: This is so that we don't unboundedly save call aabb timing events
        DebugRecordingPause();
        UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(Block), DebugGetColor(ColorIndex++));
        DebugRecordingUnpause();

        if (CurrBlock->Child)
        {
            // NOTE: Descend a level
            debug_open_block* OpenBlock = DebugOpenBlockAlloc(&OpenBlockStack);
            OpenBlock->TimeBlock = CurrBlock;
            CurrBlock = CurrBlock->Child;
            PosY -= BarHeight;
        }
        else if (!CurrBlock->Next)
        {
            // NOTE: Go up a level
            while (!CurrBlock->Next && OpenBlockStack.CurrNumOpenBlocks != 0)
            {
                CurrBlock = DebugOpenBlockStackGetTop(&OpenBlockStack)->TimeBlock;
                DebugOpenBlockDealloc(&OpenBlockStack);
                PosY += BarHeight;
            }
            
            CurrBlock = CurrBlock->Next;
        }
        else
        {
            // NOTE: Move to the sibling
            CurrBlock = CurrBlock->Next;
        }
    }
    
    {
        // NOTE: Draw the frame barrier
        f64 CyclesPerSecond = CyclesForFrame / Frame->SecondsElapsed;
        f64 SecondsPerFrame = 1.0 / 60.0;
        f32 FrameBarrier = BarScaleX*(f32)((CyclesPerSecond*SecondsPerFrame)/CyclesForFrame) + BarStartX;

        // TODO: Does this get handled correctly?
        if (Frame->SecondsElapsed > SecondsPerFrame)
        {
            //InvalidCodePath;
        }

        UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(AabbMinMax(V2(FrameBarrier, BarMinY), V2(FrameBarrier + 0.01f, BarMaxY))), V4(0, 0, 0, 1));
    }    
        
    EndTempMem(TempMem);
}

internal void DebugDrawMemoryArenas(game_state* GameState)
{
#if 0
    // TODO: Because our strings are stored in tables, we can't compile this into the game at runtime
    TIMED_FUNC();

    input_state* InputState = &GameState->InputState;
    ui_state* UiState = &GameState->UiState;

    aabb2 Bounds = AabbMinMax(V2(-0.9f, -0.9f), V2(0.9f, 0.9f));
    f32 PercentBoundsX = (Bounds.Max.x - Bounds.Min.x) / 100.0f;
    f32 PercentBoundsY = (Bounds.Max.y - Bounds.Min.y) / 100.0f;
    
    f32 OffsetX = 2.0f * PercentBoundsX;
    f32 OffsetY = 2.0f * PercentBoundsY;
    f32 FrameSliderMinY = 8.0f * PercentBoundsY;
    
    aabb2 FrameSliderBounds = AabbMinMax(V2(Bounds.Min.x + OffsetX, Bounds.Max.y - FrameSliderMinY),
                                         Bounds.Max - V2(OffsetX, OffsetY));
    f32 BarMaxY = Bounds.Max.y - 10.0f * PercentBoundsY;
    f32 BarMinY = Bounds.Max.y - 96.0f * PercentBoundsY;
    f32 BarHeight = (BarMaxY - BarMinY) / 5.0f; // NOTE: Divisor is # of descents we can display
    f32 BarStartX = Bounds.Min.x + 2.0f * PercentBoundsX;
    f32 BarScaleX = 96.0f * PercentBoundsX;

    UiRect(UiState, UiConstraint_None, 0, DebugConvertToPixels(Bounds), V4(0.0f, 0.0f, 0.0f, 0.8f));
    DebugDrawFrameSlider(InputState, UiState, FrameSliderBounds);
    
    u16 ViewFrameIndex = DebugState->CurrDisplayFrameId;
    debug_frame* Frame = DebugState->FrameArray + ViewFrameIndex;
    if (!Frame->Root)
    {
        return;
    }

    // NOTE: Draw memory arenas
    v2i MenuTopLeft;
    i32 MenuOptionDimX;
    UiScrollMenuBegin(&MenuTopLeft, &MenuOptionDimX);

    i32 TextDimY = 10;
    i32 MemoryDimY = 40;
    i32 ArenaSectionDimY = TextDimY + MemoryDimY;
    aabb2i ArenaSectionBounds = AabbiMinMax(V2i(MenuTopLeft.x, MenuTopLeft.y - ArenaSectionDimY),
                                            V2i(MenuTopLeft.x + MenuOptionDimX, MenuTopLeft.y));

    // NOTE: Calculate our normalization value for arenas so that they fit in their section
    u32 MaxArenaSize = 0;
    for (u32 ArenaId = 0; ArenaId < Frame->NumArenas; ++FrameId)
    {
        debug_arena_metadata* ArenaMetaData = Frame->ArenaMetaDatas + ArenaId;
        MaxArenaSize = Max(MaxArenaSize, ArenaMetaData->Size);
    }    
    
    for (u32 ArenaId = 0; ArenaId < Frame->NumArenas; ++FrameId)
    {
        debug_arena_metadata* ArenaMetaData = Frame->ArenaMetaDatas + ArenaId;

        // NOTE: Save top section for arena name text
        aabb2i TextBounds = AabbiMinMax(ArenaSectionBounds.Min + V2i(0, MemoryDimY), ArenaSectionBounds.Max);
        UiText(Enlarge(TextBounds, V2i(-1, -1)), ArenaMetaData->Name);

        // NOTE: Draw the memory usage for the arena
        aabb2i ArenaBounds = AabbiMinMax(ArenaSectionBounds.Min, ArenaSectionBounds.Max - V2i(0, TextBounds));
        ArenaBounds = Enlarge(ArenaBounds, V2i(-1, -1));

        i32 ArenaBoundsDimX = AabbGetDim(ArenaBounds).x;

        i32 UsedX = i32(f32(ArenaMetaData->Used) / f32(MaxArenaSize));
        i32 SizeX = i32(f32(ArenaMetaData->Size) / f32(MaxArenaSize));

        aabb2i UsedBounds = Aabbi(ArenaBounds.Min, V2i(ArenaBounds.Min.x + UsedX, ArenaBounds.Max.y));
        aabb2i SizeBounds = Aabbi(ArenaBounds.Min, V2i(ArenaBounds.Min.x + SizeX, ArenaBounds.Max.y));

        UiRect(SizeBounds);
        UiRect(UsedBounds);
    }
    
    UiScrollMenuEnd();
#endif
}

internal void DebugDrawTopClockList(game_state* GameState)
{
    TIMED_FUNC();
    
    input_state* InputState = &GameState->InputState;
    ui_state* UiState = &GameState->UiState;
    
    aabb2 Bounds = AabbMinMax(V2(-0.9f, -0.9f), V2(0.9f, 0.9f));
    f32 PercentBoundsX = (Bounds.Max.x - Bounds.Min.x) / 100.0f;
    f32 PercentBoundsY = (Bounds.Max.y - Bounds.Min.y) / 100.0f;
    
    f32 OffsetX = 2.0f * PercentBoundsX;
    f32 OffsetY = 2.0f * PercentBoundsY;
    f32 FrameSliderMinY = 8.0f * PercentBoundsY;
    aabb2 FrameSliderBounds = AabbMinMax(V2(Bounds.Min.x + OffsetX, Bounds.Max.y - FrameSliderMinY),
                                         Bounds.Max - V2(OffsetX, OffsetY));
    DebugDrawFrameSlider(InputState, UiState, FrameSliderBounds);

    // NOTE: Move bounds to be below the slider
    Bounds.Max.y -= AabbGetDim(FrameSliderBounds).y + 0.05f;
    
    debug_frame* Frame = DebugState->FrameArray + DebugState->CurrDisplayFrameId;

    // NOTE: Only give records if we haven't recompiled since
    temp_mem TempMem = BeginTempMem(&DebugState->Arena);
    u32 TotalNumFunctionInfos = 0;
    char** Options = 0;
    if (Frame->LibId == DebugState->CurrLibId)
    {
        // NOTE: Do a initial pass to figure out how many records we have, and save them for sorting
        {
            block_list_block* CurrBlock = Frame->FunctionInfoSentinel.Next;
            while (CurrBlock != &Frame->FunctionInfoSentinel)
            {
                TotalNumFunctionInfos += CurrBlock->NumEntries;
                CurrBlock = CurrBlock->Next;
            }
        }

        // NOTE: Write out the options table in sorted order
        Options = PushArray(&DebugState->Arena, char*, TotalNumFunctionInfos);
        sort_key* SortKeys = PushArray(&DebugState->Arena, sort_key, TotalNumFunctionInfos);
        
        u32 TempStrSize = 400;
        u32 GlobalFunctionInfoId = 0;
        {
            block_list_block* CurrBlock = Frame->FunctionInfoSentinel.Next;
            while (CurrBlock != &Frame->FunctionInfoSentinel)
            {
                for (u32 FunctionInfoId = 0; FunctionInfoId < CurrBlock->NumEntries; ++FunctionInfoId)
                {
                    debug_function_info* FunctionInfo = (debug_function_info*)CurrBlock->Data + FunctionInfoId;
                    if (FunctionInfo->GUID)
                    {
                        Options[GlobalFunctionInfoId] = PushArray(&DebugState->Arena, char, TempStrSize);

                        DebugSnprintf(Options[GlobalFunctionInfoId], TempStrSize, "%s HitCount: %llu CycleCount:  %llu", FunctionInfo->GUID,
                                      FunctionInfo->HitCount, FunctionInfo->CycleCount);

                        Assert(u64(u32(FunctionInfo->CycleCount)) == FunctionInfo->CycleCount);
                        SortKeys[GlobalFunctionInfoId].Key = U32_MAX - u32(FunctionInfo->CycleCount);
                        SortKeys[GlobalFunctionInfoId].Id = GlobalFunctionInfoId;
                        ++GlobalFunctionInfoId;
                    }
                }

                CurrBlock = CurrBlock->Next;
            }
        }

        SortRadix(&DebugState->Arena, SortKeys, TotalNumFunctionInfos);
        // TODO: Figure out how we want to actually retraverse the data, do we want the sort function to do the copying?
        char** TempOptions = PushArray(&DebugState->Arena, char*, TotalNumFunctionInfos);
        for (u32 OptionId = 0; OptionId < TotalNumFunctionInfos; ++OptionId)
        {
            TempOptions[OptionId] = Options[SortKeys[OptionId].Id];
        }
        Options = TempOptions;
    }
    
    UiScrollMenu(UiState, UiConstraint_None, Font_General, DebugConvertToPixels(Bounds), 40, 0, 30, TotalNumFunctionInfos, Options);
    EndTempMem(TempMem);
}

inline void DebugDrawToolTip(ui_state* UiState, v2 Pos, f32 CharHeight, char* Text)
{
    Pos += V2(0.0f, CharHeight);
    aabb2 TextBounds = UiGetTextBounds(UiState, UiConstraint_None, Font_General, CharHeight, Pos, 0, Text);

    UiRect(UiState, UiConstraint_None, 0, Enlarge(TextBounds, V2(2, 2)), V4(0.0f, 0.1f, 1.0f, 1.0f));
    UiText(UiState, UiConstraint_None, Font_General, CharHeight, Pos, 0, Text);
}

INPUT_INTERACTION_HANDLER(DebugHandleInteraction)
{
    b32 Result = false;

    if (!(InputState->Hot.Type == Interaction_DebugFrameBar ||
          InputState->Hot.Type == Interaction_DebugProfileBar))
    {
        return Result;
    }

    ui_state* UiState = &GameState->UiState;
    
    input_pointer* MainPointer = &InputState->CurrInput.MainPointer;
    switch (InputState->Hot.Type)
    {
        case Interaction_DebugFrameBar:
        {
            debug_frame_bar_interaction* FrameBarInteraction = &InputState->Hot.DebugFrameBar;

            char TextBuffer[1024];
            DebugSnprintf(TextBuffer, sizeof(TextBuffer), "%d", FrameBarInteraction->FrameId);
            DebugDrawToolTip(UiState, MainPointer->PixelPos, 20, TextBuffer);

            if (MainPointer->ButtonFlags & MouseButtonFlag_PressedOrHeld)
            {
                InputState->PrevHot = InputState->Hot;
            }
            
            if (MainPointer->ButtonFlags & MouseButtonFlag_Released)
            {
                DebugState->PauseCpuProfiling = true;
                if (FrameBarInteraction->FrameId == DebugGetPrevFrameId(DebugState->CurrFrameId))
                {
                    DebugState->PauseCpuProfiling = false;
                }
                DebugState->CurrDisplayFrameId = FrameBarInteraction->FrameId;
            }

            Result = true;
        } break;

        case Interaction_DebugProfileBar:
        {
            debug_profile_bar_interaction* ProfileBarInteraction = &InputState->Hot.DebugProfileBar;
            DebugDrawToolTip(UiState, MainPointer->PixelPos, 20, ProfileBarInteraction->Block->GUID);
            
            if (MainPointer->ButtonFlags & MouseButtonFlag_PressedOrHeld)
            {
                InputState->PrevHot = InputState->Hot;
            }

            Result = true;
        } break;

        default:
        {
            InvalidCodePath;
        } break;
    }

    return Result;
}

internal void DebugDrawOverlay(game_state* GameState)
{
    ui_state* UiState = &GameState->UiState;

    switch (DebugState->UiOverlayState)
    {
        case DebugOverlay_None:
        {
        } break;

        case DebugOverlay_FrameTimes:
        {
            DebugDrawFrameTimes(GameState);
        } break;
        
        case DebugOverlay_ProfileGraph:
        {
            DebugDrawProfileBars(GameState);
        } break;

        case DebugOverlay_TopClockList:
        {
            DebugDrawTopClockList(GameState);
        } break;
    }
    
    char* Options[] =
        {
            "None",
            "Frame Times",
            "Profile Graph",
            "Top Clock List",
        };
    UiDropDown(UiState, UiConstraint_TopEdge, Font_General, AabbMinMax(V2(400, -20), V2(650, 2)), 2, 30, ArrayCount(Options),
               ArrayCount(Options), Options, (u32*)&DebugState->UiOverlayState);

#if 0
#if DEBUG_DRAW_MOUSE_BOUNDS
    debug_frame* Frame = DebugState->FrameArray + DebugState->CurrDisplayFrameId;
    f32 CharScale = 0.0006f;
    asset_font* Font = GetFont(&GameState->Assets, Font_General);
    char FpsText[256];
    Snprintf(FpsText, sizeof(FpsText), "Frame Ms: %.2f", Frame->SecondsElapsed*1000.0f);
    PushText(GameState, Font, V2(-1.0f, -0.8f - CharScale*Font->MaxAscent), Layer_UI,
             CharScale, FpsText, V4(1, 1, 1, 1));
#endif
#endif
}

//==========================================================================================================================================
//==========================================================================================================================================
// NOTE: Debug visualizations
//==========================================================================================================================================
//==========================================================================================================================================

// NOTE: Ui Visualization
#if 0
{
            // NOTE: FlipY, AspectCorrect, Text, TintColor
        UiButton(UiState, UiConstraint_None, Aabb2iMinMax(V2i(0, 0), V2i(100, 100)), 0,
                 Texture_UiIconHolder, "Test");
        UiButton(UiState, UiConstraint_None, Aabb2iMinMax(V2i(100, 100), V2i(200, 200)), 0,
                 Texture_UiIconHolder, Font_General, 20, 3, "TestTestTestTestTestTestTestTest");
        UiButton(UiState, V2(0.4f, 0.4f), 0, Texture_UiIconHolder, "TestTestT\nTest\nTest\nTest\nTest");
        
        /*
          ui_panel Panel = UiBeginPanel(UiState, V2(0.2f, 0.7f), 0.1f, 0.4f);

          UiEndPanel(&Panel);
        */
        
        /*
          local_global b32 IsInit = true;
          if (IsInit)
          {
          IsInit = false;
          UiFadeImage(UiState, AabbCenterRadius(V2(0.5f, 0.5f), V2(0.2f, 0.2f)), Texture_White,
          V4(1, 1, 1, 1));
          }
        */
        
        /*
          local_global f32 Percent = 0.0f;
          UiHorizontalSlider(UiState, UiConstraint_Center, Aabb2iCenterRadius(V2i(0, 0), V2i(100, 20)), V2i(20, 20), &Percent);
        */

        /*
          local_global f32 Percent = 0.0f;
          UiVerticalSlider(UiState, UiConstraint_Center, Aabb2iCenterRadius(V2i(0, 0), V2i(20, 100)), V2i(20, 20), &Percent);
        */
        
        /*
        {
            char* Options[] =
                {
                    "Option1",
                    "Option2",
                    "Option3",
                    "Option4",
                    "Option5",
                    "Option6",
                    "Option7",
                };
            UiScrollMenu(UiState, UiConstraint_None, Font_General, AabbiCenterRadius(V2i(200, 200), V2i(100, 50)), 30, 2, 30,
                         ArrayCount(Options), Options);
        }
        */
        
        /*
          UiRenderImage(UiState, AabbCenterRadius(V2(0.4f, 0.4f), V2(0.3f, 0.3f)), Texture_White);
          UiRenderImage(UiState, AabbCenterRadius(V2(0.5f, 0.5f), V2(0.3f, 0.3f)), Texture_White, V4(1, 0, 1, 1));
        */

        /*
        {
            char* Options[] =
                {
                    "Option1",
                    "Option2",
                    "Option3",
                    "Option4",
                    "Option5",
                    "Option6",
                    "Option7",
                };
            local_global u32 ChosenOption = 0;
            UiDropDown(UiState, UiConstraint_RightEdge, Font_General, V2i(-500, 500), 2, 200, 20, 30, ArrayCount(Options), 3, Options, &ChosenOption);
        }
        //*/

        /*
        UiText(UiState, Font_General, 50, V2i(10, 200), 100, "TESTING", V4(1, 0, 0, 1));
        */
}
#endif

// NOTE: Monstar motion Visualization
#if 0
    {
        /*
            NOTE: For adding this to monstars movement, we need the monstar to know when its in the start circle, line in between,
                  and end circle. We need to know what the departure point is. Once the monstar hits the departure point, we can move
                  straight until we are 2*radius from our second target position which is were we are trying to get to. Radius is given
                  by the monstars current parameters so we can calculate it. We need target angle to calculate if we hit the second circle
                  but then we are doing sin/cos calls every movement step to know where we are. 

                  Can we somehow calculate the departure point without storing extra info? I don't think we can for cases where our
                  circles cross so probably not.

                  Other option is to just store all 3 points (departure, re entry, final target). We don't need the target angle then..
                  I think I'll just do this for now and see how it works. We then need to have a id which says which target point we are
                  currently looking to get to.
         */
        
        m4 OrthoProj = VkOrthoProjM4(0, 1, 1, 0, 0.001, 1000);
        DebugPushLineProjection(OrthoProj);

        v2 StartPos = V2(0.5f, 0.5f);
        f32 StartAngle = 0.0f;
        v2 TargetPos = PlayState->CurrInput.ScreenPos; //V2(0.2f, 0.37f); 
        f32 TargetAngle = 1.0f;

        f32 Speed = 0.05f;
        f32 TurnRate = 1.0f;
        
        // NOTE: Draw angles
        {
            v2 StartAngleEnd = StartPos + 0.1f*V2(Cos(StartAngle), Sin(StartAngle));
            DebugPushLine(V3(StartPos, 1.0f), V3(StartAngleEnd, 1.0f), V3(0, 1, 1), V3(0, 1, 1));

            v2 TargetAngleEnd = TargetPos + 0.1f*V2(Cos(TargetAngle), Sin(TargetAngle));
            DebugPushLine(V3(TargetPos, 1.0f), V3(TargetAngleEnd, 1.0f), V3(0, 1, 1), V3(0, 1, 1));
        }

        /*
            NOTE: Math to get path angle (we are working with only the right circle)

              Radius = Speed / TurnRate
              StartCenterLeft = StartPos + Radius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
              StartCenterRight = StartPos + Radius*V2(Cos(StartAngle - Pi32/2.0f), Sin(StartAngle - Pi32/2.0f));
              EndCenterLeft = TargetPos + Radius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
              EndCenterRight = TargetPos + Radius*V2(Cos(TargetAngle - Pi32/2.0f), Sin(TargetAngle - Pi32/2.0f));

              StartOffset = Radius*V2(-Sin(PathAngle), Cos(PathAngle))
              ReEntry = StartCenter___ +/- StartOffset;
              Departure = EndCenter___ +/- StartOffset;
              
              We want to find the start offset which will give us the departure and re entry point for each circle.

              We have 2 cases, one where we go left -> left/right -> right and another where we go left -> right/right -> left

              Case 1: Left -> Left / Right -> Right

                We can construct a triangle where one side is the start offset vector, and another is any vector with a smaller angle (lets
                call it D). Then

                Angle(StartOffset) = Angle(D) + ArcCos(Radius / Length(D));

                Once we get this angle, we can get

                StartOffset = Radius*V2(Cos(Angle(StartOffset)), Sin(Angle(StartOffset)));

                Which gives us our departure and re entry points.
              
              Case 2: Left -> Right / Right -> Left

                In this case, we construct a triangle where one side is the start offset vector, and another is a the midpoint between
                both circle centers (from online, this seems to be the assumption made, that the departure -> reentry vector intersects it
                at the mid point.

                So, we get Angle(StartOffset) = Angle(EndCenter___ - StartCenter___) + ArcCos(Radius / DistanceToMidPoint);

                Once we get this angle, we can get

                StartOffset = Radius*V2(Cos(Angle(StartOffset)), Sin(Angle(StartOffset)));

                Which gives us our departure and re entry points.
         */

        f32 Radius = Speed / TurnRate;
        v2 StartCenterLeft = StartPos + Radius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
        v2 StartCenterRight = StartPos + Radius*V2(Cos(StartAngle - Pi32/2.0f), Sin(StartAngle - Pi32/2.0f));
        v2 EndCenterLeft = TargetPos + Radius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
        v2 EndCenterRight = TargetPos + Radius*V2(Cos(TargetAngle - Pi32/2.0f), Sin(TargetAngle - Pi32/2.0f));

        // NOTE: This is a sanity check that distance is actually linear here
        {
            f32 D0 = 0.0f;
            {
                f32 TestRadius = 0.2f;
                v2 Pos0 = StartPos + TestRadius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
                v2 Pos1 = TargetPos + TestRadius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
                D0 = Length(Pos1 - Pos0);
            }

            f32 D1 = 0.0f;
            {
                f32 TestRadius = 0.4f;
                v2 Pos0 = StartPos + TestRadius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
                v2 Pos1 = TargetPos + TestRadius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
                D1 = Length(Pos1 - Pos0);
            }

            f32 D2 = 0.0f;
            {
                f32 TestRadius = 0.6f;
                v2 Pos0 = StartPos + TestRadius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
                v2 Pos1 = TargetPos + TestRadius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
                D2 = Length(Pos1 - Pos0);
            }

            f32 D3 = 0.0f;
            {
                f32 TestRadius = 0.8f;
                v2 Pos0 = StartPos + TestRadius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
                v2 Pos1 = TargetPos + TestRadius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
                D3 = Length(Pos1 - Pos0);
            }

            f32 Slope0 = (D1 - D0) / 0.2f;
            f32 Slope1 = (D2 - D1) / 0.2f;
            f32 Slope2 = (D3 - D2) / 0.2f;
            int i = 0;
        }
        
#if 0
        // NOTE: This is a sanity check on our distance formula (fails currently)
        {
            f32 D0 = Length(EndCenterLeft - StartCenterLeft);
            f32 D1 = Length(TargetPos - StartPos);
            
            //f32 TestRadius = 0.4f;

#define TestFunc(TestRadius)                                            \
            {                                                           \
                v2 Pos0 = StartPos + TestRadius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f)); \
                v2 Pos1 = TargetPos + TestRadius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f)); \
                f32 ActualDistance = Length(Pos0 - Pos1);               \
                f32 CalculatedDistance = Lerp(D1, D0, TestRadius / Radius); \
                DebugPrintLog("Actual: %f, Expected: %f\n", ActualDistance, CalculatedDistance); \
            }

            TestFunc(0.2f);
            TestFunc(0.4f);
            TestFunc(0.6f);
            TestFunc(0.8f);
        }
#endif
        
        // NOTE: We use distance from circle centers as a heuristic to choose which circles to create paths from
        v2 CurrStartCenter = {};
        v2 CurrEndCenter = {};
        b32 Cross = false;
        f32 Multiplier = 1.0f;
        {
            f32 CurrDistanceSquared = F32_MAX;

            // NOTE: Check left to left
            {
                f32 CheckDistance = LengthSquared(StartCenterLeft - EndCenterLeft);
                if (CheckDistance <= CurrDistanceSquared)
                {
                    CurrStartCenter = StartCenterLeft;
                    CurrEndCenter = EndCenterLeft;
                    Multiplier = -1.0f;
                    Cross = false;

                    CurrDistanceSquared = CheckDistance;
                }
            }

#if 0
            // NOTE: Check left to right
            {
                f32 CheckDistance = LengthSquared(StartCenterLeft - EndCenterRight);
                if (CheckDistance <= CurrDistanceSquared)
                {
                    CurrStartCenter = StartCenterLeft;
                    CurrEndCenter = EndCenterRight;
                    Multiplier = -1.0f;
                    Cross = true;

                    CurrDistanceSquared = CheckDistance;
                }
            }

            // NOTE: Check right to left
            {
                f32 CheckDistance = LengthSquared(StartCenterRight - EndCenterLeft);
                if (CheckDistance <= CurrDistanceSquared)
                {
                    CurrStartCenter = StartCenterRight;
                    CurrEndCenter = EndCenterLeft;
                    Multiplier = 1.0f;
                    Cross = true;

                    CurrDistanceSquared = CheckDistance;
                }
            }

            // NOTE: Check right to right
            {
                f32 CheckDistance = LengthSquared(StartCenterRight - EndCenterRight);
                if (CheckDistance <= CurrDistanceSquared)
                {
                    CurrStartCenter = StartCenterRight;
                    CurrEndCenter = EndCenterRight;
                    Multiplier = 1.0f;
                    Cross = false;

                    CurrDistanceSquared = CheckDistance;
                }
            }
#endif
        }

        // NOTE: If our circles intersect, we make them smaller
        f32 D0 = LengthSquared(CurrEndCenter - CurrStartCenter);
        f32 MinDistanceSquared = Square(2.0f*Radius);
        if (D0 <= MinDistanceSquared)
        {
            /*
                NOTE: Problem statement is, if we notice that our circles intersect, we want to adjust radius such that they dont. The
                      issue is that if we change r, we also change the circle centers which can also influence the distance between the
                      circle centers (meaning a change in r might actually move us a bit farther than we want away from the circles).

                      Distance is not linear. I previously thought it was but it isn't! I can probably interpolate distance in each
                      component linearly, but not the distance itself. It seems to be almost linear, but not exactly.

                      D0 = CurrEndPos - CurrStartPos;
                      D1 = TargetPos - StartPos;
                      
                      DistanceX(NewRadius) = D1x + (NewRadius / OldRadius)*(D0x - D1x);
                      DistanceY(NewRadius) = D1y + (NewRadius / OldRadius)*(D0y - D1y);

                      We want 4.0f*(NewRadius^2) = DistanceX(NewRadius)^2 + DistanceY(NewRadius)^2 (we are working with dist^2)

                      4.0f*(NewRadius^2) = (D1x + (NewRadius / OldRadius)*(D0x - D1x))^2 +
                                           (D1y + (NewRadius / OldRadius)*(D0y - D1y))^2;
                      4.0f*(NewRadius^2) = (D1x^2 + ((NewRadius / OldRadius)^2)*(D0x - D1x))^2 +
                                           (D1y + (NewRadius / OldRadius)*(D0y - D1y))^2;
                                           
             */
#if 0
            D0 = SquareRoot(D0);
            f32 D1 = Length(TargetPos - StartPos);

            f32 NewRadius = D1 / (2.0f - (1.0f / Radius)*(D0 - D1));
            f32 NewSpeed = NewRadius * TurnRate;
            
            CurrStartCenter -= StartPos;
            CurrStartCenter *= (NewRadius / Radius);
            CurrStartCenter += StartPos;

            CurrEndCenter -= TargetPos;
            CurrEndCenter *= (NewRadius / Radius);
            CurrEndCenter += TargetPos;

            Assert(NewRadius <= Radius);

            f32 PrevRadius = Radius;
            Radius = NewRadius;
            Speed = NewSpeed;
            
            CurrStartCenter = StartPos + Radius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
            CurrEndCenter = TargetPos + Radius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
            
            f32 NewDist = Length(CurrEndCenter - CurrStartCenter);
            f32 ExpectedDist = Lerp(D1, D0, NewRadius / PrevRadius);
#endif
            //DebugPrintLog("NewDist: %f, ExpectedDist: %f\n", NewDist, ExpectedDist);
            
            //Assert(ExpectedDist - 0.001f <= NewDist &&
            //       NewDist <= ExpectedDist + 0.001f);
        }
        
        v2 Departure = {};
        v2 ReEntry = {};

        if (Cross)
        {
            v2 VecToCenters = CurrEndCenter - CurrStartCenter;

            f32 Angle = ArcCos(Radius / (Length(VecToCenters) * 0.5f));
            f32 MidPointAngle = ArcTan(VecToCenters.x, VecToCenters.y);

            v2 MidPoint = Lerp(CurrStartCenter, CurrEndCenter, 0.5f);
        
            Departure = CurrStartCenter + Radius*V2(Cos(MidPointAngle + Multiplier*Angle), Sin(MidPointAngle + Multiplier*Angle));
            v2 TangentToMidPoint = MidPoint - Departure;
            ReEntry = Departure + (2.0f * TangentToMidPoint);
        }
        else
        {
            v2 VecToCenters = CurrEndCenter - CurrStartCenter;

            f32 Angle = Multiplier*Pi32/2.0f;
            f32 MidPointAngle = ArcTan(VecToCenters.x, VecToCenters.y);

            v2 StartOffset = Radius*V2(Cos(MidPointAngle + Angle), Sin(MidPointAngle + Angle));
            v2 Tangent = GetPerp(StartOffset);
            Departure = CurrStartCenter + StartOffset;
            ReEntry = CurrEndCenter + StartOffset;

            f32 Temp = Dot(ReEntry - Departure, StartOffset);
            Assert(Temp <= 0.0001f);
        }

        // NOTE: Walk first circle
        
        // NOTE: Walk the path between circles

        // NOTE: Walk the second circle

        // NOTE: Draw theoretical path
        {
            u32 NumPoints = 128;

            // NOTE: Recalc since we might have changed radius
            StartCenterLeft = StartPos + Radius*V2(Cos(StartAngle + Pi32/2.0f), Sin(StartAngle + Pi32/2.0f));
            StartCenterRight = StartPos + Radius*V2(Cos(StartAngle - Pi32/2.0f), Sin(StartAngle - Pi32/2.0f));
            EndCenterLeft = TargetPos + Radius*V2(Cos(TargetAngle + Pi32/2.0f), Sin(TargetAngle + Pi32/2.0f));
            EndCenterRight = TargetPos + Radius*V2(Cos(TargetAngle - Pi32/2.0f), Sin(TargetAngle - Pi32/2.0f));
            
            // NOTE: Draw left circle
            for (u32 PointId = 1; PointId < NumPoints; ++PointId)
            {
                f32 CircleStartAngle = (2.0f*Pi32)*f32(PointId - 1) / f32(NumPoints);
                f32 CircleEndAngle = (2.0f*Pi32)*f32(PointId) / f32(NumPoints);

                v2 CircleStartPos = StartCenterLeft + Radius*V2(Cos(CircleStartAngle), Sin(CircleStartAngle));
                v2 CircleEndPos = StartCenterLeft + Radius*V2(Cos(CircleEndAngle), Sin(CircleEndAngle));

                DebugPushLine(V3(CircleStartPos, 1.0f), V3(CircleEndPos, 1.0f), V3(0, 1, 1), V3(0, 1, 1));
            }
            
            // NOTE: Draw right circle
            for (u32 PointId = 1; PointId < NumPoints; ++PointId)
            {
                f32 CircleStartAngle = (2.0f*Pi32)*f32(PointId - 1) / f32(NumPoints);
                f32 CircleEndAngle = (2.0f*Pi32)*f32(PointId) / f32(NumPoints);

                v2 CircleStartPos = StartCenterRight + Radius*V2(Cos(CircleStartAngle), Sin(CircleStartAngle));
                v2 CircleEndPos = StartCenterRight + Radius*V2(Cos(CircleEndAngle), Sin(CircleEndAngle));

                DebugPushLine(V3(CircleStartPos, 1.0f), V3(CircleEndPos, 1.0f), V3(0, 1, 1), V3(0, 1, 1));
            }
            
            DebugPushLine(V3(Departure, 1.0f), V3(ReEntry, 1.0f), V3(0, 1, 1), V3(0, 1, 1));

            // NOTE: Draw left circle
            for (u32 PointId = 1; PointId < NumPoints; ++PointId)
            {
                f32 CircleStartAngle = (2.0f*Pi32)*f32(PointId - 1) / f32(NumPoints);
                f32 CircleEndAngle = (2.0f*Pi32)*f32(PointId) / f32(NumPoints);

                v2 CircleStartPos = EndCenterLeft + Radius*V2(Cos(CircleStartAngle), Sin(CircleStartAngle));
                v2 CircleEndPos = EndCenterLeft + Radius*V2(Cos(CircleEndAngle), Sin(CircleEndAngle));

                DebugPushLine(V3(CircleStartPos, 1.0f), V3(CircleEndPos, 1.0f), V3(0, 1, 1), V3(0, 1, 1));
            }

            // NOTE: Draw right circle
            for (u32 PointId = 1; PointId < NumPoints; ++PointId)
            {
                f32 CircleStartAngle = (2.0f*Pi32)*f32(PointId - 1) / f32(NumPoints);
                f32 CircleEndAngle = (2.0f*Pi32)*f32(PointId) / f32(NumPoints);

                v2 CircleStartPos = EndCenterRight + Radius*V2(Cos(CircleStartAngle), Sin(CircleStartAngle));
                v2 CircleEndPos = EndCenterRight + Radius*V2(Cos(CircleEndAngle), Sin(CircleEndAngle));

                DebugPushLine(V3(CircleStartPos, 1.0f), V3(CircleEndPos, 1.0f), V3(0, 1, 1), V3(0, 1, 1));
            }

            // NOTE: Draw vectors from center to offsets
            DebugPushLine(V3(CurrStartCenter, 1.0f), V3(Departure, 1.0f), V3(1, 0, 1), V3(1, 0, 1));
            DebugPushLine(V3(CurrEndCenter, 1.0f), V3(ReEntry, 1.0f), V3(1, 0, 1), V3(1, 0, 1));
        }
    }
#endif

// NOTE: Projectile motion 
#if 0

// NOTE: Visualize projectile motion 2d
    {
        /*
          NOTE: Trying out different solutions to launching projectiles

          What I want:

          - Have the launch angle get lower as the target gets closer (its adaptive)
          - Have speed be a constant
            
          1) We constrain speed and target, and choose a launch angle. The problem is that given some gravity, we might not have
          enough speed to reach our target. We need to guarentee that we can always reach our target though which would require
          changing gravity, but that complicates the equation.

          2) What if the speed we set is actually the horizontal speed, NOT the launch speed (which contains a vertical component).

          p1x = p0x + v0*cos(a)*t, p1y = p0y + v0*sin(a)*t + 0.5*g*t*t

          So in the above, we know p0x, p1x, p0y, p1y, v0*cos(a) as a whole and g.
          Our unknowns are a, v0, and t.

          First we solve for t in the x equation:

          (p1x - p0x) / (v0x) = t
          R / (v0x) = t

          Plugging this back in to y equation gives us:

          p1y     p0y    v0*sin(a)*R     0.5*g*R*R
          =      +  -----------  +  ----------
          v0x          v0x*v0x
          
          So in the above, we got 2 unknowns but 1 equation. We want to be able to cancel out a term or somehow generate a new
          equation to get both a and v0. We do still have the following equation technically:

          v0x = v0*cos(a)

          This might help us cancel v0. So

          p1y     p0y     tan(a)*R       0.5*g*R*R
          =      +  -----------  +  ----------
          v0x*v0x

          Here we got 1 unknown now which is only a. So lets solve for it.

          (p1y - p0y - 0.5*g*R*R/(v0x*v0x))/R = tan(a)
          p1y/R - p0y/R - 0.5*g*R/(v0x*v0x) = tan(a)
          a = ArcTan(p1y/R - p0y/R - 0.5*g*R/(v0x*v0x))

          Then we can use a to solve for v0 in the v0x equation! I think this still has the same constraint on distance as above
          equation just that now we get better control over how long it takes us to get to the target (cuz gravity is fixed)

          3) So we need gravity to be adaptive but we can make t be a known variable. The game wants to have speed be constant
          because we want to make the time to hit the target be controlled. So we have the following equations:

          p1x = p0x + v0*cos(a)*t, p1y = p0y + v0*sin(a)*t + g*t*t, t = (p1x - p0x) / (v0*cos(a))

          The third equation gives us a calculation for total time. Its essentially the linear movement, no drag or anything in 1d.

          Now our unknowns are v0, a, and g. So lets play around. Easiest seems to be to put t equation into x equation.

          R = v0*cos(a)*R

          Before I finish above, turns out equation 3 is just a rearrangement of equation 1 XD. Maybe we can still do something in
          equation 2 given that we know t.

          H = v0*sin(a)*t + g*t*t

          So we got v0, a and g. If we play with x equation, we can solve for v0.

          R = v0*cos(a)*t
          R / (cos(a)*t) = v0

          H = R*sin(a)*t / (cos(a)*t) + g*t*t
          H = R*tan(a) + g*t*t

          So problem in above is that we got 2 unknowns still with no new equation to work with, even tho we know t. We can maybe
          create a equation using quadratic formula:

          t = -0 +- Sqrt(0*0 - 4*g*(R*tan(a) - H)) / 2*g
          t = +- Sqrt(-g*(R*tan(a) - H)) / g
          t = +- Sqrt(H - R*tan(a)) / Sqrt(g)
          Sqrt(g) = +- Sqrt(H - R*tan(a)) / t
          g = (H - R*tan(a)) / t*t

          Now we have a equation for g but we can't rearrange the above quadratic since that gives us the exact same equation
          as what we got using the quadratic formula (we need new information, but we didn't get it). 

          g = (H - R*tan(a)) / (t*t)

          The main issue I think is that the formula for t doesn't bring us new information/relationships between the variables.
          We can't plug into formula for x/t since we got a v0 there..

          Previously, we constrained g by setting a max height parameter, and that would let us generate all the free variables.
          Because g is unconstrained, does that give it too much freedom? Can we get to the target location in more than one way?

          We don't have full control over v0 because of v0*cos(a) being a constant we set. Actually, can we use that equation for a?

          v0x = v0*cos(a)

          Probably not because if we put in our equation for it, we add g. So g must be constrained somehow. I think that this is
          unsolveable as is, and the reason is that you can make cos(a) -> 0 and make v0 -> inf and set different params so that
          these work out, and then scale g such that we get there in time in the y axis. So unsolvable IMO.

          4) We can take the above method, and set heights. My main issue with setting heights is that we want the height
          to be adaptive, based on the distance the projectile has to travel, but how we set this is kinda arbitrary. It might be
          fine to just tune it to look good though, idk.

          5) Since we are thinking about setting some params arbitrarly to just look good, we can instead configure the angle we
          shoot at to be based on distance and make g adaptive. So we would have only g and v0 be unknowns. I think this is
          probably the better option to go for.

          p1x = p0x + v0*cos(a)*t, p1y = p0y + v0*sin(a)*t + g*t*t, v0x = v0*cos(a)

          R = v0x*t

          H = v0*tan(a)/R + g*t*t
          g = (H - v0*tan(a) / R) / t*t
        */

        v2 SpawnPos = V2(0.5f, 0.5f);
        v2 TargetPos = PlayState->CurrInput.ScreenPos;
        //v2 TargetPos = SpawnPos + V2(0.2f, 0.0f);
        f32 HorizontalSpeed = 1.0f;
        
        m4 OrthoProj = VkOrthoProjM4(0, 1, 1, 0, 0.001, 1000);
        DebugPushLineProjection(OrthoProj);

        // NOTE: Draw target pos
        {
            f32 Radius = 0.01f;
            v3 Corner0 = V3(TargetPos, 1.0f) + V3(Radius, 0.0f, 0.0f);
            v3 Corner1 = V3(TargetPos, 1.0f) + V3(Radius, Radius, 0.0f);
            v3 Corner2 = V3(TargetPos, 1.0f) + V3(0.0f, Radius, 0.0f);
            v3 Corner3 = V3(TargetPos, 1.0f) + V3(0.0f, 0.0f, 0.0f);

            DebugPushLine(Corner0, Corner1, V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));
            DebugPushLine(Corner1, Corner2, V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));
            DebugPushLine(Corner2, Corner3, V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));
            DebugPushLine(Corner3, Corner0, V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));
        }
        
        f32 DistanceHorizontal = Abs(TargetPos.x - SpawnPos.x);
        f32 DistanceVertical = TargetPos.y - SpawnPos.y;

        // NOTE: Method 2
        f32 Gravity = -10.0f;
        v2 InitialVelocity = {};
        {
            // NOTE: a = ArcTan(H/R - 0.5*g*R/(v0x*v0x))
            f32 LaunchAngle = (f32)atan((DistanceVertical / DistanceHorizontal) - ((0.5f*Gravity*DistanceHorizontal) / (HorizontalSpeed*HorizontalSpeed)));

            // NOTE: v0x = v0*cos(a) => v0x / cos(a) = v0
            f32 Speed = HorizontalSpeed / Cos(LaunchAngle);
            InitialVelocity.x = HorizontalSpeed;
            // NOTE: v0y = v0*sin(a)
            InitialVelocity.y = Speed*Sin(LaunchAngle);
            
            DebugPrintLog("Launch Angle: %f\n", LaunchAngle);
            DebugPrintLog("Initial Velocity: %f, %f : %f\n", InitialVelocity.x, InitialVelocity.y, Speed);
        }

        // NOTE: Plot proj movement
        v2 CurrPos = SpawnPos;
        v2 CurrVel = InitialVelocity;
        f32 dt = 0.005f;
        for (f32 t = 0.0f; t < 10.0f; t += dt)
            //for (f32 t = 0.0f; t < 3.0f; t += 3.0f)
        {
            v2 NewPos = 0.5f*V2(0.0f, Gravity)*dt*dt + CurrVel*dt + CurrPos;
            v2 NewVel = CurrVel + V2(0.0f, Gravity)*dt;

            DebugPushLine(V3(CurrPos, 1.0f), V3(NewPos, 1.0f), V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));

            CurrVel = NewVel;
            CurrPos = NewPos;
        }
    }

    // NOTE: Visualize projectile motion 3d
    {
        v3 SpawnPos = V3(20.0f, 20.0f, 0.0f);
        v3 TargetPos = PlayState->CurrInput.TerrainPos;
        //v2 TargetPos = SpawnPos + V2(0.2f, 0.0f);
        f32 HorizontalSpeed = 10.0f;

        DebugPushLineProjection(CameraGetVP(RenderState, &PlayState->Camera));

        // NOTE: Draw target pos
        DebugPushLineBox(TargetPos, V3(0.4f, 0.4f, 0.4f), V3(0.0f, 1.0f, 1.0f));
        
        f32 DistanceHorizontal = Length(TargetPos.xy - SpawnPos.xy);
        f32 DistanceVertical = TargetPos.z - SpawnPos.z;

        // NOTE: Method 2
        f32 Gravity = -10.0f;
        v3 InitialVelocity = {};
        {
            // NOTE: a = ArcTan(H/R - 0.5*g*R/(v0x*v0x))
            f32 LaunchAngle = (f32)atan((DistanceVertical / DistanceHorizontal) - ((0.5f*Gravity*DistanceHorizontal) / (HorizontalSpeed*HorizontalSpeed)));

            // NOTE: v0x = v0*cos(a) => v0x / cos(a) = v0
            f32 Speed = HorizontalSpeed / Cos(LaunchAngle);

            // NOTE: We have to divide the horizontal speed into the horizontal direction
            v2 HorizontalDir = Normalize(TargetPos.xy - SpawnPos.xy);
            InitialVelocity.xy = HorizontalSpeed*HorizontalDir;
            // NOTE: v0y = v0*sin(a)
            InitialVelocity.z = Speed*Sin(LaunchAngle);
            
            DebugPrintLog("Launch Angle: %f\n", LaunchAngle);
            DebugPrintLog("Initial Velocity: %f, %f, %f: %f\n", InitialVelocity.x, InitialVelocity.y, InitialVelocity.z, Speed);
        }

        // NOTE: Plot proj movement
        v3 CurrPos = SpawnPos;
        v3 CurrVel = InitialVelocity;
        f32 dt = 0.009f;
        for (f32 t = 0.0f; t < 10.0f; t += dt)
            //for (f32 t = 0.0f; t < 3.0f; t += 3.0f)
        {
            v3 GravityVec = V3(0.0f, 0.0f, Gravity);
            v3 NewPos = 0.5f*GravityVec*dt*dt + CurrVel*dt + CurrPos;
            v3 NewVel = CurrVel + GravityVec*dt;

            DebugPushLine(CurrPos, NewPos, V3(0.0f, 1.0f, 1.0f), V3(0.0f, 1.0f, 1.0f));

            CurrVel = NewVel;
            CurrPos = NewPos;
        }
    }

#endif

#if 0

// NOTE: This is unit tests for block arena and block lists
    {
        block_arena Arena = BlockSubArena(&DebugState->Arena, 100, 10);

        for (u32 i = 0; i < 10; ++i)
        {
            PushBlock(&Arena);
        }

        ArenaClear(&Arena);

        void* Blocks[10];
        for (u32 i = 0; i < 10; ++i)
        {
            Blocks[i] = PushBlock(&Arena);
        }

        for (u32 i = 0; i < 10; ++i)
        {
            BlockArenaFreeBlock(&Arena, Blocks[i]);
        }

        for (u32 i = 0; i < 10; ++i)
        {
            Blocks[i] = PushBlock(&Arena);
        }

        ArenaClear(&Arena);

        block_list_block Sentinel;
        BlockListInitSentinel(&Sentinel);

        for (u32 i = 0; i < 10; ++i)
        {
            for (u32 j = 0; j < 17; ++j)
            {
                BlockListAddEntry(&Arena, &Sentinel, u32);
            }
        }

        BlockListClear(&Arena, &Sentinel);

        for (u32 i = 0; i < 10; ++i)
        {
            for (u32 j = 0; j < 17; ++j)
            {
                BlockListAddEntry(&Arena, &Sentinel, u32);
            }
        }

        BlockListClear(&Arena, &Sentinel);

        block_list_block* Ptrs[10];
        for (u32 i = 0; i < 10; ++i)
        {
            Ptrs[i] = BlockListAddBlock(&Arena, &Sentinel);
        }

        for (u32 i = 0; i < 10; ++i)
        {
            for (u32 j = 0; j < 10; ++j)
            {
                if (i != j)
                {
                    Assert(Ptrs[i] != Ptrs[j]);
                }
            }

            if (i != 9)
            {
                Assert(uptr(Ptrs[i+1]) - uptr(Ptrs[i]) == 100);
            }
        }
        
        for (u32 i = 0; i < 10; ++i)
        {
            BlockListFreeBlock(&Arena, Ptrs[i]);
        }

        for (u32 i = 0; i < 10; ++i)
        {
            BlockListAddBlock(&Arena, &Sentinel);
        }
        
        int i = 0;
    }

#endif

// NOTE: Enum Parsing
#if 0

struct enum_entry
{
    string Name;
    u32 Value;
};

struct enum_data
{
    u32 NumEntries;
    enum_entry* Entries;
};

enum level_parser_state
{
    LevelParserState_None,
    LevelParserState_InMonstar,
};

struct monstar_data
{
    u32 MaxNumTemplates;
    u32 NumTemplates;
    file_monstar_template* Templates;
    string* TemplateNames;
};

struct ability_template;
struct ability_data
{
    u32 MaxNumAbilities;
    u32 NumAbilities;
    ability_template* Abilities;
    string* AbilityNames;
};

// NOTE: Loading enum tables
inline b32 LevelIsNameChar(char C)
{
    b32 Result = IsLetter(C) || C == '_';
    return Result;
}

inline u32 LevelNumNameChars(string Body)
{
    u32 Result = 0;

    // NOTE: First char cannot be a numeric
    if (LevelIsNameChar(Body.Chars[0]))
    {
        while ((LevelIsNameChar(Body.Chars[Result]) || IsNumeric(Body.Chars[Result])) &&
               Result < Body.NumChars)
        {
            Result += 1;
        }
    }
    
    return Result;
}

inline string LevelReadNameAndAdvance(string* Body)
{
    string Result = {};
    
    u32 NumCharsInName = LevelNumNameChars(*Body);
    Result = String(Body->Chars, NumCharsInName);
    AdvanceString(Body, NumCharsInName);

    return Result;
}

inline string LevelCopyNameAndAdvance(string* Body)
{
    string Result = {};
    
    u32 NumCharsInName = LevelNumNameChars(*Body);
    Result.NumChars = NumCharsInName;
    Result.Chars = (char*)malloc(sizeof(char)*NumCharsInName);
    Copy(Body->Chars, Result.Chars, sizeof(char)*NumCharsInName);
    
    AdvanceString(Body, NumCharsInName);

    return Result;
}

inline enum_data ParseEnum(string FileStr, string EnumNameStr)
{
    enum_data Result = {};

    string EnumStr = String("enum", 4);
    string CommaStr = String(",", 1);
    string OpenCurlyStr = String("{", 1);
    string CloseCurlyStr = String("}", 1);
    string CommentStr = String("//", 2);
    string EqualStr = String("=", 1);
    string ShiftLeftStr = String("<<", 2);

    // NOTE: Scan file to find our enum and record number of enum entries
    string EnumStartStr = {};
    bool InsideEnum = false;
    string CurrChar = FileStr;
    while (CurrChar.NumChars > 0)
    {
        if (InsideEnum)
        {
            if (StringContained(CommaStr, CurrChar))
            {
                Result.NumEntries += CommaStr.NumChars;
                AdvanceString(&CurrChar, CommaStr.NumChars);
            }
            else if (StringContained(CommentStr, CurrChar))
            {
                // NOTE: Ignore comments
                AdvanceCharsToNewline(&CurrChar);
            }
            else if (StringContained(CloseCurlyStr, CurrChar))
            {
                // NOTE: Reached end of our enum
                break;
            }
            else
            {
                AdvanceString(&CurrChar, 1);
            }
        }
        else
        {
            if (WordContained(EnumStr, CurrChar))
            {
                // NOTE: We found a enum, check if its the one we are looking for
                AdvanceString(&CurrChar, EnumStr.NumChars);
                AdvancePastSpaces(&CurrChar);

                if (WordContained(EnumNameStr, CurrChar))
                {
                    // NOTE: We found the enum we are looking for
                    AdvanceString(&CurrChar, EnumNameStr.NumChars);
                    AdvancePastDeadSpaces(&CurrChar);
                    
                    // NOTE: Get past curly bracket
                    Assert(StringContained(OpenCurlyStr, CurrChar));
                    AdvanceString(&CurrChar, OpenCurlyStr.NumChars);
                    AdvancePastDeadSpaces(&CurrChar);
                    
                    // NOTE: Setup string for the start of the enum entries
                    EnumStartStr = CurrChar;
                    InsideEnum = true;
                }
            }
            else
            {
                AdvanceString(&CurrChar, 1);
            }
        }
    }

    // NOTE: Check if we found our enum
    Assert(InsideEnum == true);

    // NOTE: Allocate data for our enum
    Result.Entries = (enum_entry*)malloc(sizeof(enum_entry)*Result.NumEntries);

    // NOTE: Populate enum entries
    i32 AutomaticEnumValue = 0;
    enum_entry* CurrEntry = Result.Entries;
    CurrChar = EnumStartStr;
    while (CurrChar.NumChars > 0)
    {
        if (LevelIsNameChar(CurrChar.Chars[0]))
        {
            // NOTE: Get the entry name
            CurrEntry->Name = LevelCopyNameAndAdvance(&CurrChar);
            AdvancePastSpaces(&CurrChar);
            
            // NOTE: Calcuate the enum value for the entry
            if (StringContained(CommaStr, CurrChar))
            {
                // NOTE: No value is given, so its the previous value + 1
                CurrEntry->Value = AutomaticEnumValue;
                AutomaticEnumValue += 1;
                AdvanceString(&CurrChar, CommaStr.NumChars);
            }
            else if (StringContained(EqualStr, CurrChar))
            {
                // NOTE: A value is provided, parse the value
                // IMPORTANT: Currently we only support shifted values
                AdvanceString(&CurrChar, EqualStr.NumChars);
                AdvancePastSpaces(&CurrChar);
                    
                // NOTE: Read LHS of shift
                Assert(IsUInt(CurrChar.Chars[0]));
                u32 LHS;
                ReadUIntAndAdvance(&CurrChar, &LHS);
                    
                AdvancePastSpaces(&CurrChar);

                // NOTE: Make sure there is a shift
                Assert(StringContained(ShiftLeftStr, CurrChar));
                AdvanceString(&CurrChar, ShiftLeftStr.NumChars);
                AdvancePastSpaces(&CurrChar);

                // NOTE: Read RHS of shift
                Assert(IsUInt(CurrChar.Chars[0]));
                u32 RHS;
                ReadUIntAndAdvance(&CurrChar, &RHS);

                // NOTE: Get past comma
                Assert(StringContained(CommaStr, CurrChar));
                AdvanceString(&CurrChar, CommaStr.NumChars);

                CurrEntry->Value = LHS << RHS;
            }

            CurrEntry += 1;
        }
        else if (StringContained(CommentStr, CurrChar))
        {
            // NOTE: Ignore comments
            AdvanceCharsToNewline(&CurrChar);
        }
        else if (CurrChar.Chars[0] == '}')
        {
            InsideEnum = false;
            break;
        }
        else
        {
            AdvanceString(&CurrChar, 1);
        }
    }

    // NOTE: Check if we actually parsed the whole enum
    Assert(InsideEnum == false);
    
    return Result;
}

inline i32 CheckEnumForEntry(enum_data Enum, string EntryName)
{
    i32 Result = -1;

    for (u32 EntryId = 0; EntryId < Enum.NumEntries; ++EntryId)
    {
        if (StringsEqual(Enum.Entries[EntryId].Name, EntryName))
        {
            Result = EntryId;
            break;
        }
    }

    return Result;
}

#endif
