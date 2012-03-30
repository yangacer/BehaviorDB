#include "sql.hpp"
#include <boost/spirit/include/qi.hpp>

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace BDBSQL = BehaviorDB::SQL;

BOOST_FUSION_ADAPT_STRUCT(
  BDBSQL::create_table,
  (std::string, tab_name)
  (BDBSQL::col_list_t, col_name)
);

BOOST_FUSION_ADAPT_STRUCT(
  BDBSQL::drop_table,
  (std::string, tab_name) 
);

BOOST_FUSION_ADAPT_STRUCT(
  BDBSQL::alter_table,
  (std::string, tab_name)
  (BDBSQL::alter_table::new_tab_or_col, new_tab_or_col_name)
);

BOOST_FUSION_ADAPT_STRUCT(
  BDBSQL::column,
  (std::string, tab_name)
  (std::string, col_name)
);

BOOST_FUSION_ADAPT_STRUCT(
  BDBSQL::insert,
  (bool, replace)
  (std::string, tab_name)
  (BDBSQL::insert::default_or_col, default_or_col_name)
  (std::vector<BDBSQL::expr>, expr_list)
)

namespace BehaviorDB{
namespace SQL{

struct bad_binary_operator{};

// XXX Remember to add types here every time a new
// syntax node created
typedef boost::variant<
  insert,
  create_table, alter_table, drop_table,
  create_index, drop_index,
  std::vector<boost::recursive_variant_>
  > sql;

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii = qi::ascii;

template <typename T>
struct strict_real_policies : qi::real_policies<T>
{
    static bool const expect_dot = true;
};

template<typename Iter>
struct sql_grammar
: qi::grammar<Iter, sql(), ascii::space_type>
{
  sql_grammar()
  : sql_grammar::base_type(sql_r)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::char_;
    using qi::string;
    using qi::space;
    using namespace qi::labels;
    using phoenix::at_c;
    using phoenix::push_back;
    using phoenix::construct;
    using phoenix::bind;

    quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];
    
    col_list_r %= '(' >> 
      ( quoted_string % ',' ) >> 
      ')'
      ;

    column_r %= 
      quoted_string >> '.' >> quoted_string
    ;  
    
    literal_r %=
      int64_ | strict_real_ | quoted_string;

    expr_r = 
      and_r [_val = _1] >>
      -(lit("OR") >> and_r [ _val = construct<binary_op<or_> >(_val, _1)] ) 
      ;
    
    and_r =
      eq_neq_r [_val = _1] >>
      -(lit("AND") >> eq_neq_r [ _val = construct<binary_op<and_> >(_val, _1)] )
      ;
    
    eq_neq_r =
      rel_r [_val = _1] >>
      -( 
        ( char_('=') >> *char_('=') >> rel_r 
          [_val = construct<binary_op<eq> >(_val, _1) ])
        | ( (lit("!=") | lit("<>")) >> rel_r 
          [_val = construct<binary_op<neq> >(_val, _1)])
       // | ( lit("IS") >> rel_r
       //   [_val = construct<binary_op<is> >(_val,_1)])
       // | ( lit("IS") >> lit("NOT") >> rel_r
       //   [_val = construct<binary_op<is_not> >(_val,_1)])
        | ( lit("LIKE") >> rel_r
          [_val = construct<binary_op<like> >(_val, _1)])
      )
      ;
    
    rel_r = 
      column_r [_val =  _1]
      | literal_r [_val = _1]
      | ('(' >> expr_r >> ')') [_val = _1]
      ;
    
    expr_list_r %=
      '(' >> ( expr_r % ',' ) >> ')';     

    cTab_r %= 
      lit("CREATE") >> lit("TABLE") >> 
      quoted_string >> 
      col_list_r
      ;

    dTab_r %=
      lit("DROP") >> lit("TABLE") >>
      quoted_string
      ;

    aTab_r %=
      lit("ALTER") >> lit("TABLE") >>
      quoted_string >>
      (
       ( lit("RENAME") >>  lit("TO") >> 
         quoted_string ) |
       ( lit("ADD") >> col_list_r ) 
      )
      ;

    ins_r = 
      ( string("INSERT") | string("REPLACE") ) 
        [ at_c<0>(_val) = (_1 == "REPLACE") ]
      >> lit("INTO") >> quoted_string [ at_c<1>(_val) = _1]
      >> 
        ( (string("DEFAULT") >> lit("VALUES") ) 
            [at_c<2>(_val) = _1 == "DEFAULT"] |
          ( col_list_r 
              [at_c<2>(_val) = _1] >> 
            lit("VALUES") >> expr_list_r 
              [at_c<3>(_val) = _1] )
        )
      ;

    sql_r %= ins_r | cTab_r | dTab_r | aTab_r | (';' >> sql_r);  
  }

  qi::int_parser< boost::int64_t >    int64_;
  qi::real_parser< 
      double, 
      strict_real_policies<double> 
    > 
    strict_real_;

  qi::rule<Iter, std::string(), ascii::space_type>  quoted_string;
  qi::rule<Iter, col_list_t(), ascii::space_type>   col_list_r;
  qi::rule<Iter, column(), ascii::space_type>       column_r;

  qi::rule<Iter, sql(), ascii::space_type>          sql_r;
  qi::rule<Iter, create_table(), ascii::space_type> cTab_r;

  qi::rule<Iter, alter_table(), ascii::space_type>  aTab_r;
  qi::rule<Iter, drop_table(), ascii::space_type>   dTab_r;
  qi::rule<Iter, expr(), ascii::space_type>          
    expr_r,
    and_r,
    eq_neq_r,
    rel_r,
    factor_r,
    concate_r,
    operand_r,
    literal_r;

  qi::rule<Iter, std::vector<expr>(), 
    ascii::space_type>  expr_list_r;

  qi::rule<Iter, insert(), ascii::space_type>       ins_r;

  //qi::rule<Iter, create_index>    cIdx_r;
  //qi::rule<Iter, drop_index>      dIdx_r;

  // qi::rule<Iter, > sql_r;
  
};

}} // namespace BehaviorDB::SQL
