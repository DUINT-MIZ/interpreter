#pragma once
#include "basics.hpp"

#include <stdexcept>
#include <array>
#include <cctype>
#include <functional>

namespace noble {

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

class Lexer {
    public :

    auto now() const noexcept { return it; }
    auto bol() const noexcept { return beg; } // Beginning of Line
    auto eol() const noexcept { return end; } // End of Line
    const noble::Token& tok_now() const noexcept { return curr_tok; }

    bool at_eol() const noexcept { return it == end; }

    bool newline() {
        ++ln_count;
        bool status = feed(this->line);
        set_ptrs();
        return status;
    }

    void next_token() {
        if(at_eol() && !newline()) {
            curr_tok.tag = noble::token_tag::SENT;
            curr_tok.view = {};
        }
        noble::ret_type ret_val = 0;
        while(it < end) {
            if(std::isspace((unsigned)(*it))) {
                ++it;
                continue;
            }

            if((ret_val = noble::match_op1c(this->curr_tok, it)) != noble::NO_MATCH) break;
            if((ret_val = noble::match_num(this->curr_tok, it, end)) != noble::NO_MATCH) break;
            if((ret_val = noble::match_ident(this->curr_tok, it,end)) != noble::NO_MATCH) break;
            error("Invalid Syntax");
        }
        it += ret_val;
    }

    void error(const std::string& err_msg) {
        throw ParserError(
            "Error at line " + std::to_string(ln_count)
            + " at column " + std::to_string(it - beg)
            + " : " + err_msg
        );
    }

    void prepare() {
        if(!feed)
            throw std::runtime_error("Lexer::prepare() : Empty feed functor");
        if(line.empty() && !newline()) 
            throw std::runtime_error("Lexer::prepare() : Empty line and failed to fetch new line");
        set_ptrs();
    }

    std::function<bool(std::string_view&)> feed;
    private :
    void set_ptrs() noexcept {
        beg = line.data();
        it = beg;
        end = beg + line.size();
    }

    noble::Token curr_tok;
    std::size_t ln_count = 0;
    std::string_view line;
    const char* beg = nullptr;
    const char* it = nullptr;
    const char* end = nullptr;
};

}