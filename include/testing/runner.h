#pragma once
#include <type_traits>
#include <functional>
#include <string>

#define NAMEOF(x) #x
#define FACT(fn) \
    inline void fn(); \
    const auto fn##_spec = new Fact(NAMEOF(fn), fn); \
    void fn()

#define THEORY(data, fn) \
    inline void fn(); \
    const auto fn##_spec = new Theory(NAMEOF(fn), fn, data); \
    void fn

template <typename T>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)>
{
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
    static constexpr auto arity = sizeof...(Args);
};

template <typename FunctionPtr>
struct TestCase
{
    std::string name;
    std::function<FunctionPtr> execute;
};

struct Fact : TestCase<void ()>
{
    explicit Fact(const std::string& name, const std::function<void ()>& execute) : TestCase(name, execute)
    {
    }
};

template <typename FunctionPtr>
struct Theory : TestCase<FunctionPtr>
{
    using data_t = function_traits<FunctionPtr>::args_tuple;

    std::function<FunctionPtr> execute;
    data_t data;

    explicit Theory(const std::string& name, const std::function<FunctionPtr>& execute, const data_t data)
        : TestCase<FunctionPtr>(name, execute),
          data(data)
    {
    }
};