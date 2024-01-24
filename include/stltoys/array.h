#pragma once
#include "iterator"

namespace ccat {

	namespace detail {
		template<typename T, std::size_t N>
		struct raw_array {
			T data_[N];
			CONSTEXPR auto address() noexcept ->T* {
				return data_;
			}
			CONSTEXPR auto address() const noexcept ->const T* {
				return data_;
			}
			CONSTEXPR auto swap(raw_array& other) noexcept ->void {
				std::ranges::swap(data_, other.data_);
			}
		};
		
		template<typename T>
		struct raw_array<T, 0> {
			CONSTEXPR auto address() const noexcept ->T* {
				return nullptr;
			}
			CONSTEXPR auto swap(raw_array&) noexcept ->void {}
		};
		
	}
	
	template<typename T, std::size_t N> requires std::move_constructible<T> && std::is_move_assignable_v<T>
	struct array {
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		
		NODISCARD CONSTEXPR auto data() noexcept ->pointer {
			return elems_.address();
		}
		
		NODISCARD CONSTEXPR auto data() const noexcept ->const_pointer {
			return elems_.address();
		}
		
		NODISCARD CONSTEXPR auto operator[] (size_type pos) noexcept ->reference {
			return elems_.address()[pos];
		}
		
		NODISCARD CONSTEXPR auto operator[] (size_type pos) const noexcept ->const_reference {
			return elems_.address()[pos];
		}
		
		NODISCARD CONSTEXPR auto at(size_type pos) ->reference {
			if (pos > size()) throw std::out_of_range{"ccat::array::at: the parameter `pos` out of range"};
			return elems_.address()[pos];
		}
		
		NODISCARD CONSTEXPR auto at(size_type pos) const ->const_reference {
			if (pos > size()) throw std::out_of_range{"ccat::array::at: the parameter `pos` out of range"};
			return elems_.address()[pos];
		}
		
		NODISCARD CONSTEXPR auto front() noexcept ->reference {
			return elems_.address()[0];
		}
		
		NODISCARD CONSTEXPR auto front() const noexcept ->const_reference {
			return elems_.address()[0];
		}
		
		NODISCARD CONSTEXPR auto back() noexcept ->reference {
			return elems_.address()[N - 1];
		}
		
		NODISCARD CONSTEXPR auto back() const noexcept ->const_reference {
			return elems_.address()[N - 1];
		}
		
		NODISCARD CONSTEXPR auto begin() noexcept ->iterator {
			return elems_.address();
		}
		
		NODISCARD CONSTEXPR auto begin() const noexcept ->const_iterator {
			return elems_.address();
		}
		
		NODISCARD CONSTEXPR auto cbegin() const noexcept ->const_iterator {
			return begin();
		}
		
		NODISCARD CONSTEXPR auto end() noexcept ->iterator {
			return elems_.address() + N;
		}
		
		NODISCARD CONSTEXPR auto end() const noexcept ->const_iterator {
			return elems_.address() + N;
		}
		
		NODISCARD CONSTEXPR auto cend() const noexcept ->const_iterator {
			return end();
		}
		
		NODISCARD CONSTEXPR auto rbegin() noexcept ->reverse_iterator {
			return std::make_reverse_iterator(begin());
		}
		
		NODISCARD CONSTEXPR auto rbegin() const noexcept ->const_reverse_iterator {
			return std::make_reverse_iterator(begin());
		}
		
		NODISCARD CONSTEXPR auto crbeign() const noexcept ->const_reverse_iterator {
			return rbegin();
		}
		
		NODISCARD CONSTEXPR auto rend() noexcept ->reverse_iterator {
			return std::make_reverse_iterator(end());
		}
		
		NODISCARD CONSTEXPR auto rend() const noexcept ->const_reverse_iterator {
			return std::make_reverse_iterator(end());
		}
		
		NODISCARD CONSTEXPR auto crend() const noexcept ->const_reverse_iterator {
			return rend();
		}
		
		NODISCARD CONSTEXPR auto size() const noexcept ->size_type {
			return N;
		}
		
		NODISCARD CONSTEXPR auto empty() const noexcept ->bool {
			return N == 0;
		}
		
		NODISCARD CONSTEXPR auto max_size() const noexcept ->size_type {
			return N;
		}
		
		CONSTEXPR auto fill(const_reference value) noexcept(std::is_nothrow_copy_assignable_v<value_type>) ->void {
			for (size_type i{}; i < N; ++i) elems_.address()[i] = value;
		}
		
		CONSTEXPR auto swap(array& other) noexcept ->void {
			elems_.swap(other.elems_);
		}
		
		friend CONSTEXPR auto operator== (const array& lhs, const array& rhs) noexcept ->bool {
			for (size_type i{}; i < N; ++i) {
				if (lhs[i] != rhs[i]) return false;
			}
			return true;
		}
		
		friend CONSTEXPR auto operator<=> (const array& lhs, const array& rhs) noexcept requires std::three_way_comparable_with<value_type, value_type> {
			for (size_type i{}; i < N; ++i) {
				if (auto cmp_res = lhs[i] <=> rhs[i]; cmp_res == 0) continue;
				else return cmp_res;
			}
			return 0;
		}
		
		detail::raw_array<value_type, N> elems_;
	};
	
	namespace detail {
		template<typename T>
		constexpr bool is_ccat_array_v = false;
		
		template<typename T, std::size_t N>
		constexpr bool is_ccat_array_v<array<T, N>> = true;
	}
	
	template<std::size_t I, typename Array> requires detail::is_ccat_array_v<std::remove_cvref_t<Array>>
	CONSTEXPR decltype(auto) get(Array&& array) noexcept {
		return forward_like<Array>(array[I]);
	}
	
	template<typename T, typename... Args> requires (... && std::same_as<T, Args>)
	array(T, Args...) -> array<T, sizeof...(Args) + 1>;
}

template<typename T, std::size_t N>
struct std::tuple_size<ccat::array<T, N>> : std::integral_constant<std::size_t, N> {};

template<std::size_t I, class T, std::size_t N>
struct std::tuple_element<I, ccat::array<T,N>> {
	using type = T;
};