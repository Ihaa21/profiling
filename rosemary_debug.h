#pragma once

/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Ihor Szlachtycz $
   $Notice: (C) Copyright 2014 by Dream.Inc, Inc. All Rights Reserved. $
   ======================================================================== */

#if ROSEMARY_DEBUG

#define DEBUG_MAX_NUM_FRAMES 200

//
// NOTE: Platform Specific Data
//

#if ROSEMARY_ANDROID

// NOTE: Should be #include <sys\types.h> but android doesn't define it here for some reason
#include <unistd.h>

#if ROSEMARY_X86
#include <x86intrin.h>
#else
#include <time.h>
#endif

#elif ROSEMARY_WIN32

#if ROSEMARY_X86
#include <intrin.h>
#endif

#endif

inline u32 DebugGetThreadId()
{
#if ROSEMARY_WIN32
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    u32 Result = *(u32 *)(ThreadLocalStorage + 0x48);
#elif ROSEMARY_ANDROID
    u32 Result = gettid();
#endif
    
    return Result;
}

inline u64 DebugGetCpuCycle()
{
#if ROSEMARY_X86
    u64 Result = __rdtsc();
#elif ROSEMARY_ARM
    /* NOTE: Incase we find a way to do this faster
           https://github.com/google/benchmark/blob/v1.1.0/src/cycleclock.h#L116
           https://stackoverflow.com/questions/3247373/how-to-measure-program-execution-time-in-arm-cortex-a8-processor
    */

    // NOTE: https://www.gamasutra.com/view/feature/171774/getting_high_precision_timing_on_.php?page=3
    timespec TimeSpec;
    Assert(clock_gettime(CLOCK_MONOTONIC, &TimeSpec) == 0);
    u64 Result = TimeSpec.tv_sec*1000*1000*1000 + TimeSpec.tv_nsec;
#endif
    return Result;
}

//
// NOTE: Debug Memory Profiling
//

struct debug_arena_info
{
    u64 Id;
    mm Size;
    mm Used;
};

//
// NOTE: Debug Cpu Profiling
//

// TODO: REMOVE, push dynamically
#define NUM_DEBUG_EVENTS_RECORDED 700

enum debug_time_stamp_type
{
    DebugTimeStamp_BeginBlock,
    DebugTimeStamp_EndBlock,
    DebugTimeStamp_FrameTime,
    DebugTimeStamp_FrameMarker,
};

struct debug_cpu_timestamp
{
    union
    {
        // NOTE: Used for frame markers to get the total number of seconds
        f64 SecondsElapsed;
        // NOTE: Used to get the regular clock cycle timings
        u64 Cycle;
    };

    u8 ThreadId;
    u8 Type;
    char* Name;
    char* GUID;
};

// NOTE: For top clocks
struct debug_function_info
{
    char* GUID;
    u64 CycleCount;
    u64 HitCount;
};

// NOTE: For profile graphs
struct debug_time_block
{
    u64 StartCycle;
    u64 NumCycles;
    u8 ThreadId;
    char* Name;
    char* GUID;

    debug_time_block* Child;
    debug_time_block* Next;
};

struct debug_open_block
{
    u32 FrameIndex;
    debug_time_block* TimeBlock;
};

struct debug_open_block_stack
{
    u32 MaxNumOpenBlocks;
    u32 CurrNumOpenBlocks;
    debug_open_block* Blocks;
};

//
// NOTE: Interactions
//

enum debug_ui_overlay_state
{
    DebugOverlay_None,
    
    DebugOverlay_FrameTimes,
    DebugOverlay_ProfileGraph,
    DebugOverlay_TopClockList,
};

struct debug_frame_bar_interaction
{
    u32 FrameId;
};

struct debug_profile_bar_interaction
{
    debug_time_block* Block;
};

//
// NOTE: Rendering
//

struct render_state;
struct debug_render_state
{
    render_state* RenderState;
    
    // NOTE: Point objects
    u32 PointMaxNumProjs;
    u32 PointCurrProj;
    m4* PointProjs;
    u32* PointNumVerts;

    u32 MaxNumPoints;
    u32 CurrPoint;
    v3* PointPosArray;
    v3* PointColorArray;
    f32* PointSizeArray;

    // NOTE: Line objects
    u32 LineMaxNumProjs;
    u32 LineCurrProj;
    m4* LineProjs;
    u32* LineNumVerts;
    
    u32 MaxNumLineVerts;
    u32 CurrLineVert;
    v3* LinePosArray;
    v3* LineColorArray;

    u32 NumInteractiveModels;
};

struct debug_frame
{
    // NOTE: We use this value to know if our strings are still valid post recompilation
    u32 LibId;
    
    // NOTE: Profiling data for the frame
    f64 SecondsElapsed;
    u64 BeginCycle;
    u64 EndCycle;
    u32 FrameIndex;

    // NOTE: This is for profile graph
    block_list_block TimeBlockSentinel;
    // NOTE: This is for top clocks
    block_list_block FunctionInfoSentinel;
    // NOTE: This is for memory tracking
    block_list_block ArenaInfoSentinel;
};

#define DEBUG_BLOCK_SIZE (KiloBytes(8))
struct debug_state
{
    linear_arena Arena;
    block_arena BlockArena;

    debug_render_state RenderState;

    // NOTE: Debug Ui
    u32 CurrDisplayFrameId;
    debug_ui_overlay_state UiOverlayState;

    // NOTE: Global debug data info
    b32 DidRecompile;
    u32 CurrLibId;
    u32 CurrFrameId;
    debug_frame* FrameArray;

    // NOTE: Memory allocation data
    b32 PauseMemoryProfiling;
    block_list_block ArenaInfoSentinel;
    
    // NOTE: Profiling data
    b32 PauseCpuProfiling; // NOTE: For pausing all timestamp collections
    u32 PausedTimeStampId; // NOTE: This is for pausing timetsamp collection of certain functions
    u32 CurrTimeStampId;
    debug_cpu_timestamp* CpuTimeStamps;
    debug_cpu_timestamp* PrevCpuTimeStamps;
};

global debug_state* DebugState;

//
// NOTE: Memory Profiling
//

inline void DebugRecordAllocation(linear_arena* Arena)
{
#if DEBUG_MEMORY_PROFILING
    if (!DebugState || DebugState->PauseMemoryProfiling)
    {
        return;
    }

    // NOTE: Search if we have this arena info already recorded
    b32 Found = false;
    uptr SearchId = uptr(Arena);
    block_list_block* CurrBlock = DebugState->ArenaInfoSentinel.Next;
    while (CurrBlock != &DebugState->ArenaInfoSentinel)
    {
        for (u32 InfoId = 0; InfoId < CurrBlock->NumEntries; ++InfoId)
        {
            debug_arena_info* CurrInfo = (debug_arena_info*)CurrBlock->Data + InfoId;
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
        debug_arena_info* ArenaInfo = BlockListAddEntry(&DebugState->BlockArena, &DebugState->ArenaInfoSentinel, debug_arena_info);
        ArenaInfo->Id = SearchId;
        ArenaInfo->Size = Arena->Size;
        ArenaInfo->Used = Arena->Used;
    }
#endif
}

//
// NOTE: CPU Profiling
//

#if DEBUG_PROFILE_DATA

inline void DebugRecordTimeStamp(char* Name, char* GUID, u8 Type)
{
    // TODO: For multi threaded, atomic add here
    Assert(DebugState->CurrTimeStampId < NUM_DEBUG_EVENTS_RECORDED);
    debug_cpu_timestamp* CurrTimeStamp = DebugState->CpuTimeStamps + DebugState->CurrTimeStampId++;
    CurrTimeStamp->Cycle = DebugGetCpuCycle();
    CurrTimeStamp->ThreadId = (u8)DebugGetThreadId();
    CurrTimeStamp->Type = Type;
    CurrTimeStamp->Name = Name;
    CurrTimeStamp->GUID = GUID;
}

inline void DebugRecordFrameTime(char* Name, char* GUID, f64 Time)
{
    // TODO: For multi threaded, atomic add here
    Assert(DebugState->CurrTimeStampId < NUM_DEBUG_EVENTS_RECORDED);
    debug_cpu_timestamp* CurrTimeStamp = DebugState->CpuTimeStamps + DebugState->CurrTimeStampId++;
    CurrTimeStamp->SecondsElapsed = Time;
    CurrTimeStamp->ThreadId = (u8)DebugGetThreadId();
    CurrTimeStamp->Type = DebugTimeStamp_FrameTime;
    CurrTimeStamp->Name = Name;
    CurrTimeStamp->GUID = GUID;
}

struct debug_timed_block
{
    debug_timed_block(char* Name, char* GUID)
    {
        DebugRecordTimeStamp(Name, GUID, DebugTimeStamp_BeginBlock);
    }

    ~debug_timed_block()
    {
        DebugRecordTimeStamp("", "", DebugTimeStamp_EndBlock);
    }
};

#define DEBUG_NAME__(A, B, C, D) #A "|" #B "|" #C "|" #D
#define DEBUG_NAME_(A, B, C, D) DEBUG_NAME__(A, B, C, D)
#define DEBUG_NAME(Name) DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__, Name)

#define TIMED_BLOCK__(Name, GUID, Number) debug_timed_block TimedBlock_##Number((char*)Name, (char*)GUID)
#define TIMED_BLOCK_(Name, GUID, Number) TIMED_BLOCK__(Name, GUID, Number)
#define TIMED_BLOCK(Name) TIMED_BLOCK_(DEBUG_NAME(Name), Name, __COUNTER__)
#define TIMED_FUNC(...) TIMED_BLOCK(__FUNCTION__)

#define FRAME_MARKER(Seconds) \
    DebugRecordFrameTime(DEBUG_NAME("Frame Marker"), "Frame Marker", Seconds); \
    DebugRecordTimeStamp(DEBUG_NAME("Frame Marker"), "Frame Marker", DebugTimeStamp_FrameMarker)

#else

#define TIMED_BLOCK(...)
#define FRAME_MARKER(...)
#define TIMED_FUNC(...)

#endif

#include <cstdio> // NOTE: This is for snprintf

#if ROSEMARY_WIN32

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
    
#elif ROSEMARY_ANDROID

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

/*
#define DebugPrintLog(Text, ...)                                    \
    {                                                               \
        __android_log_print(ANDROID_LOG_INFO, Text, __VA_ARGS__);   \
    }                                                               \

#define DebugPrintError(Text, ...)                      \
    {                                                   \
        __android_log_print(ANDROID_LOG_ERROR, Text, __VA_ARGS__); \
    }                                                   \
*/
//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "threaded_app", __VA_ARGS__))

#endif

#else

inline void DebugPrintLog(char* Text, ...) {}
inline void DebugPrintError(char* Text, ...) {}
inline void DebugPrintLog(char* PrintText, char* Text, ...) {}
inline void DebugPrintError(char* PrintText, char* Text, ...) {}

#endif

