#pragma once
#include "Serialize.h"

namespace dxlib {
namespace dto {

class FViewRT
{
  public:
    /// <summary> fView相机坐标. </summary>
    Vector3 viewPosition;

    /// <summary> fView相机旋转. </summary>
    Quaternion viewRotation;

    /// <summary> (逆)fView相机坐标. </summary>
    Vector3 iViewPosition;

    /// <summary> (逆)fView相机旋转. </summary>
    Quaternion iViewRotation;

    std::wstring toJson() const
    {
        using namespace rapidjson;
        using namespace dxlib::json;

        auto doc = JsonHelper::creatEmptyObjectDocW();
        auto& allocator = doc.GetAllocator();

        Serialize::AddMember(doc, L"viewPosition", viewPosition, allocator);
        Serialize::AddMember(doc, L"viewRotation", viewRotation, allocator);
        Serialize::AddMember(doc, L"iViewPosition", iViewPosition, allocator);
        Serialize::AddMember(doc, L"iViewRotation", iViewRotation, allocator);
        return JsonHelper::toStr(doc);
    }

    void readJson(const rapidjson::DocumentW& doc)
    {
        using namespace rapidjson;

        dxlib::json::Serialize::GetObj(doc[L"viewPosition"], viewPosition);
        dxlib::json::Serialize::GetObj(doc[L"viewRotation"], viewRotation);
        dxlib::json::Serialize::GetObj(doc[L"iViewPosition"], iViewPosition);
        dxlib::json::Serialize::GetObj(doc[L"iViewRotation"], iViewRotation);
    }
};
} // namespace dto
} // namespace dxlib