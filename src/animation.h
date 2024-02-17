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
    int32 num_bones;
    //Skeleton *skeleton;

    int32 id;
    String name;
    String filename;
    
    real32 duration;
    Bone_Channel *bone_channels; // skeleton->num_bones channels

    Skeletal_Animation *table_next;
    Skeletal_Animation *table_prev;
};

Skeletal_Animation copy(Allocator *allocator, Skeletal_Animation *source) {
    Skeletal_Animation result = *source;
    result.allocator = allocator;

    // copy strings
    result.name = copy(allocator, source->name);
    result.filename = copy(allocator, source->filename);

    // copy bone channels
    result.bone_channels = (Bone_Channel *) allocate(allocator, sizeof(Bone_Channel) * source->num_bones);
    memcpy(result.bone_channels, source->bone_channels, sizeof(Bone_Channel) * source->num_bones);

    // copy each bone channel's frames
    for (int32 i = 0; i < source->num_bones; i++) {
        int32 num_frames = source->bone_channels[i].num_frames;
        result.bone_channels[i].frames = (Bone_Frame *) allocate(allocator,
                                                                 sizeof(Bone_Frame) * num_frames);
        memcpy(result.bone_channels[i].frames, source->bone_channels[i].frames,
               sizeof(Bone_Frame) * num_frames);
    }

    return result;
}

void deallocate(Skeletal_Animation *animation) {
    assert(!"implement");
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
