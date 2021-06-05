/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

//
// NOTE: Platform Specific Data
//

#ifdef ANDROID_PROFILING

// NOTE: Should be #include <sys\types.h> but android doesn't define it here for some reason
#include <unistd.h>

#ifdef X86_PROFILING
#include <x86intrin.h>
#elif defined(ARM_PROFILING)
#include <time.h>
#endif

#elif defined(WIN32_PROFILING)

#define X86_PROFILING
#include <intrin.h>

#endif

inline u32 ProfilerGetThreadId()
{
    u32 Result = 0;
#ifdef WIN32_PROFILING
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    Result = *(u32 *)(ThreadLocalStorage + 0x48);
#elif defined(ANDROID_PROFILING)
    Result = gettid();
#endif
    
    return Result;
}

inline u64 ProfilerGetCpuCycle()
{
    u64 Result = 0;
    
#ifdef X86_PROFILING
    Result = __rdtsc();
#elif defined(ARM_PROFILING)
    /* NOTE: Incase we find a way to do this faster
           https://github.com/google/benchmark/blob/v1.1.0/src/cycleclock.h#L116
           https://stackoverflow.com/questions/3247373/how-to-measure-program-execution-time-in-arm-cortex-a8-processor
    */

    // NOTE: https://www.gamasutra.com/view/feature/171774/getting_high_precision_timing_on_.php?page=3
    timespec TimeSpec;
    Assert(clock_gettime(CLOCK_MONOTONIC, &TimeSpec) == 0);
    Result = TimeSpec.tv_sec*1000*1000*1000 + TimeSpec.tv_nsec;
#endif
    return Result;
}

//
// NOTE: Logging (TODO: Should be part of a debug utils lib?)
//

#include <cstdio> // NOTE: This is for snprintf

inline void DebugSnprintf(char* Dst, u32 StringSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(Dst, StringSize, Text, Arguments);
    va_end(Arguments);
}

#ifdef WIN32_PROFILING

#include <windows.h>

// NOTE: https://stackoverflow.com/questions/1056411/how-to-pass-variable-number-of-arguments-to-printf-sprintf
inline void DebugPrintLog(char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    char PrintText[1024];
    vsnprintf(PrintText, sizeof(PrintText), Text, Arguments);
    va_end(Arguments);

    OutputDebugStringA("LOG: ");
    OutputDebugStringA(PrintText);
}

inline void DebugPrintError(char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    char PrintText[1024];
    vsnprintf(PrintText, sizeof(PrintText), Text, Arguments);
    va_end(Arguments);

    OutputDebugStringA("ERROR: ");
    OutputDebugStringA(PrintText);
}

inline void DebugPrintLog(char* PrintText, u32 PrintTextSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(PrintText, PrintTextSize, Text, Arguments);
    va_end(Arguments);

    OutputDebugStringA("LOG: ");
    OutputDebugStringA(PrintText);
}

inline void DebugPrintError(char* PrintText, u32 PrintTextSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(PrintText, PrintTextSize, Text, Arguments);
    va_end(Arguments);

    OutputDebugStringA("ERROR: ");
    OutputDebugStringA(PrintText);
}
    
#elif defined(ANDROID_PROFILING)

// TODO: Check if var args works
inline void DebugPrintLog(char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    char PrintText[1024];
    vsnprintf(PrintText, sizeof(PrintText), Text, Arguments);
    va_end(Arguments);

    __android_log_print(ANDROID_LOG_INFO, "ROSEMARY", PrintText);
}

inline void DebugPrintError(char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    char PrintText[1024];
    vsnprintf(PrintText, sizeof(PrintText), Text, Arguments);
    va_end(Arguments);

    __android_log_print(ANDROID_LOG_ERROR, "ROSEMARY", PrintText);
}

inline void DebugPrintLog(char* PrintText, u32 PrintTextSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(PrintText, PrintTextSize, Text, Arguments);
    va_end(Arguments);
    
    __android_log_print(ANDROID_LOG_INFO, "ROSEMARY", PrintText);
}

inline void DebugPrintError(char* PrintText, u32 PrintTextSize, char* Text, ...)
{
    va_list Arguments;
    va_start(Arguments, Text);
    vsnprintf(PrintText, PrintTextSize, Text, Arguments);
    va_end(Arguments);
    
    __android_log_print(ANDROID_LOG_ERROR, "ROSEMARY", PrintText);
}

#endif

//
// NOTE: CPU Profiling
//

#ifdef CPU_PROFILING
inline void ProfilerCpuRecordTimeStamp(char* Name, char* GUID, u8 Type)
{
    // TODO: For multi threaded, do per thread arenas
    profiler_cpu_timestamp* CurrTimeStamp = PushStruct(&ProfilerState->CpuTimeStampArena, profiler_cpu_timestamp);
    CurrTimeStamp->Cycle = ProfilerGetCpuCycle();
    CurrTimeStamp->ThreadId = (u8)ProfilerGetThreadId();
    CurrTimeStamp->Type = Type;
    CurrTimeStamp->Name = Name;
    CurrTimeStamp->GUID = GUID;
}

inline void ProfilerCpuRecordFrameTime(char* Name, char* GUID, f64 Time)
{
    // TODO: For multi threaded, do per thread arenas
    profiler_cpu_timestamp* CurrTimeStamp = PushStruct(&ProfilerState->CpuTimeStampArena, profiler_cpu_timestamp);
    CurrTimeStamp->SecondsElapsed = Time;
    CurrTimeStamp->ThreadId = (u8)ProfilerGetThreadId();
    CurrTimeStamp->Type = ProfilerTimeStamp_FrameTime;
    CurrTimeStamp->Name = Name;
    CurrTimeStamp->GUID = GUID;
}
#endif

//
// NOTE: Memory Profiling
//

inline void ProfilerRecordAllocation(linear_arena* Arena)
{
#ifdef MEMORY_PROFILING
    if (!ProfilerState || ProfilerState->PauseMemoryProfiling)
    {
        return;
    }

    // NOTE: Search if we have this arena info already recorded
    b32 Found = false;
    uptr SearchId = uptr(Arena);
    block_list_block* CurrBlock = ProfilerState->ArenaInfoSentinel.Next;
    while (CurrBlock != &ProfilerState->ArenaInfoSentinel)
    {
        for (u32 InfoId = 0; InfoId < CurrBlock->NumEntries; ++InfoId)
        {
            profiler_arena_info* CurrInfo = (profiler_arena_info*)CurrBlock->Data + InfoId;
            if (CurrInfo->Id == SearchId)
            {
                CurrInfo->Size = Arena->Size;
                CurrInfo->Used = Arena->Used;
                Found = true;
                break;
            }
        }

        if (Found)
        {
            break;
        }

        CurrBlock = CurrBlock->Next;
    }

    if (!Found)
    {
        profiler_arena_info* ArenaInfo = BlockListAddEntry(&ProfilerState->BlockArena, &ProfilerState->ArenaInfoSentinel, profiler_arena_info);
        ArenaInfo->Id = SearchId;
        ArenaInfo->Size = Arena->Size;
        ArenaInfo->Used = Arena->Used;
    }
#endif
}

//
// NOTE: Profiler Processing
//

#if 0
inline void DebugRecordingPause()
{
    DebugState->PausedTimeStampId = DebugState->CurrTimeStampId;
}

inline void DebugRecordingUnpause()
{
    DebugState->CurrTimeStampId = DebugState->PausedTimeStampId;
}
#endif

inline profiler_frame* ProfilerFrameCreate(u32 FrameIndex, u32 LibId)
{
    profiler_frame* Result = 0;

    if (ProfilerState->CurrFrame && ProfilerState->CurrFrame->FrameIndex == FrameIndex)
    {
        Result = ProfilerState->CurrFrame;
    }
    else
    {
        Result = PushStruct(&ProfilerState->Arena, profiler_frame);
        *Result = {};
        Result->SecondsElapsed = 0.0;
        Result->BeginCycle = 0;
        Result->EndCycle = 0;
        Result->FrameIndex = FrameIndex;
        Result->LibId = LibId;
        Result->TimeBlockArena = DynamicArenaCreate(KiloBytes(16));
        Result->FunctionInfoArena = DynamicArenaCreate(KiloBytes(16));
        Result->AllocationInfoArena = DynamicArenaCreate(KiloBytes(16));
        ProfilerState->CurrFrame = Result;
    }    

    return Result;
}

//
// NOTE: CSV Functions
//

inline profiler_csv_block_col* ProfilerCsvBlockColGetOrCreate(dynamic_arena* Arena, profiler_csv_block_col* Sentinel, char* GUID)
{
    profiler_csv_block_col* Result = 0;
    
    for (profiler_csv_block_col* CurrCol = Sentinel->Next;
         CurrCol != Sentinel;
         CurrCol = CurrCol->Next)
    {
        if (CurrCol->GUID == GUID)
        {
            Result = CurrCol;
            break;
        }
    }

    if (!Result)
    {
        Result = PushStruct(Arena, profiler_csv_block_col);
        *Result = {};
        Result->GUID = GUID;
        LinkedListSentinelAppend(*Sentinel, Result);
    }

    return Result;
}

internal void ProfilerOutputCsv()
{
#ifdef CPU_PROFILING
    // NOTE: We output frame times in each row, columns are our time stamp categories
    // TODO: Get real world time so we get a file name here
    FILE* CsvFile = fopen("temp.csv", "wb");

    dynamic_temp_mem TempMem = BeginTempMem(&ProfilerState->TempArena);

    profiler_csv_block_col Sentinel = {};
    LinkedListSentinelCreate(Sentinel);

    // NOTE: First, figure out how many columns we have in total
    for (dynamic_arena_header* FrameHeader = ProfilerState->Arena.Next;
         FrameHeader;
         FrameHeader = FrameHeader->Next)
    {
        // TODO: We should probably make this a function below, we use it everywhere
        u32 NumFrames = u32(DynamicArenaHeaderGetSize(FrameHeader) / sizeof(profiler_frame));
        for (u32 FrameId = 0; FrameId < NumFrames; ++FrameId)
        {
            profiler_frame* CurrFrame = (profiler_frame*)DynamicArenaHeaderGetData(FrameHeader) + FrameId;

            for (dynamic_arena_header* FunctionInfoHeader = CurrFrame->FunctionInfoArena.Next;
                 FunctionInfoHeader;
                 FunctionInfoHeader = FunctionInfoHeader->Next)
            {
                u32 NumInfos = u32(DynamicArenaHeaderGetSize(FunctionInfoHeader) / sizeof(profiler_function_info));
                for (u32 FunctionInfoId = 0; FunctionInfoId < NumInfos; ++FunctionInfoId)
                {
                    profiler_function_info* CurrInfo = (profiler_function_info*)DynamicArenaHeaderGetData(FunctionInfoHeader) + FunctionInfoId;
                    profiler_csv_block_col* CurrCol = ProfilerCsvBlockColGetOrCreate(&ProfilerState->TempArena, &Sentinel, CurrInfo->GUID);
                }
            }
        }
    }

    // NOTE: Loop through all time blocks and print out headers
    for (profiler_csv_block_col* CurrCol = Sentinel.Next;
         CurrCol != &Sentinel;
         CurrCol = CurrCol->Next)
    {
        // TODO: Kinda slow
        char Text[256];
        DebugSnprintf(Text, sizeof(Text), "%s,", CurrCol->GUID);
        fwrite(Text, strlen(Text)*sizeof(char), 1, CsvFile);
    }
    fwrite("\n", sizeof(char), 1, CsvFile);

    // NOTE: Loop through all frames and print out data for each row
    for (dynamic_arena_header* FrameHeader = ProfilerState->Arena.Next;
         FrameHeader;
         FrameHeader = FrameHeader->Next)
    {
        // TODO: We should probably make this a function below, we use it everywhere
        u32 NumFrames = u32(DynamicArenaHeaderGetSize(FrameHeader) / sizeof(profiler_frame));
        for (u32 FrameId = 0; FrameId < NumFrames; ++FrameId)
        {
            profiler_frame* CurrFrame = (profiler_frame*)DynamicArenaHeaderGetData(FrameHeader) + FrameId;

            // NOTE: We have a last frame which is partially recorded/empty which outputs 0s. This if prevents that
            if (CurrFrame->FunctionInfoArena.Next)
            {
                // NOTE: Find the curr frame value for each of our columns
                for (dynamic_arena_header* FunctionInfoHeader = CurrFrame->FunctionInfoArena.Next;
                     FunctionInfoHeader;
                     FunctionInfoHeader = FunctionInfoHeader->Next)
                {
                    u32 NumInfos = u32(DynamicArenaHeaderGetSize(FunctionInfoHeader) / sizeof(profiler_function_info));
                    for (u32 FunctionInfoId = 0; FunctionInfoId < NumInfos; ++FunctionInfoId)
                    {
                        profiler_function_info* CurrInfo = (profiler_function_info*)DynamicArenaHeaderGetData(FunctionInfoHeader) + FunctionInfoId;
                        profiler_csv_block_col* CurrCol = ProfilerCsvBlockColGetOrCreate(&ProfilerState->TempArena, &Sentinel, CurrInfo->GUID);
                        CurrCol->CurrTiming = CurrInfo->CycleCount;
                    }
                }

                // NOTE: Loop through all our columns and write out their value to CSV
                for (profiler_csv_block_col* CurrCol = Sentinel.Next;
                     CurrCol != &Sentinel;
                     CurrCol = CurrCol->Next)
                {
                    // TODO: Kinda slow
                    char Text[256];
                    DebugSnprintf(Text, sizeof(Text), "%llu,", CurrCol->CurrTiming);
                    fwrite(Text, strlen(Text)*sizeof(char), 1, CsvFile);

                    CurrCol->CurrTiming = 0;
                }
                fwrite("\n", sizeof(char), 1, CsvFile);
            }
        }
    }

    EndTempMem(TempMem);
    
    fclose(CsvFile);
#endif
}

//
// NOTE: Frame Graph
//

inline void ProfilerOpenBlockStackInit(profiler_open_block_stack* OutStack)
{
    *OutStack = {};
    LinkedListSentinelCreate(OutStack->Sentinel);
}

inline profiler_open_block* ProfilerOpenBlockStackGetTop(profiler_open_block_stack* Stack)
{
    profiler_open_block* Result = Stack->Sentinel.Prev;
    return Result;
}

inline profiler_open_block* ProfilerOpenBlockAlloc(profiler_open_block_stack* Stack)
{
    profiler_open_block* Result = {};
    Result = PushStruct(&ProfilerState->TempArena, profiler_open_block);
    LinkedListSentinelAppend(Stack->Sentinel, Result);
    Result->FrameIndex = ProfilerState->CurrFrameId;

    return Result;
}

inline void ProfilerOpenBlockDealloc(profiler_open_block_stack* Stack)
{
    // IMPORTANT: We have to store the remove block in temporary for the macro to work...
    profiler_open_block* RemoveBlock = Stack->Sentinel.Prev;
    LinkedListSentinelRemove(RemoveBlock);
}

inline profiler_function_info* ProfilerFunctionInfoGetOrCreate(profiler_frame* Frame, char* GUID)
{
    profiler_function_info* Result = 0;

    // TODO: Do we want this to be a hashtable? 
    // NOTE: Loop through our linked list and check if we have this GUID
    b32 Found = false;
    for (dynamic_arena_header* Header = Frame->FunctionInfoArena.Next;
         Header && !Found;
         Header = Header->Next)
    {
        u32 NumInfos = u32(DynamicArenaHeaderGetSize(Header) / sizeof(profiler_function_info));
        for (u32 InfoId = 0; InfoId < NumInfos; ++InfoId)
        {
            profiler_function_info* CurrInfo = (profiler_function_info*)DynamicArenaHeaderGetData(Header) + InfoId;
            if (CurrInfo->GUID == GUID)
            {
                Found = true;
                Result = CurrInfo;
                break;
            }
        }
    }
        
    if (!Found)
    {
        Result = PushStruct(&Frame->FunctionInfoArena, profiler_function_info);
        *Result = {};
        Result->GUID = GUID;
    }
    
    return Result;
}

//
// NOTE: Profiler State Functions
//

internal void ProfilerStateCreate(u32 Flags)
{
    // IMPORTANT: We assume here the demo is allocating the profiler state
    *ProfilerState = {};
    ProfilerState->Flags = Flags;
    ProfilerState->Arena = DynamicArenaCreate(KiloBytes(4));
    ProfilerState->ArenaTempMem = BeginTempMem(&ProfilerState->Arena);
    ProfilerState->TempArena = DynamicArenaCreate(KiloBytes(4));

#ifdef CPU_PROFILING
    ProfilerState->CpuTimeStampArena = DynamicArenaCreate(KiloBytes(16));
#endif
}

internal void ProfilerStateDestroy()
{
    if (ProfilerState->Flags & ProfilerFlag_OutputCsv)
    {
        // TODO: Once we support GPU time stamps and arena info, also dump here based on whats enabled
        ProfilerOutputCsv();
    }
}

internal void ProfilerStateClearData()
{
    // NOTE: Loop through all frames and clear their data
    for (dynamic_arena_header* Header = ProfilerState->Arena.Next;
         Header;
         Header = Header->Next)
    {
        u32 Offset = 0;
        if (Header == ProfilerState->Arena.Next)
        {
            Offset += sizeof(profiler_state);
        }
        
        u32 NumFrames = u32((DynamicArenaHeaderGetSize(Header) - Offset) / sizeof(profiler_frame));
        for (u32 FrameId = 0; FrameId < NumFrames; ++FrameId)
        {
            profiler_frame* CurrFrame = (profiler_frame*)((u8*)DynamicArenaHeaderGetData(Header) + Offset) + FrameId;
            ArenaClear(&CurrFrame->TimeBlockArena);
            ArenaClear(&CurrFrame->FunctionInfoArena);
            ArenaClear(&CurrFrame->AllocationInfoArena);
        }
    }

    // NOTE: Clears our main frame arena to just store the profiler_state
    EndTempMem(ProfilerState->ArenaTempMem);
}

internal void ProfilerProcessData()
{
    // TODO: First frame will be garbage here
    if (ProfilerState->Flags & ProfilerFlag_AutoSetEndOfFrame)
    {
        // TODO: Handle this cleaner, and maybe use query perf timer for all of win32
        local_global LARGE_INTEGER PrevTime = {};
        LARGE_INTEGER CurrTime = {};
        Assert(QueryPerformanceCounter(&CurrTime) == BOOL(true));

        LARGE_INTEGER TimerFrequency = {};
        Assert(QueryPerformanceFrequency(&TimerFrequency) == BOOL(true));
        
        f64 Time = ((f64)(CurrTime.QuadPart - PrevTime.QuadPart) / (f64)TimerFrequency.QuadPart);
        
        CPU_FRAME_MARKER(Time);
    }
    
    // NOTE: Get the current frame
    profiler_frame* CurrFrame = ProfilerFrameCreate(ProfilerState->CurrFrameId, ProfilerState->CurrLibId - ProfilerState->DidRecompile);
    ProfilerState->DidRecompile = false;
    ProfilerState->CurrDisplayFrame = CurrFrame;

#ifdef CPU_PROFILING

    dynamic_temp_mem TempMem = BeginTempMem(&ProfilerState->TempArena);
    profiler_open_block_stack OpenBlockStack = {};
    ProfilerOpenBlockStackInit(&OpenBlockStack);

    for (dynamic_arena_header* Header = ProfilerState->CpuTimeStampArena.Next;
         Header;
         Header = Header->Next)
    {
        u32 NumRecords = u32(DynamicArenaHeaderGetSize(Header) / sizeof(profiler_cpu_timestamp));
        for (u32 EventIndex = 0; EventIndex < NumRecords; ++EventIndex)
        {
            profiler_cpu_timestamp* ProfilerCpuTimeStamp = (profiler_cpu_timestamp*)DynamicArenaHeaderGetData(Header) + EventIndex;

            switch (ProfilerCpuTimeStamp->Type)
            {
                case ProfilerTimeStamp_BeginBlock:
                {
                    profiler_time_block* Parent = 0;
                    if (!LinkedListSentinelEmpty(OpenBlockStack.Sentinel))
                    {
                        Parent = ProfilerOpenBlockStackGetTop(&OpenBlockStack)->TimeBlock;
                    }
                    else
                    {
                        // TODO: We add a fake parent? 
                        Parent = PushStruct(&CurrFrame->TimeBlockArena, profiler_time_block);
                        *Parent = {};
                    }
                
                    profiler_time_block* Block = PushStruct(&CurrFrame->TimeBlockArena, profiler_time_block);
                    *Block = {};
                    Block->StartCycle = ProfilerCpuTimeStamp->Cycle;
                    Block->ThreadId = u8(ProfilerCpuTimeStamp->ThreadId);
                    Block->Name = ProfilerCpuTimeStamp->Name;
                    Block->GUID = ProfilerCpuTimeStamp->GUID;

                    Block->Next = Parent->Child;
                    Parent->Child = Block;

                    profiler_open_block* OpenBlock = ProfilerOpenBlockAlloc(&OpenBlockStack);
                    OpenBlock->TimeBlock = Block;
                } break;
            
                case ProfilerTimeStamp_EndBlock:
                {
                    if (!LinkedListSentinelEmpty(OpenBlockStack.Sentinel))
                    {
                        // NOTE: Add to our profile bars data
                        profiler_time_block* CurrBlock = ProfilerOpenBlockStackGetTop(&OpenBlockStack)->TimeBlock;
                        CurrBlock->NumCycles = ProfilerCpuTimeStamp->Cycle - CurrBlock->StartCycle;

                        // NOTE: Add to our top clock list
                        profiler_function_info* FunctionInfo = ProfilerFunctionInfoGetOrCreate(CurrFrame, CurrBlock->GUID);
                        FunctionInfo->HitCount += 1;
                        FunctionInfo->CycleCount += CurrBlock->NumCycles;
                    
                        ProfilerOpenBlockDealloc(&OpenBlockStack);
                    }
                    else
                    {
                        // TODO: We sometime run into this case, shouldn't it be illegal?
                        InvalidCodePath;
                    }
                } break;

                case ProfilerTimeStamp_FrameTime:
                {
                    CurrFrame->SecondsElapsed = ProfilerCpuTimeStamp->SecondsElapsed;
                } break;
            
                case ProfilerTimeStamp_FrameMarker:
                {
                    CurrFrame->EndCycle = ProfilerCpuTimeStamp->Cycle;

                    ProfilerState->CurrFrameId += 1;
                    CurrFrame = ProfilerFrameCreate(ProfilerState->CurrFrameId, ProfilerState->CurrLibId - ProfilerState->DidRecompile);
                    CurrFrame->BeginCycle = ProfilerCpuTimeStamp->Cycle;
                } break;
            }
        }
    }
    
    EndTempMem(TempMem);

    ArenaClear(&ProfilerState->CpuTimeStampArena);
#endif

#ifdef MEMORY_PROFILING

#if 0
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
    
#endif
}
