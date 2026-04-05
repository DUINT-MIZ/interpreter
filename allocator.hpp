#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <iterator>
#include <bit>

namespace noble {

struct ChunkInfo {
    void* chunk_ptr;
    std::size_t alignment;
};

struct VirDes {
    public :
    virtual ~VirDes() = default;
};

template <typename T>
struct Holder__ : public VirDes{
    public :
    T* object_ptr;
    std::size_t arr_size;

    Holder__(T* ptr, std::size_t n) : object_ptr(ptr), arr_size(n) {}

    ~Holder__() {
        for(auto i = 0uz; i < arr_size; i++) {
            object_ptr[i].~T();
        }
    }
};

template <> struct Holder__<void> : public VirDes { Holder__(void*, std::size_t) {} };

template <typename T>
concept trivial_type = std::is_trivial_v<T>;

template <typename T>
struct HolderSelector {
    using type = Holder__<T>;
};

template <trivial_type T>
struct HolderSelector<T> {
    using type = Holder__<void>;
};

template <typename T>
using Holder = typename HolderSelector<T>::type;

class AllocImpl {
    public :

    void set_chunksize(std::size_t size) { chunksize = size; }

    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        // std::cerr << "[?] Allocation starts\n";
        if(size > chunksize) {
            // std::cerr << "[!] Size exceeds chunksize\n";
            return new_chunk(size, alignment);
        }

        std::size_t space_left = end - curr;
        void* current_ptr = curr;
        void* aligned_ptr = std::align(alignment, size, current_ptr, space_left);
        if(!aligned_ptr) {
            // std::cerr << "[!] Failed alignment #1\n";
            beg = new_chunk(chunksize, alignment);
            current_ptr = curr = beg;
            end = beg + chunksize;
            space_left = chunksize;
            
            if(!(aligned_ptr = std::align(alignment, size, current_ptr, space_left))) {
                // std::cerr << "[!] Failed alignment #2\n";
                return new_chunk(size, alignment);
            }
        }

        // std::cerr << "[*] Found address at reserved space\n"
        //           << "    ptr : " << (void*)aligned_ptr
        //           << "  alignment : " << alignment << "  size : " << size << "\n";
        curr = static_cast<std::byte*>(aligned_ptr) + size;
        return aligned_ptr;
    }

    ~AllocImpl() {
        for(auto& info : chunks) {
            ::operator delete(info.chunk_ptr, std::align_val_t{info.alignment});
        }
    }

    private :

    std::byte* new_chunk(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        auto chunk_ptr = ::operator new(size, std::align_val_t{alignment});
        try {
            chunks.push_back(ChunkInfo{chunk_ptr, alignment});
        } catch(...) {
            ::operator delete(chunk_ptr, std::align_val_t{alignment});
            throw std::bad_alloc(); // vector bad_alloc could meant something is wrong with the heap, let caller know this.
        }
        // std::cerr << "New Chunk allocated at address " << (void*)chunk_ptr << " with alignment of " << alignment << std::endl;
        return static_cast<std::byte*>(chunk_ptr);
    }

    std::size_t chunksize = 10240;
    std::vector<ChunkInfo> chunks;
    std::byte* beg = nullptr;
    std::byte* curr = nullptr;
    std::byte* end = nullptr;
};

class Alloc {
    private :

    AllocImpl resource_allocator;
    AllocImpl virdes_allocator;
    std::vector<VirDes*> virtual_destructors;

    template <typename T>
    void new_virtual_destructors(T* obj_ptr, std::size_t n) {
        // std::cerr << "[?] Allocating virtual destructor\n";
        auto virdes_mem = virdes_allocator.allocate(sizeof(Holder<T>));
        virtual_destructors.push_back(new(virdes_mem) Holder<T>(obj_ptr, n));
    }

    public :

    ~Alloc() {
        for(auto it = virtual_destructors.rbegin(); it != virtual_destructors.rend(); ++it) {
            (*it)->~VirDes();
        }
    }

    Alloc() { virdes_allocator.set_chunksize(5120); }

    template <typename ItType>
    auto marray_alloc(ItType beg, std::size_t n) {
        using T = std::iterator_traits<ItType>::value_type;
        static_assert(
            std::move_constructible<T>,
            "marray_alloc() requires value_type to be move constructible"
        );

        auto space = static_cast<T*>(resource_allocator.allocate(sizeof(T) * n, alignof(T)));
        // std::cerr << "[!] MARRAY_ALLOC : Initializing\n";
        std::uninitialized_move_n(beg, n, space);
        new_virtual_destructors(space, n);
        return space;
    }

    template <typename ItType>
    auto carray_alloc(ItType beg, std::size_t n) {
        using T = std::iterator_traits<ItType>::value_type;
        static_assert(
            std::copy_constructible<T>,
            "carray_alloc() requires value_type to be copy constructible"
        );

        auto space = static_cast<std::remove_const_t<T>*>(resource_allocator.allocate(sizeof(T) * n, alignof(T)));
        std::uninitialized_copy_n(beg, n, space);
        new_virtual_destructors(space, n);
        return const_cast<T*>(space);
    }

    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        // std::cerr << "[?] Emplace start\n";
        auto space = static_cast<T*>(resource_allocator.allocate(sizeof(T), alignof(T)));
        std::construct_at(space, std::forward<Args>(args)...);
        new_virtual_destructors(space, 1);
        // std::cerr << "[?] Emplace stop\n";
        return *space;
    }

    void virdes_chunksize(std::size_t size) { virdes_allocator.set_chunksize(size); }
    void resource_chunksize(std::size_t size) { resource_allocator.set_chunksize(size); }
};

}