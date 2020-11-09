#pragma once

#include <optional>
#include <functional>

typedef unsigned int uint;

template<typename T>
using Optional = std::optional<std::reference_wrapper<T>>;
