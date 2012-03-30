#ifndef BDB_SQL_EXPR_HPP_
#define BDB_SQL_EXPR_HPP_
#include <string>
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/cstdint.hpp>

namespace BehaviorDB {
namespace SQL{

struct column
{
  std::string tab_name;
  std::string col_name;
};

struct cat;
struct mul;
struct div;
struct mod;
struct add;
struct sub;
struct eq;
struct neq;
struct lt;
struct le;
struct gt;
struct ge;
struct and_;
struct or_;

struct like;
struct is;
struct is_not;
struct regex;
struct glob;

template<typename Op>
struct binary_op;

typedef boost::variant<
  int64_t, std::string, double 
  > literal;

typedef boost::variant<
    boost::recursive_wrapper<binary_op<eq> >,
    boost::recursive_wrapper<binary_op<neq> >,
    boost::recursive_wrapper<binary_op<is> >,
    boost::recursive_wrapper<binary_op<is_not> >,
    boost::recursive_wrapper<binary_op<like> >
  > eq_neq_op;

typedef boost::variant<
    column,
    literal,
    boost::recursive_wrapper<binary_op<cat> >,
    boost::recursive_wrapper<binary_op<mul> >,
    boost::recursive_wrapper<binary_op<div> >,
    boost::recursive_wrapper<binary_op<mod> >,
    boost::recursive_wrapper<binary_op<add> >,
    boost::recursive_wrapper<binary_op<sub> >,
    boost::recursive_wrapper<binary_op<lt> >,
    boost::recursive_wrapper<binary_op<le> >,
    boost::recursive_wrapper<binary_op<gt> >,
    boost::recursive_wrapper<binary_op<ge> >,
    boost::recursive_wrapper<binary_op<eq> >,
    boost::recursive_wrapper<binary_op<neq> >,
    boost::recursive_wrapper<binary_op<is> >,
    boost::recursive_wrapper<binary_op<is_not> >,
    boost::recursive_wrapper<binary_op<like> >,
    //boost::recursive_wrapper<binary_op<regex> >,
    //boost::recursive_wrapper<binary_op<glob> >,
    boost::recursive_wrapper<binary_op<and_> >,
    boost::recursive_wrapper<binary_op<or_> >
  > expr;


template<typename Op>
struct binary_op
{
  expr left,right;
  binary_op(expr const& l, expr const& r)
  : left(l), right(r)
  {}
};


}} // namespace BehaviorDB::SQL

#endif
