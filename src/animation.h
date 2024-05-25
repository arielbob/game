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
// TODO: Bone_Frame should probably be Bone_Sample..
struct Bone_Frame {
    int32 frame_num;
    Transform local_transform; // relative to bone's parent
};

struct Bone_Channel {
    Allocator *allocator;

    String name; // the bone name; should correspond with bone in skeleton
    int32 num_samples;
    Bone_Frame *samples;
};

struct Skeletal_Animation {
    Allocator *allocator;
    int32 num_bones;
    //Skeleton *skeleton;

    int32 id;
    String name;
    String filename;
    
    int32 frame_end;
    int32 fps;
    Mat4 bone_to_model;
    Bone_Channel *bone_channels; // skeleton->num_bones channels

    int32 watcher_id;
    
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

    // copy each bone channel's samples
    for (int32 i = 0; i < source->num_bones; i++) {
        result.bone_channels[i].allocator = allocator;

        result.bone_channels[i].name = copy(allocator, source->bone_channels[i].name);
        int32 num_samples = source->bone_channels[i].num_samples;
        result.bone_channels[i].samples = (Bone_Frame *) allocate(allocator,
                                                                 sizeof(Bone_Frame) * num_samples);
        memcpy(result.bone_channels[i].samples, source->bone_channels[i].samples,
               sizeof(Bone_Frame) * num_samples);
    }

    return result;
}

void deallocate(Bone *bone) {
    deallocate(bone->name);
}

void deallocate(Bone_Channel *bone_channel) {
    // nothing to deallocate in Bone_Frame struct, just the allocation for the
    // Bone_Frame themselves
    deallocate(bone_channel->allocator, bone_channel->samples);
    deallocate(bone_channel->name);
}

void deallocate(Skeletal_Animation *animation) {
    deallocate(animation->name);
    deallocate(animation->filename);

    for (int32 i = 0; i < animation->num_bones; i++) {
        deallocate(&animation->bone_channels[i]);
    }
    deallocate(animation->allocator, animation->bone_channels);
}

void deallocate(Skeleton *skeleton) {
    for (int32 i = 0; i < skeleton->num_bones; i++) {
        deallocate(&skeleton->bones[i]);
    }
    deallocate(skeleton->allocator, skeleton->bones);
}

#endif
