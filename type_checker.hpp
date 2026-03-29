#pragma once
#include "basics.hpp"
#include "expr.hpp"
#include "opcode.hpp"

class TypeCheck : public expr::Traversor {
    public :
    static bool valid_binary_tag(token_tag tag) {
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
};