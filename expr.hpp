#pragma once
#include "basics.hpp"

namespace noble {

class Traversor;

class Expr {
    public :
    virtual void accept(Traversor&) = 0;
    virtual ~Expr() = default;
};

class IntExpr : public Expr {
    public :
    IntExpr(int_t n) : data(n) {}
    virtual void accept(Traversor& tr) override;
    int_t data = 0;
};

class FloatExpr : public Expr {
    public :
    FloatExpr(float_t n) : data(n) {}
    virtual void accept(Traversor& tr) override;
    float_t data = 0;
};

class IdentExpr : public Expr {
    public :
    IdentExpr(const std::string_view& view) : data(view) {}
    virtual void accept(Traversor& tr) override;
    std::string_view data = {};
};

class BinExpr : public Expr {
    public :
    BinExpr(token_tag tag) : tag(tag) {}
    virtual void accept(Traversor& tr) override;

    Expr* lhs = nullptr;
    Expr* rhs = nullptr;
    token_tag tag = token_tag::SENT;
};

class UnaryExpr : public Expr {
    public :
    UnaryExpr(token_tag tag, Expr* rhs = nullptr) : tag(tag), rhs(rhs) {}
    virtual void accept(Traversor& tr) override;
    Expr* rhs = nullptr;
    token_tag tag = token_tag::SENT;
};

class Traversor {
    public :

    virtual void visit(UnaryExpr&) {}
    virtual void visit(BinExpr&) {}
    virtual void visit(IntExpr&) {}
    virtual void visit(FloatExpr&) {}
    virtual void visit(IdentExpr&) {}
};

void UnaryExpr::accept(Traversor& tr) { tr.visit(*this); }
void BinExpr::accept(Traversor& tr) { tr.visit(*this); }
void IntExpr::accept(Traversor& tr) { tr.visit(*this); }
void FloatExpr::accept(Traversor& tr) { tr.visit(*this); }
void IdentExpr::accept(Traversor& tr) { tr.visit(*this); }

bool valid_binary_tag(token_tag tag) {
        switch (tag)
        {
        case token_tag::PLUS : case token_tag::MINUS :
        case token_tag::STAR : case token_tag::SLASH :
        case token_tag::EQUAL :
            return true;
        
        default :
            return false;
        }
    }

}