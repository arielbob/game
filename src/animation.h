#ifndef ANIMATION_H
#define ANIMATION_H

struct Skeletal_Animation {
    Allocator *allocator;

    int32 num_bones;
    real32 duration;

    int32 num_frames;
    Skeletal_Frame *frames;
};

struct Skeletal_Frame {
    int32 num_bones;

    // TODO: try putting the inverse bind transform in this transform as well
    // - you could just imagine it as basically moving the bind position of the bone
    //   directly to the final position
    // - instead of first moving it to be in bone-space (multiplying by inverseBind matrix)
    // TODO: for updating animation each frame, just go by dt
    // - don't need to call some time function again; just use the dt we calculate at the
    //   start of update() in game.cpp

    real32 timestamp; // in seconds
    
    Transform bone_pose_model_transform;
};

#endif
