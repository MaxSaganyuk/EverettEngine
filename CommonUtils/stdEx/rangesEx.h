#ifndef RANGESEX
#define RANGESEX

#if __cplusplus >= 202002L || _HAS_CXX20

namespace stdEx
{
	namespace views
	{
		struct _AntiFilterFunctor
		{
			template<typename Predicate>
			constexpr static auto operator()(Predicate&& predicate)
				noexcept(std::is_nothrow_constructible_v<std::decay_t<Predicate>, Predicate>)
			{
				return std::views::filter([predicate = std::forward<Predicate>(predicate)](auto& value) -> bool {
					return !predicate(value);
				});
			}
		};

		constexpr _AntiFilterFunctor antifilter;
	}
}

#endif
#endif