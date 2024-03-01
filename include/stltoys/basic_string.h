#pragma once
#include "basic_string_base.h"
#include <initializer_list>

namespace ccat {
	template<typename CharT, typename Traits, typename Alloc>
	class basic_string : public detail::basic_string_base<CharT, Traits, Alloc> {
	private:
		using base = detail::basic_string_base<CharT, Traits, Alloc>;
		using typename base::slice_type;
		using typename base::view_type;
		using typename base::allocator_traits;
	public:
		using typename base::value_type;
		using typename base::traits_type;
		using typename base::allocator_type;
		using typename base::size_type;
		using typename base::difference_type;
		using typename base::pointer;
		using typename base::const_pointer;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = typename slice_type::iterator;
		using const_iterator = typename slice_type::const_iterator;
		using reverse_iterator = typename slice_type::reverse_iterator;
		using const_reverse_iterator = typename slice_type::const_reverse_iterator;
	public: // constructors and destructors
		basic_string() = default;
		
		CONSTEXPR basic_string(std::nullptr_t) = delete;
		
		CONSTEXPR explicit basic_string(const allocator_type& alloc) noexcept(std::is_nothrow_copy_constructible_v<allocator_type>) : base(alloc) {}
		
		CONSTEXPR basic_string(size_type count, value_type ch, const allocator_type& alloc = allocator_type()) : base(alloc, count, ch) {}
		
		CONSTEXPR basic_string(const basic_string& other, size_type pos, size_type count, const allocator_type& alloc = allocator_type()) : base(alloc, static_cast<view_type>(other).substr(pos, count)) {}
		
		CONSTEXPR basic_string(const basic_string& other, size_type pos, const allocator_type& alloc = allocator_type()) : base(alloc, static_cast<view_type>(other).substr(pos))  {}
		
		CONSTEXPR basic_string(const_pointer s, const allocator_type& alloc = allocator_type()) : base(alloc, s, traits_type::length(s)) {}
		
		CONSTEXPR basic_string(const_pointer s, size_type count, const allocator_type& alloc = allocator_type()) : base(alloc, s, count) {}
		
		template<std::input_iterator InputIt> requires std::convertible_to<std::iter_value_t<InputIt>, value_type>
		CONSTEXPR basic_string(InputIt first, InputIt last, const allocator_type& alloc =  allocator_type()) : base(alloc) {
			for (auto it = first; it != last; ++it) push_back(*it);
		}
		
		basic_string(const basic_string& other) = default;
		
		basic_string(basic_string&& other) = default;
		
		CONSTEXPR basic_string(basic_string&& other, const allocator_type& alloc) : base(alloc, std::move(other))  {}
		
		CONSTEXPR basic_string(std::initializer_list<value_type> ilist, const allocator_type& alloc = allocator_type()) : base(alloc, ilist.begin(), ilist.size()) {}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR explicit basic_string(const StringViewLike& t, const allocator_type& alloc = allocator_type()) : base(alloc, static_cast<view_type>(t)) {}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR basic_string(const StringViewLike& t, size_type pos, size_type n, const allocator_type& alloc = allocator_type()) : base(alloc, static_cast<view_type>(t).substr(pos, n)) {}
		
		~basic_string() = default;
		
	public:
		using base::size;
		using base::empty;
		using base::max_size;
		using base::length;
		using base::capacity;
		using base::clear;
		
		auto operator= (const basic_string&) ->basic_string& = default;
		auto operator= (basic_string&&) ->basic_string& = default;
		
		CONSTEXPR auto operator= (const_pointer s) ->basic_string& {
			clear();
			return append(s);
		}
		
		CONSTEXPR auto operator= (value_type ch) ->basic_string& {
			clear();
			return append(1, ch);
		}
		
		CONSTEXPR auto operator= (std::initializer_list<value_type> ilist) ->basic_string& {
			clear();
			return append(ilist);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto operator=(const StringViewLike& t) ->basic_string& {
			clear();
			return append(t);
		}
		
		CONSTEXPR auto operator= (std::nullptr_t) ->basic_string&  = delete;
		
		CONSTEXPR auto assign(size_type count, value_type ch) ->basic_string& {
			clear();
			return append(count, ch);
		}
		
		CONSTEXPR auto assign(const basic_string& str) ->basic_string& {
			return *this = str;
		}
		
		CONSTEXPR auto assign(const basic_string& str, size_type pos, size_type count = npos) ->basic_string& {
			clear();
			return append(str, pos, count);
		}
		
		CONSTEXPR auto assign(basic_string&& str) noexcept(std::is_nothrow_move_assignable_v<basic_string>) ->basic_string& {
			return *this = std::move(str);
		}
		
		CONSTEXPR auto assign(const_pointer s, size_type count) ->basic_string& {
			clear();
			return append(s, count);
		}
		
		CONSTEXPR auto assign(const_pointer s) ->basic_string& {
			return *this = s;
		}
		
		template<std::input_iterator InputIt>
		CONSTEXPR auto assign(InputIt first, InputIt second) ->basic_string& {
			clear();
			return append(first, second);
		}
		
		CONSTEXPR auto assgin(std::initializer_list<value_type> ilist) ->basic_string& {
			return *this = ilist;
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto assign(const StringViewLike& t) ->basic_string& {
			return *this = t;
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto assign(const StringViewLike& t, size_type pos, size_type count = npos) ->basic_string& {
			clear();
			return append(t, pos, count);
		}
		
		CONSTEXPR auto push_back(value_type c) ->void {
			if (size() == capacity()) reserve(calculate_next_capacity(size()));
			*slice_.end_ = c;
			++slice_.end_;
			null_terminated();
		}
		
		CONSTEXPR auto pop_back() ->void {
			--slice_.end_;
			null_terminated();
		}
		
		CONSTEXPR auto insert(size_type index, size_type count, value_type ch) ->basic_string& {
			if (index > size()) throw std::out_of_range{"in function `ccat::basic_string::insert`: the parameter `index` is out of range"};
			if (count <= (capacity() - size())) {
				traits_type::move(slice_.beg_ + index + count, slice_.beg_ + index, size() - index);
				traits_type::assign(slice_.beg_ + index, count, ch);
				slice_.end_ += count;
				null_terminated();
				return *this;
			}
			auto new_size = size() + count;
			auto new_cap = calculate_next_capacity(new_size);
			auto new_space = allocate(new_cap);
			traits_type::move(new_space, slice_.beg_, index);
			traits_type::move(new_space + index + count, slice_.beg_ + index, size() - index);
			traits_type::assign(new_space + index, count, ch);
			deallocate();
			slice_.beg_ = new_space;
			slice_.end_ = new_space + new_size;
			cap_ = new_space + new_cap;
			null_terminated();
			return *this;
		}
		
		CONSTEXPR auto insert(size_type index, const_pointer s, size_type count) ->basic_string& {
			if (index > size()) throw std::out_of_range{"in function `ccat::basic_string::insert`: the parameter `index` is out of range"};
			if (count <= (capacity() - size())) {
				traits_type::move(slice_.beg_ + index + count, slice_.beg_ + index, size() - index);
				traits_type::move(slice_.beg_ + index, s, count);
				slice_.end_ += count;
				null_terminated();
				return *this;
			}
			auto new_size = size() + count;
			auto new_cap = calculate_next_capacity(new_size);
			auto new_space = allocate(new_cap);
			traits_type::move(new_space, slice_.beg_, index);
			traits_type::move(new_space + index + count, slice_.beg_ + index, size() - index);
			traits_type::move(new_space + index, s, count);
			deallocate();
			slice_.beg_ = new_space;
			slice_.end_ = new_space + new_size;
			cap_ = new_space + new_cap;
			null_terminated();
			return *this;
		}
		
		CONSTEXPR auto insert(size_type index, const_pointer s) ->basic_string& {
			return insert(index, s, traits_type::length(s));
		}
		
		CONSTEXPR auto insert(size_type index, const basic_string& str) ->basic_string& {
			return insert(index, str.c_str(), str.size());
		}
		
		CONSTEXPR auto insert(size_type index, const basic_string& str, size_type s_index, size_type count = npos) ->basic_string& {
			if (s_index > str.size()) throw std::out_of_range("in function `ccat::basic_string::insert`: the parameter `s_index` is out of range");
			return insert(index, str.c_str() + s_index, std::min(count, str.size() - s_index));
		}
		
		CONSTEXPR auto insert(const_iterator pos, size_type count, value_type ch) ->iterator {
			auto index = pos - cbegin();
			insert(index, count, ch);
			return begin() + index;
		}
		
		CONSTEXPR auto insert(const_iterator pos, value_type ch) ->iterator {
			insert(pos, 1, ch);
		}
		
		template<std::input_iterator InputIt>
		CONSTEXPR auto insert(const_iterator pos, InputIt first, InputIt last) ->iterator {
			auto index = pos - cbegin();
			for (auto it = first; it != last; ++it) {
				pos = insert(pos, *it);
			}
			return begin() + index;
		}
		
		CONSTEXPR auto insert(const_iterator pos, std::initializer_list<value_type> ilist) -> iterator {
			return insert(pos, std::ranges::begin(ilist), std::ranges::end(ilist));
		}
		
		template<class StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto insert(size_type index, const StringViewLike& t) ->basic_string& {
			auto sv = static_cast<view_type>(t);
			return insert(index, sv.data(), sv.size());
		}
		
		template<class StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto insert(size_type index, const StringViewLike& t, size_type t_index, size_type count = npos) ->basic_string& {
			auto sv = static_cast<view_type>(t).substr(t_index, count);
			return insert(index, sv.data(), sv.size());
		}
		
		CONSTEXPR auto append(size_type count, value_type ch) ->basic_string& {
			return insert(size(), count, ch);
		}
		
		CONSTEXPR auto append(const basic_string& str) ->basic_string& {
			return insert(size(), str);
		}
		
		CONSTEXPR auto append(const basic_string& str, size_type pos, size_type count = npos) ->basic_string& {
			return insert(size(), str, pos, count);
		}
		
		CONSTEXPR auto append(const_pointer s, size_type count) ->basic_string& {
			return insert(size(), s, count);
		}
		
		CONSTEXPR auto append(const_pointer s) ->basic_string& {
			return insert(size(), s);
		}
		
		template<std::input_iterator InputIt>
		CONSTEXPR auto append(InputIt first, InputIt last) ->basic_string& {
			insert(end(), first, last);
			return *this;
		}
		
		CONSTEXPR auto append(std::initializer_list<value_type> ilist) ->basic_string& {
			return append(std::ranges::begin(ilist), std::ranges::end(ilist));
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto append(const StringViewLike& t) ->basic_string& {
			return insert(size(), static_cast<view_type>(t));
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto append(const StringViewLike& t, size_type pos, size_type count = npos) ->basic_string& {
			return insert(size(), static_cast<view_type>(t).substr(pos, count));
		}
		
		friend CONSTEXPR auto operator+ (const basic_string& lhs, const basic_string& rhs) ->basic_string {
			auto res = lhs;
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (const basic_string& lhs, const_pointer rhs) ->basic_string {
			auto res = lhs;
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (const basic_string& lhs, value_type rhs) ->basic_string {
			auto res = lhs;
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (const_pointer lhs, const basic_string& rhs) ->basic_string {
			basic_string res(lhs, rhs.get_allocator());
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (value_type lhs, const basic_string& rhs) ->basic_string {
			basic_string res(1, lhs, rhs.get_allocator());
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (basic_string&& lhs, basic_string&& rhs) ->basic_string {
			basic_string res = std::move(lhs);
			basic_string rhs_ = std::move(rhs);
			res.append(rhs_);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (basic_string&& lhs, const basic_string& rhs) ->basic_string {
			basic_string res = std::move(lhs);
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (basic_string&& lhs, const_pointer rhs) ->basic_string {
			basic_string res = std::move(lhs);
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (basic_string&& lhs, value_type rhs) ->basic_string {
			basic_string res = std::move(lhs);
			res.append(rhs);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (const basic_string& lhs, basic_string&& rhs) ->basic_string {
			basic_string res(lhs, rhs.get_allocator());
			basic_string rhs_ = std::move(rhs);
			res.append(rhs_);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (const_pointer lhs, basic_string&& rhs) ->basic_string {
			basic_string res(lhs, rhs.get_allocator());
			basic_string rhs_ = std::move(rhs);
			res.append(rhs_);
			return res;
		}
		
		friend CONSTEXPR auto operator+ (value_type lhs, basic_string&& rhs) ->basic_string {
			basic_string res(1, lhs, rhs.get_allocator());
			basic_string rhs_ = std::move(rhs);
			res.append(rhs_);
			return res;
		}
		
		CONSTEXPR auto operator+= (const basic_string& other) ->basic_string& {
			return append(other);
		}
		
		CONSTEXPR auto operator+= (value_type ch) ->basic_string& {
			return append(1, ch);
		}
		
		CONSTEXPR auto operator+= (const_pointer s) ->basic_string& {
			return append(s);
		}
		
		CONSTEXPR auto operator+= (std::initializer_list<value_type> ilist) ->basic_string& {
			return append(ilist);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto operator+= (const StringViewLike& t) ->basic_string& {
			return append(t);
		}
		
		CONSTEXPR auto erase(size_type index = 0, size_type count = npos) ->basic_string& {
			if (index > size()) throw std::out_of_range{"in function `ccat::basic_string::erase`: the parameter `index` is out of range"};
			count = std::min(count, size() - index);
			traits_type::move(slice_.beg_ + index, slice_.beg_ + index + count, size() - (index + count));
			slice_.end_ -= count;
			null_terminated();
			return *this;
		}
		
		CONSTEXPR auto erase(const_iterator pos) ->iterator {
			for (size_type i = pos - cbegin(); i < size(); ++i) {
				traits_type::assign(slice_.beg_[i], slice_.beg_[i+1]);
			}
			--slice_.end_;
			null_terminated();
			return begin() + (pos - cbegin());
		}
		
		CONSTEXPR auto erase(const_iterator first, const_iterator last) ->iterator {
			auto index = first - cbegin();
			auto count = last - first;
			traits_type::move(slice_.beg_ + index, slice_.beg_ + index + count, size() - (index + count));
			slice_.end_ -= count;
			null_terminated();
			return begin() + index;
		}
		
		CONSTEXPR auto replace(const_iterator first, const_iterator last, const basic_string& str) ->basic_string& {
			insert(erase(first, last), str.begin(), str.end());
			return *this;
		}
		
		CONSTEXPR auto replace(size_type pos, size_type count, const basic_string& str, size_type pos2, size_type count2 = npos) ->basic_string& {
			erase(pos, count);
			return insert(pos, static_cast<view_type>(str).substr(pos2, count2));
		}
		
		CONSTEXPR auto replace(size_type pos, size_type count, const_pointer cstr, size_type count2) ->basic_string& {
			erase(pos, count);
			return insert(pos, cstr, count2);
		}
		
		CONSTEXPR auto replace(const_iterator first, const_iterator last, const_pointer cstr, size_type count2) ->basic_string& {
			return insert(erase(first, last) - begin(), cstr, count2);
		}
		
		CONSTEXPR auto replace(size_type pos, size_type count, const_pointer cstr) ->basic_string& {
			return replace(pos, count, cstr, traits_type::length(cstr));
		}
		
		CONSTEXPR auto replace(const_iterator first, const_iterator last, const_pointer cstr) ->basic_string& {
			return insert(erase(first, last) - begin(), cstr);
		}
		
		CONSTEXPR auto replace(size_type pos, size_type count, size_type count2, value_type ch) ->basic_string& {
			erase(pos, count);
			return insert(pos, count2, ch);
		}
		
		CONSTEXPR auto replace(const_iterator first, const_iterator last, size_type count2, value_type ch) ->basic_string& {
			insert(erase(first, last), count2, ch);
			return *this;
		}
		
		template<std::input_iterator InputIt>
		CONSTEXPR auto replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) ->basic_string& {
			insert(erase(first, last), first2, last2);
			return *this;
		}
		
		CONSTEXPR auto replace(const_iterator first, const_iterator last, std::initializer_list<value_type> ilist) ->basic_string& {
			insert(erase(first, last), ilist);
			return *this;
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto replace(size_type pos, size_type count, const StringViewLike& t) ->basic_string& {
			erase(pos, count);
			return insert(pos, t);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto replace(const_iterator first, const_iterator last, const StringViewLike& t) ->basic_string& {
			return insert(erase(first, last) - begin(), t);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		CONSTEXPR auto replace(size_type pos, size_type count, const StringViewLike& t, size_type pos2, size_type count2 = npos) ->basic_string& {
			erase(pos, count);
			return insert(pos, t, pos2, count2);
		}
		
		CONSTEXPR operator view_type() const noexcept {
			return view_type{slice_.beg_, slice_.size()};
		}
		
		NODISCARD CONSTEXPR auto begin() noexcept ->iterator {
			return slice_.begin();
		}
		
		NODISCARD CONSTEXPR auto begin() const noexcept ->const_iterator {
			return slice_.cbegin();
		}
		
		NODISCARD CONSTEXPR auto end() noexcept ->iterator {
			return slice_.end();
		}
		
		NODISCARD CONSTEXPR auto end() const noexcept ->const_iterator {
			return slice_.cend();
		}
		
		NODISCARD CONSTEXPR auto rbegin() noexcept ->reverse_iterator {
			return slice_.rbegin();
		}
		
		NODISCARD CONSTEXPR auto rbegin() const noexcept ->const_reverse_iterator {
			return slice_.crbegin();
		}
		
		NODISCARD CONSTEXPR auto rend() noexcept ->reverse_iterator {
			return slice_.rend();
		}
		
		NODISCARD CONSTEXPR auto rend() const noexcept ->const_reverse_iterator {
			return slice_.crend();
		}
		
		NODISCARD CONSTEXPR auto cbegin() const noexcept ->const_iterator {
			return begin();
		}
		
		NODISCARD CONSTEXPR auto cend() const noexcept ->const_iterator {
			return end();
		}
		
		NODISCARD CONSTEXPR auto crbegin() const noexcept ->const_reverse_iterator {
			return rbegin();
		}
		
		NODISCARD CONSTEXPR auto crend() const noexcept ->const_reverse_iterator {
			return rend();
		}
		
		NODISCARD CONSTEXPR auto at(size_type pos) ->reference {
			return slice_.at(pos);
		}
		
		NODISCARD CONSTEXPR auto at(size_type pos) const ->const_reference {
			return slice_.at(pos);
		}
		
		NODISCARD CONSTEXPR auto operator[] (size_type pos) noexcept ->reference {
			return slice_[pos];
		}
		
		NODISCARD CONSTEXPR auto operator[] (size_type pos) const noexcept ->const_reference {
			return slice_[pos];
		}
		
		NODISCARD CONSTEXPR auto front() noexcept ->reference {
			return slice_.front();
		}
		
		NODISCARD CONSTEXPR auto front() const noexcept ->const_reference {
			return slice_.front();
		}
		
		NODISCARD CONSTEXPR auto back() noexcept ->reference {
			return slice_.back();
		}
		
		NODISCARD CONSTEXPR auto back() const noexcept ->const_reference {
			return slice_.back();
		}
		
		NODISCARD CONSTEXPR auto data() noexcept ->pointer {
			return slice_.data();
		}
		
		NODISCARD CONSTEXPR auto data() const noexcept ->const_pointer {
			return slice_.data();
		}
		
		NODISCARD CONSTEXPR auto c_str() const noexcept ->const_pointer {
			return slice_.data();
		}
		
		NODISCARD CONSTEXPR auto get_allocator() const ->allocator_type {
			return alloc_;
		}
		
		NODISCARD CONSTEXPR auto find(const basic_string& str, size_type pos = 0) const noexcept ->size_type {
			return slice_.find(str, pos);
		}
		
		NODISCARD CONSTEXPR auto find(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.find(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto find(const_pointer s, size_type pos = 0) const noexcept ->size_type {
			return slice_.find(s, pos);
		}
		
		NODISCARD CONSTEXPR auto find(value_type c, size_type pos = 0) const noexcept ->size_type {
			return slice_.find(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto find(const StringViewLike& sv, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.find(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto rfind(const basic_string& str, size_type pos = npos) const noexcept ->size_type {
			return slice_.rfind(str, pos);
		}
		
		NODISCARD CONSTEXPR auto rfind(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.rfind(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto rfind(const_pointer s, size_type pos = npos) const noexcept ->size_type {
			return slice_.rfind(s, pos);
		}
		
		NODISCARD CONSTEXPR auto rfind(value_type c, size_type pos = npos) const noexcept ->size_type {
			return slice_.rfind(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto rfind(const StringViewLike& sv, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.rfind(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_of(const basic_string& str, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_of(str, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.find_first_of(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto find_first_of(const_pointer s, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_of(s, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_of(value_type c, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_of(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto find_first_of(const StringViewLike& sv, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.find_first_of(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_not_of(str, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_not_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.find_first_not_of(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto find_first_not_of(const_pointer s, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_not_of(s, pos);
		}
		
		NODISCARD CONSTEXPR auto find_first_not_of(value_type c, size_type pos = 0) const noexcept ->size_type {
			return slice_.find_first_not_of(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto find_first_not_of(const StringViewLike& sv, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.find_first_not_of(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_of(const basic_string& str, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_of(str, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.find_last_of(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto find_last_of(const_pointer s, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_of(s, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_of(value_type c, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_of(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto find_last_of(const StringViewLike& sv, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.find_last_of(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_not_of(str, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_not_of(const_pointer s, size_type pos, size_type count) const noexcept ->size_type {
			return slice_.find_last_not_of(s, pos, count);
		}
		
		NODISCARD CONSTEXPR auto find_last_not_of(const_pointer s, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_not_of(s, pos);
		}
		
		NODISCARD CONSTEXPR auto find_last_not_of(value_type c, size_type pos = npos) const noexcept ->size_type {
			return slice_.find_last_not_of(c, pos);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto find_last_not_of(const StringViewLike& sv, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->size_type {
			return slice_.find_last_not_of(sv, pos);
		}
		
		NODISCARD CONSTEXPR auto starts_with(view_type sv) const noexcept ->bool {
			return slice_.starts_with(sv);
		}
		
		NODISCARD CONSTEXPR auto starts_with(value_type c) const noexcept ->bool {
			return slice_.starts_with(c);
		}
		
		NODISCARD CONSTEXPR auto starts_with(const_pointer s) const noexcept ->bool {
			return slice_.starts_with(s);
		}
		
		NODISCARD CONSTEXPR auto ends_with(view_type sv) const noexcept ->bool {
			return slice_.ends_with(sv);
		}
		
		NODISCARD CONSTEXPR auto ends_with(value_type c) const noexcept ->bool {
			return slice_.ends_with(c);
		}
		
		NODISCARD CONSTEXPR auto ends_with(const_pointer s) const noexcept ->bool {
			return slice_.ends_with(s);
		}
		
		NODISCARD CONSTEXPR auto contains(view_type sv) const noexcept ->bool {
			return slice_.contains(sv);
		}
		
		NODISCARD CONSTEXPR auto contains(value_type c) const noexcept ->bool {
			return slice_.contains(c);
		}
		
		NODISCARD CONSTEXPR auto contains(const_pointer s) const noexcept ->bool {
			return slice_.contains(s);
		}
		
		NODISCARD CONSTEXPR auto compare(const basic_string& str) const noexcept ->int {
			return slice_.compare(str.slice_);
		}
		
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const basic_string& str) const ->int {
			return slice_.compare(pos1, count1, str.slice_);
		}
		
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const basic_string& str, size_type pos2, size_type count2) const ->int {
			return slice_.compare(pos1, count1, str, pos2, count2);
		}
		
		NODISCARD CONSTEXPR auto compare(const_pointer s) const ->int {
			return slice_.compare(s);
		}
		
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const_pointer s) const ->int {
			return slice_.compare(pos1, count1, s);
		}
		
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const ->int {
			return slice_.compare(pos1, count1, s, count2);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto compare(const StringViewLike& sv) const noexcept(std::is_nothrow_convertible_v<const StringViewLike&, view_type>) ->int {
			return slice_.compare(sv);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const StringViewLike& sv) const ->int {
			return slice_.compare(pos1, count1, sv);
		}
		
		template<typename StringViewLike> requires std::convertible_to<const StringViewLike&, view_type> && (!std::convertible_to<const StringViewLike&, const_pointer>)
		NODISCARD CONSTEXPR auto compare(size_type pos1, size_type count1, const StringViewLike& sv, size_type pos2, size_type count2) const ->int {
			return slice_.compare(pos1, count1, sv, pos2, count2);
		}
		
		CONSTEXPR auto copy(pointer dest, size_type count, size_type pos = 0) const ->size_type {
			return slice_.copy(dest, count, pos);
		}
		
		CONSTEXPR auto swap(basic_string& other) noexcept(
			std::allocator_traits<allocator_type>::propagate_on_container_swap::value ||
			std::allocator_traits<allocator_type>::is_always_equal::value
		) ->void {
			swap(static_cast<base&>(other));
		}
		
		CONSTEXPR auto substr(size_type pos = 0, size_type count = npos) const& ->basic_string {
			return basic_string(*this, pos, count);
		}
		
		CONSTEXPR auto substr(size_type pos = 0, size_type count = npos) && ->basic_string {
			return basic_string(std::move(*this), pos, count);
		}
		
		CONSTEXPR auto slice(size_type pos = 0, size_type count = npos) & noexcept ->slice_type { // Cra3z extension
			return slice_.substr(pos, count);
		}
		
		CONSTEXPR auto reserve(size_type new_cap) ->void {
			if (new_cap <= capacity()) return; // noop
			auto new_space = allocate(new_cap);
			size_type size_ = size();
			traits_type::move(new_space, slice_.beg_, size_);
			deallocate();
			slice_.beg_ = new_space;
			slice_.end_ = new_space + size_;
			cap_ = new_space + new_cap;
			null_terminated();
		}
		
		CONSTEXPR auto shrink_to_fit() ->void {
			if (capacity() == size() || capacity() == sso_size) return; // noop
			auto size_ = size();
			auto new_cap = std::max(size_, sso_size);
			auto new_space = allocate(new_cap);
			traits_type::move(new_space, slice_.beg_, size_);
			deallocate();
			slice_.beg_ = new_space;
			slice_.end_ = new_space + size_;
			cap_ = new_space + new_cap;
			null_terminated();
		}
		
		CONSTEXPR auto resize(size_type count, value_type c = null_char) ->void {
			if (count > max_size()) throw std::length_error{"int function `ccat::basic_string::resize`: the parameter `count` is too big"};
			if (count <= capacity()) {
				for (size_type i = size(); i < count; ++i) {
					slice_.beg_[i] = c;
				}
				slice_.end_ = slice_.beg_ + count;
				null_terminated();
			}
			else {
				auto old_size = size();
				auto new_cap = calculate_least_next_capacity(count);
				auto new_space = allocate(new_cap);
				traits_type::move(new_space, slice_.beg_, old_size);
				deallocate();
				slice_.beg_ = new_space;
				slice_.end_ = new_space + count;
				cap_ = new_space + new_cap;
				for (size_type i = old_size; i < count; ++i) {
					slice_.beg_[i] = c;
				}
				null_terminated();
			}
		}
		
		friend CONSTEXPR auto operator== (const basic_string& lhs, const basic_string& rhs) noexcept ->bool {
			return lhs.slice_ == rhs.slice_;
		}
		
		friend CONSTEXPR auto operator== (const basic_string& lhs, const_pointer rhs) noexcept ->bool {
			return static_cast<view_type>(lhs) == static_cast<view_type>(rhs);
		}
		
		friend CONSTEXPR auto operator<=> (const basic_string& lhs, const basic_string& rhs) noexcept ->std::strong_ordering {
			return lhs.slice_ <=> rhs.slice_;
		}
		
		friend CONSTEXPR auto operator<=> (const basic_string& lhs, const_pointer rhs) noexcept ->std::strong_ordering {
			return static_cast<view_type>(lhs) <=> static_cast<view_type>(rhs);
		}
		
		friend auto operator<< (std::basic_ostream<CharT, std::char_traits<CharT>>& os, const basic_string& str) ->std::basic_ostream<CharT, std::char_traits<CharT>>& {
			return os << str.slice_;
		}
		
	private:
		CONSTEVAL static auto log2_ceil(std::size_t n) noexcept ->std::size_t {
			std::size_t k = 0;
			while (n > (std::size_t{1} << k)) {
				++k;
			}
			return k;
		}
		
		CONSTEXPR static auto calculate_next_capacity(size_type current_size) noexcept ->size_type {
			size_type next_capacity_ = current_size;
			for (size_type i = 0; i < log2_ceil_size_type_bit; ++i) {
				next_capacity_ |= next_capacity_ >> (size_type{1} << i);
			}
			if constexpr (std::is_unsigned_v<size_type>) {
				return next_capacity_ == static_cast<size_type>(-1) ? next_capacity_ : next_capacity_ + 1;
			}
			else {
				return next_capacity_ > (size_type{1} << (size_type_bit - 1)) ? next_capacity_ : next_capacity_ + 1;
			}
		}
		
		CONSTEXPR static auto calculate_least_next_capacity(size_type least) noexcept ->size_type {
			return least <= 1 ? 1 : calculate_next_capacity(least - 1);
		}
		
	public:
		static constexpr size_type npos = slice_type::npos;
	private:
		static constexpr std::size_t size_type_bit = sizeof(size_type) * CHAR_BIT;
		static constexpr std::size_t log2_ceil_size_type_bit = log2_ceil(size_type_bit);
	private:
		using base::null_char;
		using base::alloc_;
		using base::sso_size;
		using base::slice_;
		using base::cap_;
		using base::allocate;
		using base::deallocate;
		using base::null_terminated;
		using base::swap;
		using base::set_ptrs;
	};
	
}

namespace ccat {
	using string = basic_string<char>;
	using wstring = basic_string<wchar_t>;
}