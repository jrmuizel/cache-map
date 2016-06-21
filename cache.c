int compute(int input)
{
	return -input;
}

#define CACHE_SIZE 32

// the best way to implement an lru cache
// seems to be using a linked list and a map
// the linked list keeps the lru ordering
// and the map provides lookup to the value
// and the key entry in the list
//
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
// http://timday.bitbucket.org/lru.html
//
// fifo caches are simpler and can be implemented using a ring buffer
// but degenerate to no cache if the working set is bigger than
// the cache size
int get(int input)
{

	static struct {int in; int out} cache[CACHE_SIZE];
	static int start;
	for (int i=0; i<CACHE_SIZE; i++) {

		if (cache[i].in == input)
			return cache[i].out;
	}

	return compute(input);
}

int main()
{
}
