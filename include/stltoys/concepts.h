#pragma once
#include <concepts>
#include <type_traits>
#include <memory>
#include <memory_resource>

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
	
	template<typename Alloc>
	concept allocator = requires (Alloc alloc) {
		std::allocator_traits<Alloc>::allocate(alloc, std::declval<typename std::allocator_traits<Alloc>::size_type>());
	};
	
	template<typename Alloc, typename... Args>
	concept allocator_which_can_construct_object = allocator<Alloc> && (!std::same_as<Alloc, std::pmr::polymorphic_allocator<typename std::allocator_traits<Alloc>::value_type>>) && requires (Alloc alloc, typename std::allocator_traits<Alloc>::pointer p) {
		alloc.construct(p, std::declval<Args>()...);
	};
	
	template<typename Alloc>
	concept allocator_which_can_destroy_object = allocator<Alloc> && (!std::same_as<Alloc, std::pmr::polymorphic_allocator<typename std::allocator_traits<Alloc>::value_type>>) && requires (Alloc alloc, typename std::allocator_traits<Alloc>::pointer p) {
		alloc.destroy(p);
	};
}