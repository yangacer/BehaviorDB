#include <sys/time.h>
#include <iostream>
#include <iomanip>

#define TimeBeg(X) gettimeofday(&X, 0)
#define TimeEnd(X) { \
	timeval end; \
	gettimeofday(&end, 0); \
	unsigned long sec, usec; \
	sec = end.tv_sec - X.tv_sec; \
	if(end.tv_usec < X.tv_usec){ \
		usec = end.tv_usec + 1000000 - X.tv_usec; \
		sec--; \
	}else \
		usec = end.tv_usec - X.tv_usec; \
	cout<<sec<<"."<<setfill('0')<<setw(6)<<usec<<endl; \
}
