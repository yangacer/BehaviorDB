#Limitation

##Memory

1. IDPool - An IDPool uses a dynamic bitmap to maintain  acquired/released IDs. The bitmap can contain 2^32 bits at most.

2. IDValPool - Other than a dynamic bitmap, a 4 bytes address is associated to an ID. The bitmap can contain 2^32 bits at most.

###Transaction IDPool(pool::idPool_)

Each pool has an idPool_ of size 2^32 bits. Within default configuration, a BDB has 16 pools so that implies, without considering a maximum size of memory supported by an OS, we will require 2^(32+4) bits, or write 32 GB, in memory to supply miximum number of IDs. Note that each idPool_ does not fully allocate in initialization but does allocate 4K bits only. Resize(reallocation) will take place when an ID that out of current range is acquired.

###Global IDValPool(BDBImpl::global_id_)

Clients can configure size of this structure via BDB::Config::beg and BDB::Config::end. The BDB will fully initiate the bitmap and an array for storing the associated addresses. Assume a client passes (1, N+1] as the range of global IDs to a BDB, an initiated IDValPool will be of size ceil(N/8) + 4N bytes. 

##Disk

###System Limitation 

By using fseeko, type of an offset parameter is off_t which is of size 64 bit and is a signed number. As a result, in case of BDB's usage which always uses SEEK_SET, we can seek bytes between (0, 2^63-1). 

###Header File(*.fpo)

Each object of a header file is of fixed size and currently is 8 bytes. Thus, we can store 2^63/8 = 2^60 headers at most.

###Transaction File(*.trans)

This file dose not require seek.

###Pool File(*.pool)

If we assume a chunk of a pool file is of size C bytes, then a pool file can store 2^63/C objects at most.

##Combined

[to be added]