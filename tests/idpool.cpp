#include "id_pool.hpp"
#include "fixedPool.hpp"
#include "chunk.h"
#include <string>
int main(int argc, char **argv)
{
  using namespace BDB;

  char const *work_dir = argv[1];
  std::string prefix;

  AddrType addr;

  {
    prefix = work_dir;
    prefix.append("chk_");
    IDPool<fixed_pool<ChunkHeader,8> > header_pool(0, prefix.c_str(), 1, 101, BDB::dynamic);
    prefix = work_dir;
    prefix.append("gid_");
    IDPool<vec_wrapper<AddrType> > addr_pool(0, prefix.c_str(), 1, 101, BDB::full);

    addr = header_pool.Acquire(4u);
    ChunkHeader ch;
    ch.size = 1024;
    header_pool.Commit(addr, ch);

    addr_pool.Acquire(3u);
    addr_pool.Commit(3u, 123);
  }
  
  {
    prefix = work_dir;
    prefix.append("chk_");
    IDPool<fixed_pool<ChunkHeader,8> > header_pool(0, prefix.c_str(), 1, 101, BDB::dynamic);
    prefix = work_dir;
    prefix.append("gid_");
    IDPool<vec_wrapper<AddrType> > addr_pool(0, prefix.c_str(), 1, 101, BDB::full);

    ChunkHeader ch;
    ch = header_pool.Find(addr);
    assert(ch.size == 1024);

    AddrType val = addr_pool.Find(3u);
    assert(val == 123);
  }

  return 0;
}
