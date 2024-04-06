#pragma once
#include "deque.h"

namespace ccat {
	template<typename T, typename Container>
	class queue {
	public:
		using container_type = Container;
		using value_type = typename container_type::value_type;
		using size_type = typename container_type::size_type;
		using reference = typename container_type::reference;
		using const_reference = typename container_type::const_reference;
	public:
		queue() : queue(container_type()) {}

		explicit queue(const container_type& cont) : container_(cont) {}

		explicit queue(container_type&& cont) noexcept(std::is_nothrow_move_constructible_v<container_type>) : container_(std::move(cont)) {}

		template<std::input_iterator InputIt>
		queue(InputIt first, InputIt last) : container_(first, last) {}

		template<typename Alloc>
		explicit queue(const Alloc& alloc) : container_(alloc) {}

		template<typename Alloc>
		queue(const container_type& cont, const Alloc& alloc) : container_(cont, alloc) {}

		template<typename Alloc>
		queue(container_type&& cont, const Alloc& alloc) : container_(std::move(cont), alloc) {}

		template<typename Alloc>
		queue(const queue& other, const Alloc& alloc) : container_(other.container_, alloc) {}

		template<typename Alloc>
		queue(queue&& other, const Alloc& alloc) : container_(std::move(other.container_), alloc) {}

		template<std::input_iterator InputIt, typename Alloc>
		queue(InputIt first, InputIt last, const Alloc& alloc) : container_(first, last, alloc) {}

		template<concepts::container_compatible_range<value_type> Range>
		queue(from_range_t, Range&& rng) : container_(from_range, std::forward<Range>(rng)) {}

		template<concepts::container_compatible_range<value_type> Range, class Alloc>
		queue(from_range_t, Range&& rng, const Alloc& alloc) : container_(from_range, std::forward<Range>(rng), alloc) {}

		NODISCARD auto front() noexcept(noexcept(std::declval<container_type&>().front())) ->reference {
			container_.front();
		}

		NODISCARD auto front() const noexcept(noexcept(std::declval<const container_type&>().front())) ->const_reference {
			container_.front();
		}

		NODISCARD auto back() noexcept(noexcept(std::declval<container_type&>().back())) ->reference {
			container_.back();
		}

		NODISCARD auto back() const noexcept(noexcept(std::declval<const container_type&>().back())) ->const_reference {
			container_.back();
		}

		auto push(const value_type& value) ->void {
			container_.push_back(value);
		}

		auto push(value_type&& value) ->void {
			container_.push_back(std::move(value));
		}

		template<typename... Args>
		decltype(auto) emplace(Args&&... args) {
			return container_.emplace_back(std::forward<Args>(args)...);
		}

		template<std::ranges::input_range Range> requires std::convertible_to<std::ranges::range_value_t<Range>, value_type> &&
			requires (container_type c) {c.append_range(std::declval<Range>());}
		auto push_range(Range&& rng) ->void {
			container_.append_range(std::forward<Range>(rng));
		}

		template<std::ranges::input_range Range> requires std::convertible_to<std::ranges::range_value_t<Range>, value_type>
		auto push_range(Range&& rng) ->void {
			std::ranges::copy(rng, std::back_inserter(container_));
		}

		auto pop() ->void {
			container_.pop_front();
		}

		NODISCARD auto empty() const noexcept(noexcept(std::declval<const container_type&>().empty())) ->bool {
			return container_.empty();
		}

		auto size() const noexcept(noexcept(std::declval<const container_type&>().size())) ->size_type {
			return container_.size();
		}

		auto swap(queue& other) noexcept(std::is_nothrow_swappable_v<container_type>) ->void {
			std::ranges::swap(container_, other.container_);
		}

		friend auto operator== (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ == rhs.container_)) ->bool {
			return lhs.container_ == rhs.container_;
		}

		friend auto operator!= (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ != rhs.container_)) ->bool {
			return lhs.container_ != rhs.container_;
		}

		friend auto operator> (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ > rhs.container_)) ->bool {
			return lhs.container_ > rhs.container_;
		}

		friend auto operator< (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ < rhs.container_)) ->bool {
			return lhs.container_ < rhs.container_;
		}

		friend auto operator>= (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ >= rhs.container_)) ->bool {
			return lhs.container_ >= rhs.container_;
		}

		friend auto operator<= (const queue& lhs, const queue& rhs) noexcept(noexcept(lhs.container_ <= rhs.container_)) ->bool {
			return lhs.container_ <= rhs.container_;
		}

		friend auto operator<=> (const queue& lhs, const queue& rhs) requires std::three_way_comparable<container_type> ->std::compare_three_way_result_t<container_type> {
			return lhs.container_ <=> rhs.container_;
		}

		template<typename Container>
		queue(Container) -> queue<typename Container::value_type, Container>;

		template<typename Container, typename Alloc>
		queue(Container, Alloc) -> queue<typename Container::value_type, Container>;

		template<std::input_iterator InputIt>
		queue(InputIt, InputIt) -> queue<typename std::iterator_traits<InputIt>::value_type>;

		template<std::ranges::input_range R>
		queue(from_range_t, R&&) -> queue<std::ranges::range_value_t<R>>;

	private:
		container_type container_;
	};
}

template<typename T, typename Container, typename Alloc>
struct std::uses_allocator<ccat::queue<T, Container>, Alloc> : std::uses_allocator<Container, Alloc>::type {};