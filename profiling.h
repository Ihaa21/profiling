#pragma once

/*

  IMPORTANT: We assume that the following macros are defined/not defined above the include for this file:

    - CPU_PROFILING
    - MEMORY_PROFILING
    - WIN32_PROFILING
    - ANDROID_PROFILING
    - X86_PROFILING
    - ARM_PROFILING
    - VK_PROFILING
    - DX12_PROFILING

    ^ If any of the above are not defined, then the code will disable that functionality (you can turn profiling features on/off using
      these macros. 

    If we are on windows, we assume that we are on a x86/64 machine while on Android, you can toggle between arm and x86.

    TODO: Setup system to just store all the data in its raw form, we can maybe compress or do somethign special, but overall just
          store it as it comes. Each profiler output should transform it as needed.
    
 */

#include "memory/memory.h"

enum profiler_time_stamp_type
{
    ProfilerTimeStamp_BeginBlock,
    ProfilerTimeStamp_EndBlock,
    ProfilerTimeStamp_FrameTime,
    ProfilerTimeStamp_FrameMarker,
};

//
// NOTE: Cpu Profiling
//

#ifdef CPU_PROFILING

struct profiler_cpu_timestamp
{
    union
    {
        // NOTE: Used for frame markers to get the total number of seconds
        f64 SecondsElapsed;
        // NOTE: Used to get the regular clock cycle timings
        u64 Cycle;
    };

    u32 ThreadId;
    u32 Type;
    char* Name;
    char* GUID;
};

inline void ProfilerCpuRecordTimeStamp(char* Name, char* GUID, u8 Type);
inline void ProfilerCpuRecordFrameTime(char* Name, char* GUID, f64 Time);

struct profiler_cpu_timed_block
{
    profiler_cpu_timed_block(char* Name, char* GUID)
    {
        ProfilerCpuRecordTimeStamp(Name, GUID, ProfilerTimeStamp_BeginBlock);
    }

    ~profiler_cpu_timed_block()
    {
        ProfilerCpuRecordTimeStamp("", "", ProfilerTimeStamp_EndBlock);
    }
};

inline void ProfilerRecordTimeStamp(char* Name, char* GUID, u8 Type);
inline void ProfilerRecordFrameTime(char* Name, char* GUID, f64 Time);

#define PROFILER_NAME__(A, B, C, D) #A "|" #B "|" #C "|" #D
#define PROFILER_NAME_(A, B, C, D) PROFILER_NAME__(A, B, C, D)
#define PROFILER_NAME(Name) PROFILER_NAME_(__FILE__, __LINE__, __COUNTER__, Name)

#define CPU_TIMED_BLOCK__(Name, GUID, Number) profiler_cpu_timed_block CpuTimedBlock_##Number((char*)Name, (char*)GUID)
#define CPU_TIMED_BLOCK_(Name, GUID, Number) CPU_TIMED_BLOCK__(Name, GUID, Number)
#define CPU_TIMED_BLOCK(Name) CPU_TIMED_BLOCK_(PROFILER_NAME(Name), Name, __COUNTER__)
#define CPU_TIMED_FUNC(...) CPU_TIMED_BLOCK(__FUNCTION__)

#define CPU_FRAME_MARKER(Seconds) \
    ProfilerCpuRecordFrameTime(PROFILER_NAME("Frame Marker"), "Frame Marker", Seconds); \
    ProfilerCpuRecordTimeStamp(PROFILER_NAME("Frame Marker"), "Frame Marker", ProfilerTimeStamp_FrameMarker)

#else

#define CPU_TIMED_BLOCK(...)
#define CPU_FRAME_MARKER(...)
#define CPU_TIMED_FUNC(...)

#endif

// NOTE: For top clocks
struct profiler_function_info
{
    char* GUID;
    u64 CycleCount;
    u64 HitCount;
};

// NOTE: For profile graphs
struct profiler_time_block
{
    u64 StartCycle;
    u64 NumCycles;
    u8 ThreadId;
    char* Name;
    char* GUID;

    profiler_time_block* Child;
    profiler_time_block* Next;
};

//
// NOTE: Profiler Gpu Profiling
//

// TODO: Add GPU profiling

//
// NOTE: Profiler Memory Profiling
//

struct profiler_arena_info
{
    u64 Id;
    mm Size;
    mm Used;
};

//
// NOTE: Profiler Data Processing
//

struct profiler_open_block
{
    profiler_open_block* Prev;
    profiler_open_block* Next;
    
    u32 FrameIndex;
    profiler_time_block* TimeBlock;
};

struct profiler_open_block_stack
{
    profiler_open_block Sentinel;
};

struct profiler_frame
{
    // NOTE: We use this value to know if our strings are still valid post recompilation
    u32 LibId;
    
    // NOTE: Profiling data for the frame
    f64 SecondsElapsed;
    u64 BeginCycle;
    u64 EndCycle;
    u32 FrameIndex;

    // TODO: Save string names so we don't crash on reloads
    
    // NOTE: This is for profile graph
    dynamic_arena TimeBlockArena;
    // NOTE: This is for top clocks
    dynamic_arena FunctionInfoArena;
    // NOTE: This is for memory tracking
    dynamic_arena AllocationInfoArena;
};

//
// NOTE: CSV Gen
//

// TODO: Use a string hash, will be quicker
struct profiler_csv_block_col
{
    char* GUID;
    u64 CurrTiming;
    profiler_csv_block_col* Next;
    profiler_csv_block_col* Prev;
};

//
// NOTE: Profiler State
//

enum profiler_state_flags
{
    ProfilerFlag_OutputCsv = 1 << 0,
    ProfilerFlag_ClearEveryFrame = 1 << 1,
    ProfilerFlag_AutoSetEndOfFrame = 1 << 2,
};

struct profiler_state
{
    u32 Flags;
    
    // NOTE: We store profiler state and frames here
    dynamic_arena Arena;
    dynamic_arena TempArena;
    // NOTE: We use this if we wanna clear the above arena but preserve profiler_state
    dynamic_temp_mem ArenaTempMem;

    u32 CurrFrameId;
    profiler_frame* CurrFrame;
    profiler_frame* CurrDisplayFrame;
    
    // NOTE: Global profiler data info
    b32 DidRecompile;
    u32 CurrLibId;

    // NOTE: Memory allocation data
    b32 PauseMemoryProfiling;
    
    // NOTE: Profiling data
#ifdef CPU_PROFILING
    dynamic_arena CpuTimeStampArena;
#endif
};

global profiler_state* ProfilerState;

inline void DebugPrintLog(char* Text, ...);
inline void DebugPrintError(char* Text, ...);
inline void DebugPrintLog(char* PrintText, u32 PrintTextSize, char* Text, ...);
inline void DebugPrintError(char* PrintText, u32 PrintTextSize, char* Text, ...);

#include "profiling.cpp"
#include "profiling_ui.h"
#include "profiling_ui.cpp"
