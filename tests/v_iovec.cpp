#include "v_iovec.hpp"
#include <cstdio>
#include <cassert>
#include <iostream>
#include <cstring>

int main(int argc, char** argv)
{
  using namespace std;
  using namespace BDB;

  char const *file = argv[1];
  char const *str = argv[2];
  char const *dest_file = argv[3];

  char buf[4] = {};
  size_t const bsize = 3;

  assert(argc >= 4 && "missing arguments");

  FILE* fp = fopen(file, "rb");
  FILE* dest = fopen(dest_file, "wb");

  assert(fp && dest && "fopen failed");
  
  viov vv[3];

  file_src fs;
  fs.fp = fp;
  fs.off = 0;

  vv[1].data = fs;
  vv[1].size = 4;

  vv[0].data = str;
  vv[0].size = strlen(str);

  fs.off = 4;

  vv[2].data = fs;
  vv[2].size = 7;

  write_viov wv;
  wv.dest = dest;
  wv.dest_pos = 2;
  wv.buf = buf;
  wv.bsize = bsize;
  
  for(int i=0;i<2;++i){
    wv.size = vv[i].size;
    // seek to begin to see if the visitor still
    // work correctly
    fseeko(dest, 0, SEEK_SET);
    boost::apply_visitor(wv, vv[i].data);
    wv.dest_pos += vv[i].size;
  }

  fflush(dest);
  
  return 0;
}
