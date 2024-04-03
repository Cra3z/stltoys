#pragma once
#include <concepts>
#include <type_traits>

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
}