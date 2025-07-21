#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <unordered_map>
#include <typeindex>
#include <functional>

class LGLUniformHasher
{
private:
	using ShaderProgID = unsigned int;
	using Location = int;
	using Hash = size_t;

	std::unordered_map<ShaderProgID, std::unordered_map<Location, Hash>> uniformHashes;

	constexpr static size_t FNVOffsetBasis = 0xcbf29ce484222325ull;
	constexpr static size_t FNVPrime = 0x100000001b3ull;

	inline size_t ConvertToULL(const int value)
	{
		return static_cast<size_t>(value);
	}

	inline size_t ConvertToULL(const unsigned int value)
	{
		return static_cast<size_t>(value);
	}

	inline size_t ConvertToULL(const float value)
	{
		size_t res = 0;

		std::memcpy(&res, &value, sizeof(float));

		return res;
	}

	// Hashing values smaller than Hash size is excessive, storing them in full
	Hash HashValue(int valueToHash)
	{
		static_assert(
			(sizeof(int) <= sizeof(Hash)), 
			"Critical, fundamental is somehow larger than unsigned long long. How..?"
		);

		return ConvertToULL(valueToHash);
	}

	Hash HashValue(unsigned int valueToHash)
	{
		static_assert(
			(sizeof(unsigned int) <= sizeof(Hash)), 
			"Critical, fundamental is somehow larger than unsigned long long. How..?"
		);

		return ConvertToULL(valueToHash);
	}

	Hash HashValue(float valueToHash)
	{
		static_assert(
			(sizeof(float) <= sizeof(Hash)), 
			"Critical, fundamental is somehow larger than unsigned long long. How..?"
		);

		return ConvertToULL(valueToHash);
	}

	// vec2 have 8 bytes of size which is equivalent to size_t.
	// Hashing is excessive and simple storage with padding is faster
	Hash HashValue(const glm::ivec2& valuesToHash)
	{
		Hash pseudoHash = 0;

		pseudoHash += valuesToHash[0];
		pseudoHash += (static_cast<Hash>(valuesToHash[1]) << (8 * sizeof(int)));

		return pseudoHash;
	}

	Hash HashValue(const glm::uvec2& valuesToHash)
	{
		Hash pseudoHash = 0;

		pseudoHash += valuesToHash[0];
		pseudoHash += (static_cast<Hash>(valuesToHash[1]) << (8 * sizeof(unsigned int)));

		return pseudoHash;
	}

	Hash HashValue(const glm::vec2& valuesToHash)
	{
		size_t pseudoHash = 0;
		size_t inter = 0;

		std::memcpy(&pseudoHash, &valuesToHash[0], sizeof(float));
		std::memcpy(&inter, &valuesToHash[1], sizeof(float));
		pseudoHash += (inter << (8 * sizeof(float)));

		return pseudoHash;
	}

	// For the rest of the types: preform real hashing
	// FNV-1a is used https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
	// No need for encryption, occational collisions are fine, speed is the main priority here
	template<typename LargerGLMType>
	Hash HashValue(const LargerGLMType& valuesToHash)
	{
		Hash hash = FNVOffsetBasis;
		const LargerGLMType::value_type* firstValuePtr = glm::value_ptr(valuesToHash);

		for (size_t i = 0; i < sizeof(LargerGLMType) / sizeof(*firstValuePtr); ++i)
		{
			hash ^= ConvertToULL(*(firstValuePtr + i));
			hash *= FNVPrime;
		}

		return hash;
	}

	// Hashing vectors might get too slow, returning ~0, which indicates - send
	template<typename Type>
	Hash HashValue(const std::vector<Type>& vectOfValuesToHash)
	{
		return ~Hash{};
	}

public:
	template<typename Type>
	bool CheckIfDiffersAndHashValue(const ShaderProgID shaderProgID, const Location uniformLocation, const Type& value)
	{
		Hash hash = HashValue(value);

		if (uniformHashes.find(shaderProgID) == uniformHashes.end())
		{
			uniformHashes[shaderProgID] = std::unordered_map<Location, Hash>{};
		}

		auto& currentShaderUniformHashes = uniformHashes[shaderProgID];

		if (hash == ~Hash{} || 
			currentShaderUniformHashes.find(uniformLocation) == currentShaderUniformHashes.end() ||
			currentShaderUniformHashes[uniformLocation] != hash)
		{
			currentShaderUniformHashes[uniformLocation] = hash;
			return true;
		}

		return false;
	}

	void ResetHashesByShader(const ShaderProgID shaderProgID)
	{
		bool validShaderName = uniformHashes.find(shaderProgID) != uniformHashes.end();

		if (validShaderName)
		{
			uniformHashes[shaderProgID].clear();
		}
	}

	void ResetHasher()
	{
		uniformHashes.clear();
	}
};
