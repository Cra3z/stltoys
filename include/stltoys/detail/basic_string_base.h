#pragma once
#include "../basic_string_view.h"
#include "../array.h"

#define SET_CCAT_BASIC_STRING_SSO_SIZE__(CharT) template<> constexpr std::size_t basic_string_sso_size<CharT> = CCAT_sso_size_of_basic_string_##CharT;

namespace ccat {
	
	template<typename CharT>
	constexpr std::size_t basic_string_sso_size = 15;

#ifdef CCAT_sso_size_of_basic_string_char
	SET_CCAT_BASIC_STRING_SSO_SIZE__(char)
#endif

#ifdef CCAT_sso_size_of_basic_string_wchar_t
	SET_CCAT_BASIC_STRING_SSO_SIZE__(wchar_t)
#endif

#ifdef CCAT_sso_size_of_basic_string_char8_t
	SET_CCAT_BASIC_STRING_SSO_SIZE__(char8_t)
#endif

#ifdef CCAT_sso_size_of_basic_string_char16_t
	SET_CCAT_BASIC_STRING_SSO_SIZE__(char16_t)
#endif

#ifdef CCAT_sso_size_of_basic_string_char32_t
	SET_CCAT_BASIC_STRING_SSO_SIZE__(char32_t)
#endif

}


namespace ccat::detail {
	
	template<typename CharT, typename Traits, typename AllocType>
	class basic_string_base {
	protected:
		using view_type = ccat::basic_string_view<CharT, Traits>;
		using slice_type = ccat::basic_string_slice<CharT, Traits>;
		using allocator_traits = std::allocator_traits<AllocType>;
		using value_type = CharT;
		using traits_type = Traits;
		using allocator_type = AllocType;
		using size_type = typename std::allocator_traits<allocator_type>::size_type;
		using difference_type = typename std::allocator_traits<allocator_type>::difference_type;
		using pointer = value_type*;
		using const_pointer = const value_type*;
	protected:
		basic_string_base() = default;
		
		template<typename Alloc> requires std::same_as<std::remove_cvref_t<Alloc>, allocator_type>
		CONSTEXPR basic_string_base(Alloc&& alloc) noexcept(std::is_nothrow_constructible_v<allocator_type, Alloc&&>) : alloc_(std::forward<Alloc>(alloc)) {}
		
		template<typename Alloc> requires std::same_as<std::remove_cvref_t<Alloc>, allocator_type>
		CONSTEXPR basic_string_base(Alloc&& alloc, size_type init_size, value_type c = null_char) : alloc_(std::forward<Alloc>(alloc)) {
			set_ptrs(init_size, c);
		}
		
		template<typename Alloc> requires std::same_as<std::remove_cvref_t<Alloc>, allocator_type>
		CONSTEXPR basic_string_base(Alloc&& alloc, const_pointer s, size_type init_size) : alloc_(std::forward<Alloc>(alloc)) {
			set_ptrs(s, init_size);
		}
		
		template<typename Alloc> requires std::same_as<std::remove_cvref_t<Alloc>, allocator_type>
		CONSTEXPR basic_string_base(Alloc&& alloc, view_type v) : basic_string_base(std::forward<Alloc>(alloc), v.data(), v.size()) {}

		CONSTEXPR basic_string_base(const basic_string_base& other) : basic_string_base(other.alloc_, other.slice_) {}
		
		CONSTEXPR basic_string_base(basic_string_base&& other) noexcept : alloc_(std::move(other.alloc_)), sso(std::move(other.sso)) {
			swap_slice(other);
		}
		
		CONSTEXPR basic_string_base(const allocator_type& alloc, basic_string_base&& other) : alloc_(alloc), sso(std::move(other.sso))  {
			swap_slice(other);
		}
		
		CONSTEXPR ~basic_string_base() noexcept {
			deallocate();
		}
		
		CONSTEXPR auto operator= (const basic_string_base& other) ->basic_string_base& {
			basic_string_base tmp = other; // copy and swap
			swap(tmp);
			return *this;
		}
		
		CONSTEXPR auto operator= (basic_string_base&& other) noexcept(
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
		    std::allocator_traits<allocator_type>::is_always_equal::value
		) ->basic_string_base& {
			basic_string_base tmp = std::move(other);
			swap(other);
			return *this;
		}
	public:
		
		NODISCARD CONSTEXPR auto size() const noexcept ->size_type {
			return slice_.size();
		}
		
		NODISCARD CONSTEXPR auto length() const noexcept ->size_type {
			return slice_.size();
		}
		
		NODISCARD CONSTEXPR auto empty() const noexcept ->bool {
			return slice_.empty();
		}
		
		NODISCARD CONSTEXPR auto max_size() const noexcept ->size_type {
			return slice_.max_size();
		}
		
		NODISCARD CONSTEXPR auto capacity() const noexcept ->size_type {
			return cap_ - slice_.beg_;
		}
		
		CONSTEXPR auto clear() noexcept ->void {
			slice_.end_ = slice_.beg_;
			null_terminated();
		}
		
		CONSTEXPR auto swap(basic_string_base& other) noexcept(
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
			std::allocator_traits<allocator_type>::is_always_equal::value
		) ->void {
			std::ranges::swap(alloc_, other.alloc_);
			std::ranges::swap(sso, other.sso);
			swap_slice(other);
		}
		
	protected:
		NODISCARD CONSTEXPR auto allocate(size_type new_cap) const ->pointer {
			if (new_cap <= sso_size) return sso.data();
			++new_cap;
			pointer new_space = nullptr;
			try {
				new_space = allocator_traits::allocate(alloc_, new_cap);
			}
			catch (const std::bad_alloc&) {
				throw;
			}
			catch (...) {
				allocator_traits::deallocate(alloc_, new_space, new_cap);
				throw;
			}
			return new_space;
		}
		
		CONSTEXPR auto deallocate() const noexcept ->void {
			auto ptr = slice_.beg_;
			if (ptr == sso.data()) return; // noop
			auto cap = capacity();
			++cap;
			allocator_traits::deallocate(alloc_, ptr, cap);
		}
		
		CONSTEXPR auto set_ptrs() noexcept ->void {
			slice_ = {sso.data(), 0};
			cap_ = &sso.back();
			null_terminated();
		}
		
		CONSTEXPR auto set_ptrs(size_type init_size, value_type c = null_char) ->void {
			auto init_cap = std::max(init_size, sso_size);
			slice_.beg_ = init_size <= sso_size ? sso.data() : allocate(init_cap);
			slice_.end_ = slice_.beg_ + init_size;
			cap_ = slice_.beg_ + init_cap;
			for (size_type i{}; i < init_size; ++i) traits_type::assign(slice_[i], c);
			null_terminated();
		}
		
		CONSTEXPR auto set_ptrs(const_pointer s, size_type init_size) ->void {
			auto init_cap = std::max(init_size, sso_size);
			slice_.beg_ = init_size <= sso_size ? sso.data() : allocate(init_cap);
			slice_.end_ = slice_.beg_ + init_size;
			cap_ = slice_.beg_ + init_cap;
			for (size_type i{}; i < init_size; ++i) traits_type::assign(slice_[i], s[i]);
			null_terminated();
		}
		
		CONSTEXPR auto set_ptrs(view_type v) ->void {
			set_ptrs(v.data(), v.size());
		}
		
		CONSTEXPR auto null_terminated() noexcept ->void {
			*slice_.end_ = null_char;
		}
		
		CONSTEXPR auto swap_slice(basic_string_base& other) noexcept ->void {
			auto size1 = size();
			auto cap1 = capacity();
			auto size2 = other.size();
			auto cap2 = other.capacity();
			if (in_sso() && other.in_sso()) {} // noop
			else if (!in_sso() && other.in_sso()) {
				other.slice_.beg_ = slice_.beg_;
				slice_.beg_ = sso.data();
			}
			else if (in_sso() && !other.in_sso()) {
				slice_.beg_ = other.slice_.beg_;
				other.slice_.beg_ = other.sso.data();
			}
			else std::ranges::swap(slice_.beg_, other.slice_.beg_);
			slice_.end_ = slice_.beg_ + size2;
			cap_ = slice_.beg_ + cap2;
			other.slice_.end_ = other.slice_.beg_ + size1;
			other.cap_ = other.slice_.beg_ + cap1;
		}
		
	protected:
		NODISCARD CONSTEXPR auto in_sso() const noexcept ->bool {
			return slice_.beg_ == sso.data();
		}
	protected:
		static constexpr value_type null_char = static_cast<value_type>(0);
		static constexpr size_type sso_size = basic_string_sso_size<value_type>;
	protected:
		mutable allocator_type alloc_{};
		mutable array<value_type, sso_size + 1> sso{};
		slice_type slice_{sso.data(), 0};
		pointer cap_{&sso.back()};
	};
}