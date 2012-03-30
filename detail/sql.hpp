#ifndef BDB_SQL_HPP_
#define BDB_SQL_HPP_

#include <vector>
#include <string>
#include <boost/variant.hpp>
#include "sql_expr.hpp"

namespace BehaviorDB {
namespace SQL {

typedef std::vector<std::string> col_list_t;
typedef std::vector<std::string> str_list_t;

// -------------- Table Operation --------------------

struct create_table
{
  std::string tab_name;
  col_list_t col_name;
};

struct alter_table
{
  typedef boost::variant<
    std::string,
    str_list_t
  > new_tab_or_col;

  std::string tab_name;
  new_tab_or_col new_tab_or_col_name;
};

struct drop_table
{
  std::string tab_name;
};

// --------------- Insert Operation -------------------

struct insert
{
  typedef boost::variant<bool, col_list_t> default_or_col;

  bool replace;
  std::string tab_name;
  default_or_col default_or_col_name;
  std::vector<expr> expr_list;   
};

// --------------- Delete Operation -------------------

struct deletion
{
  std::string tab_name;
  expr where;
};

// --------------- Select Operation --------------------

struct result_col_spec
{
  std::string tab_name;
  std::string filter;
  std::string alias;
};

struct result_column
{
  bool all;
  result_col_spec col_spec;
  
};

struct select_core
{
  bool distinct;
  std::vector<result_column> res_col;
  str_list_t sources;
  expr where;
};

// ---------------- Index Operation --------------------

struct create_index
{
  std::string idx_name;
  std::string tab_name;
  col_list_t col_name;
};

struct drop_index
{
  std::string idx_name;  
};

} // namespace SQL
} // namespace BehaviorDB

#endif
