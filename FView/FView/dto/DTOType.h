#pragma once

namespace dxlib {
namespace dto {

///-------------------------------------------------------------------------------------------------
/// <summary> A vector 3. </summary>
///
/// <remarks> Xian Dai, 2018/1/23. </remarks>
///-------------------------------------------------------------------------------------------------
struct Vector3
{
    float x{0};
    float y{0};
    float z{0};
};

///-------------------------------------------------------------------------------------------------
/// <summary> A quaternion. </summary>
///
/// <remarks> Xian Dai, 2018/1/23. </remarks>
///-------------------------------------------------------------------------------------------------
struct Quaternion
{
    float x{0};
    float y{0};
    float z{0};
    float w{1};
};

} // namespace dto
} // namespace dxlib