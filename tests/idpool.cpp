#include "idPool.hpp"

int main()
{
	IDPool<unsigned int> idp(2);
	idp.init_transaction("idpool.trans");

	unsigned int id = idp.Acquire();
	printf("acquired id: %lu\n", id);
	printf("after acquire: %d\n", idp.isAcquired(id));
	idp.Release(id);
	printf("after release: %d\n", idp.isAcquired(id));

	printf("%d\n", idp.avail());
	printf("%lu\n", idp.size());
	printf("%lu\n", idp.max_size());
	
	idp.Release(id);
}
