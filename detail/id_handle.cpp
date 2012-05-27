#include "id_handle_def.hpp"
#include "id_pool.hpp"
#include "fixedPool.hpp"
#include "chunk.h"

namespace BDB {
  template struct id_handle<IDPool<fixed_pool<ChunkHeader, 8> > >;
  template struct id_handle<IDPool<vec_wrapper<AddrType> > >;
}

