// Protocol Buffers - Google's data interchange format
// Copyright 2012 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This file is an internal atomic implementation, use atomicops.h instead.

#ifndef GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_RISCV64_GCC_H_
#define GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_RISCV64_GCC_H_

#define ATOMICOPS_COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")

namespace google {
namespace protobuf {
namespace internal {

// Atomically execute:
//      result = *ptr;
//      if (*ptr == old_value)
//        *ptr = new_value;
//      return result;
//
// I.e., replace "*ptr" with "new_value" if "*ptr" used to be "old_value".
// Always return the old value of "*ptr"
//
// This routine implies no memory barriers.
inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32* ptr,
                                         Atomic32 old_value,
                                         Atomic32 new_value) {
  Atomic32 prev, tmp;
  Atomic32 check;
  __asm__ __volatile__(
                       "1:\n"
                       "lr.w %0, (%5)\n"  // prev = *ptr
                       "bne %0, %3, 2f\n"  // if (prev != old_value) goto 2
	  	  	   	   	   "move %1, %4\n"  // tmp = new_value
                       "sc.w %2, %1, (%5)\n"  // *ptr = tmp (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "nop\n"  // delay slot nop
                       "2:\n"
                       : "=&r" (prev), "=&r" (tmp), "=&r"(check)
                       : "r" (old_value), "r" (new_value), "r" (ptr)
                       : "memory");
  return prev;
}

inline Atomic64 NoBarrier_CompareAndSwap(volatile Atomic64* ptr,
                                         Atomic64 old_value,
                                         Atomic64 new_value) {
  Atomic64 prev, tmp;
  Atomic64 check;
  __asm__ __volatile__(
		  	  	  	   "1:\n"
                       "lr.d %0, (%5)\n"  // prev = *ptr
                       "bne %0, %3, 2f\n"  // if (prev != old_value) goto 2
	  	  	   	   	   "move %1, %4\n"  // tmp = new_value
                       "sc.d %2, %1, (%5)\n"  // *ptr = tmp (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "nop\n"  // delay slot nop
                       "2:\n"
                       : "=&r" (prev), "=&r" (tmp), "=&r"(check)
                       : "r" (old_value), "r" (new_value), "r" (ptr)
                       : "memory");
  return prev;
}

// Atomically store new_value into *ptr, returning the previous value held in
// *ptr.  This routine implies no memory barriers.
inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32* ptr,
                                         Atomic32 new_value) {
  Atomic32 temp, old;
  Atomic32 check;
  __asm__ __volatile__(
                       "1:\n"
                       "lr.w %1, (%4)\n"  // old = *ptr
                       "move %0, %3\n"  // temp = new_value
                       "sc.w %2, %0, (%4)\n"  // *ptr = temp (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "nop\n"  // delay slot nop
                       : "=&r" (temp), "=&r" (old), "=&r"(check)
                       : "r" (new_value), "r" (ptr)
                       : "memory");

  return old;
}

inline Atomic64 NoBarrier_AtomicExchange(volatile Atomic64* ptr,
                                         Atomic64 new_value) {
  Atomic64 temp, old;
  Atomic64 check;
  __asm__ __volatile__(
                       "1:\n"
                       "lr.d %1, (%4)\n"  // old = *ptr
                       "move %0, %3\n"  // temp = new_value
                       "sc.d %2, %0, (%4)\n"  // *ptr = temp (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "nop\n"  // delay slot nop
                       : "=&r" (temp), "=&r" (old), "=&r"(check)
                       : "r" (new_value), "r" (ptr)
                       : "memory");

  return old;
}

// Atomically increment *ptr by "increment".  Returns the new value of
// *ptr with the increment applied.  This routine implies no memory barriers.
inline Atomic32 NoBarrier_AtomicIncrement(volatile Atomic32* ptr,
                                          Atomic32 increment) {
  Atomic32 temp, temp2;
  Atomic32 check;
  __asm__ __volatile__(
                       "1:\n"
                       "lr.w %0, (%4)\n"  // temp = *ptr
                       "addu %1, %0, %3\n"  // temp2 = temp + increment
                       "sc.w %2, %1, (%4)\n"  // *ptr = temp2 (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "addu %1, %0, %3\n"  // temp2 = temp + increment
                       : "=&r" (temp), "=&r" (temp2), "=&r" (check)
                       : "r" (increment), "r" (ptr)
                       : "memory");
  // temp2 now holds the final value.
  return temp2;
}

inline Atomic64 NoBarrier_AtomicIncrement(volatile Atomic64* ptr,
                                          Atomic64 increment) {
  Atomic64 temp, temp2;
  Atomic64 check;
  __asm__ __volatile__(
                       "1:\n"
                       "lr.d %0, (%4)\n"  // temp = *ptr
                       "addu %1, %0, %3\n"  // temp2 = temp + increment
                       "sc.d %2, %1, (%4)\n"  // *ptr = temp2 (with atomic check)
                       "bnez %2, 1b\n"  // start again on atomic error
                       "addu %1, %0, %3\n"  // temp2 = temp + increment
                       : "=&r" (temp), "=&r" (temp2), "=&r" (check)
                       : "r" (increment), "r" (ptr)
                       : "memory");
  // temp2 now holds the final value.
  return temp2;
}

inline Atomic32 Barrier_AtomicIncrement(volatile Atomic32* ptr,
                                        Atomic32 increment) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic32 res = NoBarrier_AtomicIncrement(ptr, increment);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

inline Atomic64 Barrier_AtomicIncrement(volatile Atomic64* ptr,
                                        Atomic64 increment) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic64 res = NoBarrier_AtomicIncrement(ptr, increment);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

// "Acquire" operations
// ensure that no later memory access can be reordered ahead of the operation.
// "Release" operations ensure that no previous memory access can be reordered
// after the operation.  "Barrier" operations have both "Acquire" and "Release"
// semantics.   A MemoryBarrier() has "Barrier" semantics, but does no memory
// access.
inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32* ptr,
                                       Atomic32 old_value,
                                       Atomic32 new_value) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic32 res = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

inline Atomic64 Acquire_CompareAndSwap(volatile Atomic64* ptr,
                                       Atomic64 old_value,
                                       Atomic64 new_value) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic64 res = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

inline Atomic32 Release_CompareAndSwap(volatile Atomic32* ptr,
                                       Atomic32 old_value,
                                       Atomic32 new_value) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic32 res = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

inline Atomic64 Release_CompareAndSwap(volatile Atomic64* ptr,
                                       Atomic64 old_value,
                                       Atomic64 new_value) {
  ATOMICOPS_COMPILER_BARRIER();
  Atomic64 res = NoBarrier_CompareAndSwap(ptr, old_value, new_value);
  ATOMICOPS_COMPILER_BARRIER();
  return res;
}

inline void NoBarrier_Store(volatile Atomic32* ptr, Atomic32 value) {
  *ptr = value;
}

inline void NoBarrier_Store(volatile Atomic64* ptr, Atomic64 value) {
  *ptr = value;
}

inline void MemoryBarrier() {
  __asm__ __volatile__("fence rw, rw" : : : "memory");
}

inline void Acquire_Store(volatile Atomic32* ptr, Atomic32 value) {
  *ptr = value;
  MemoryBarrier();
}

inline void Acquire_Store(volatile Atomic64* ptr, Atomic64 value) {
  *ptr = value;
  MemoryBarrier();
}

inline void Release_Store(volatile Atomic32* ptr, Atomic32 value) {
  MemoryBarrier();
  *ptr = value;
}

inline void Release_Store(volatile Atomic64* ptr, Atomic64 value) {
  MemoryBarrier();
  *ptr = value;
}

inline Atomic32 NoBarrier_Load(volatile const Atomic32* ptr) {
  return *ptr;
}

inline Atomic64 NoBarrier_Load(volatile const Atomic64* ptr) {
  return *ptr;
}

inline Atomic32 Acquire_Load(volatile const Atomic32* ptr) {
  Atomic32 value = *ptr;
  MemoryBarrier();
  return value;
}

inline Atomic64 Acquire_Load(volatile const Atomic64* ptr) {
  Atomic64 value = *ptr;
  MemoryBarrier();
  return value;
}

inline Atomic32 Release_Load(volatile const Atomic32* ptr) {
  MemoryBarrier();
  return *ptr;
}


inline Atomic64 Release_Load(volatile const Atomic64* ptr) {
  MemoryBarrier();
  return *ptr;
}

}  // namespace internal
}  // namespace protobuf
}  // namespace google

#undef ATOMICOPS_COMPILER_BARRIER

#endif  // GOOGLE_PROTOBUF_ATOMICOPS_INTERNALS_RISCV_GCC_H_
