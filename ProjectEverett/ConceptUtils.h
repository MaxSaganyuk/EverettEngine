#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

template<typename FundamentalType>
concept OnlyFundamental = std::is_fundamental_v<FundamentalType>;

template<typename FundamentalType>
concept OnlyFundamentalNotBool = std::is_fundamental_v<FundamentalType> && !std::is_same_v<FundamentalType, bool>;

template<typename EnumType>
concept OnlyEnums = std::is_enum_v<EnumType>;

template<typename Type>
struct IsGLMVec
{
	constexpr static bool Value = false;
};

template<size_t N, typename ValueType, glm::qualifier Q>
struct IsGLMVec<glm::vec<N, ValueType, Q>>
{
	constexpr static bool Value = true;
};

template<typename Type>
struct IsGLMMat
{
	constexpr static bool Value = false;
};

template<size_t N, size_t K, typename ValueType, glm::qualifier Q>
struct IsGLMMat<glm::mat<N, K, ValueType, Q>>
{
	constexpr static bool Value = true;
};

template<typename Type>
struct IsGLMQua
{
	constexpr static bool Value = false;
};

template<typename ValueType, glm::qualifier Q>
struct IsGLMQua<glm::qua<ValueType, Q>>
{
	constexpr static bool Value = true;
};

template<typename Type>
concept OnlyGLMs = IsGLMVec<Type>::Value || IsGLMMat<Type>::Value || IsGLMQua<Type>::Value;

template<typename ClassType, typename MemberFunc, typename... Params>
concept ConfirmMemberOf = requires(ClassType object, MemberFunc memberFunc, Params&&... values)
{
	(object.*memberFunc)(std::forward<Params>(values)...);
};