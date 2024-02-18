#include "animation.h"

Mat4 *get_bone_matrices(Allocator *allocator, Skeleton *skeleton, Skeletal_Animation *animation, real32 t) {
    // allocator should probably be the frame allocator
    assert(allocator == frame_arena);

    t = fmodf(t, animation->duration);

    assert(skeleton->num_bones == animation->num_bones);
    
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

namespace Animation_Loader {
    enum Token_Type {
        END,
        COMMENT,
        INTEGER,
        REAL,
        LABEL,
        OPEN_BRACKET,
        CLOSE_BRACKET,
        STRING
    };

    struct Token {
        Token_Type type;
        String string;
    };

    Token get_token(Tokenizer *tokenizer);
    Token make_token(Token_Type type, char *contents, int32 length);
    bool32 token_text_equals(Token token, char *str);
    bool32 animation_parse_error(char **error_out, char *error_string);
    bool32 check_label(Token *token, char *name);
    
    bool32 parse_real(Tokenizer *tokenizer, real32 *result, char **error);
    bool32 parse_int(Tokenizer *tokenizer, int32 *result, char **error);
    bool32 parse_uint(Tokenizer *tokenizer, uint32 *result, char **error);
    bool32 parse_vec2(Tokenizer *tokenizer, Vec2 *result, char **error);
    bool32 parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error);
    bool32 parse_vec4(Tokenizer *tokenizer, Vec4 *result, char **error);
    bool32 parse_vec3_int32(Tokenizer *tokenizer, Vec3_int32 *result, char **error);
    bool32 parse_vec3_uint32(Tokenizer *tokenizer, Vec3_uint32 *result, char **error);
    bool32 parse_vec4_int32(Tokenizer *tokenizer, Vec4_int32 *result, char **error);
    bool32 parse_vec4_uint32(Tokenizer *tokenizer, Vec4_uint32 *result, char **error);
    bool32 parse_quaternion(Tokenizer *tokenizer, Quaternion *result, char **error);

    bool32 parse_animation_info(Tokenizer *tokenizer, Skeletal_Animation *animation, char **error);
    bool32 parse_bone_frame(Allocator *allocator, Tokenizer *tokenizer,
                            Bone_Frame *frame, char **error);
    bool32 parse_bone_channel(Allocator *allocator, Tokenizer *tokenizer,
                              Bone_Channel *bone_channel, char **error);
    bool32 parse_bones(Tokenizer *tokenizer, Skeletal_Animation *animation, char **error);

    bool32 load_animation(Allocator *allocator, String name, String filename,
                                            Skeletal_Animation **animation_result, char **error);
}

inline Animation_Loader::Token Animation_Loader::make_token(Token_Type type, char *contents, int32 length) {
    Token token = {
        type,
        make_string(contents, length)
    };
    return token;
}

Animation_Loader::Token Animation_Loader::get_token(Tokenizer *tokenizer) {
    Token token = {};

    do {
        consume_leading_whitespace(tokenizer);

        if (is_end(tokenizer)) {
            return make_token(END, NULL, 0);
        }
    
        char c = *tokenizer->current;

        if (is_digit(c) || c == '-' || c == '.') {
            uint32 start = tokenizer->index;
        
            bool32 has_period = false;
            bool32 is_negative = false;
            if (c == '.') {
                has_period = true;
            } else if (c == '-') {
                is_negative = true;
            }

            increment_tokenizer(tokenizer);
        
            while (!is_end(tokenizer) &&
                   !is_whitespace(*tokenizer->current)) {

                if (!is_digit(*tokenizer->current) &&
                    *tokenizer->current != '.') {
                    assert(!"Expected digit or period");
                }
            
                if (*tokenizer->current == '.') {
                    if (!has_period) {
                        has_period = true;
                    } else {
                        assert(!"More than one period in number");
                    }
                }

                if (*tokenizer->current == '-') {
                    assert(!"Negatives can only be at the start of a number");
                }

                increment_tokenizer(tokenizer);
            }
        
            uint32 length = tokenizer->index - start;

            Token_Type token_type;
            if (has_period) {
                token_type = REAL;
            } else {
                token_type = INTEGER;
            }

            // NOTE: we don't include the null terminator here, and just set bounds using a length member
            token = make_token(token_type, &tokenizer->contents[start], length);
        } else if (tokenizer->current[0] == ';' &&
                   tokenizer->current[1] == ';') {
            increment_tokenizer(tokenizer, 2);
            uint32 start = tokenizer->index;
        
            while (!is_end(tokenizer) &&
                   !is_line_end(tokenizer->current)) {
                increment_tokenizer(tokenizer);
            }

            uint32 length = tokenizer->index - start;
            token = make_token(COMMENT, &tokenizer->contents[start], length);
        } else if (is_letter(tokenizer->current[0])) {
            uint32 start = tokenizer->index;

            increment_tokenizer(tokenizer);
        
            while (!is_end(tokenizer) &&
                   !is_whitespace(tokenizer->current[0])) {

                char current_char = *tokenizer->current;
                if (!(is_letter(current_char) || current_char == '_')) {
                    assert(!"Labels can only contain letters and underscores.");
                }

                increment_tokenizer(tokenizer);
            }

        
            uint32 length = tokenizer->index - start;
            token = make_token(LABEL, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '{') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(OPEN_BRACKET, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '}') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(CLOSE_BRACKET, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '"') {
            increment_tokenizer(tokenizer);
            // set start after we increment tokenizer so that we don't include the quote in the token string.
            int32 start = tokenizer->index;

            while (!is_end(tokenizer) &&
                   !(*tokenizer->current == '"')) {
                increment_tokenizer(tokenizer);
            }

            if (*tokenizer->current != '"') {
                assert(!"Expected a closing quote.");
            } else {
                int32 length = tokenizer->index - start;
                increment_tokenizer(tokenizer);
                token = make_token(STRING, &tokenizer->contents[start], length);
            }
        } else {
            assert(!"Token type not recognized");
            return {};
        }
    } while (token.type == COMMENT);

    return token;
}

inline bool32 Animation_Loader::animation_parse_error(char **error_out, char *error_string) {
    *error_out = error_string;
    return false;
}

bool32 Animation_Loader::check_label(Token *token, char *name) {
    return (token->type == LABEL) && string_equals(token->string, name);
}

bool32 Animation_Loader::parse_real(Tokenizer *tokenizer, real32 *result, char **error) {
    Token token = get_token(tokenizer);
    real32 num;
    
    if (token.type == REAL || token.type == INTEGER) {
        bool32 parse_result = string_to_real32(token.string, &num);

        if (!parse_result) {
            return animation_parse_error(error, "Invalid number value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return animation_parse_error(error, "Expected number.");
    }
}

bool32 Animation_Loader::parse_int(Tokenizer *tokenizer, int32 *result, char **error) {
    Token token = get_token(tokenizer);
    int32 num;
    
    if (token.type == INTEGER) {
        bool32 parse_result = string_to_int32(token.string, &num);

        if (!parse_result) {
            return animation_parse_error(error, "Invalid integer value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return animation_parse_error(error, "Expected integer.");
    }
}

bool32 Animation_Loader::parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error) {
    Vec3 v;
    
    for (int i = 0; i < 3; i++) {
        if (!parse_real(tokenizer, &v[i], error)) {
            return animation_parse_error(error, "Invalid number in Vec3.");
        }
    }

    *result = v;
    return true;
}

bool32 Animation_Loader::parse_quaternion(Tokenizer *tokenizer, Quaternion *result, char **error) {
    Quaternion quat;
    
    if (!parse_real(tokenizer, &quat.w, error)) {
        return false;
    }
    
    for (int i = 0; i < 3; i++) {
        if (!parse_real(tokenizer, &(quat.v[i]), error)) {
            return false;
        }
    }

    *result = quat;
    return true;
}

bool32 Animation_Loader::parse_animation_info(Tokenizer *tokenizer, Skeletal_Animation *animation, char **error) {
    Token token = get_token(tokenizer);
    if (!check_label(&token, "animation_info")) {
        return animation_parse_error(error, "Expected animation_info label.");
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return animation_parse_error(error, "Expected animation_info open bracket.");
    }

    token = get_token(tokenizer);
    if (!check_label(&token, "name")) {
        return animation_parse_error(error, "Expected animation name label.");
    }

    token = get_token(tokenizer);
    if (token.type != STRING) {
        return animation_parse_error(error, "Expected animation name string.");
    }

    token = get_token(tokenizer);
    if (!check_label(&token, "duration")) {
        return animation_parse_error(error, "Expected animation duration label.");
    }
    
    Allocator *allocator = animation->allocator;
    animation->name = copy(allocator, token.string);

    if (!parse_real(tokenizer, &animation->duration, error)) {
        return false;
    }

    token = get_token(tokenizer);
    if (!check_label(&token, "num_bones")) {
        return animation_parse_error(error, "Expected animation num_bones label.");
    }
    
    if (!parse_int(tokenizer, &animation->num_bones, error)) {
        return false;
    }

    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        return animation_parse_error(error, "Expected animation_info close bracket.");
    }

    return true;
}

bool32 Animation_Loader::parse_bone_frame(Allocator *allocator, Tokenizer *tokenizer,
                                          Bone_Frame *frame, char **error) {
    Token token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return animation_parse_error(error, "Expected frame open bracket.");
    }

    bool32 has_position = false;
    bool32 has_rotation = false;
    bool32 has_scale = false;

    Vec3 position = {};
    Quaternion rotation = make_quaternion();
    Vec3 scale = { 1.0f, 1.0f, 1.0f };

    token = get_token(tokenizer);
    if (!check_label(&token, "timestamp")) {
        return animation_parse_error(error, "Expected bone label for bone channel.");
    }

    if (!parse_real(tokenizer, &frame->timestamp, error)) {
        return false;
    }

    token = get_token(tokenizer);
    while (token.type != CLOSE_BRACKET) {
        if (token.type == LABEL) {
            if (string_equals(token.string, "pos")) {
                if (!parse_vec3(tokenizer, &position, error)) {
                    return false;
                }
                has_position = true;
            } else if (string_equals(token.string, "rot")) {
                if (!parse_quaternion(tokenizer, &rotation, error)) {
                    return false;
                }
                has_rotation = true;
            } else if (string_equals(token.string, "scale")) {
                if (!parse_vec3(tokenizer, &scale, error)) {
                    return false;
                }
                has_scale = true;
            } else {
                return animation_parse_error(error, "Unknown transform label. Should be either pos, rot, or scale.");
            }
        } else {
            return animation_parse_error(error, "Expected close bracket or label (pos, rot, scale).");
        }

        token = get_token(tokenizer);
    }

    // TODO: i don't think we really want to inherit the last frame if an attribute is missing
    //       on the current frame, but we may want to make scale optional..
    if (!has_position) return animation_parse_error(error, "A bone frame was missing position.");
    if (!has_rotation) return animation_parse_error(error, "A bone frame was missing rotation.");
    //if (!has_scale) return animation_parse_error(error, "A bone frame was missing scale.");
    
    frame->local_transform = make_transform(position, rotation, scale);
    return true;
}

bool32 Animation_Loader::parse_bone_channel(Allocator *allocator, Tokenizer *tokenizer,
                                            Bone_Channel *bone_channel, char **error) {
    Token token = get_token(tokenizer);
    if (!check_label(&token, "bone")) {
        return animation_parse_error(error, "Expected bone label for bone channel.");
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return animation_parse_error(error, "Expected bone open bracket.");
    }

    token = get_token(tokenizer);
    if (!check_label(&token, "num_frames")) {
        return animation_parse_error(error, "Expected num_frames label for bone channel.");
    }

    if (!parse_int(tokenizer, &bone_channel->num_frames, error)) {
        return false;
    }

    // parse bone frames
    Bone_Frame *bone_frames = (Bone_Frame *) allocate(allocator, sizeof(Bone_Frame) * bone_channel->num_frames);
    token = get_token(tokenizer);
    if (!check_label(&token, "frames")) {
        return animation_parse_error(error, "Expected frames label for bone frames.");
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return animation_parse_error(error, "Expected bone frames open bracket.");
    }

    for (int32 i = 0; i < bone_channel->num_frames; i++) {
        if (!parse_bone_frame(allocator, tokenizer, &bone_frames[i], error)) {
            return false;
        }
    }

    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        return animation_parse_error(error, "Expected bone frames close bracket.");
    }

    // done bone channel
    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        return animation_parse_error(error, "Expected bone close bracket.");
    }

    bone_channel->allocator = allocator;
    bone_channel->frames = bone_frames;

    return true;
}

bool32 Animation_Loader::parse_bones(Tokenizer *tokenizer, Skeletal_Animation *animation, char **error) {
    Token token = get_token(tokenizer);
    if (!check_label(&token, "bones")) {
        return animation_parse_error(error, "Expected bones label.");
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return animation_parse_error(error, "Expected bones open bracket.");
    }

    Bone_Channel *bone_channels = (Bone_Channel *) allocate(animation->allocator,
                                                            sizeof(Bone_Channel) * animation->num_bones);
    for (int32 i = 0; i < animation->num_bones; i++) {
        parse_bone_channel(animation->allocator, tokenizer, &bone_channels[i], error);
    }

    animation->bone_channels = bone_channels;
    
    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        return animation_parse_error(error, "Expected bones close bracket.");
    }

    return true;
}

bool32 Animation_Loader::load_animation(Allocator *allocator, String name, String filename,
                                        Skeletal_Animation **animation_result, char **error) {
    // do all allocations on the temp region, so that we don't have to do annoying conditional
    // deletions based on what we've actually allocated up to a failure point.
    // when we fail, we just throw away the temp region. when we succeed, we just do a copy.
    // slow? maybe. when we really want to be fast, we'll just remove all error checking and
    // just replace the temp region with the actual allocator. easy.
    Allocator *temp_region = begin_region();

    File_Data file_data = platform_open_and_read_file(temp_region, filename);
    Tokenizer tokenizer = make_tokenizer(file_data);
    
    Skeletal_Animation animation = {};
    animation.allocator = temp_region;

    if (!parse_animation_info(&tokenizer, &animation, error)) {
        end_region(temp_region);
        return false;
    }
    
    if (!parse_bones(&tokenizer, &animation, error)) {
        end_region(temp_region);
        return false;
    }

    animation.name = name;
    animation.filename = filename;
    
    *animation_result = (Skeletal_Animation *) allocate(allocator, sizeof(Skeletal_Animation));
    **animation_result = copy(allocator, &animation);

    end_region(temp_region);
    return true;
}
