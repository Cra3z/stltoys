#pragma once
#include <cstring>
#include <ios>

namespace ccat {
	
	template<typename T>
	struct char_traits;
	
	template<typename CharT>
	struct char_traits_types;
	
	template<>
	struct char_traits_types<char> {
		using char_type = char;
		using int_type = int;
		using off_type = std::streamoff;
		using pos_type = std::streampos;
		using state_type = std::mbstate_t;
		using cmp_type = unsigned char;
	};
	
	template<>
	struct char_traits_types<char8_t> {
		using char_type = char8_t;
		using int_type = unsigned int;
		using off_type = std::streamoff;
		using pos_type = std::u8streampos;
		using state_type = std::mbstate_t;
		using cmp_type = char_type;
	};
	
	template<>
	struct char_traits_types<wchar_t> {
		using char_type = wchar_t;
		using int_type = std::wint_t;
		using off_type = std::streamoff;
		using pos_type = std::wstreampos;
		using state_type = std::mbstate_t;
		using cmp_type = char_type;
	};
	
	template<>
	struct char_traits_types<char16_t> {
		using char_type = char16_t;
		using int_type = std::uint_least16_t ;
		using off_type = std::streamoff;
		using pos_type = std::u16streampos;
		using state_type = std::mbstate_t;
		using cmp_type = char_type;
	};
	
	template<>
	struct char_traits_types<char32_t> {
		using char_type = char32_t;
		using int_type = std::uint_least32_t ;
		using off_type = std::streamoff;
		using pos_type = std::u32streampos;
		using state_type = std::mbstate_t;
		using cmp_type = char_type;
	};
	
	template<typename CharT>
	struct char_traits_base : char_traits_types<CharT> {
		using traits = char_traits<CharT>;
		using base = char_traits_types<CharT>;
		using typename base::char_type;
		using typename base::int_type;
		using typename base::off_type;
		using typename base::pos_type;
		using typename base::state_type;
		using typename base::cmp_type;
		CONSTEXPR static auto assign(char_type& c1, const char_type& c2) noexcept ->void {
			c1 = c2;
		}
		CONSTEXPR static auto assign(char_type* ptr, std::size_t count, char_type c) noexcept ->void {
			for (std::size_t i{}; i < count; ++i) {
				assign(ptr[i], c);
			}
		}
		CONSTEXPR static auto eq(char_type a, char_type b) noexcept ->bool {
			return static_cast<cmp_type>(a) == static_cast<cmp_type>(b);
		}
		CONSTEXPR static auto lt(char_type a, char_type b) noexcept ->bool {
			return static_cast<cmp_type>(a) <  static_cast<cmp_type>(b);
		}
		CONSTEXPR static auto move(char_type* dest, const char_type* src, std::size_t count) noexcept ->char_type* {
			if (std::is_constant_evaluated()) {
				if (dest > src) {
					while (count > 0) {
						dest[count - 1] = src[count - 1];
						--count;
					}
				}
				else {
					for (std::size_t i{}; i < count; ++i) {
						dest[i] = src[i];
					}
				}
				return dest;
			}
			else return static_cast<char_type*>(std::memmove(dest, src, count * sizeof(char_type)));
		}
		CONSTEXPR static auto copy(char_type* dest, const char_type* src, std::size_t count) noexcept ->char_type* {
			if (std::is_constant_evaluated()) {
				for (std::size_t i{}; i < count; ++i) {
					dest[i] = src[i];
				}
				return dest;
			}
			else return static_cast<char_type*>(std::memcpy(dest, src, count * sizeof(char_type)));
		}
		CONSTEXPR static auto default_compare(const char_type* s1, const char_type* s2, std::size_t count) noexcept ->int {
			for (std::size_t i{}; i < count; ++i) {
				if (auto cmp_res = to_int_type(s1[i]) - to_int_type(s2[i]); cmp_res != 0) return cmp_res;
			}
			return 0;
		}
		CONSTEXPR static auto to_int_type(char_type c) noexcept ->int_type {
			return static_cast<int_type>(c);
		}
		CONSTEXPR static auto to_char_type(int_type i) noexcept ->char_type {
			return static_cast<char_type>(i);
		}
		CONSTEXPR static auto eq_int_type(int_type c1, int_type c2) noexcept ->bool {
			return c1 == c2;
		}
		CONSTEXPR static auto not_eof(int_type e) noexcept ->int_type {
			return eq_int_type(e, traits::eof()) ? !traits::eof() : e;
		}
	};
	
	template<>
	struct char_traits<char> : char_traits_base<char> {
		CONSTEXPR static auto compare(const char_type* s1, const char_type* s2, std::size_t count) noexcept ->int {
			if (std::is_constant_evaluated()) return default_compare(s1, s2, count);
			else return std::strncmp(s1, s2, count);
		}
		CONSTEXPR static auto length(const char_type* s) noexcept ->std::size_t {
			if (std::is_constant_evaluated()) {
				std::size_t len{};
				while (s && *s != '\0') {
					++s;
					++len;
				}
				return len;
			}
			else return std::strlen(s);
		}
		CONSTEXPR static auto find(const char_type* ptr, std::size_t count, const char_type& ch) noexcept ->const char_type* {
			if (ptr == nullptr) return nullptr;
			for (std::size_t i{}; i < count; ++i) {
				if (ptr[i] == ch) return ptr + i;
			}
			return nullptr;
		}
		CONSTEXPR static auto eof() noexcept ->int_type {
			return static_cast<int_type>(EOF);
		}
	};
	
	template<>
	struct char_traits<wchar_t> : char_traits_base<wchar_t> {
		CONSTEXPR static auto compare(const char_type* s1, const char_type* s2, std::size_t count) noexcept ->int {
			if (std::is_constant_evaluated()) return default_compare(s1, s2, count);
			else return std::wcsncmp(s1, s2, count);
		}
		CONSTEXPR static auto length(const char_type* s) noexcept ->std::size_t {
			if (std::is_constant_evaluated()) {
				std::size_t len{};
				while (s && *s != '\0') {
					++s;
					++len;
				}
				return len;
			}
			else return std::wcslen(s);
		}
		CONSTEXPR static auto find(const char_type* ptr, std::size_t count, const char_type& ch) noexcept ->const char_type* {
			if (ptr == nullptr) return nullptr;
			for (std::size_t i{}; i < count; ++i) {
				if (ptr[i] == ch) return ptr + i;
			}
			return nullptr;
		}
		CONSTEXPR static auto eof() noexcept ->int_type {
			return static_cast<int_type>(WEOF);
		}
	};
}