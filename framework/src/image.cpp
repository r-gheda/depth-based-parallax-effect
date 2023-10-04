#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image.h"

#include <algorithm>

template <>
float stbToType<float>(const stbi_uc* src) { 
    return float(*src) / 255.0f; 
}

template <>
glm::vec3 stbToType<glm::vec3>(const stbi_uc* src) { 
    return glm::vec3(float(src[0]), float(src[1]), float(src[2])) / 255.0f;
}

template <>
float stbfToType<float>(const float* src)
{
    return *src;
}

template <>
glm::vec3 stbfToType<glm::vec3>(const float* src)
{
    return glm::vec3(src[0], src[1], src[2]);
}




inline auto floatToStb(const float value)
{
    return stbi_uc(std::min<float>(std::max<float>(value, 0.0f), 1.0f) * 255);
}

template<>
void typeToRgbUint8(stbi_uc* dst, const float& value)
{
    auto f_val = floatToStb(value);
    dst[0] = f_val;
    dst[1] = f_val;
    dst[2] = f_val;
}

template <>
void typeToRgbUint8(stbi_uc* dst, const glm::vec3& value) {
    dst[0] = floatToStb(value.r);
    dst[1] = floatToStb(value.g);
    dst[2] = floatToStb(value.b);
}
