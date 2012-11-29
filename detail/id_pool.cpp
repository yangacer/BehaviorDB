#include "id_pool_def.hpp"
#include "chunk.h"
#include "fixedPool.hpp"
#include "addr_wrapper.hpp"

namespace BDB {

template class IDPool<fixed_pool<ChunkHeader, 8> >;
template class IDPool<fixed_pool<addr_wrapper, sizeof(AddrType)> >;
template class IDPool<vec_wrapper<AddrType> >;

}// namespace BDB
