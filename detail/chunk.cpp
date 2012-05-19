#include "chunk.h"

#include <cstdlib>
#include <cstring>
#include <istream>
#include <sstream>
#include <ostream>
#include <iomanip>

//#include <iostream>

using std::hex;


std::istream& 
operator>>(std::istream &is, ChunkHeader &ch)
{
  static char buf[9];
  buf[8] = 0;
  is.read(buf, 8);
  ch.size = strtoul(buf, 0, 16);
  return is;  
}

std::ostream& 
operator<<(std::ostream &os, ChunkHeader const &ch)
{
  
  using std::ios;
  using std::setfill;
  using std::setw;
  
  ios::fmtflags oldflag = os.flags();
  os.unsetf(oldflag);
  os<<setfill('0')<<setw(8)<<hex<<ch.size;
  os.flags(oldflag);

  return os;  
}

FILE*
operator>>(FILE* fp, ChunkHeader &ch)
{
  static char buf[9];
  buf[8] = 0;
  if(8 != fread(buf, 1, 8, fp))
    return 0;
  ch.size = strtoul(buf, 0, 16);
  return fp;
}

FILE*
operator<<(FILE* fp, ChunkHeader const &ch)
{
  using namespace std;
  stringstream cvt;
  cvt<<setfill('0')<<setw(8)<<hex<<ch.size;

  if( 8 != fwrite(cvt.str().c_str(), 1, 8, fp))
    return 0;
  
  return fp;

}


int
read_header(FILE* fp, ChunkHeader &ch)
{
  static char buf[9];
  buf[8] = 0;
  if(8 != fread(buf, 1, 8, fp)){
    return -1;
  }
  ch.size = strtoul(buf, 0, 16);
  return 0;
}

int
write_header(FILE* fp, ChunkHeader const& ch)
{ 
  if(0 >  fprintf(fp, "%08x", ch.size)){
    return -1;
  }
  return 0;
}

