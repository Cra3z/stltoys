#pragma once
#include <iterator>
#include <ranges>
#include "config.h"

namespace ccat::detail {
	template<bool Mutable, typename T, typename IteratorConcept>
	class generic_random_access_iterator {
        template<bool Mutable_, typename T_, typename IteratorConcept_>
		friend class generic_random_access_iterator;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using iterator_concept = IteratorConcept;
		using value_type = T;
		using pointer = std::conditional_t<Mutable, value_type*, const value_type*>;
		using reference = std::conditional_t<Mutable, value_type&, const value_type&>;
		using difference_type = std::ptrdiff_t;
	public:
		CONSTEXPR generic_random_access_iterator() noexcept = default;
		
		CONSTEXPR generic_random_access_iterator(pointer ptr) noexcept: ptr_(ptr) {}
		
		CONSTEXPR generic_random_access_iterator(const generic_random_access_iterator&) noexcept = default;
		
		template<bool OtherMutable> requires (!Mutable && OtherMutable)
		CONSTEXPR generic_random_access_iterator(const generic_random_access_iterator<OtherMutable, value_type, IteratorConcept>& other) noexcept : ptr_(other.ptr_) {}
		
		CONSTEXPR ~generic_random_access_iterator() noexcept = default;
	
	public:
		CONSTEXPR auto operator= (const generic_random_access_iterator&) noexcept -> generic_random_access_iterator& = default;
		
		template<bool OtherMutable> requires (!Mutable && OtherMutable)
		CONSTEXPR auto operator= (const generic_random_access_iterator<OtherMutable, value_type, IteratorConcept>& other) noexcept -> generic_random_access_iterator& {
			ptr_ = other.ptr_;
			return *this;
		}
		
		CONSTEXPR auto operator++ () noexcept -> generic_random_access_iterator& {
			++ptr_;
			return *this;
		}
		
		CONSTEXPR auto operator++ (int) noexcept -> generic_random_access_iterator {
			return ptr_++;
		}
		
		CONSTEXPR auto operator-- () noexcept -> generic_random_access_iterator& {
			--ptr_;
			return *this;
		}
		
		CONSTEXPR auto operator-- (int) noexcept -> generic_random_access_iterator {
			return ptr_--;
		}
		
		CONSTEXPR auto operator+=(difference_type n) noexcept -> generic_random_access_iterator& {
			ptr_ += n;
			return *this;
		}
		
		CONSTEXPR friend auto operator+ (generic_random_access_iterator it, difference_type n) noexcept -> generic_random_access_iterator {
			return it.ptr_ + n;
		}
		
		CONSTEXPR friend auto operator+ (difference_type n, generic_random_access_iterator it) noexcept -> generic_random_access_iterator {
			return n + it.ptr_;
		}
		
		CONSTEXPR auto operator-= (difference_type n) noexcept -> generic_random_access_iterator& {
			ptr_ -= n;
			return *this;
		}
		
		CONSTEXPR auto operator- (difference_type n) const noexcept -> generic_random_access_iterator {
			return ptr_ - n;
		}
		
		CONSTEXPR auto operator- (generic_random_access_iterator other) const noexcept -> difference_type {
			return ptr_ - other.ptr_;
		}
		
		NODISCARD CONSTEXPR auto operator[] (difference_type n) const noexcept -> reference {
			return ptr_[n];
		}
		
		NODISCARD CONSTEXPR auto operator* () const noexcept -> reference {
			return *ptr_;
		}
		
		CONSTEXPR auto operator-> () const noexcept -> pointer {
			return ptr_;
		}
		
		friend CONSTEXPR auto operator== (generic_random_access_iterator lhs, generic_random_access_iterator rhs) noexcept -> bool = default;
		
		friend CONSTEXPR auto operator<=> (generic_random_access_iterator lhs, generic_random_access_iterator rhs) noexcept -> std::strong_ordering {
			return lhs.ptr_ <=> rhs.ptr_;
		}
	
	private:
		pointer ptr_ = nullptr;
	};

	template<bool Mutable, typename T>
	using contiguous_iterator = generic_random_access_iterator<Mutable, T, std::contiguous_iterator_tag>;

	template<bool Mutable, typename T>
	using random_access_iterator = generic_random_access_iterator<Mutable, T, std::random_access_iterator_tag>;
}