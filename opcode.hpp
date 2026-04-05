#pragma once
#include "basics.hpp"
#include <cstdint>
#include <variant>

namespace noble {

namespace opcode {
    using op_t = std::uint8_t;
    constexpr std::size_t OP_LEN = sizeof(op_t);

    constexpr op_t NONE = 0;
    constexpr op_t PUSH_INT = 1;
    constexpr op_t PUSH_FLOAT = 2;
    constexpr op_t STORE_NEW = 3;
    constexpr op_t STORE = 4;
    constexpr op_t LOAD = 5;
    constexpr op_t ADD = 6;
    constexpr op_t SUB = 7;
    constexpr op_t MUL = 8;
    constexpr op_t DIV = 9;
}

template <typename T>
constexpr const char* write_addr(const T& obj) { return reinterpret_cast<const char*>(&obj); }

}
