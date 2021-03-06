

// init
// all of the objects have mru_next == mru_prev
#define htable_size_bits 8
#define htable_size (1 << htable_size_bits)
#define SIZE 255
#define empty SIZE
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* There are two tables: 'htable' and 'table':
 *
 * 'htable' maps a hash bucket index to an index
 * into 'table'
 *
 * 'table' contains linked lists of items matching a
 * particular hash bucket index.
 * It also maintains a list of most recently used items.
 *
 * Accessing an entry is at most O(number of collisions) however
 * the most recently accessed item in that hash bucket will be at
 * the front so repeated accesses are fast. (The linked list of
 * colliding items is not mru sorted but could be)
 *
 * Every time an item is added the least recently used item
 * is dropped from the table.
 */
static const uint32_t kGoldenRatioU32 = 0x9E3779B9U;
int hash(int a)
{
    return kGoldenRatioU32 * a;
}
typedef int Index;
template <class Key, class Value>
class CacheMap {
public:
    CacheMap()
    {
        for (int i = 0; i < htable_size; i++) {
            htable[i] = empty;
        }
        for (int i = 0; i < SIZE; i++) {
            // hook up the mru list
            table[i].mru_next = i + 1;
            table[i].mru_prev = i - 1;
            // each item links to itself
            table[i].next = i;
            table[i].prev = i;
        }
        // complete the loop
        table[0].mru_prev = SIZE-1;
        table[SIZE-1].mru_next = 0;

        // the choice of mru doesn't matter
        mru = 0;
    }

    template <class T>
    const Value get(Key key, T compute) {
        const int mask = (1 <<  htable_size_bits) - 1;

#if 0
        for (int i = 0; i < htable_size; i++) {
            if (htable[i] != empty) {
                assert((hash(table[htable[i]].key) & mask) == i);
            }
        }
#endif

        int h = hash(key);
        Index n = htable[h & mask];

        Index orig = n;
        if (n != empty) do {
            if (table[n].key == key) {
                // we've found what we're looking for
                if (mru == n)
                    return table[n].value;

                // remove from old location in mru list
                Index next = table[n].mru_next;
                Index prev = table[n].mru_prev;
                table[prev].mru_next = next;
                table[next].mru_prev = prev;

                // insert at head of mru list
                prev = table[mru].mru_prev;
                table[n].mru_prev = prev;
                table[n].mru_next = mru;
                table[mru].mru_prev = n;
                table[prev].mru_next = n;

                mru = n;

                // rotate the bucket linked list so that n is at the head
                // alternatively we could move the entry as part of the
                // list so that each cache bucket is also mru ordered
                htable[h & mask] = n;
                return table[n].value;
            } else {
                // keep searching
                n = table[n].next;
            }
        } while (n != orig);

        // we didn't find what we're looking for.
        // we're going to replace the lru item
        Index lru = table[mru].mru_prev;

        int lru_h = hash(table[lru].key);
        // XXX we need to update htable if it currently points at us
        if (htable[lru_h & mask] == lru) {
            if (table[lru].next == lru) {
                htable[lru_h & mask] = empty;
            } else {
                htable[lru_h & mask] = table[lru].next;
            }
        }
        // remove from old location
        Index next = table[lru].next;
        Index prev = table[lru].prev;
        table[prev].next = next;
        table[next].prev = prev;

        // insert at head
        if (orig == empty) {
            table[lru].next = lru;
            table[lru].prev = lru;
        } else {
            prev = table[orig].prev;
            table[lru].prev = prev;
            table[lru].next = orig;
            table[orig].prev = lru;
            table[prev].next = lru;
        }
        table[lru].key = key;
        table[lru].value = compute(key);

        mru = lru;
        htable[h & mask] = lru;
        return table[lru].value;
    }
private:
    struct {
        Key key;
        Value value;
        Index next;
        Index prev;
        Index mru_next;
        Index mru_prev;
    } table[SIZE];
    Index mru;
    Index htable[htable_size];
};

int main()
{
    CacheMap<int, int> m;
    for (int i=0; i<260; i++) {
        printf("%d\n", m.get(i, [](int x) { printf("get %d'\n", x); return x + 1; }));
    }
    for (int i=259; i>=0; i--) {
        printf("%d\n", m.get(i, [](int x) { printf("get %d'\n", x); return x + 1; }));
    }
}
