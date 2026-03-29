#pragma once
#include "basics.hpp"
#include <cstdint>
#include <variant>

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

namespace value {
    using int_t = std::uint32_t;
    using float_t = double;

    constexpr std::size_t INT_LEN = sizeof(int_t);
    constexpr std::size_t FLOAT_LEN = sizeof(float_t);

    using Value = std::variant<std::monostate, int_t, float_t>;

    namespace hlpr {
        struct v_info {
            const char* name = "UNKNOWN";
        };

        template <typename T> struct vinfo_select
            { static constexpr v_info info{}; };
        
        template <> struct vinfo_select<int_t>
            { static constexpr v_info info{"INTEGER"}; };
        
        template <> struct vinfo_select<float_t>
            { static constexpr v_info info{"FLOAT"}; };

        template <> struct vinfo_select<std::monostate>
            { static constexpr v_info info{"NONE"}; };
    }

    template <typename T>
    constexpr const hlpr::v_info& tell_info() { return hlpr::vinfo_select<T>::info; }

    constexpr const hlpr::v_info& tell_info(const Value& v) {
        return std::visit([](auto&& arg) -> const hlpr::v_info& {
            using T = std::remove_cvref_t<decltype(arg)>;
            return tell_info<T>();
        }, v);
    }
}