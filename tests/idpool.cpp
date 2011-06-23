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
	
	IDValPool<unsigned int, unsigned int> idvp(1, 11);
	idvp.replay_transaction("idvpool.trans");
	idvp.init_transaction("idvpool.trans");

	unsigned int id2 = idvp.Acquire(123);
	idvp.Release(id2);

	idp.Release(id);
}
