#include "animation.h"

Mat4 *get_bone_matrices(Allocator *allocator, Skeletal_Animation *animation, real32 t) {
    // allocator should probably be the frame allocator
    assert(allocator == frame_arena);

    t = fmodf(t, animation->duration);

    // TODO: go through each channel in the animation,
    assert(animation->skeleton);
    Skeleton *skeleton = animation->skeleton;

    // it's assumed that bones/their transforms in arrays are laid out such that parents
    // always come before their children. this way, we can just iterate from start to end
    // and ensure that a bone will always have its parents transforms calculated already.
    Mat4 *matrices = (Mat4 *) allocate(allocator, skeleton->num_bones * sizeof(Mat4));

    // the parent matrix for the root converts points in root-space to model-space.
    // that's done by multiplying by the root's local transform.
    // if you want a pose to match the bind pose, then each joint's local transforms need to
    // result in the joints in model-space being where they are in the bind pose.
    
    // the root's child (c_1) local transform transforms it from c_1-space to root-space.
    // to convert c_1 points to model-space, you multiply p by c_1's local transform (c_1 -> root),
    // then by root's local transform (root -> model).
    // the matrix that gets put in this array for c_1 is the "c_1 -> root -> model" matrix.
    // then for c_2, its "parent matrix" (the matrix that goes in this array) is the
    // "c_2 -> c_1 -> root -> model" matrix.
    Allocator *temp_region = begin_region();
    Mat4 *bone_to_model_matrices = (Mat4 *) allocate(temp_region, skeleton->num_bones * sizeof(Mat4));
    
    for (int32 i = 0; i < skeleton->num_bones; i++) {
        Bone *bone = &skeleton->bones[i];
        Bone_Channel *bone_channel = &animation->bone_channels[i];

        // it's assumed that an animation has at least 1 frame
        assert(bone_channel->num_frames > 0);
        
        // find the two frames we're inbetween
        int32 frame_a_index = 0;
        int32 frame_b_index = frame_a_index;
        
        for (int32 frame_i = 0; frame_i < bone_channel->num_frames; frame_i++) {
            Bone_Frame *frame = &bone_channel->frames[frame_i];
            if (frame->timestamp > t) break;

            frame_a_index = frame_i;
        }

        frame_b_index = (frame_a_index + 1) % bone_channel->num_frames;
        
        // interpolate between the two frames
        Bone_Frame *frame_a = &bone_channel->frames[frame_a_index];
        Bone_Frame *frame_b = &bone_channel->frames[frame_b_index];

        // we assume that the timestamp of the first frame in any animation  is 0
        real32 start_t = frame_a->timestamp;
        real32 end_t = frame_b->timestamp;

        // if we wrapped around, add the duration to make end_t come after
        if (frame_b_index < frame_a_index) {
            end_t += animation->duration;
        }
        real32 frame_t = (t - start_t) / (end_t - start_t);
        if (end_t - start_t < 0.00001f) {
            frame_t = start_t;
        }

        Transform *transform_a = &frame_a->local_transform;
        Transform *transform_b = &frame_b->local_transform;

        // interpolate the bone transforms
        // TODO: this shit is fucked; the cube is gaining volume between keyframes
        Transform interpolated_transform;
        interpolated_transform.position = lerp(transform_a->position,
                                               transform_b->position,
                                               frame_t);
        interpolated_transform.rotation = slerp(transform_a->rotation,
                                                transform_b->rotation,
                                                frame_t);

        // this may not be necessary, and scale can just be assumed to always be 1
        interpolated_transform.scale = lerp(transform_a->scale,
                                            transform_b->scale,
                                            frame_t);

        Mat4 parent_to_model = make_mat4_identity();
        if (bone->parent_index >= 0) {
            parent_to_model = bone_to_model_matrices[bone->parent_index];
        }

        // local_transform * p_in_bone_space -> parent-space
        // parent_to_model * local_transform * p_in_bone_space -> model-space
        Mat4 bone_to_model = parent_to_model * get_model_matrix(interpolated_transform);

        // matrix we send to the shader needs the inverse bind matrix sent as well, since
        // the model-space points need to be in bone-space.
        Mat4 model_to_bone = bone->model_to_bone_matrix;
        matrices[i] = bone_to_model * model_to_bone;

        // save the bone->model matrix so that this bone's children can use it
        bone_to_model_matrices[i] = bone_to_model;
    }

    end_region(temp_region);
    
    return matrices;    
}

#if 0
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
#endif
