#pragma once
#include "iterator.h"
#include "char_traits.h"

namespace ccat {
	template<typename CharT, typename Traits = char_traits<CharT>, typename Alloc = std::allocator<CharT>>
	class basic_string;
	
	namespace detail {
		template<typename CharT, typename Traits, typename Alloc>
		class basic_string_base;
		
		template<bool Mutable, typename CharT, typename Traits>
		class basic_string_view_like;
		
		template<typename T, bool Mutable, typename CharT, typename Traits>
		concept exists_user_defined_non_ctor_convertion_to_basic_string_view_like = requires (std::remove_cvref_t<T> d) {
			d.operator basic_string_view_like<Mutable, CharT, Traits>();
		};
		
		template<bool Mutable, typename CharT, typename Traits = char_traits<CharT>>
		class basic_string_view_like {
			template<typename CharT_, typename Traits_, typename Alloc>
			friend class ccat::basic_string;
			
			template<typename CharT_, typename Traits_, typename Alloc>
			friend class basic_string_base;
		public:
			using value_type = CharT;
			using traits_type = Traits;
			using readonly_view = basic_string_view_like<false, value_type, traits_type>;
			using pointer = value_type*;
			using const_pointer = const value_type*;
			using reference = value_type&;
			using const_reference = const value_type&;
			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;
			using iterator = detail::generic_contiguous_iterator<Mutable, value_type>;
			using const_iterator = detail::generic_contiguous_iterator<false, value_type>;
			using reverse_iterator = std::reverse_iterator<iterator>;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		private:
			using iter_pointer_type_ = std::conditional_t<Mutable, pointer, const_pointer>;
			using iter_reference_type_ = std::conditional_t<Mutable, reference, const_reference>;
		private:
			template<bool FromLeftToRight>
			static CONSTEXPR auto brute_force_search(readonly_view txt, readonly_view pat) noexcept ->size_type {
				if (txt.size() < pat.size()) return npos;
				size_type last = txt.size() - pat.size();
				for (size_type i = 0; i <= last; ++i) {
					size_type idx = FromLeftToRight ? i : last - i;
					bool skip = false;
					for (size_type j{}; j < pat.size(); ++j) {
						if (txt[idx + j] != pat[j]) {
							skip = true;
							break;
						}
					}
					if (!skip) return idx;
				}
				return npos;
			}
			
		public: // constructors and destructors
			CONSTEXPR basic_string_view_like() noexcept = default;
			CONSTEXPR basic_string_view_like(std::nullptr_t) = delete;
			CONSTEXPR basic_string_view_like(const basic_string_view_like& other) noexcept = default;
			CONSTEXPR basic_string_view_like(iter_pointer_type_ s, size_type count) noexcept : beg_(s), end_(s + count) {}
			CONSTEXPR basic_string_view_like(iter_pointer_type_ s) noexcept : basic_string_view_like(s, traits_type::length(s)) {}
			template<typename It, typename Sent> requires
				std::contiguous_iterator<It> && std::sentinel_for<Sent, It> &&
				    std::same_as<std::iter_value_t<It>, value_type> && (!std::convertible_to<Sent, size_type>)
			CONSTEXPR basic_string_view_like(It first, Sent last) noexcept(noexcept(std::to_address(first)) && noexcept(std::ranges::distance(first, last))) : basic_string_view_like(std::to_address(first), std::ranges::distance(first, last)) {}
			
			template<typename Rng> requires (!std::same_as<std::remove_cvref_t<Rng>, basic_string_view_like>) &&
				std::ranges::contiguous_range<Rng> && std::ranges::sized_range<Rng> &&
				std::same_as<std::ranges::range_value_t<Rng>, value_type> &&
				(!std::convertible_to<Rng, const_pointer>) &&
				(!exists_user_defined_non_ctor_convertion_to_basic_string_view_like<Rng, Mutable, value_type, traits_type>)
			CONSTEXPR explicit basic_string_view_like(Rng&& rng) noexcept(noexcept(std::to_address(std::ranges::begin(rng))) && noexcept(std::ranges::size(rng))) : basic_string_view_like(std::to_address(std::ranges::begin(rng)), std::ranges::size(rng)) {}
			
			template<bool OtherMutable> requires (!Mutable && OtherMutable)
			CONSTEXPR basic_string_view_like(basic_string_view_like<OtherMutable, value_type, traits_type> other) noexcept : basic_string_view_like(other.data(), other.size()) {}
			
			template<bool OtherMutable> requires (Mutable && !OtherMutable)
			CONSTEXPR basic_string_view_like(basic_string_view_like<OtherMutable, value_type, traits_type> other) = delete;
			
			CONSTEXPR ~basic_string_view_like() noexcept = default;
		public:
			CONSTEXPR auto operator= (const basic_string_view_like& view) noexcept ->basic_string_view_like& = default;
			
			NODISCARD CONSTEXPR auto max_size() const noexcept ->size_type {
				return std::min(static_cast<size_t>(PTRDIFF_MAX), static_cast<size_t>(-1) / sizeof(value_type));
			}
			
			NODISCARD CONSTEXPR auto begin() const noexcept ->iterator {
				return beg_;
			}
			
			NODISCARD CONSTEXPR auto end() const noexcept ->iterator {
				return end_;
			}
			
			NODISCARD CONSTEXPR auto cbegin() const noexcept ->const_iterator {
				return beg_;
			}
			
			NODISCARD CONSTEXPR auto cend() const noexcept ->const_iterator {
				return end_;
			}
			
			NODISCARD CONSTEXPR auto rbegin() const noexcept ->reverse_iterator {
				return std::make_reverse_iterator(std::ranges::prev(end_));
			}
			
			NODISCARD CONSTEXPR auto rend() const noexcept ->reverse_iterator {
				return std::make_reverse_iterator(std::ranges::prev(beg_));
			}
			
			NODISCARD CONSTEXPR auto crbegin() const noexcept ->const_reverse_iterator {
				return std::make_reverse_iterator(std::ranges::prev(end_));
			}
			
			NODISCARD CONSTEXPR auto crend() const noexcept ->const_reverse_iterator {
				return std::make_reverse_iterator(std::ranges::prev(beg_));
			}
			
			NODISCARD CONSTEXPR auto data() const noexcept ->iter_pointer_type_ {
				return beg_;
			}
			
			NODISCARD CONSTEXPR auto c_str() const noexcept ->const_pointer {
				return data();
			}
			
			NODISCARD CONSTEXPR auto size() const noexcept ->size_type {
				return std::ranges::distance(beg_, end_);
			}
			
			NODISCARD CONSTEXPR auto length() const noexcept ->size_type {
				return size();
			}
			
			NODISCARD CONSTEXPR auto empty() const noexcept ->bool {
				return beg_ == end_;
			}
			
			NODISCARD CONSTEXPR auto operator[] (size_type pos) const noexcept ->iter_reference_type_ {
				return beg_[pos];
			}
			
			NODISCARD CONSTEXPR auto at(size_type pos) const ->iter_reference_type_ {
				if (pos >= size()) throw std::out_of_range{"in function `ccat::basic_string_view_like::at`: the parameter `pos` is out of range"};
				return beg_[pos];
			}
			
			NODISCARD CONSTEXPR auto front() const noexcept ->iter_reference_type_ {
				return beg_[0];
			}
			
			NODISCARD CONSTEXPR auto back() const noexcept ->iter_reference_type_ {
				return *(end_ - 1);
			}
			
			CONSTEXPR auto swap(basic_string_view_like& other) noexcept ->void {
				std::swap(beg_, other.beg_);
				std::swap(end_, other.end_);
			}
			
			CONSTEXPR auto remove_prefix(size_type n) noexcept ->void {
				beg_ += n;
			}
			
			CONSTEXPR auto remove_suffix(size_type n) noexcept ->void {
				end_ -= n;
			}
			
			CONSTEXPR auto copy(value_type* dest, size_type count, size_type pos = 0) const ->size_type {
				if (pos > size()) throw std::out_of_range{"in function `ccat::basic_string_view_like::copy`: the parameter `pos` is out of range"};
				count = std::min(count, size() - pos);
				traits_type::copy(dest, beg_ + pos, count);
				return count;
			}
			
			NODISCARD CONSTEXPR auto substr(size_type pos = 0, size_type count = npos) const ->basic_string_view_like {
				if (pos > size()) throw std::out_of_range{"in function `ccat::basic_string_view_like::substr`: the parameter `pos` is out of range"};
				return basic_string_view_like{beg_ + pos, std::min(count, size() - pos)};
			}
			
			NODISCARD CONSTEXPR auto compare(readonly_view other) const noexcept ->int {
				if (auto res = traits_type::compare(data(), other.data(), std::min(size(), other.size())); res == 0) {
					if (size() > other.size()) return 1;
					else if (size() < other.size()) return -1;
					else return 0;
				}
				else return res;
			}
			
			NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, readonly_view v) const ->int {
				return substr(pos1, count1).compare(v);
			}
			
			NODISCARD CONSTEXPR auto compare(
				size_type pos1, size_type count1, readonly_view v,
                size_type pos2, size_type count2) const ->int {
				return substr(pos1, count1).compare(v.substr(pos2, count2));
			}
			
			NODISCARD CONSTEXPR auto compare(const_pointer s) const ->int {
				return compare(readonly_view{s});
			}
			
			NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1,
                      const_pointer s) const ->int {
				return substr(pos1, count1).compare(readonly_view{s});
			}
			
			NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1,
                      const_pointer s, size_type count2) const ->int {
				return  substr(pos1, count1).compare(readonly_view{s, count2});
			}
			
			NODISCARD CONSTEXPR auto starts_with(readonly_view other) const noexcept ->bool {
				if (other.size() > size()) return false;
				return traits_type::compare(data(), other.data(), other.size()) == 0;
			}
			
			NODISCARD CONSTEXPR auto starts_with(value_type c) const noexcept ->bool {
				return !empty() && traits_type::eq(front(), c);
			}
			
			NODISCARD CONSTEXPR auto starts_with(const_pointer s) const noexcept ->bool {
				for (size_type i{}; i < size() && *s; ++i) {
					if (beg_[i] != *s) return false;
					++s;
				}
				if (*s) return false;
				else return true;
			}
			
			NODISCARD CONSTEXPR auto ends_with(readonly_view other) const noexcept ->bool {
				if (other.size() > size()) return false;
				return traits_type::compare(beg_ + size() - other.size(), other.data(), other.size()) == 0;
			}
			
			NODISCARD CONSTEXPR auto ends_with(value_type c) const noexcept ->bool {
				return !empty() && traits_type::eq(back(), c);
			}
			
			NODISCARD CONSTEXPR auto ends_with(const_pointer s) const noexcept ->bool { // ensure `s` not null
				return ends_with(readonly_view{s});
			}
			
			NODISCARD CONSTEXPR auto find(readonly_view other, size_type pos = 0) const noexcept ->size_type {
				if (pos >= size() || other.size() > size()) return npos;
				return brute_force_search<true>(readonly_view{beg_ + pos, end_}, other);
			}
			
			NODISCARD CONSTEXPR auto find(value_type c, size_type pos = 0) const noexcept ->size_type {
				return find(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto find(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return find(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto find(const_pointer s, size_type pos = 0) const noexcept ->size_type {
				return find(readonly_view{s}, pos);
			}
			
			NODISCARD CONSTEXPR auto contains(readonly_view other) const noexcept ->bool {
				return find(other) != npos;
			}
			
			NODISCARD CONSTEXPR auto contains(value_type c) const noexcept ->bool {
				return find(c) != npos;
			}
			
			NODISCARD CONSTEXPR auto contains(const_pointer s) const noexcept ->bool {
				return find(s) != npos;
			}
			
			NODISCARD CONSTEXPR auto rfind(readonly_view other, size_type pos = npos) const noexcept ->size_type {
				auto sz = size();
				if (other.size() > sz) return npos;
				auto last = pos >= sz ? sz : pos + std::min(sz - pos, other.size());
				return brute_force_search<false>(readonly_view{beg_, last}, other);
			}
			
			NODISCARD CONSTEXPR auto rfind(value_type c, size_type pos = npos) const noexcept ->size_type {
				return rfind(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto rfind(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return rfind(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto rfind(const_pointer s, size_type pos = npos) const noexcept ->size_type {
				return rfind(readonly_view{s}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_of(readonly_view chars, size_type pos = 0) const noexcept ->size_type {
				if (pos >= size()) return npos;
				for (size_type i = pos; i < size(); ++i) {
					if (chars.contains(beg_[i])) return i;
				}
				return npos;
			}
			
			NODISCARD CONSTEXPR auto find_first_of(value_type c, size_type pos = 0) const noexcept ->size_type {
				return find_first_of(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return find_first_of(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_of(const_pointer s, size_type pos = 0) const noexcept ->size_type {
				return find_first_of(readonly_view{s}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_of(readonly_view chars, size_type pos = npos) const noexcept ->size_type {
				if (empty()) return npos;
				auto last = std::min(pos, size() - 1);
				for (size_type i{}; i <= last; ++i) {
					if (chars.contains(beg_[last - i])) return last - i;
				}
				return npos;
			}
			
			NODISCARD CONSTEXPR auto find_last_of(value_type c, size_type pos = npos) const noexcept ->size_type {
				return find_last_of(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return find_last_of(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_of(const_pointer s, size_type pos = npos) const noexcept ->size_type {
				return find_last_of(readonly_view{s}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_not_of(readonly_view chars, size_type pos = 0) const noexcept ->size_type {
				if (pos >= size()) return npos;
				for (size_type i = pos; i < size(); ++i) {
					if (!chars.contains(beg_[i])) return i;
				}
				return npos;
			}
			
			NODISCARD CONSTEXPR auto find_first_not_of(value_type c, size_type pos = 0) const noexcept ->size_type {
				return find_first_not_of(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_not_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return find_first_not_of(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_first_not_of(const_pointer s, size_type pos = 0) const noexcept ->size_type {
				return find_first_not_of(readonly_view{s}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_not_of(readonly_view chars, size_type pos = npos) const noexcept ->size_type {
				if (empty()) return npos;
				auto last = std::min(pos, size() - 1);
				for (size_type i{}; i <= last; ++i) {
					if (!chars.contains(beg_[last - i])) return last - i;
				}
				return npos;
			}
			
			NODISCARD CONSTEXPR auto find_last_not_of(value_type c, size_type pos = npos) const noexcept ->size_type {
				return find_last_not_of(readonly_view{std::addressof(c), 1}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_not_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
				return find_last_not_of(readonly_view{s, count}, pos);
			}
			
			NODISCARD CONSTEXPR auto find_last_not_of(const_pointer s, size_type pos = npos) const noexcept ->size_type {
				return find_last_not_of(readonly_view{s}, pos);
			}
			
			friend CONSTEXPR auto operator== (basic_string_view_like lhs, basic_string_view_like rhs) noexcept ->bool {
				return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
			}
			
			friend CONSTEXPR auto operator<=> (basic_string_view_like lhs, basic_string_view_like rhs) noexcept ->std::strong_ordering {
				if (auto res = lhs.compare(rhs); res == 0) return std::strong_ordering::equal;
				else if (res < 0) return std::strong_ordering::less;
				else return std::strong_ordering::greater;
			}
			
			friend auto operator<< (std::ostream& os, basic_string_view_like svl) ->std::ostream& {
				for (size_type i{}; i < svl.size(); ++i) {
					os << svl[i];
				}
				return os;
			}
		public:
			static constexpr size_type npos = static_cast<size_type>(-1);
		private:
			iter_pointer_type_ beg_ = nullptr, end_ = nullptr;
		};
	}
	template<typename CharT, typename Traits = char_traits<CharT>>
	using basic_string_view = detail::basic_string_view_like<false, CharT, Traits>;
	template<typename CharT, typename Traits = char_traits<CharT>>
	using basic_string_slice = detail::basic_string_view_like<true, CharT, Traits>;
}

namespace ccat {
	using string_view = basic_string_view<char>;
	using wstring_view = basic_string_view<wchar_t>;
	using string_slice = basic_string_slice<char>;
	using wstring_slice = basic_string_slice<wchar_t>;
}

template<bool Mutable, class CharT, class Traits>
inline constexpr bool std::ranges::enable_borrowed_range<ccat::detail::basic_string_view_like<Mutable, CharT, Traits>> = true;

template<bool Mutable, class CharT, class Traits>
inline constexpr bool std::ranges::enable_view<ccat::detail::basic_string_view_like<Mutable, CharT, Traits>> = true;