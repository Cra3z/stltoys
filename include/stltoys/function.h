#pragma once
#include <type_traits>
#include <functional>
#include <utility>

namespace ccat {

	template<typename T>
    class function;

    template<typename R, typename... Args>
    class function<R(Args...)> {
        using pointer = void*;
        using on_destroy = void(*)(pointer);
        using on_copy = pointer(*)(pointer);
        using on_invoke = R(*)(pointer, Args...);
    public:
        using result_type = R;
    public:
        function() = default;

        function(std::nullptr_t) noexcept : function() {}

        function(const function& other) : ptr(other ? other.on_copy_(other.ptr) : nullptr), on_destroy_(other.on_destroy_), on_copy_(other.on_copy_), on_invoke_(other.on_invoke_), target_type_(other.target_type_) {}

        function(function&& other) noexcept : ptr(std::exchange(other.ptr, {})), on_destroy_(std::exchange(other.on_destroy_, {})), on_copy_(std::exchange(other.on_copy_, {})), on_invoke_(std::exchange(other.on_invoke_, {})), target_type_(std::exchange(other.target_type_, typeid(void))) {}

        template<typename Fn> requires (!std::same_as<std::remove_cvref_t<Fn>, function>) && std::is_invocable_r_v<R, Fn, Args...> && std::copy_constructible<std::decay_t<Fn>>
        function(Fn&& fn) :
            ptr(new auto(std::forward<Fn>(fn))),
            on_destroy_(+[](void* ping) ->void {
                auto pong = static_cast<std::decay_t<Fn>*>(ping);
                delete pong;
            }),
            on_copy_(+[](void* ping) ->pointer {
                auto pong = static_cast<std::decay_t<Fn>*>(ping);
                return new auto(*pong);
            }),
            on_invoke_(+[](void* ping, Args... args) ->R {
                auto pong = static_cast<std::decay_t<Fn>*>(ping);
                return std::invoke(*pong, std::forward<Args>(args)...);
            }),
            target_type_(typeid(fn)) {}

        ~function() {
            if (on_destroy_) on_destroy_(ptr);
        }

        auto operator= (std::nullptr_t) noexcept ->function& {
            function(nullptr).swap(*this);
            return *this;
        }

        auto operator= (const function& other) ->function& {
            function(other).swap(*this);
            return *this;
        }

        auto operator= (function&& other) noexcept ->function& {
            function(std::move(other)).swap(*this);
            return *this;
        }

        template<typename Fn> requires (!std::same_as<std::remove_cvref_t<Fn>, function>) && std::is_invocable_r_v<R, Fn, Args...> && std::copy_constructible<std::decay_t<Fn>>
        auto operator= (Fn&& fn) ->function& {
            function(std::forward<Fn>(fn)).swap(*this);
            return *this;
        }

        template<typename Fn>
        auto operator= (std::reference_wrapper<Fn> fn) noexcept ->function& {
            function(fn).swap(*this);
            return *this;
        }

        auto operator() (Args... args) const ->R {
            if (on_invoke_ == nullptr) throw std::bad_function_call{};
            return on_invoke_(ptr, std::forward<Args>(args)...);
        }

        [[nodiscard]]
        explicit operator bool() const noexcept {
            return bool(ptr);
        }

        auto swap(function& other) noexcept ->void {
            std::ranges::swap(ptr, other.ptr);
            std::ranges::swap(on_destroy_, other.on_destroy_);
            std::ranges::swap(on_copy_, other.on_copy_);
            std::ranges::swap(on_invoke_, other.on_invoke_);
            std::ranges::swap(target_type_, other.target_type_);
        }

        friend auto swap(function& lhs, function& rhs) noexcept ->void {
            lhs.swap(rhs);
        }

        auto target_type() const noexcept ->const std::type_info& {
            return target_type_;
        }

        template<typename T>
        auto target() noexcept ->T* {
            return target_type() == typeid(T) ? static_cast<T*>(ptr) : nullptr;
        }

        template<typename T>
        auto target() const noexcept ->const T* {
            return target_type() == typeid(T) ? static_cast<const T*>(ptr) : nullptr;
        }

        friend auto operator== (const function& f, std::nullptr_t) noexcept ->bool {
            return bool(f);
        }

    private:
        pointer                                      ptr          = nullptr;
        on_destroy                                   on_destroy_  = nullptr;
        on_copy                                      on_copy_     = nullptr;
        on_invoke                                    on_invoke_   = nullptr;
        std::reference_wrapper<const std::type_info> target_type_ = typeid(void);
    };

    namespace detail {

        template<typename T>
        struct remove_member_function_cv_ref_noexcept_helper_;

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) const noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) const& noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) volatile noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) volatile& noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) const volatile noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename R, typename... Args, bool Nothrow>
        struct remove_member_function_cv_ref_noexcept_helper_<R(Args...) const volatile& noexcept(Nothrow)> {
            using type = R(Args...);
        };

        template<typename T>
        struct operator_invoke_type_helper_;

        template<typename FunctionType, typename Cls>
        struct operator_invoke_type_helper_<FunctionType Cls::*> : remove_member_function_cv_ref_noexcept_helper_<FunctionType> {};

        template<typename T>
        using operator_invoke_type_ = typename operator_invoke_type_helper_<T>::type;
    }

    template<typename R, typename... Args>
    function(R(*)(Args...)) -> function<R(Args...)>;

    template<typename Fn>
    function(Fn fn) -> function<detail::operator_invoke_type_<decltype(&Fn::operator())>>;

    namespace detail {
        template<class R, class F, class... Args> requires std::is_invocable_r_v<R, F, Args...>
        constexpr auto invoke_r(F&& f, Args&&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>) ->R {
            if constexpr (std::is_void_v<R>) std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            else return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }

        template<char Ref, bool Const, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base;

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'N', false, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'N', true, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) const noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'L', false, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) & noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'L', true, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) const& noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'R', false, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) && noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_base<'R', true, NoThrow, R, Args...> {
            virtual auto operator() (Args... args) const&& noexcept(NoThrow) ->R = 0;
            virtual ~move_only_fn_storage_base() = default;
        };

        template<typename F, char Ref, bool Const, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t;

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'N', false, NoThrow, R, Args...> : move_only_fn_storage_base<'N', false, NoThrow, R, Args...> {
            auto operator() (Args... args) noexcept(NoThrow) ->R override {
                return invoke_r<R>(f, std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}
            F f;
        };

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'N', true, NoThrow, R, Args...> : move_only_fn_storage_base<'N', true, NoThrow, R, Args...>  {
            auto operator() (Args... args) const noexcept(NoThrow) ->R override {
                return invoke_r<R>(static_cast<const F&>(f), std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}

            F f;
        };

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'L', false, NoThrow, R, Args...> : move_only_fn_storage_base<'L', false, NoThrow, R, Args...> {
            auto operator() (Args... args) & noexcept(NoThrow) ->R override {
                return invoke_r<R>(f, std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}

            F f;
        };

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'L', true, NoThrow, R, Args...> : move_only_fn_storage_base<'L', true, NoThrow, R, Args...>  {
            auto operator() (Args... args) const& noexcept(NoThrow) ->R override {
                return invoke_r<R>(static_cast<const F&>(f), std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}

            F f;
        };

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'R', false, NoThrow, R, Args...> : move_only_fn_storage_base<'R', false, NoThrow, R, Args...> {
            auto operator() (Args... args) && noexcept(NoThrow) ->R override {
                return invoke_r<R>(static_cast<F&&>(f), std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}

            F f;
        };

        template<typename F, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_storage_t<F, 'R', true, NoThrow, R, Args...> : move_only_fn_storage_base<'R', true, NoThrow, R, Args...> {
            auto operator() (Args... args) const&& noexcept(NoThrow) ->R override {
                return invoke_r<R>(static_cast<const F&&>(f), std::forward<Args>(args)...);
            }
            move_only_fn_storage_t(const F& f_) : f(f_) {}

            move_only_fn_storage_t(F&& f_) noexcept(std::is_nothrow_move_constructible_v<F>) : f(std::move(f_)) {}

            template<typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, CArgs&&... args) : f(std::forward<CArgs>(args)...) {}

            template<typename U, typename... CArgs>
            move_only_fn_storage_t(std::in_place_t, std::initializer_list<U> il, CArgs&&... args) : f(il, std::forward<CArgs>(args)...) {}

            F f;
        };

        template<typename F, char Ref, bool Const, bool NoThrow, typename U>
        struct is_callable_from : std::false_type {};

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'N', Const, false, R(Args...)> { // const(?) noexcept(false)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_invocable_r_v<R, VT, Args...> && std::is_invocable_r_v<R, VT&, Args...>;
        };

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'N', Const, true, R(Args...)> { // const(?) noexcept(true)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_nothrow_invocable_r_v<R, VT, Args...> && std::is_nothrow_invocable_r_v<R, VT&, Args...>;
        };

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'L', Const, false, R(Args...)> { // const(?) & noexcept(false)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_invocable_r_v<R, VT&, Args...>;
        };

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'L', Const, true, R(Args...)> { // const(?) & noexcept(true)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_nothrow_invocable_r_v<R, VT&, Args...>;
        };

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'R', Const, false, R(Args...)> { // const(?) && noexcept(false)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_invocable_r_v<R, VT, Args...>;
        };

        template<typename F, bool Const, typename R, typename... Args>
        struct is_callable_from<F, 'R', Const, true, R(Args...)> { // const(?) && noexcept(true)
            using T = std::decay_t<F>;
            using VT = std::conditional_t<Const, const T, T>;
            static constexpr bool value = std::is_nothrow_invocable_r_v<R, VT, Args...>;
        };

        template<char Ref, bool Const, bool NoThrow, typename R, typename... Args>
        struct move_only_fn_impl {
            using result_type = R;

            move_only_fn_impl() = default;

            move_only_fn_impl(std::nullptr_t) noexcept {}

            template<typename F> requires is_callable_from<F, Ref, Const, NoThrow, R(Args...)>::value
            move_only_fn_impl(F&& f) : ptr(new move_only_fn_storage_t<std::decay_t<F>, Ref, Const, NoThrow, R, Args...>(std::forward<F>(f)))  {}

            template<typename T, typename... TArgs> requires is_callable_from<T, Ref, Const, NoThrow, R(Args...)>::value
            explicit move_only_fn_impl(std::in_place_type_t<T>, TArgs&&... args) : ptr(new move_only_fn_storage_t<T, Ref, Const, NoThrow, R, Args...>(std::in_place, std::forward<Args>(args)...)) {}

            template<typename T, typename U, typename... TArgs> requires is_callable_from<T, Ref, Const, NoThrow, R(Args...)>::value
            explicit move_only_fn_impl(std::in_place_type_t<T>, std::initializer_list<U> il, TArgs&&... args) : ptr(new move_only_fn_storage_t<T, Ref, Const, NoThrow, R, Args...>(std::in_place, il, std::forward<Args>(args)...)) {}

            move_only_fn_impl(const move_only_fn_impl&) = delete;

            move_only_fn_impl(move_only_fn_impl&& other) noexcept : ptr(std::exchange(other.ptr, {})) {}

            ~move_only_fn_impl() noexcept {
                delete ptr;
            }

            auto operator= (const move_only_fn_impl& other) noexcept ->move_only_fn_impl& = delete;

            auto operator= (move_only_fn_impl&& other) noexcept ->move_only_fn_impl& {
                ptr = std::exchange(other.ptr, {});
                return *this;
            }

            auto operator= (std::nullptr_t) noexcept ->move_only_fn_impl& {
                delete std::exchange(ptr, {});
                return *this;
            }

            template<typename F> requires (!std::derived_from<std::remove_cvref_t<F>, move_only_fn_impl>)
            auto operator= (F&& f) ->move_only_fn_impl& {
                move_only_fn_impl tmp = std::forward<F>(f);
                swap(tmp);
                return *this;
            }

            friend auto operator== (const move_only_fn_impl& lhs, std::nullptr_t) noexcept ->bool {
                return lhs.ptr == nullptr;
            }

            explicit operator bool() const noexcept {
                return ptr;
            }

            auto swap(move_only_fn_impl& other) noexcept ->void {
                std::swap(ptr, other.ptr);
            }

            friend auto swap(move_only_fn_impl& lhs, move_only_fn_impl& rhs) noexcept ->void {
                lhs.swap(rhs);
            }

            move_only_fn_storage_base<Ref, Const, NoThrow, R, Args...>* ptr = nullptr;
        };

    }

    template<typename... >
    class move_only_function;

    template<typename R, typename... Args >
    class move_only_function<R(Args...)> : public detail::move_only_fn_impl<'N', false, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'N', false, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) noexcept> : public detail::move_only_fn_impl<'N', false, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'N', false, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) noexcept ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) &>: public detail::move_only_fn_impl<'L', false, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'L', false, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) & ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) & noexcept>: public detail::move_only_fn_impl<'L', false, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'L', false, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) & noexcept ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) &&> : public detail::move_only_fn_impl<'R', false, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'R', false, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) && ->R {
            return std::move(*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) && noexcept> : public detail::move_only_fn_impl<'R', false, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'R', false, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) && noexcept ->R {
            return std::move(*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args >
    class move_only_function<R(Args...) const> : public detail::move_only_fn_impl<'N', true, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'N', true, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args>
    class move_only_function<R(Args...) const noexcept> : public detail::move_only_fn_impl<'N', true, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'N', true, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const noexcept ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args>
    class move_only_function<R(Args...) const&> : public detail::move_only_fn_impl<'L', true, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'L', true, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const& ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args>
    class move_only_function<R(Args...) const& noexcept> : public detail::move_only_fn_impl<'L', true, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'L', true, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const& noexcept ->R {
            return (*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args>
    class move_only_function<R(Args...) const &&> : public detail::move_only_fn_impl<'R', true, false, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'R', true, false, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const && ->R {
            return std::move(*this->ptr)(std::forward<Args>(args)...);
        };
    };

    template<typename R, typename... Args>
    class move_only_function<R(Args...) const&& noexcept> : public detail::move_only_fn_impl<'R', true, true, R, Args...> {
        using base_type_ = detail::move_only_fn_impl<'R', true, true, R, Args...>;
    public:
        using base_type_::base_type_;
        using base_type_::operator=;

        auto operator() (Args... args) const&& noexcept ->R {
            return std::move(*this->ptr)(std::forward<Args>(args)...);
        };
    };
}