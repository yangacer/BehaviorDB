#include "sql_parser.hpp"
#include <boost/range/algorithm/copy.hpp>
#include <iostream>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace BehaviorDB::SQL;
  
  sql_grammar<string::iterator> grammar;
  string input = argv[1];
  
  string::iterator beg = input.begin(),
    end = input.end();

  sql s;

  if(!qi::parse(beg, end, grammar, s)){
    cerr<<"parsing failed at\n";
    cerr<<(beg-input.begin())<<"\n";
  }else{
    create_table &ctab = boost::get<create_table>(s);
    cout<<"tab_name:"<<ctab.tab_name<<"\n";
    cout<<"col_list:";
    boost::copy(
      ctab.col_name, 
      ostream_iterator<string>(cout, " "));  
  
  }

  return 0;
}
