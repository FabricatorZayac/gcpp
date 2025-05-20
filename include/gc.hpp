#include "boost/pfr/core.hpp"

#include <alloca.h>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <new>
#include <sys/types.h>
#include <type_traits>
#include <vector>

struct GC {
private:
    template<typename T>
    struct Allocation {
        std::bitset<64> ptrbits;
        T data;
    };
    struct Root {
        size_t refcount;
        void *data; // 8 bytes after allocation start

        uint64_t ptrbits() const {
            return *((uint64_t *)data - 1);
        }
    };

public:
    template<typename T>
    struct RootPtr {
        RootPtr(const RootPtr &other) : ptr(other.ptr), gc(other.gc) {
            ptr.refcount += 1;
        }
        ~RootPtr() {
            ptr.refcount -= 1;

            if (ptr.refcount == 0) {
                // idk man 
            }
        }

        T &operator *() {
            return *std::launder((T *)ptr.data);
        }

        T *operator->() {
            return std::launder((T *)ptr.data);
        }
    private:
        friend struct GC;

        RootPtr(Root &ptr, GC &gc) : ptr(ptr), gc(gc) { }

        Root &ptr;
        GC &gc;
    };

    // for getting a pointer into an inner allocation onto the stack
    template<typename T>
    auto bind(T *ptr) -> RootPtr<T> {
        roots.push_back({
            .refcount = 1,
            .data = ptr,
        });

        return RootPtr<T>(roots.back(), *this);
    }

    template<typename T>
    auto create() -> RootPtr<T> {
        return bind(allocate<T>());
    }
private:
    template<typename T>
    auto make_ptr_field(T) -> T {
        return T{};
    }

    template<typename T>
    auto make_ptr_field(T *ptr) -> T * {
        (void)ptr;
        return allocate<T>();
    }

    template<typename T>
    auto allocate() -> T * {
        Allocation<T> *allocation = new Allocation<T>;
        allocation->ptrbits = 0;

        if (alignof(T) == 8) {
            // set ptrbits via pfr magic
            // for each word check if pointer and set according bit to 1
            // When encountering pointer, create pointed to memory recursively
            boost::pfr::for_each_field(allocation->data, [&](auto &field, std::size_t i){
                using field_type = std::remove_reference_t<decltype(field)>;
                if (std::is_pointer_v<field_type>) {
                    allocation->ptrbits[i] = true;
                    // allocation->ptrbits |= 1 << i;
                    field = make_ptr_field(field);
                }
            });
        }

        allocations.push_back(allocation);

        return &allocation->data;
    }

    std::vector<Root> roots;
    std::vector<void *> allocations;
};
