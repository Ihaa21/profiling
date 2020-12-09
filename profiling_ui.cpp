
#if 0
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

#endif

internal void ProfilerPrintTimeStamps()
{
    CPU_TIMED_FUNC();

    DebugPrintLog("Beginning Of Frame\n");

    profiler_frame* Frame = ProfilerState->CurrDisplayFrame;
    for (dynamic_arena_header* Header = Frame->FunctionInfoArena.Next;
         Header;
         Header = Header->Next)
    {
        u32 NumInfos = u32(DynamicArenaHeaderGetSize(Header) / sizeof(profiler_function_info));
        for (u32 InfoId = 0; InfoId < NumInfos; ++InfoId)
        {
            profiler_function_info* FunctionInfo = (profiler_function_info*)DynamicArenaHeaderGetData(Header) + InfoId;
            if (FunctionInfo->GUID) // TODO: All fo them should have a GUID if we save strings
            {
                DebugPrintLog("%s HitCount: %llu CycleCount: %llu\n", FunctionInfo->GUID,
                              FunctionInfo->HitCount, FunctionInfo->CycleCount);
            }
        }
    }

    DebugPrintLog("End Of Frame\n");
}
