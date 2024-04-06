#pragma once
#include <concepts>
#include <type_traits>
#include <ranges>

namespace ccat::concepts {
	template<typename T>
	concept char_or_wchar =
		std::same_as<std::remove_cvref_t<T>, char> ||
		std::same_as<std::remove_cvref_t<T>, signed char> ||
		std::same_as<std::remove_cvref_t<T>, unsigned char> ||
		std::same_as<std::remove_cvref_t<T>, wchar_t>;

	template<typename T>
	concept character =
		char_or_wchar<T> ||
		std::same_as<std::remove_cvref_t<T>, char8_t> ||
		std::same_as<std::remove_cvref_t<T>, char16_t> ||
		std::same_as<std::remove_cvref_t<T>, char32_t>;

	namespace detail {
		template<typename T, typename Alloc>
		concept alloc_default_insertable = requires(Alloc alloc, T* p) {
			alloc.construct(p);
		};

		template<typename T, typename Alloc>
		concept alloc_copy_insertable = requires(Alloc alloc, T* p) {
			alloc.construct(p, std::declval<const T&>());
		};

		template<typename T, typename Alloc>
		concept alloc_move_insertable = requires(Alloc alloc, T* p) {
			alloc.construct(p, std::declval<T>());
		};

		template<typename T, typename Alloc, typename... Args>
		concept alloc_emplace_constructible = requires(Alloc alloc, T* p) {
			alloc.construct(p, std::declval<Args>()...);
		};

		template<typename T, typename Alloc>
		concept alloc_nothrow_move_insertable = alloc_move_insertable<T, Alloc> &&
			noexcept(std::declval<Alloc&>().construct(std::declval<T*>(), std::declval<T>()));

		template<typename T, typename Alloc>
		concept alloc_erasable = requires(Alloc alloc, T* p) {
			alloc.destroy(p);
		};
	}

	template<typename T>
	concept move_assignable = std::is_move_assignable_v<T>;

	template<typename T>
	concept copy_assignable = move_assignable<T> && std::is_copy_assignable_v<T>;

	template<typename T, typename Container>
	concept default_insertable_into = detail::alloc_default_insertable<T, typename Container::allocator_type> || std::default_initializable<T>;

	template<typename T, typename Container>
	concept copy_insertable_into = detail::alloc_copy_insertable<T, typename Container::allocator_type> || std::copy_constructible<T>;

	template<typename T, typename Container>
	concept move_insertable_into = detail::alloc_move_insertable<T, typename Container::allocator_type> || std::move_constructible<T>;

	template<typename T, typename Container>
	concept nothrow_move_insertable_into = move_insertable_into<T, Container> &&
		(detail::alloc_move_insertable<T, typename Container::allocator_type> ?
			detail::alloc_nothrow_move_insertable<T, typename Container::allocator_type> :
			std::is_nothrow_move_constructible_v<T>
		);

	template<typename T, typename Container, typename... Args>
	concept emplace_constructible_from = detail::alloc_emplace_constructible<T, typename Container::allocator_type, Args...> || std::constructible_from<T, Args...>;

	template<typename T, typename Alloc>
	concept erasable = detail::alloc_erasable<T, Alloc> || std::destructible<T>;

	template<typename Range, typename T>
	concept container_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_value_t<Range>, T>;

}
