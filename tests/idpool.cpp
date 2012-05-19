#include "id_pool.hpp"
#include "fixedPool.hpp"
#include "chunk.h"

int main(int argc, char **argv)
{
  using namespace BDB;

  char const *work_dir = argv[1];
  
  IDPool<fixed_pool<ChunkHeader,8> > header_pool(0, work_dir, 1, 101, BDB::full);
  IDPool<vec_wrapper<AddrType> > addr_pool(0, work_dir, 1, 100, BDB::full);

  return 0;
}
