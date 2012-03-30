#include "sql_parser.hpp"
#include <boost/range/algorithm/copy.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <iostream>

namespace BDBSQL = BehaviorDB::SQL;

struct expr_visitor
: boost::static_visitor<void>
{
  template<typename T>
  void operator()(T const &val) const
  { std::cout<<"[unkown]"; }
  
  void operator()(BDBSQL::literal const& lit) const
  { std::cout<<"[literal]"; }

  void operator()(BDBSQL::column const& col) const
  { std::cout<<"[col]"; }

  void operator()(BDBSQL::binary_op<BDBSQL::or_> const& binary) const
  {
    std::cout<<"(";
    boost::apply_visitor(expr_visitor(), binary.left);
    std::cout<<" OR ";
    boost::apply_visitor(expr_visitor(), binary.right);
    std::cout<<")";
  }

  void operator()(BDBSQL::binary_op<BDBSQL::and_> const& binary) const
  {
    std::cout<<"(";
    boost::apply_visitor(expr_visitor(), binary.left);
    std::cout<<" AND ";
    boost::apply_visitor(expr_visitor(), binary.right);
    std::cout<<")";
  }

  void operator()(BDBSQL::binary_op<BDBSQL::eq> const& binary) const
  {
    std::cout<<"(";
    boost::apply_visitor(expr_visitor(), binary.left);
    std::cout<<" EQ ";
    boost::apply_visitor(expr_visitor(), binary.right);
    std::cout<<")";
  }

  void operator()(BDBSQL::binary_op<BDBSQL::neq> const& binary) const
  {
    std::cout<<"(";
    boost::apply_visitor(expr_visitor(), binary.left);
    std::cout<<" NEQ ";
    boost::apply_visitor(expr_visitor(), binary.right);
    std::cout<<")";
  }

  
  void operator()(BDBSQL::binary_op<BDBSQL::like> const& binary) const
  {
    std::cout<<"(";
    boost::apply_visitor(expr_visitor(), binary.left);
    std::cout<<" LIKE ";
    boost::apply_visitor(expr_visitor(), binary.right);
    std::cout<<")";
  }


};

std::ostream &operator<<(std::ostream &os, BDBSQL::insert const& ins)
{
  using namespace std;
  using boost::copy;
  using boost::get;
  using BDBSQL::insert;

  cout<<"is_rep: "<<ins.replace<<"\n";
  cout<<"tab: "<<ins.tab_name<<"\n";
  if(get<bool>(&ins.default_or_col_name))
    cout<<"is_default: true\n";
  else{
    cout<<"cols: ";
    copy(
      get<vector<string> >(ins.default_or_col_name),
      ostream_iterator<string>(cout, " "));
  }
  cout<<"expr size: "<<ins.expr_list.size()<<"\n";
  if(ins.expr_list.size())
    boost::apply_visitor(expr_visitor(), ins.expr_list[0]);
  cout<<"\n";
}  

int main(int argc, char **argv)
{
  using namespace std;
  using boost::spirit::qi::ascii::space;
  using namespace BehaviorDB::SQL;
  
  sql_grammar<string::const_iterator> grammar;
  string input = argv[1];
  
  string::const_iterator beg = input.begin(),
    end = input.end();

  sql s;

  if(!qi::phrase_parse(beg, end, grammar, space, s)){
    cerr<<"parsing failed at\n";
    cerr<<(beg-input.begin())<<"\n";
  }else{
    if(boost::get<insert>(&s)){
      insert &ins = boost::get<insert>(s);
      cout<<ins;
    }
    cout<<"GREAT!!!\n";
  }

  return 0;
}
