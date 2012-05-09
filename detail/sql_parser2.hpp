
#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <string>
#include <vector>


namespace ast {

    struct add;
    struct sub;
    struct mul;
    struct div;
    struct func_call;

    template<typename OpTag> struct binary_op;

    typedef boost::variant<
            double, 
            std::string, 
            boost::recursive_wrapper<binary_op<add> >, 
            boost::recursive_wrapper<binary_op<sub> >,
            boost::recursive_wrapper<binary_op<mul> >, 
            boost::recursive_wrapper<binary_op<div> >,
            boost::recursive_wrapper<func_call>
        > expression;

    template<typename OpTag>
    struct binary_op {
        expression left;
        expression right;

        binary_op(const expression & lhs, const expression & rhs) :
            left(lhs), right(rhs) {
        }
    };

    struct func_call {
        std::string callee;
        std::vector<expression> args;

        func_call(const std::string func, const std::vector<expression> &args) :
            callee(func), args(args) {
        }
    };

    struct prototype {
        std::string name;
        std::vector<std::string> args;

        prototype(const std::string &name, const std::vector<std::string> &args) :
            name(name), args(args) {
        }
    };

    struct function {
        prototype proto;
        expression body;

        function(const prototype &proto, const expression &body) :
            body(body), proto(proto) {
        }
    };

}

BOOST_FUSION_ADAPT_STRUCT(ast::binary_op<ast::add>, (ast::expression, left) (ast::expression, right));
BOOST_FUSION_ADAPT_STRUCT(ast::binary_op<ast::sub>, (ast::expression, left) (ast::expression, right));
BOOST_FUSION_ADAPT_STRUCT(ast::binary_op<ast::mul>, (ast::expression, left) (ast::expression, right));
BOOST_FUSION_ADAPT_STRUCT(ast::binary_op<ast::div>, (ast::expression, left) (ast::expression, right));
BOOST_FUSION_ADAPT_STRUCT(ast::func_call,           (std::string, callee) (std::vector<ast::expression>, args));

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
using namespace qi;

template<typename Iterator>
struct expression: qi::grammar<Iterator, ast::expression(), ascii::space_type> {

    static ast::expression make_binop(char discriminant, const ast::expression& left, const ast::expression& right)
    {
        switch(discriminant)
        {
            case '+': return ast::binary_op<ast::add>(left, right);
            case '-': return ast::binary_op<ast::sub>(left, right);
            case '/': return ast::binary_op<ast::div>(left, right);
            case '*': return ast::binary_op<ast::mul>(left, right);
        }
        throw std::runtime_error("unreachable in make_binop");
    }

    expression() :
        expression::base_type(expr) {
        number %= lexeme[double_];
        varname %= lexeme[alpha >> *(alnum | '_')];

        simple = varname | number;
        binop %= (simple >> char_("-+*/") >> expr) [ _val = phx::bind(make_binop, qi::_2, qi::_1, qi::_3) ]; 

        expr %= binop | simple;

        BOOST_SPIRIT_DEBUG_NODE(number);
        BOOST_SPIRIT_DEBUG_NODE(varname);
        BOOST_SPIRIT_DEBUG_NODE(binop);
        BOOST_SPIRIT_DEBUG_NODE(simple);
        BOOST_SPIRIT_DEBUG_NODE(expr);
    }

    qi::rule<Iterator, ast::expression(), ascii::space_type> simple, expr, binop;
    qi::rule<Iterator, std::string(), ascii::space_type> varname;
    qi::rule<Iterator, double(), ascii::space_type> number;
};


