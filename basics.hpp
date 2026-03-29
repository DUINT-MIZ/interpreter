#pragma once
#include <string_view>
#include <array>
#include <cstdint>
#include <concepts>

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