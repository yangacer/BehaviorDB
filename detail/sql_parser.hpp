#include "sql.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  BehaviorDB::SQL::create_table,
  (std::string, tab_name)
  (BehaviorDB::SQL::col_list_t, col_name)
)

BOOST_FUSION_ADAPT_STRUCT(
  BehaviorDB::SQL::drop_table,
  (std::string, tab_name) 
)

BOOST_FUSION_ADAPT_STRUCT(
  BehaviorDB::SQL::alter_table,
  (std::string, tab_name)
  (BehaviorDB::SQL::alter_table::new_tab_or_col, new_tab_or_col_name)
)

namespace BehaviorDB{
namespace SQL{

typedef boost::variant<
  create_table, alter_table, drop_table,
  create_index, drop_index,
  std::vector<boost::recursive_variant_>
  > sql;

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template<typename Iterator>
struct sql_grammar
: qi::grammar<Iterator, sql()>
{
  sql_grammar()
  : sql_grammar::base_type(sql_r)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::char_;
    using qi::skip;
    using qi::space;
    using namespace qi::labels;
    using phoenix::at_c;
    using phoenix::push_back;

    quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];
    
    col_list_r %= skip(space)[
      '(' >> 
      ( quoted_string % ',' ) >>
      ')'
      ]
      ;

    cTab_r %= 
      skip(space)[
        lit("CREATE") >> lit("TABLE") >> 
        quoted_string >> 
        col_list_r
        ]
      ;

    dTab_r %=
      skip(space)[
        lit("DROP") >> lit("TABLE") >>
        quoted_string
      ]
      ;

    aTab_r =
      skip(space)[
        lit("ALTER") >> lit("TABLE") >>
        quoted_string >>
        (
          ( lit("RENAME") >>  lit("TO") >> 
            quoted_string ) |
          ( lit("ADD") >> col_list_r ) 
        )
      ]
      ;
    
    sql_r %= cTab_r | dTab_r | aTab_r | (';' >> sql_r);  
  }

  qi::rule<Iterator, std::string()>   quoted_string;
  qi::rule<Iterator, col_list_t()>    col_list_r;
  
  qi::rule<Iterator, sql()>           sql_r;
  qi::rule<Iterator, create_table()>  cTab_r;

  qi::rule<Iterator, alter_table()>   aTab_r;
  qi::rule<Iterator, drop_table()>    dTab_r;
  //qi::rule<Iterator, create_index>    cIdx_r;
  //qi::rule<Iterator, drop_index>      dIdx_r;

  // qi::rule<Iterator, > sql_r;
  
};

}} // namespace BehaviorDB::SQL
