#pragma once
#include "lexical.hpp"
#include "../expr.hpp"
#include "../basics.hpp"
#include "../opcode.hpp"
#include "../type_checker.hpp"
#include <memory>
#include <charconv>
#include <string>
#include <stdexcept>
#include <iostream>

class ParserError : public std::exception {
    public :
    std::string err_msg;
    ParserError(const std::string& msg) : err_msg(msg) {}
    ParserError(std::string&& msg) : err_msg(std::move(msg)) {}

    const char* what() const noexcept override { return err_msg.c_str(); }
}; 

class ParserState {
    public :

    auto now() const noexcept { return it; }
    auto bol() const noexcept { return beg; } // Beginning of Line
    auto eol() const noexcept { return end; } // End of Line
    const Token& tok_now() const noexcept { return curr_tok; }

    bool at_eol() const noexcept { return it == end; }

    bool newline() {
        ++ln_count;
        bool status = feed(this->line);
        set_ptrs();
        return status;
    }

    void next_token() {
        if(at_eol() && !newline()) {
            curr_tok.tag = token_tag::SENT;
            curr_tok.view = {};
        }
        lexical::ret_type ret_val = 0;
        while(it < end) {
            if(std::isspace((unsigned)(*it))) {
                ++it;
                continue;
            }

            if((ret_val = lexical::match_op1c(this->curr_tok, it)) != lexical::NO_MATCH) break;
            if((ret_val = lexical::match_num(this->curr_tok, it, end)) != lexical::NO_MATCH) break;
            if((ret_val = lexical::match_ident(this->curr_tok, it,end)) != lexical::NO_MATCH) break;
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
            throw std::runtime_error("ParserState::prepare() : Empty feed functor");
        if(line.empty() && !newline()) 
            throw std::runtime_error("ParserState::prepare() : Empty line and failed to fetch new line");
        set_ptrs();
    }

    bool (*feed)(std::string&);
    private :
    void set_ptrs() noexcept {
        beg = line.data();
        it = beg;
        end = beg + line.size();
    }

    Token curr_tok;
    std::size_t ln_count = 0;
    std::string line;
    const char* beg = nullptr;
    const char* it = nullptr;
    const char* end = nullptr;
};

std::unique_ptr<expr::Expr> nud_li(ParserState& ps) {
    const Token& tok = ps.tok_now();
    const char* vbeg = tok.view.data(); // view beginning
    const char* vend = tok.view.size() + vbeg; 

    switch (tok.tag)
    {
    case token_tag::INT :
        {
            value::int_t n = 0;
            auto [ptr, ec] = std::from_chars(vbeg, vend, n);
            if((ec != std::errc{}) || (ptr != vend))
                ps.error("BUG : Invalid int value");
            return std::make_unique<expr::IntExpr>(n);
        }
    
    case token_tag::FLOAT :
        {
            value::float_t n = 0;
            auto [ptr, ec] = std::from_chars(vbeg, vend, n);
            if((ec != std::errc{}) || (ptr != vend))
                ps.error("BUG : Invalid float value");
            return std::make_unique<expr::FloatExpr>(n);
        }
    
    case token_tag::IDENT :
        {
            return std::make_unique<expr::IdentExpr>(std::string(tok.view));
        }
    
    default:
        return nullptr;
    }

}

std::unique_ptr<expr::Expr> nud(ParserState& ps) {
    
    ps.next_token();
    if(ps.tok_now().tag == token_tag::EOFILE) return nullptr;
    std::unique_ptr<expr::Expr> res{};
    if((res = nud_li(ps))) {
        
        return res;
    }
    ps.error("Invalid token");
    return nullptr;
}

std::unique_ptr<expr::Expr> parse_expr(ParserState& ps, float min_bp);

void led(ParserState& ps, std::unique_ptr<expr::Expr>& lhs, const tag_info& op_info) {
    
    if(!TypeCheck::valid_binary_tag(op_info.tag)) ps.error("Invalid binary operator");
    auto op = std::make_unique<expr::BinExpr>(op_info.tag);
    op->lhs = std::move(lhs);
    op->rhs = parse_expr(ps, op_info.bp.right);
    if(!op->rhs) ps.error("Empty right hand side");
    lhs = std::move(op);
    
}

std::unique_ptr<expr::Expr> parse_expr(ParserState& ps, float min_bp = 0) {
    
    std::unique_ptr<expr::Expr> lhs = nud(ps);
    if(!lhs) return nullptr;

    ps.next_token();
    
    while(true) {
        auto tinfo = tell_info(ps.tok_now().tag);
        if(tinfo.tag == token_tag::EOFILE) break;
        if(min_bp > tinfo.bp.left) break;
        led(ps, lhs, tinfo);
    }
    
    return lhs;
}