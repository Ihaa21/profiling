#pragma once

#if 0

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

#endif
