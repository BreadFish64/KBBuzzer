#include <assert.h>

template <typename T, size_t max_size>
class RingQueue {
private:
    T queue[max_size] = {};
    size_t queue_in = 0, queue_out = 0;

public:
    inline void put(T new_item) {
        // full
        assert(queue_in != (queue_out - 1 + max_size) % max_size);

        queue[queue_in] = new_item;
        queue_in = (queue_in + 1) % max_size;
    }

    inline T get() {
        // empty
        if (queue_in == queue_out)
            return nullptr;

        auto& old = queue[queue_out];
        queue_out = (queue_out + 1) % max_size;

        return old;
    }

    inline void reset() {
        queue_in = queue_out;
    }
};
