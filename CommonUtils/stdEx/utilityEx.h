#ifndef UTILITYEX
#define UTILITYEX

#include <cassert>
#include <utility>
#include <atomic>
#include <vector>
#include <set>

#if __cplusplus >= 201703L || _HAS_CXX17
#include <optional>
#endif

#if __cplusplus >= 202002L || _HAS_CXX20
#include <iostream>
#include <array>
#endif

namespace stdEx
{
#if __cplusplus >= 202002L || _HAS_CXX20

	namespace _stdEx
	{
		template<typename MArray, size_t rank, size_t n>
		constexpr void GetExtent(std::array<size_t, rank>& extent)
		{
			extent[n] = std::extent_v<MArray, n>;

			if constexpr (n + 1 < rank)
			{
				GetExtent<MArray, rank, n + 1>(extent);
			}
		}

		template<typename MArray>
		constexpr auto GetDimensionExtent()
		{
			constexpr size_t rank = std::rank_v<MArray>;
			std::array<size_t, rank> extent;

			GetExtent<MArray, rank, 0>(extent);

			return extent;
		}

		template<typename MArray, size_t rank, size_t n>
		void PrintNDimension(MArray& mArray, std::array<size_t, rank> extent)
		{
			for (size_t i = 0; i < extent[n]; ++i)
			{
				if constexpr (n + 1 < rank)
				{
					std::cout << '{';
					PrintNDimension<std::remove_extent_t<MArray>, rank, n + 1>(mArray[i], extent);
					std::cout << '}';
				}
				else
				{
					std::cout << mArray[i];
					if (i < extent[n] - 1)
					{
						std::cout << ' ';
					}
				}
			}
		}
	}

	template<typename MArray>
	constexpr void PrintMArray(MArray& mArray)
	{
		using namespace _stdEx;

		constexpr auto extent = GetDimensionExtent<MArray>();

		PrintNDimension<MArray, extent.size(), 0>(mArray, extent);
	}

#endif
	template<typename Type>
	class ValWithBackup
	{
#if __cplusplus >= 201703L || _HAS_CXX17
		std::optional<Type> value;
#else
		template<typename Type>
		class PseudoOptional
		{
			Type val;
			bool check;

		public:
			PseudoOptional()
				: val(Type{}), check(false) {
			}

			bool has_value() const
			{
				return check;
			}

			Type value() const
			{
				return val;
			}

			Type& value()
			{
				return val;
			}

			PseudoOptional& operator=(Type&& value)
			{
				check = true;
				val = std::move(value);

				return *this;
			}

			PseudoOptional& operator=(const PseudoOptional& pseudoOpt)
			{
				val = pseudoOpt.val;
				check = pseudoOpt.check;

				return *this;
			}
		};

		PseudoOptional<Type> value;
#endif
		Type* backup;

	public:
		void ResetValue()
		{
#if __cplusplus >= 201703L || _HAS_CXX17
			this->value = std::optional<Type>();
#else
			this->value = PseudoOptional<Type>();
#endif
		}

		ValWithBackup(Type* backup = nullptr)
			: backup(backup) {
		}

		void ResetBackup(Type* backup = nullptr)
		{
			this->backup = backup;
		}

		bool HasValue() const
		{
			return value.has_value();
		}

		ValWithBackup& operator=(const ValWithBackup& valWb)
		{
			value = valWb.value;
			backup = valWb.backup;

			return *this;
		}

		ValWithBackup& operator=(Type&& value)
		{
			this->value = std::move(value);

			return *this;
		}

		operator Type() const
		{
			assert(backup && "Backup value is nullptr");
			return value.has_value() ? value.value() : *backup;
		}

		operator Type& ()
		{
			assert(backup && "Backup value is nullptr");
			return value.has_value() ? value.value() : *backup;
		}
	};


	enum class ObjectCounterType
	{
		General,
		Stack,
		Heap
	};

	template<typename Type>
	class ObjectCounter
	{
	private:

		struct ObjectCounterInfo
		{
			size_t created;
			size_t active;

			ObjectCounterInfo& operator++()
			{
				++created;
				++active;

				return *this;
			}
		};

		struct ObjectCounterStorage
		{
			ObjectCounterInfo generalCounter;
			ObjectCounterInfo stackCounter;
			ObjectCounterInfo heapCounter;
		};

		static ObjectCounterStorage allThreadStorage;
		static thread_local ObjectCounterStorage localThreadStorage;

		static void UpdateLocalThreadStorage()
		{
			localThreadStorage = allThreadStorage;
		}

		static thread_local bool stackCheck;

		static ObjectCounterInfo& GetCorrectStorageAndInfo(ObjectCounterType objType, bool localThread)
		{
			if (localThread)
			{
				UpdateLocalThreadStorage();
			}

			return GetCorrectInfo(objType, localThread ? localThreadStorage : allThreadStorage);
		}

		static ObjectCounterInfo& GetCorrectInfo(ObjectCounterType objType, ObjectCounterStorage& objStorage)
		{
			switch (objType)
			{
			case ObjectCounterType::General:
				return objStorage.generalCounter;
			case ObjectCounterType::Stack:
				return objStorage.stackCounter;
			case ObjectCounterType::Heap:
				return objStorage.heapCounter;
			default:
				assert(false && "Unreachable");
			}
		}

		ObjectCounter(std::nullptr_t) {}

	public:

		static size_t GetCreatedObjAmount(ObjectCounterType objType, bool localThread = false)
		{
			return GetCorrectStorageAndInfo(objType, localThread).created;
		}

		static size_t GetActiveObjAmount(ObjectCounterType objType, bool localThread = false)
		{
			return GetCorrectStorageAndInfo(objType, localThread).active;
		}

		ObjectCounter()
		{
			++allThreadStorage.generalCounter;

			if (stackCheck)
			{
				++allThreadStorage.stackCounter;
			}

			stackCheck = true;
		}

		ObjectCounter(const ObjectCounter&)
			: ObjectCounter() {
		}

		void* operator new(size_t amount)
		{
			stackCheck = false;
			++allThreadStorage.heapCounter;
			return ::new ObjectCounter(nullptr);
		}

		void operator delete(void* pointer)
		{
			++allThreadStorage.stackCounter.active;
			--allThreadStorage.heapCounter.active;

			::delete(pointer);
		}

	protected:
		virtual ~ObjectCounter()
		{
			--allThreadStorage.generalCounter.active;
			--allThreadStorage.stackCounter.active;
		}
	};

	template<typename Type>
	typename ObjectCounter<Type>::ObjectCounterStorage ObjectCounter<Type>::allThreadStorage = { 0, 0, 0 };

	template<typename Type>
	typename thread_local ObjectCounter<Type>::ObjectCounterStorage ObjectCounter<Type>::localThreadStorage = { 0, 0, 0 };

	template<typename Type>
	bool thread_local ObjectCounter<Type>::stackCheck = true;

#define TrackedClass(className) class className : public stdEx::ObjectCounter<className>

	enum class RelationType
	{
		NoRelation,   // A  x  B
		LeftToRight,  // A  -> B
		RightToLeft,  // A <-  B
		Bidirectional // A <-> B
	};

	template<typename Type>
	class RelationGraph
	{
		struct Element;

		class SharedPtrValueComparator
		{
		public:
			using is_transparent = void;

			bool operator()(const std::shared_ptr<Element>& left, const std::shared_ptr<Element>& right) const
			{
				return *left < *right;
			}

			bool operator()(const Element& left, const std::shared_ptr<Element>& right) const
			{
				return left < *right;
			}

			bool operator()(const std::shared_ptr<Element>& left, const Element& right) const
			{
				return *left < right;
			}
		};

		class WeakPtrValueComparator
		{
		public:
			using is_transparent = void;

			bool operator()(const std::weak_ptr<Element>& left, const std::weak_ptr<Element>& right) const
			{
				if (right.expired())
				{
					return false;
				}
				if (left.expired())
				{
					return true;
				}

				return *left.lock() < *right.lock();
			}
		};

		struct Element
		{
			Type value;
			std::set<std::weak_ptr<Element>, WeakPtrValueComparator> relations;

			bool operator<(const Element& otherElement) const
			{
				return value < otherElement.value;
			}
		};

		std::set<std::shared_ptr<Element>, SharedPtrValueComparator> elements;
	public:

		void AddElements(Type firstValue, Type secondValue, RelationType relationType)
		{
			const Element protoLeftElement = { firstValue };
			const Element protoRightElement = { secondValue };

			auto leftIter = elements.find(protoLeftElement);
			auto rightIter = elements.find(protoRightElement);

			if (leftIter == elements.end())
			{
				leftIter = elements.insert(std::make_shared<Element>(protoLeftElement)).first;
			}
			if (rightIter == elements.end())
			{
				rightIter = elements.insert(std::make_shared<Element>(protoRightElement)).first;
			}

			auto leftElement = *leftIter;
			auto rightElement = *rightIter;

			if (relationType == RelationType::Bidirectional || relationType == RelationType::LeftToRight)
			{
				leftElement->relations.insert(rightElement);
			}
			if (relationType == RelationType::Bidirectional || relationType == RelationType::RightToLeft)
			{
				rightElement->relations.insert(leftElement);
			}
		}

		void RemoveElement(Type value)
		{
			elements.erase(Element{ value });
		}

		std::vector<Type> GetValuesRelatedTo(Type value)
		{
			std::vector<Type> values;
			values.reserve(elements.size());

			auto elementIter = elements.find(Element{ value });

			if (elementIter != elements.end())
			{
				auto element = *elementIter;
				auto& relations = element->relations;

				for (auto relationIter = relations.begin(); relationIter != relations.end();)
				{
					if (!relationIter->expired())
					{
						values.push_back(relationIter->lock()->value);
						++relationIter;
					}
					else
					{
						relationIter = relations.erase(relationIter);
					}
				}
			}

			return values;
		}
	};

}
#endif