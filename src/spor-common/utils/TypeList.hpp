#pragma once

#include <variant>

// Simple type list
template<typename... Ts>
struct TypeList {};

// ToVariant with overloads for single and multiple TypeLists
template<typename... Args>
struct ToVariant;

// Single TypeList
template<typename... Ts>
struct ToVariant<TypeList<Ts...>> {
    using type = std::variant<Ts...>;
};

// Multiple TypeLists
template<typename... Ts1, typename... Ts2, typename... Rest>
struct ToVariant<TypeList<Ts1...>, TypeList<Ts2...>, Rest...> {
    using type = typename ToVariant<TypeList<Ts1..., Ts2...>, Rest...>::type;
};

// Alias template for cleaner syntax
template<typename... Args>
using ToVariant_t = typename ToVariant<Args...>::type;
