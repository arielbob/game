#include "animation.h"

Mat4 *get_bone_matrices(Allocator *allocator, Skeletal_Animation *animation, real32 t) {
    // allocator should probably be the frame allocator
    assert(allocator == frame_arena);

    t = fmodf(t, animation->duration);

    int32 frame_a_index = 0;
    int32 frame_b_index = frame_a_index;

    for (int32 i = 0; i < animation->num_frames; i++) {
        Skeletal_Frame *frame = &animation->frames[i];
        if (frame->timestamp > t) break;
        frame_a_index = i;
    }

    frame_b_index = (frame_a_index + 1) % animation->num_frames;

    // interpolate between frames
    Skeletal_Frame *frame_a = &animation->frames[frame_a_index];
    Skeletal_Frame *frame_b = &animation->frames[frame_b_index];

    // we assume that the first frame's timestamp is 0
    real32 start_t = frame_a->timestamp;
    real32 end_t = frame_b->timestamp;
    if (frame_b_index < frame_a_index) {
        end_t += animation->duration;
    }
    real32 frame_t = (t - start_t) / (end_t - start_t);

    // the final transforms array will be size of:
    // num_bones * Mat4

    Mat4 *matrices = (Mat4 *) allocate(allocator, animation->num_bones * sizeof(Mat4));
    
    // each frame has the transforms for every single bone in the skeleton!
    #if 0
    for (int32 i = 0; i < animation->num_bones; i++) {
        Transform *bone_transform_a = &frame_a->bone_pose_model_transforms[i];
        Transform *bone_transform_b = &frame_b->bone_pose_model_transforms[i];

        // interpolate the bone transforms
        Transform interpolated_transform;
        interpolated_transform.position = lerp(bone_transform_a->position,
                                               bone_transform_b->position,
                                               t);
        interpolated_transform.rotation = slerp(bone_transform_a->rotation,
                                                bone_transform_b->rotation,
                                                t);

        // this may not be necessary, and scale can just be assumed to always be 1
        interpolated_transform.scale = lerp(bone_transform_a->scale,
                                            bone_transform_b->scale,
                                            t);

        Mat4 model_to_bone = animation->model_to_bone_matrices[i];
        matrices[i] = get_model_matrix(interpolated_transform) * model_to_bone;
    }
    #endif

    return matrices;
}
