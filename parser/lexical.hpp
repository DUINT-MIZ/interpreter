#pragma once
#include "../basics.hpp"
#include <array>
#include <cctype>

namespace lexical {

using ret_type = std::size_t;

constexpr ret_type NO_MATCH = (ret_type)-1;

template <typename T>
constexpr std::array<T, 256>
chrtag_table(std::initializer_list<std::pair<unsigned char, T>> init) {
    std::array<T, 256> res{};

    for(const auto& kv : init) {
        res[kv.first] = kv.second;
    }
    
    return res;
}

constexpr auto chrtag = chrtag_table<token_tag>({
    {'+', token_tag::PLUS},
    {'-', token_tag::MINUS},
    {'*', token_tag::STAR},
    {'/', token_tag::SLASH},
    {'=', token_tag::EQUAL},
    {';', token_tag::EOFILE}
});

ret_type match_op1c(Token& buf, const char* c) {
    if((buf.tag = chrtag[(unsigned)(*c)]) == token_tag::SENT) return NO_MATCH;
    buf.view = std::string_view(c, 1);
    return 1;
}

ret_type match_num(Token& buf, const char* beg, const char* end) {
    const char* it = beg;
    bool has_digit = false;
    bool has_pm = false;
    bool has_e = false;
    bool has_coma = false;

    unsigned char c;
    while(it < end) {
        c = *(it++);
        if(std::isdigit(c)) {
            has_digit = true;
            has_pm = false;
            continue;
        }

        switch (c)
        {
        case '-' :
        case '+' :
            if(has_digit || has_pm) return NO_MATCH;
            has_pm = true;
            break;

        case '.' :
            if(!has_digit || has_e || has_coma) return NO_MATCH;
            has_coma = true;
            break;

        case 'e' :
        case 'E' :
            if(!has_digit || has_e) return NO_MATCH;
            has_e = true;
            has_digit = false;
            has_pm = false;
            break;
        
        default:
            --it;
            end = it;
            break;
        }
    }

    if(!has_digit) return NO_MATCH;
    buf.tag = ((has_coma || has_e) ? token_tag::FLOAT : token_tag::INT);
    buf.view = std::string_view(beg, it - beg);
    return buf.view.size();
}

ret_type match_ident(Token& buf, const char* beg, const char* end) {
    const char* it = beg;
    Token dummy;
    if(std::isdigit((unsigned)(*it)))
        return NO_MATCH;
    while(it < end) {
        if(std::isspace((unsigned)(*it))) break;
        if(match_op1c(dummy, it) != NO_MATCH) break; // change if needed (bit risky solution later)
        ++it;
    }
    if(it == beg) return NO_MATCH;
    buf.tag = token_tag::IDENT;
    buf.view = std::string_view(beg, it - beg);
    return buf.view.size();
}

}

