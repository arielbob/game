#ifndef ANIMATION_H
#define ANIMATION_H

struct Bone {
    String name; // also stored on Mesh's allocator
    // TODO: make a Mat3x4 struct for this? idk
    // - we can just store the 12 values in the file, but then when we
    //   parse it and store the data here, we can just have it be a Mat4
    Mat4 model_to_bone_matrix; // inverse bind matrix
    int32 parent_index = -1;
};

struct Skeleton {
    Allocator *allocator;
    int32 num_bones;
    Bone *bones;
};

// the terms "bone" and "joint" are used interchangeably, but they
// both really mean joint.
struct Bone_Frame {
    real32 timestamp;
    Transform local_transform; // relative to bone's parent
};

struct Bone_Channel {
    int32 num_frames;
    Bone_Frame *frames;
};

struct Skeletal_Animation {
    Allocator *allocator;
    Skeleton *skeleton;

    int32 id;
    String name;
    
    real32 duration;
    Bone_Channel *bone_channels; // skeleton->num_bones channels

    Skeletal_Animation *table_next;
    Skeletal_Animation *table_prev;
};

void deallocate(Skeletal_Animation *animation) {
    deallocate(animation->name);
}

void deallocate(Bone *bone) {
    deallocate(bone->name);
}

void deallocate(Skeleton *skeleton) {
    for (int32 i = 0; i < skeleton->num_bones; i++) {
        deallocate(&skeleton->bones[i]);
    }
    deallocate(skeleton->allocator, skeleton->bones);
}

#endif
