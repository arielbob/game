#ifndef ANIMATION_H
#define ANIMATION_H

struct Skeletal_Frame {
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

struct Bone_Frame {
    real32 timestamp; // in seconds
    Transform bone_pose_model_transform;
};

struct Bone_Channel {
    int32 num_frames;
    Bone_Frame *frames;
};

struct Bone {
    int32 parent_index; // idk if we really need this
    int32 num_children;
    int32 *children;
};

struct Skeletal_Animation {
    Allocator *allocator;

    int32 id;
    String name;
    
    int32 num_bones;
    // TODO: make a Mat3x4 struct for this? idk
    // - we can just store the 12 values in the file, but then when we
    //   parse it and store the data here, we can just have them be Mat4s
    Mat4 *model_to_bone_matrices; // inverse bind matrices

    Bone_Channel *bone_channels;
    
    real32 duration;

    int32 num_frames;
    Skeletal_Frame *frames;

    Skeletal_Animation *table_next;
    Skeletal_Animation *table_prev;
};

void deallocate(Skeletal_Animation *animation) {
    deallocate(animation->name);
    deallocate(animation->allocator, animation->frames);
}

#endif
