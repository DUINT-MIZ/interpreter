#pragma once

#include <string_view>
#include <string>
#include <array>
#include <cstdint>
#include <concepts>
#include <variant>
#include <initializer_list>
#include <stdexcept>

namespace noble {

using tag_base = std::uint8_t;

enum class token_tag : tag_base {
    SENT = 0,
    EOFILE,
    INT,
    FLOAT,
    IDENT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,
    COUNT
};

// revert a token_tag to integer value
constexpr tag_base rev(token_tag tag) { return static_cast<tag_base>(tag); }

struct binding_power {
    float left = 0;
    float right = 0;
};

struct tag_info {
    token_tag tag = token_tag::SENT;
    const char* name = "UNKNOWN";
    binding_power bp = {0, 0};
};

constexpr std::array<tag_info, rev(token_tag::COUNT)> 
tag_table(std::initializer_list<tag_info> init) {
    std::array<tag_info, rev(token_tag::COUNT)> arr{};
    for(const tag_info& tinfo : init) { arr[rev(tinfo.tag)] = tinfo; }
    return arr;
}

constexpr auto ttable = tag_table({
    tag_info{token_tag::EOFILE, "EOFILE", {0, 0}},
    tag_info{token_tag::PLUS, "PLUS", {1, 1.1}},
    tag_info{token_tag::MINUS, "MINUS", {1, 1.1}},
    tag_info{token_tag::SLASH, "SLASH", {2, 2.1}},
    tag_info{token_tag::STAR, "STAR", {2, 2.1}},
    tag_info{token_tag::INT, "INT", {0, 0}},
    tag_info{token_tag::FLOAT, "FLOAT", {0, 0}},
    tag_info{token_tag::EQUAL, "EQUAL", {10, 0}},
    tag_info{token_tag::IDENT, "IDENTIFIER", {0, 0}}
});

constexpr const tag_info& tell_info(token_tag tag) {
    if(rev(tag) >= ttable.size()) return ttable[0];
    return ttable[rev(tag)];
}

struct Token {
    token_tag tag;
    std::string_view view;
};

class ParserError : public std::exception {
    public :
    std::string msg;
    ParserError(const std::string& e) : msg(e) {}
    ParserError(std::string&& e) : msg(std::move(e)) {}

    const char* what() const noexcept override { return msg.c_str(); }
};

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