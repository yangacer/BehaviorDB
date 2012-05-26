#include "id_pool.hpp"
#include "fixedPool.hpp"
#include "chunk.h"
#include <string>
#include <limits>
#include <stdexcept>
#include <iostream>
#include "error.hpp"
#include "id_handle_def.hpp"
#include "exception.hpp"

int main(int argc, char **argv)
{
  using namespace BDB;
  using std::cout;
  
  char const *work_dir = argv[1];
  std::string prefix;

  AddrType addr;
  AddrType end_addr = std::numeric_limits<AddrType>::max();

  typedef IDPool<fixed_pool<ChunkHeader, 8> >  header_pool_t;
  typedef IDPool<vec_wrapper<AddrType> > addr_pool_t;

  {
    prefix = work_dir;
    prefix.append("chk_");
    header_pool_t header_pool(0, prefix.c_str(), 1, end_addr, BDB::dynamic);
    prefix = work_dir;
    prefix.append("gid_");
    addr_pool_t addr_pool(0, prefix.c_str(), 1, 101, BDB::full);

    addr = header_pool.Acquire(4u);
    ChunkHeader ch;
    ch.size = 1024;
    header_pool.Commit(addr, ch);

    addr_pool.Acquire(3u);
    addr_pool.Commit(3u, 123);
    
    cout<<"header pool size: "<<header_pool.size()<<"\n";  
    cout<<"addr pool size: "<<addr_pool.size()<<"\n";  
  }
  
  {
    prefix = work_dir;
    prefix.append("chk_");
    header_pool_t header_pool(0, prefix.c_str(), 1, end_addr, BDB::dynamic);
    prefix = work_dir;
    prefix.append("gid_");
    addr_pool_t addr_pool(0, prefix.c_str(), 1, 101, BDB::full);

    ChunkHeader ch;
    ch = header_pool.Find(addr);
    assert(ch.size == 1024);

    AddrType val = addr_pool.Find(3u);
    assert(val == 123);

    cout<<"header pool size: "<<header_pool.size()<<"\n";  
    cout<<"addr pool size: "<<addr_pool.size()<<"\n";  
  }
  
  try{
    addr_pool_t error(0, "error", 0, 0, BDB::full);
  }catch(std::invalid_argument const &){
    std::cerr<<"invalid ctor parameter catched\n";    
  }
  
  { // id handle test
    prefix = work_dir;
    prefix.append("chk_");
    header_pool_t header_pool(0, prefix.c_str(), 1, end_addr, BDB::dynamic);
    AddrType addr;

    {
      id_handle<header_pool_t> ih(detail::ACQUIRE_AUTO, header_pool);
      addr = ih.addr();
    }

    assert(false == header_pool.isAcquired(addr) && 
           "non commit acquire should be released");
    
    {
      id_handle<header_pool_t> ih(detail::ACQUIRE_AUTO, header_pool);
      addr = ih.addr();
      ih.value().size = 1111;
      ih.commit();
      ChunkHeader header = header_pool.Find(addr);
      assert(header.size == 1111);
    }

    {
      while(1){
        try{
          id_handle<header_pool_t> ih(detail::ACQUIRE_SPEC, header_pool, addr);
          ih.value().size = 9999;
          ih.commit();
          break;
        }catch(BDB::invalid_addr const &ia){
          cout<<"invalid_addr catched\n";
          addr++;
        }
      }
      ChunkHeader header = header_pool.Find(addr);
      assert(header.size == 9999);
    }

    { // abort modify test
      id_handle<header_pool_t> ih(detail::MODIFY, header_pool, addr);
    }

    assert(header_pool.Find(addr).size == 9999);
    
    { // commit modify test
      id_handle<header_pool_t> ih(detail::MODIFY, header_pool, addr);
      ih.value().size = 8888;
      ih.commit();
      assert(header_pool.Find(addr).size == 8888);
    }
 
    { // abort release test
      id_handle<header_pool_t> ih(detail::RELEASE, header_pool, addr);
    }

    assert(header_pool.Find(addr).size == 8888);
    
    { // commit release test
      id_handle<header_pool_t> ih(detail::RELEASE, header_pool, addr);
      ih.commit();
      assert(header_pool.isAcquired(addr) == false);
    }

    { // readonly test
      header_pool_t::value_type val;
      while(1){
        try{
          id_handle<header_pool_t> ih(detail::READONLY, header_pool, addr);
          val = ih.const_value();
          ih.value().size = 7777; 
        }catch(BDB::invalid_addr const &ia){
          cout<<"invalid_addr is catched\n";
          addr--;
          continue;
        }catch(std::runtime_error const &re){
          cout<<"runtime_error(access to READONLY) is catched\n";
          break;
        }
      }
      assert(header_pool.Find(addr).size == val.size);
    }

  } // id handle test

  return 0;
}
