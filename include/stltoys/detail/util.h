#pragma once
#include <climits>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <utility>

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

}