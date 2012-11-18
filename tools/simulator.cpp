#include <fstream>
#include <iostream>
#include <memory>

#include "bdb.hpp"

int main(int argc, char** argv)
{
  using namespace std;

  if(argc < 3){
    cerr << "Usage: sim2 <log> <work_dir>\n";
    return 1;
  }

  ifstream fin(argv[1], ios::binary | ios::in);

  if(!fin.is_open()){
    cerr << "open file " << argv[1] << "failed\n";
    return 1;
  }

  std::string cmd;
  shared_ptr<BDB::BehaviorDB> bdb;

  fin >> cmd;
  if(cmd != "conf"){
    cerr << "no configuration log\n";
    return 1;
  }else{
    BDB::AddrType beg, end;
    unsigned int addr_prefix_len;
    uint32_t min_size;
    fin >> beg >> end >> addr_prefix_len >> min_size;
    BDB::Config conf(beg, end, addr_prefix_len, min_size, argv[2]);
    bdb.reset(new BDB::BehaviorDB(conf));
    getline(fin, cmd); // ignore others
  }
  
  uint32_t size, off;
  BDB::AddrType addr;
  std::string data;

  while(fin >> cmd){
    if(cmd == "put"){
      fin >> size;
      data.resize(size);
      bdb->put(data.c_str(), size);
    }else if(cmd == "put-spec"){
      fin >> size >> addr >> off;
      data.resize(size);
      bdb->put(data.c_str(), size, addr, off);
    }else if(cmd == "insert"){
      fin >> size >> addr >> off;
      data.resize(size);
      bdb->put(data.c_str(), size, addr, off);
    }else if(cmd == "get"){
      fin >> size >> addr >> off;
      data.resize(size);
      bdb->get(&data[0], size, addr, off);
    }else if(cmd == "string_get"){
      fin >> size >> addr >> off;
      data.resize(size);
      bdb->get(&data, size, addr, off);
    }else if(cmd == "del"){
      fin >> addr;
      bdb->del(addr);
    }else if(cmd == "partial_del"){
      fin >> addr >> off >> size;
      bdb->del(addr, off, size);
    }else if(cmd == "update"){
      fin >> size >> addr;
      data.resize(size);
      bdb->update(data.c_str(), size, addr);
    }else if(cmd == "update_put"){
      fin >> size >> addr;
      data.resize(size);
      bdb->update(data.c_str(), size, addr);
    }else{
      cerr << "unknown method " << cmd <<"\n";
      return 1;
    }
  }

  return 0;
}
