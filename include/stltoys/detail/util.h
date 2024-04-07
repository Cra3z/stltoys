#pragma once
#include <climits>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <memory>

namespace ccat {

	namespace detail {

		template<typename T, typename U>
		struct forward_like_helper_ {
			using NRT = typename std::remove_reference<T>::type;
			using NRU = typename std::remove_reference<U>::type;
			using type = typename std::conditional<std::is_const<NRT>::value, const NRU&&, NRU&&>::type;
		};

		template<typename T, typename U>
		struct forward_like_helper_<T&, U> {
			static_assert(std::is_lvalue_reference<U>::value, "bad forward_like call");
			using NRU = typename std::remove_reference<U>::type;
			using type = typename std::conditional<std::is_const<T>::value, const NRU&, NRU&>::type;
		};

		template<typename T, typename U>
		using forward_like_result_type = typename forward_like_helper_<T, U>::type;

	}

	template<typename T, typename U>
	auto forward_like(U&& u) noexcept ->detail::forward_like_result_type<T, U> {
		return static_cast<detail::forward_like_result_type<T, U>>(u);
	}

	struct from_range_t {
		explicit from_range_t() = default;
	};

	inline constexpr from_range_t from_range{};
}

namespace ccat::detail {
	template<std::input_iterator InputIt, std::forward_iterator ForwardIt, typename Alloc>
	CONSTEXPR auto alloc_uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first, Alloc& alloc) ->ForwardIt {
		ForwardIt current = d_first;
		try {
			for (; first != last; ++first, ++current) {
				std::allocator_traits<Alloc>::construct(alloc, std::to_address(current), *first);
			}
			return current;
		}
		catch (...) {
			for (; d_first != current; ++d_first) {
				std::allocator_traits<Alloc>::destroy(alloc, std::to_address(d_first));
			}
			throw;
		}
	}

	template<std::input_iterator InputIt, std::forward_iterator ForwardIt, typename Alloc>
	CONSTEXPR auto alloc_uninitialized_move(InputIt first, InputIt last, ForwardIt d_first, Alloc& alloc) ->ForwardIt {
		ForwardIt current = d_first;
		try {
			for (; first != last; ++first, ++current) {
				std::allocator_traits<Alloc>::construct(alloc, std::to_address(current), std::move(*first));
			}
			return current;
		}
		catch (...) {
			for (; d_first != current; ++d_first) {
				std::allocator_traits<Alloc>::destroy(alloc, std::to_address(d_first));
			}
			throw;
		}
	}

	template<std::forward_iterator ForwardIt, typename Alloc>
	CONSTEXPR auto alloc_uninitialized_default_construct(ForwardIt first, ForwardIt last, Alloc& alloc) ->void {
		ForwardIt current = first;
		try {
			for (; current != last; ++current) {
				std::allocator_traits<Alloc>::construct(alloc, std::to_address(current));
			}
		}
		catch (...) {
			for (; first != current; ++first) {
				std::allocator_traits<Alloc>::destroy(alloc, std::to_address(first));
			}
			throw;
		}
	}

	template<std::forward_iterator ForwardIt, typename T, typename Alloc>
	CONSTEXPR auto alloc_uninitialized_fill(ForwardIt first, ForwardIt last, const T& v, Alloc& alloc) ->void {
		ForwardIt current = first;
		try {
			for (; current != last; ++current) {
				std::allocator_traits<Alloc>::construct(alloc, std::to_address(current), v);
			}
		}
		catch (...) {
			for (; first != current; ++first) {
				std::allocator_traits<Alloc>::destroy(alloc, std::to_address(first));
			}
			throw;
		}
	}

	template<std::forward_iterator ForwardIt, typename Alloc>
	CONSTEXPR auto alloc_destroy(ForwardIt first, ForwardIt last, Alloc& alloc) ->void {
		for (; first != last; ++first) {
			std::allocator_traits<Alloc>::destroy(alloc, std::to_address(first));
		}
	}
}