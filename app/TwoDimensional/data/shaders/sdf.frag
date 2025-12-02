#version 450

layout(binding = 1) uniform sampler2D atlas;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

//void main()
//{
//    outColor.rgb = vec3(1.0, 1.0, 1.0);
//    outColor.a = texture(atlas, fragTexCoord).r;
//}
//

void main()
{
    vec4 base_color = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 outline_color = vec4(0.0, 0.0, 0.0, 1.0);
    float dist_alpha_mask = texture(atlas, fragTexCoord).r;

    float outline_min_value0 = 0.5;
    float outline_min_value1 = 0.5;
    float outline_max_value0 = 0.55;
    float outline_max_value1 = 0.55;
    float outline_factor = 1.0;

    if(dist_alpha_mask >= outline_min_value0 && dist_alpha_mask <= outline_max_value1) {
      if (dist_alpha_mask <= outline_min_value1) {
        outline_factor = smoothstep(outline_min_value0, outline_min_value1, dist_alpha_mask);
      } else {
        outline_factor = smoothstep(outline_max_value1, outline_max_value0, dist_alpha_mask);
      }
      base_color = mix(outline_color, base_color, outline_factor);
    }

//    //I'll let you manage the option as you like, it's a push constant for me
//    int soft_edges = 2;
//    float soft_edge_min = 0.55;
//    float soft_edge_max = 0.6;
//
//    if (soft_edges == 1) {
//      base_color.a *= smoothstep(soft_edge_min, soft_edge_max, dist_alpha_mask);
//    } else if (dist_alpha_mask < 0.5) {
//      base_color.a = 0.0;
//    }
//
//    //I'll let you manage the option as you like, it's a push constant for me
//    int outer_glow = 1;
//
//    if (outer_glow == 1) {
//      vec4 outer_glow_color = vec4(0.0, 0.0, 1.0, 0.2);
//      float outer_glow_min_dvalue = 0.40;
//      float outer_glow_max_dvalue = 0.5;
//
//    //the glow part is poorly done, I don't have texture / mask for it
//      if (dist_alpha_mask >= outer_glow_min_dvalue && dist_alpha_mask <= outer_glow_max_dvalue) {
//        float outer_glow_factor = smoothstep(outer_glow_min_dvalue, outer_glow_max_dvalue, dist_alpha_mask);
//        base_color = mix(outer_glow_color, outline_color, outer_glow_factor);
//      }
//    }

    outColor = base_color;
}