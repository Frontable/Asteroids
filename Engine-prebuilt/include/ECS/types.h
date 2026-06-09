#pragma once
#include <cstdint>
#include <bitset>


using Entity = std::uint32_t;
using ComponentType = std::uint8_t;

using ComponentID = std::uint8_t;
constexpr ComponentType MAX_COMPONENTS = 32;
constexpr Entity MAX_ENTITIES = 5000;
using Signature = std::bitset<MAX_COMPONENTS>;


