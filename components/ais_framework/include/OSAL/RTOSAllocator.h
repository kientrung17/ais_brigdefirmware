/*
 * custom_allocator.h
 *
 *  Created on: Jun 24, 2024
 *      Author: canhpn2
 */

#ifndef SUBMODULE_CUSTOM_ALLOCATOR_H_
#define SUBMODULE_CUSTOM_ALLOCATOR_H_

#include <cstdlib>   // For std::size_t
#include <limits>    // For std::numeric_limits
#include <new>       // For std::bad_alloc
#include "stdio.h"
#include "Platform/OSBase.h"

template <typename T>
class RTOSAllocator {
public:
    using value_type = T;

    RTOSAllocator() noexcept = default;

    template <typename U>
    RTOSAllocator(const RTOSAllocator<U>&) noexcept {}
    T* allocate(std::size_t n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(OSBase::memAlloc(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        OSBase::memFree(p);
    }

    std::size_t max_size() const noexcept {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }
};

template <typename T, typename U>
bool operator==(const RTOSAllocator<T>&, const RTOSAllocator<U>&) {
    return true;
}

template <typename T, typename U>
bool operator!=(const RTOSAllocator<T>&, const RTOSAllocator<U>&) {
    return false;
}

#endif /* FREERTOS_ALLOCATOR_H_ */
