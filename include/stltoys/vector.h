#pragma once
#include <algorithm>
#include <compare>
#include <initializer_list>
#include <stdexcept>
#include "detail/config.h"
#include "detail/iterator.h"
#include "detail/concepts.h"
#include "detail/util.h"

namespace ccat {
	template<typename T, typename Alloc = std::allocator<T>> requires std::same_as<T, std::remove_cvref_t<T>> && std::same_as<T, typename Alloc::value_type> && concepts::erasable<T, Alloc>
	class vector {
	public:
		using value_type = T;
		using allocator_type = Alloc;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = typename std::allocator_traits<allocator_type>::pointer;
		using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = detail::contiguous_iterator<true, value_type>;
		using const_iterator = detail::contiguous_iterator<false, value_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	public: // ctor and dtor

		CONSTEXPR vector() = default;

		CONSTEXPR explicit vector(const allocator_type& alloc) noexcept: alloc_(alloc) {}

		CONSTEXPR vector(size_type count, const_reference value, const allocator_type& alloc = allocator_type()) : alloc_(alloc) {
			this->resize(count, value);
		}

		CONSTEXPR explicit vector(size_type count, const allocator_type& alloc = allocator_type()) : alloc_(alloc) {
			this->resize(count);
		}

		template<std::input_iterator InputIt>
		CONSTEXPR vector(InputIt first, InputIt last, const allocator_type& alloc = allocator_type()) : alloc_(alloc) {
			this->insert(cend(), first, last);
		}

		CONSTEXPR vector(const vector& other) :
			alloc_(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator()))
		{
			this->append_range(other);
		}

		CONSTEXPR vector(const vector& other, const allocator_type& alloc) : alloc_(alloc) {
			this->append_range(other);
		}

		CONSTEXPR vector(vector&& other) noexcept : beg_(std::exchange(other.beg_, {})), end_(std::exchange(other.end_, {})), cap_(std::exchange(other.cap_, {})), alloc_(std::move(other.alloc_)) {}

		CONSTEXPR vector(vector&& other, const allocator_type& alloc) : alloc_(alloc) {
			if (alloc != other.get_allocator()) {
				this->append_range(std::ranges::subrange{std::make_move_iterator(other.begin()), std::make_move_iterator(other.end())});
			}
			else {
				beg_ = std::exchange(other.beg_, {});
				end_ = std::exchange(other.end_, {});
				cap_ = std::exchange(other.cap_, {});
			}
		}

		CONSTEXPR vector(std::initializer_list<value_type> ilist, const allocator_type& alloc = allocator_type()) : alloc_(alloc) {
			this->insert(cend(), ilist);
		}

		template<std::ranges::input_range Range>
		CONSTEXPR vector(from_range_t, Range&& rng, const allocator_type& alloc = allocator_type()) : alloc_(alloc) {
			this->append_range(std::forward<Range>(rng));
		}

		CONSTEXPR ~vector() {
			die_();
		}
	public:

		CONSTEXPR auto operator= (const vector& other) ->vector& {
			if (this == std::addressof(other)) [[unlikely]] return *this;
			if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
				if (alloc_ != other.alloc_) die_();
				alloc_ = other.alloc_;
			}
			this->assign_range(other);
			return *this;
		}

		CONSTEXPR auto operator= (vector&& other) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value || std::allocator_traits<allocator_type>::is_always_equal::value) ->vector& {
			if (this == std::addressof(other)) [[unlikely]] return *this;
			if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
				die_();
				alloc_ = std::move(other.alloc_);
				beg_ = std::exchange(other.beg_, {});
				end_ = std::exchange(other.end_, {});
				cap_ = std::exchange(other.cap_, {});
			}
			else {
				if (std::allocator_traits<allocator_type>::is_always_equal::value || alloc_ == other.alloc_) {
					die_();
					beg_ = std::exchange(other.beg_, {});
					end_ = std::exchange(other.end_, {});
					cap_ = std::exchange(other.cap_, {});
				}
				else {
					this->elem_move_assign_from_(other);
				}
			}
			return *this;
		}

		CONSTEXPR auto operator= (std::initializer_list<value_type> ilist) ->vector& {
			vector tmp(ilist);
			this->swap(tmp);
			return *this;
		}

		CONSTEXPR auto assign(size_type count, const_reference value) ->void {
			vector tmp(count, value);
			*this = tmp;
		}

		template<std::input_iterator InputIt>
		CONSTEXPR auto assign(InputIt first, InputIt last) ->void {
			vector tmp(first, last);
			*this = tmp;
		}

		CONSTEXPR auto assign(std::initializer_list<value_type> ilist) ->void {
			*this = ilist;
		}

		template<std::ranges::input_range Range>
		CONSTEXPR auto assign_range(Range&& rng) ->void requires
			std::assignable_from<T&, std::ranges::range_reference_t<Range>> &&
			concepts::emplace_constructible_from<value_type, vector, std::ranges::range_reference_t<Range>> &&
			(std::ranges::sized_range<Range> || std::ranges::forward_range<Range> ? true : concepts::move_insertable_into<value_type, vector>)
		{
			auto ptr = beg_;
			auto rng_it = std::ranges::begin(rng);
			auto rng_end = std::ranges::end(rng);
			while (ptr != end_ && rng_it != rng_end) {
				*ptr = *rng_it;
				++ptr;
				++rng_it;
			}
			detail::alloc_destroy(ptr, end_, alloc_);
			end_ = ptr;
			while (rng_it != rng_end) {
				this->push_back(*rng_it);
				++rng_it;
			}
		}

		NODISCARD CONSTEXPR auto begin() noexcept ->iterator {
			return {beg_};
		}

		NODISCARD CONSTEXPR auto begin() const noexcept ->const_iterator {
			return {beg_};
		}

		NODISCARD CONSTEXPR auto cbegin() const noexcept ->const_iterator {
			return begin();
		}

		NODISCARD CONSTEXPR auto rbegin() noexcept ->reverse_iterator {
			return std::make_reverse_iterator(end());
		}

		NODISCARD CONSTEXPR auto rbegin() const noexcept ->const_reverse_iterator {
			return std::make_reverse_iterator(end());
		}

		NODISCARD CONSTEXPR auto crbegin() const noexcept ->const_reverse_iterator {
			return rbegin();
		}

		NODISCARD CONSTEXPR auto end() noexcept ->iterator {
			return {end_};
		}

		NODISCARD CONSTEXPR auto end() const noexcept ->const_iterator {
			return {end_};
		}

		NODISCARD CONSTEXPR auto cend() const noexcept ->const_iterator {
			return end();
		}

		NODISCARD CONSTEXPR auto rend() noexcept ->reverse_iterator {
			return std::make_reverse_iterator(begin());
		}

		NODISCARD CONSTEXPR auto rend() const noexcept ->const_reverse_iterator {
			return std::make_reverse_iterator(begin());
		}

		NODISCARD CONSTEXPR auto crend() const noexcept ->const_reverse_iterator {
			return rend();
		}

		NODISCARD CONSTEXPR auto size() const noexcept ->size_type {
			return end_ - beg_;
		}

		NODISCARD CONSTEXPR auto max_size() const noexcept ->size_type {
			return std::numeric_limits<difference_type>::max();
		}

		NODISCARD CONSTEXPR auto empty() const noexcept ->bool {
			return beg_ == end_;
		}

		NODISCARD CONSTEXPR auto capacity() const noexcept ->size_type {
			return cap_ - beg_;
		}

		NODISCARD CONSTEXPR auto operator[] (size_type index) noexcept ->reference {
			return beg_[index];
		}

		NODISCARD CONSTEXPR auto operator[] (size_type index) const noexcept ->const_reference {
			return beg_[index];
		}

		NODISCARD CONSTEXPR auto front() noexcept ->reference {
			return *beg_;
		}

		NODISCARD CONSTEXPR auto front() const noexcept ->const_reference {
			return *beg_;
		}

		NODISCARD CONSTEXPR auto back() noexcept ->reference {
			return *(end_ - 1);
		}

		NODISCARD CONSTEXPR auto back() const noexcept ->const_reference {
			return *(end_ - 1);
		}

		NODISCARD CONSTEXPR auto get_allocator() const noexcept ->allocator_type {
			return alloc_;
		}

		NODISCARD CONSTEXPR auto data() noexcept ->pointer {
			return beg_;
		}

		NODISCARD CONSTEXPR auto data() const noexcept ->const_pointer {
			return beg_;
		}

		NODISCARD CONSTEXPR auto at(size_type index) ->reference {
			if (index >= size()) throw std::out_of_range{"in `ccat::vector::at`: the parameter `index` is out of range"};
			return beg_[index];
		}

		NODISCARD CONSTEXPR auto at(size_type index) const ->const_reference {
			if (index >= size()) throw std::out_of_range{"in `ccat::vector::at`: the parameter `index` is out of range"};
			return beg_[index];
		}

		CONSTEXPR auto resize(size_type new_size) ->void requires concepts::move_insertable_into<value_type, vector> && concepts::default_insertable_into<value_type, vector> {
			this->resize_impl_<false>(new_size, nullptr);
		}

		CONSTEXPR auto resize(size_type new_size, const_reference value) ->void requires concepts::copy_insertable_into<value_type, vector> {
			this->resize_impl_<true>(new_size, std::addressof(value));
		}

		CONSTEXPR auto reserve(size_type new_capacity) ->void requires concepts::move_insertable_into<value_type, vector> {
			if (new_capacity <= capacity()) return;
			size_type old_capacity = capacity();
			new_capacity = std::max(new_capacity, old_capacity + (old_capacity >> 1));
			realloc_(new_capacity);
		}

		CONSTEXPR auto shrink_to_fit() ->void requires concepts::move_insertable_into<value_type, vector> {
			realloc_(size());
		}

		CONSTEXPR auto clear() noexcept ->void {
			detail::alloc_destroy(begin(), end(), alloc_);
			end_ = beg_;
		}

		CONSTEXPR auto erase(const_iterator pos) ->iterator requires concepts::move_assignable<value_type> {
			return this->erase(pos, pos + 1);
		}

		CONSTEXPR auto erase(const_iterator first, const_iterator last) ->iterator requires concepts::move_assignable<value_type> {
			auto idx = first - cbegin();
			if (first == last) return begin() + idx;
			auto distance_ = last - first;
			auto first_ = beg_ + idx;
			auto last_ = beg_ + (last - cbegin());
			std::move(last_, end_, first_);
			detail::alloc_destroy(end_ - distance_, end_, alloc_);
			end_ -= distance_;
			return begin() + idx;
		}

		CONSTEXPR auto pop_back() ->void {
			(void) this->erase(end_ - 1);
		}

		template<typename... Args> requires concepts::move_assignable<value_type> && concepts::move_insertable_into<value_type, vector> && concepts::emplace_constructible_from<value_type, vector, Args...>
		CONSTEXPR auto emplace(const_iterator pos, Args&&... args) ->iterator {
			if (pos == cend()) {
				this->emplace_back(std::forward<Args>(args)...);
				return end_ - 1;
			}
			auto idx = (pos - cbegin());
			if (size() == capacity()) this->realloc_(std::max(size() + 1, capacity() + (capacity() >> 1)));
			auto where_ = beg_ + idx;
			value_type tmp(std::forward<Args>(args)...);
			std::allocator_traits<allocator_type>::construct(alloc_, end_, std::move(back()));
			std::move_backward(where_, end_ - 1, end_);
			*where_ = std::move(tmp);
			++end_;
			return where_;
		}

		template<typename... Args> requires concepts::move_insertable_into<value_type, vector> && concepts::emplace_constructible_from<value_type, vector, Args...>
		CONSTEXPR auto emplace_back(Args&&... args) ->reference {
			if (size() == capacity()) {
				this->realloc_and_emplace_back_(std::max(size() + 1, capacity() + (capacity() >> 1)), std::forward<Args>(args)...);
			}
			else {
				std::allocator_traits<allocator_type>::construct(alloc_, end_, std::forward<Args>(args)...);
				++end_;
			}
			return back();
		}

		CONSTEXPR auto push_back(const value_type& value) ->void requires concepts::copy_insertable_into<value_type, vector> {
			this->emplace_back(value);
		}

		CONSTEXPR auto push_back(value_type&& value) ->void requires concepts::move_insertable_into<value_type, vector> {
			this->emplace_back(std::move(value));
		}

		CONSTEXPR auto insert(const_iterator pos, const_reference value) ->iterator requires concepts::copy_assignable<value_type> && concepts::copy_insertable_into<value_type, vector> {
			return this->emplace(pos, value);
		}

		CONSTEXPR auto insert(const_iterator pos, value_type&& value) ->iterator requires concepts::move_assignable<value_type> && concepts::move_insertable_into<value_type, vector> {
			return this->emplace(pos, std::move(value));
		}

		CONSTEXPR auto insert(const_iterator pos, size_type count, const_reference value) ->iterator requires concepts::copy_assignable<value_type> && concepts::copy_insertable_into<value_type, vector> {
			if (pos == cend()) {
				this->push_back_n_(count, value);
				return end_ - count;
			}
			auto idx = pos - cbegin();
			if (size() + count > capacity()) this->realloc_(std::max(size() + count, capacity() + (capacity() >> 1)));
			auto where_ = beg_ + idx;
			detail::alloc_uninitialized_move(end_ - count, end_, end_, alloc_);
			std::move_backward(where_, end_ - count, end_);
			std::fill_n(where_, count, value);
			end_ += count;
			return where_;
		}

		template<std::input_iterator InputIt> requires concepts::emplace_constructible_from<value_type, vector, std::iter_value_t<InputIt>> && std::movable<value_type> && concepts::move_insertable_into<value_type, vector>
		CONSTEXPR auto insert(const_iterator pos, InputIt first, InputIt last) ->iterator {
			if (pos == cend()) {
				return this->append_range_impl_(first, last);
			}
			auto idx = pos - cbegin();
			if (pos == cbegin()) return begin() + idx;
			auto cur = pos;
			for (auto it = first; it != last; ++it) {
				cur = this->emplace(cur, *it);
			}
			return begin() + idx;
		}

		CONSTEXPR auto insert(const_iterator pos, std::initializer_list<value_type> ilist) ->iterator requires std::movable<value_type> && concepts::move_insertable_into<value_type, vector> {
			return this->insert(pos, ilist.begin(), ilist.end());
		}

		template<std::ranges::input_range Range>
		CONSTEXPR auto insert_range(const_iterator pos, Range&& rng) ->iterator requires concepts::emplace_constructible_from<value_type, vector, std::ranges::range_reference_t<Range>> && std::movable<value_type> && concepts::move_insertable_into<value_type, vector> {
			return this->insert(pos, std::ranges::begin(rng), std::ranges::end(rng));
		}

		template<std::ranges::input_range Range>
		CONSTEXPR auto append_range(Range&& rng) ->void requires concepts::emplace_constructible_from<value_type, vector, std::ranges::range_reference_t<Range>> && concepts::move_insertable_into<value_type, vector> {
			(void) this->append_range_impl_(std::ranges::begin(rng), std::ranges::end(rng));
		}

		CONSTEXPR auto swap(vector& other) noexcept ->void { // undefined if propagate_on_container_swap::value is false and this->alloc_ not equal to other.alloc_
			if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
				std::ranges::swap(alloc_, other.alloc_);
			}
			std::ranges::swap(beg_, other.beg_);
			std::ranges::swap(end_, other.end_);
			std::ranges::swap(cap_, other.cap_);
		}

		CONSTEXPR friend auto swap(vector& lhs, vector& rhs) noexcept ->void {
			lhs.swap(rhs);
		}

		CONSTEXPR friend auto operator== (const vector& lhs, const vector& rhs) noexcept ->bool {
			if (lhs.size() != rhs.size()) return false;
			for (size_type i = 0; i < lhs.size(); ++i) {
				if (lhs[i] != rhs[i]) return false;
			}
			return true;
		}

		CONSTEXPR friend auto operator<=> (const vector& lhs, const vector& rhs) noexcept {
			return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
		}
	private:

		CONSTEXPR auto die_() noexcept ->void {
			clear();
			std::allocator_traits<allocator_type>::deallocate(alloc_, beg_, capacity());
			beg_ = nullptr;
			end_ = nullptr;
			cap_ = nullptr;
		}

		CONSTEXPR auto elem_move_assign_from_(vector& other) ->void {
			auto ptr = beg_;
			auto rng_it = other.beg_;
			auto rng_end = other.end_;
			while (ptr != end_ && rng_it != rng_end) {
				*ptr = std::move(*rng_it);
				++ptr;
				++rng_it;
			}
			detail::alloc_destroy(ptr, end_, alloc_);
			end_ = ptr;
			while (rng_it != rng_end) {
				this->push_back(std::move(*rng_it));
				++rng_it;
			}
		}

		template<typename InputIt>
		CONSTEXPR auto append_range_impl_(InputIt first, InputIt last) ->iterator {
			auto old_size_ = size();
			for (auto it = first; it != last; ++it) {
				this->push_back(*it);
			}
			return beg_ + old_size_;
		}

		CONSTEXPR auto push_back_n_(size_type count, const_reference value) ->void {
			for (size_type i = 0; i < count; ++i) {
				this->push_back(value);
			}
		}

		CONSTEXPR auto realloc_(size_type new_capacity) ->void { // assume: new_capacity >= size()
			pointer new_storage = std::allocator_traits<allocator_type>::allocate(alloc_, new_capacity);

			try {
				if constexpr (concepts::nothrow_move_insertable_into<value_type, vector> || !concepts::copy_insertable_into<value_type, vector>) {
					detail::alloc_uninitialized_move(beg_, end_, new_storage, alloc_);
				}
				else {
					detail::alloc_uninitialized_copy(beg_, end_, new_storage, alloc_);
				}
			}
			catch (...) {
				std::allocator_traits<allocator_type>::deallocate(alloc_, new_storage, new_capacity);
				throw;
			}

			auto size_ = size();

			die_();

			beg_ = new_storage;
			end_ = beg_ + size_;
			cap_ = beg_ + new_capacity;
		}

		template<typename... Args>
		CONSTEXPR auto realloc_and_emplace_back_(size_type new_capacity, Args&&... args) ->void { // assume: new_capacity > size()
			pointer new_storage = std::allocator_traits<allocator_type>::allocate(alloc_, new_capacity);
			auto size_ = size();
			try {
				std::allocator_traits<allocator_type>::construct(alloc_, new_storage + size_, std::forward<Args>(args)...);
			}
			catch (...) {
				std::allocator_traits<allocator_type>::deallocate(alloc_, new_storage, new_capacity);
				throw;
			}

			++size_;

			try {
				if constexpr (concepts::nothrow_move_insertable_into<value_type, vector> || !concepts::copy_insertable_into<value_type, vector>) {
					detail::alloc_uninitialized_move(beg_, end_, new_storage, alloc_);
				}
				else {
					detail::alloc_uninitialized_copy(beg_, end_, new_storage, alloc_);
				}
			}
			catch (...) {
				std::allocator_traits<allocator_type>::destroy(alloc_, new_storage + size_);
				std::allocator_traits<allocator_type>::deallocate(alloc_, new_storage, new_capacity);
				throw;
			}

			clear();
			std::allocator_traits<allocator_type>::deallocate(alloc_, beg_, capacity());

			beg_ = new_storage;
			end_ = beg_ + size_;
			cap_ = beg_ + new_capacity;
		}

		template<bool HasInitValue>
		CONSTEXPR auto resize_impl_(size_type new_size, const_pointer value_ptr) ->void {
			if (new_size < size()) {
				this->erase(cbegin() + new_size, cend());
			}
			else if (new_size <= capacity()) {
				if constexpr (HasInitValue) detail::alloc_uninitialized_fill(end_ , beg_ + new_size, *value_ptr, alloc_);
				else detail::alloc_uninitialized_default_construct(end_, beg_ + new_size, alloc_);
			}
			else if (new_size > capacity()) {
				size_type new_capacity = std::max(new_size, capacity() + (capacity() >> 1));
				pointer new_storage = std::allocator_traits<allocator_type>::allocate(alloc_, new_capacity);

				try {
					if constexpr (HasInitValue) detail::alloc_uninitialized_fill(new_storage + size(), new_storage + new_size, *value_ptr, alloc_);
					else detail::alloc_uninitialized_default_construct(new_storage + size(), new_storage + new_size, alloc_);
				}
				catch (...) {
					std::allocator_traits<allocator_type>::deallocate(alloc_, new_storage, new_capacity);
					throw;
				}

				try {
					if constexpr (concepts::nothrow_move_insertable_into<value_type, vector> || !concepts::copy_insertable_into<value_type, vector>) {
						detail::alloc_uninitialized_move(beg_, end_, new_storage, alloc_);
					}
					else {
						detail::alloc_uninitialized_copy(beg_, end_, new_storage, alloc_);
					}
				}
				catch (...) {
					detail::alloc_destroy(new_storage + size(), new_storage + new_size, alloc_);
					std::allocator_traits<allocator_type>::deallocate(alloc_, new_storage, new_capacity);
					throw;
				}

				clear();
				std::allocator_traits<allocator_type>::deallocate(alloc_, beg_, capacity());

				beg_ = new_storage;
				end_ = beg_ + new_size;
				cap_ = beg_ + new_capacity;
			}
		}
	private:
		pointer beg_{}, end_{}, cap_{};
		allocator_type alloc_;
	};

	template<typename T, typename Alloc, typename U> requires std::equality_comparable_with<T, U>
	CONSTEXPR auto erase(vector<T, Alloc>& c, const U& value) ->typename vector<T, Alloc>::size_type {
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = c.end() - it;
		c.erase(it, c.end());
		return r;
	}

	template<typename T, typename Alloc, typename Pred> requires std::predicate<Pred, T&>
	CONSTEXPR auto erase_if(vector<T, Alloc>& c, Pred pred) ->typename vector<T, Alloc>::size_type {
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = c.end() - it;
		c.erase(it, c.end());
		return r;
	}

	template<std::input_iterator InputIt, typename Alloc = std::allocator<typename std::iterator_traits<InputIt>::value_type>>
	vector(InputIt, InputIt, Alloc = Alloc()) -> vector<typename std::iterator_traits<InputIt>::value_type, Alloc>;

	template<std::ranges::input_range Range, typename Alloc = std::allocator<std::ranges::range_value_t<Range>> >
	vector(from_range_t, Range&&, Alloc = Alloc()) -> vector<std::ranges::range_value_t<Range>, Alloc>;
}