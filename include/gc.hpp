#include "boost/pfr/core.hpp"

#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stack>
#include <type_traits>
#include <vector>
#include <algorithm>

struct GC {
private:
    static const size_t WORD_SIZE = 8;

    using PtrBits = std::bitset<WORD_SIZE * 8>;

    struct DynAlloc {
        PtrBits ptrbits;

        virtual ~DynAlloc() = default;
        virtual size_t size_of() const = 0;

        // TODO: Maybe add iterator for child pointers
    };

    template<typename T>
    struct Allocation : DynAlloc {
        T data;

        size_t size_of() const override {
            return sizeof(*this);
        }
    };
    struct Root {
        size_t refcount;
        void *data; // -> Allocation::data
    };

public:
    template<typename T>
    struct RootPtr {
        RootPtr(const RootPtr &other) : idx(other.idx), gc(other.gc) {
            getRoot().refcount += 1;
        }
        ~RootPtr() {
            getRoot().refcount -= 1;

            if (getRoot().refcount == 0) {
                gc.roots.erase(gc.roots.begin() + idx);
            }
        }

        T &get() {
            return *std::launder((T *)getRoot().data);
        }

        // idk if I actually need to launder here, but it's there jic
        T &operator *() {
            return get();
        }
        T *operator->() {
            return &get();
        }
    private:
        friend struct GC;

        RootPtr(size_t idx, GC &gc) : idx(idx), gc(gc) { }
        Root &getRoot() {
            return gc.roots[idx];
        }

        size_t idx;
        GC &gc;
    };

    // Binds allocation to a root and pushes it to the root list
    template<typename T>
    auto bind(this GC &self, T *ptr) -> RootPtr<T> {
        self.roots.push_back({
            .refcount = 1,
            .data = ptr,
        });

        return RootPtr<T>(self.roots.size() - 1, self);
    }

    template<typename T>
    auto create() -> RootPtr<T> {
        return bind(allocate<T>());
    }

    void operator()() {
        // if marker is true, means allocation is alive
        std::vector<bool> markers(allocations.size(), false);

        for (const auto &root : roots) {
            auto root_ptr = (uintptr_t)root.data;

            std::stack<uintptr_t> stack;
            stack.push(root_ptr);

            while (!stack.empty()) {
                auto ptr = stack.top();
                stack.pop();

                size_t idx = SIZE_MAX;
                for (size_t i = 0; i < allocations.size(); i++) {
                    auto allocation = allocations[i];
                    if (markers[i]) continue;
                    if ((uintptr_t)allocation < ptr
                    && ptr < (uintptr_t)allocation + allocation->size_of()) {
                        markers[i] = true;
                        idx = i;
                    }
                }

                // Fails if pointer doesn't point into GC memory
                assert(idx < allocations.size());
                
                if (markers[idx]) continue;
                markers[idx] = true;

                const auto &ptrbits = allocations[idx]->ptrbits;
                for (size_t i = 0; i < ptrbits.size(); i++) {
                    if (ptrbits[i]) {
                        auto next = *(uintptr_t *)(ptr + WORD_SIZE * i);
                        stack.push(next);
                    }
                }
            }
        }

        for (size_t i = allocations.size(); i --> 0;) {
            if (!markers[i]) deallocate(i);
        }
    }

    ~GC() {
        for (size_t i = allocations.size(); i --> 0;) {
            deallocate(i);
        }
    }
private:
    // Creates an allocation and pushes it to the allocation list
    template<typename T>
    auto allocate() -> T * {
        Allocation<T> *allocation = new Allocation<T>();
        allocation->ptrbits = 0;

        if (alignof(T) == WORD_SIZE) {
            // set ptrbits via pfr magic
            // for each word check if pointer and set according bit to 1
            // When encountering pointer, create pointed to memory recursively
            boost::pfr::for_each_field(allocation->data, [&](auto &field){
                using field_type = std::remove_reference_t<decltype(field)>;
                if (std::is_pointer_v<field_type>) {
                    size_t word = ((uintptr_t)&field - (uintptr_t)&allocation->data) / WORD_SIZE;
                    // std::cout<< word << std::endl;
                    allocation->ptrbits[word] = true;
                    field = make_field(field);
                }
            });
        }

        allocations.push_back(allocation);

        return &allocation->data;
    }

    // called during sweep phase
    void deallocate(size_t idx) {
        delete allocations[idx];
        allocations.erase(allocations.begin() + idx);
    }

    template<typename T>
    auto make_field(T) -> T {
        return T{};
    }
    template<typename T>
    auto make_field(T *ptr) -> T * {
        (void)ptr;
        return allocate<T>();
    }

    std::vector<Root> roots;
    std::vector<DynAlloc *> allocations;
};
