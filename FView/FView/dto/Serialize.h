#pragma once
#include "DTOType.h"

#include "JsonHelper.h"

namespace dxlib {
namespace json {

///-------------------------------------------------------------------------------------------------
/// <summary> 使用rapidjson扩展的一些固定序列化/反序列化方法. </summary>
///
/// <remarks> 使用示例:
///           //1. 添加一个成员
///           Serialize::AddMember(value, L"name", object, doc.GetAllocator());
///
///           //2. 从一个value得到一个对象
///           GetObj(value, obj);
///
///           //3. 直接去得到一个对象作为返回值
///           cv::Mat m = GetCvMat(value);
///
///           //4. 直接序列化到doc
///           auto doc = ObjectDoc(m);
///
///           //5. 反序列化到Mat
///           cv::Mat m2 = Serialize::GetCvMat(v);
///
///           Dx, 2019/3/11. </remarks>
///-------------------------------------------------------------------------------------------------
class Serialize
{
  public:
#pragma region std_array

    template <size_t _Size>
    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const std::array<std::wstring, _Size>& obj, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW jv(kArrayType);
        for (size_t i = 0; i < obj.size(); i++) {
            ValueW item; //string的话必须这样搞一下然后拷贝,它这个库分两种思路,ref的和拷贝的
            item.SetString(obj[i].c_str(), allocator);
            jv.PushBack(item, allocator);
        }
        value.AddMember(fieldName, jv, allocator);
    }

    template <typename T, size_t _Size>
    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const std::array<T, _Size>& arr, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW data(kArrayType);
        for (size_t i = 0; i < arr.size(); i++)
            data.PushBack(arr[i], allocator);
        value.AddMember(fieldName, data, allocator);
    }

    //->json 整个对象是纯array
    template <typename T, size_t _Size>
    static inline rapidjson::DocumentW ArrayDoc(const std::array<T, _Size>& arr)
    {
        rapidjson::DocumentW doc;
        doc.SetArray();
        auto& allocator = doc.GetAllocator();
        for (size_t i = 0; i < arr.size(); i++)
            doc.PushBack(arr[i], allocator);
        return doc;
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<double, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetDouble();
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<float, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetFloat();
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<int, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetInt();
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<unsigned int, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetUint();
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<bool, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetBool();
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    template <size_t _Size>
    static inline void GetObj(const rapidjson::ValueW& value, std::array<std::wstring, _Size>& obj)
    {
        if (!value.IsArray()) //它应该是一个array
            return;
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj[i] = value[i].GetString();
        }
    }

#pragma endregion

#pragma region vector

    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const std::vector<std::wstring>& obj, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW jv(kArrayType);
        for (size_t i = 0; i < obj.size(); i++) {
            ValueW item; //string的话必须这样搞一下然后拷贝,它这个库分两种思路,ref的和拷贝的
            item.SetString(obj[i].c_str(), allocator);
            jv.PushBack(item, allocator);
        }
        value.AddMember(fieldName, jv, allocator);
    }

    template <typename T>
    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const std::vector<T>& obj, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW jv(kArrayType);
        for (size_t i = 0; i < obj.size(); i++) {
            jv.PushBack(obj[i], allocator);
        }
        value.AddMember(fieldName, jv, allocator);
    }

    //->json 整个对象是纯array
    static inline rapidjson::DocumentW ArrayDoc(const std::vector<std::wstring>& obj)
    {
        rapidjson::DocumentW doc;
        doc.SetArray();
        auto& allocator = doc.GetAllocator();
        for (size_t i = 0; i < obj.size(); i++) {
            rapidjson::ValueW item;
            item.SetString(obj[i].c_str(), allocator);
            doc.PushBack(item, allocator);
        }
        return doc;
    }

    //obj -> json 整个对象是纯array
    template <typename T>
    static inline rapidjson::DocumentW ArrayDoc(const std::vector<T>& obj)
    {
        rapidjson::DocumentW doc;
        doc.SetArray();
        auto& allocator = doc.GetAllocator();
        for (size_t i = 0; i < obj.size(); i++) {
            rapidjson::ValueW item;
            doc.PushBack(obj[i], allocator);
        }
        return doc;
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, std::vector<std::wstring>& obj)
    {
        obj.clear();
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj.push_back(value[i].GetString());
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, std::vector<double>& obj)
    {
        obj.clear();
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj.push_back(value[i].GetDouble());
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, std::vector<float>& obj)
    {
        obj.clear();
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj.push_back(value[i].GetFloat());
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, std::vector<int>& obj)
    {
        obj.clear();
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj.push_back(value[i].GetInt());
        }
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, std::vector<bool>& obj)
    {
        obj.clear();
        for (unsigned int i = 0; i < value.Size(); i++) {
            obj.push_back(value[i].GetBool());
        }
    }

#pragma endregion

#pragma region string

    static inline void AddMember(rapidjson::ValueW& value, const rapidjson::GenericStringRef<wchar_t>& name, const std::wstring& str, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW valStr;
        valStr.SetString(str.c_str(), allocator); //有val的方式是拷贝一次string的方式 ,
        value.AddMember(name, valStr, allocator);

        //不拷贝的方式是使用StringRef
        // bid.AddMember("adm", rapidjson::StringRef(html_snippet.c_str(),html_snippet.size()), allocator);
    }

    static inline void AddMember(rapidjson::ValueW& value, const rapidjson::GenericStringRef<wchar_t>& name, const std::string& str, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        std::wstring wstr = JsonHelper::utf8To16(str);
        ValueW valStr;
        valStr.SetString(wstr.c_str(), allocator);
        value.AddMember(name, valStr, allocator);
    }

#pragma endregion

#pragma region Vector3

    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const dxlib::dto::Vector3& obj, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW jv(kObjectType);
        jv.AddMember(L"x", obj.x, allocator);
        jv.AddMember(L"y", obj.y, allocator);
        jv.AddMember(L"z", obj.z, allocator);
        value.AddMember(fieldName, jv, allocator);
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, dxlib::dto::Vector3& obj)
    {
        obj.x = value[L"x"].GetFloat();
        obj.y = value[L"y"].GetFloat();
        obj.z = value[L"z"].GetFloat();
    }

    //->obj 带返回值的直接GetObj
    static inline dxlib::dto::Vector3 GetDtoVector3(const rapidjson::ValueW& value)
    {
        dxlib::dto::Vector3 obj;
        obj.x = value[L"x"].GetFloat();
        obj.y = value[L"y"].GetFloat();
        obj.z = value[L"z"].GetFloat();
        return obj;
    }

#pragma endregion

#pragma region Quaternion

    static inline void AddMember(rapidjson::ValueW& value,
                                 const rapidjson::GenericStringRef<wchar_t>& fieldName,
                                 const dxlib::dto::Quaternion& obj, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        using namespace rapidjson;
        ValueW jv(kObjectType);
        jv.AddMember(L"x", obj.x, allocator);
        jv.AddMember(L"y", obj.y, allocator);
        jv.AddMember(L"z", obj.z, allocator);
        jv.AddMember(L"w", obj.w, allocator);
        value.AddMember(fieldName, jv, allocator);
    }

    //->obj 不带返回值的,参数作为结果的GetObj
    static inline void GetObj(const rapidjson::ValueW& value, dxlib::dto::Quaternion& obj)
    {
        obj.x = value[L"x"].GetFloat();
        obj.y = value[L"y"].GetFloat();
        obj.z = value[L"z"].GetFloat();
        obj.w = value[L"w"].GetFloat();
    }

    //->obj 带返回值的直接GetObj
    static inline dxlib::dto::Quaternion GetDtoQuaternion(const rapidjson::ValueW& value)
    {
        dxlib::dto::Quaternion obj;
        obj.x = value[L"x"].GetFloat();
        obj.y = value[L"y"].GetFloat();
        obj.z = value[L"z"].GetFloat();
        obj.w = value[L"w"].GetFloat();
        return obj;
    }

#pragma endregion
};
} // namespace json
} // namespace dxlib