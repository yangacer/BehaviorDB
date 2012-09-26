#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

void usage()
{
  cerr<<"Convert put log to get log"<<endl;
  cerr<<"logcvt put_log output"<<endl;
  exit(0);
}

int main(int argc, char** argv)
{
  if(argc < 3 ) usage();
  
  char ibuf[102400];
  char obuf[102400];
  ifstream fin; 
  ofstream fout;

  fin.rdbuf()->pubsetbuf(ibuf, 102400);
  fout.rdbuf()->pubsetbuf(obuf, 102400);
  fin.open(argv[1], ios::binary | ios::in);
  fout.open(argv[2], ios::binary | ios::out);
  
  if(!fin.is_open() || !fout.is_open())
    usage();

  string token;
  unsigned int size;
  unsigned int address(1);
  char fmt_log[100]={};
  int len(0);
  while(fin>>token){
    if("put" != token || 0 == fin>>token ) break;
    size = strtoul(token.c_str(), 0, 16);
    len = snprintf(fmt_log, 100, "%-12s\t%08x\t%08x\t%08x\n", 
      "get", size, address, 0); 
    fout.write(fmt_log, len);
    address++;
  }

  fout.close();
  fin.close();

  return 0;

}
