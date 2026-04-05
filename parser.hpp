#pragma once
#include "allocator.hpp"
#include "expr.hpp"
#include "lexical.hpp"
#include "basics.hpp"
#include <charconv>


namespace noble {

class ParserState {
    private :
    Alloc aux_allocator;
    Alloc main_allocator;

    public :
    ParserState() {}
    Lexer lx;
    Alloc& aux_alloc() noexcept { return aux_allocator; }
    Alloc& main_alloc() noexcept { return main_allocator; }
};

Expr* parse_expr(ParserState& ps, float min_bp);

Expr* nud_li(ParserState& ps) {
    const Token& tok = ps.lx.tok_now();
    const char* vbeg = tok.view.data(); // view beginning
    const char* vend = tok.view.size() + vbeg; 

    switch (tok.tag)
    {
    case token_tag::INT :
        {
            
            int_t n = 0;
            auto [ptr, ec] = std::from_chars(vbeg, vend, n);
            if((ec != std::errc{}) || (ptr != vend))
                ps.lx.error("BUG : Invalid int value");
            return &ps.main_alloc().emplace<IntExpr>(n);
        }
    
    case token_tag::FLOAT :
        {
            float_t n = 0;
            auto [ptr, ec] = std::from_chars(vbeg, vend, n);
            if((ec != std::errc{}) || (ptr != vend))
                ps.lx.error("BUG : Invalid float value");
            return &ps.main_alloc().emplace<FloatExpr>(n);
        }
    
    case token_tag::IDENT :
        {
            const char* str = ps.aux_alloc().marray_alloc(vbeg, tok.view.size());
            return &ps.main_alloc().emplace<IdentExpr>(std::string_view(str, tok.view.size()));
        }
    
    default:
        return nullptr;
    }
}
int nud_unary_track = 0;
Expr* nud_unary(ParserState& ps) {
    const Token& tok = ps.lx.tok_now();
    token_tag tag = tok.tag;
    switch (tag)
    {
    case token_tag::PLUS :
    case token_tag::MINUS :
        {
            Expr* rhs = parse_expr(ps, 15);
            if(!rhs)
                ps.lx.error("Unary empty right hand side");
            return &ps.main_alloc().emplace<UnaryExpr>(tag, rhs);
        }
        break;
    
    default:
        return nullptr;
        break;
    }
}

int nud_track = 0;

Expr* nud(ParserState& ps) {
    ps.lx.next_token();
    if(ps.lx.tok_now().tag == token_tag::EOFILE) return nullptr;
    Expr* res = nullptr;

    if((res = nud_li(ps))) {
        ps.lx.next_token();
        return res;
    }

    if((res = nud_unary(ps)))
        return res;
    
    ps.lx.error("Invalid token");
    return nullptr;
}

int led_track = 0;

void led(ParserState& ps, Expr*& lhs, const tag_info& op_info) {
    if(!valid_binary_tag(op_info.tag)) ps.lx.error("Invalid binary operator");
    auto* op = &ps.main_alloc().emplace<BinExpr>(op_info.tag);
    op->lhs = lhs;
    op->rhs = parse_expr(ps, op_info.bp.right);
    if(!op->rhs) ps.lx.error("Empty right hand side");
    lhs = op;
}

int parse_expr_track = 0;

Expr* parse_expr(ParserState& ps, float min_bp = 0) {
    Expr* lhs = nud(ps);
    if(ps.lx.tok_now().tag == token_tag::EOFILE) return lhs;
    if(!lhs) return nullptr;
    
    while(true) {
        auto tinfo = tell_info(ps.lx.tok_now().tag);
        if(tinfo.tag == token_tag::EOFILE) break;
        if(min_bp > tinfo.bp.left) break;
        led(ps, lhs, tinfo);
    }
    return lhs;
}

}