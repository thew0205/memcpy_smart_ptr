# memcpy_smart_ptr

**A C++ Smart Pointer bridge for C-style memory transfer APIs.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: RTOS | Embedded](https://img.shields.io/badge/Platform-RTOS%20%7C%20Embedded-blue)](https://github.com/thew0205/memcpy_smart_ptr)

## Overview

In embedded systems and RTOS environments, data is frequently moved between tasks using `memcpy`-based primitives (e.g., FreeRTOS Queues, Ring Buffers, Mailboxes). Standard C++ smart pointers (`std::unique_ptr`, `std::shared_ptr`) are incompatible with these APIs because bitwise copies bypass their move/copy constructors, leading to double-free errors or memory leaks.

**`memcpy_smart_ptr`** provides a safety-critical bridge. It allows you to wrap C-style data transfers in a way that preserves C++ ownership semantics and RAII guarantees.



## The Integrity Rule

To use these pointers safely, your storage container must adhere to the following requirement:
> The container (Queue, Buffer, Stack) must guarantee that the raw memory holding the internal smart pointer representation is not overwritten before it is successfully copied out. The data must eventually be copied out into a managed class instance to ensure automatic memory disposal.

---

## Features

* **RAII Safety:** Automatic memory cleanup even when data is passed through "dumb" C buffers.
* **Unique & Shared Variants:** Supports both `memcpy_unique_ptr` and `memcpy_shared_ptr`.
* **Atomic Reference Counting:** `memcpy_shared_ptr` supports both single-threaded and multi-threaded (atomic) reference counting using `__gnu_cxx` lock policies.
* **SFINAE Guarded:** Uses template metaprogramming to enforce correct C-API wrapper signatures at compile time.
* **Zero-Overhead:** Designed for high-performance microcontrollers (validated on ARM Cortex-M0+).

---

## Usage Example (FreeRTOS)

### 1. Sending a Unique Pointer
```cpp
auto sensorData = make_memcpy_unique_ptr<BigDataStruct>(args...);

// Transfer ownership into a FreeRTOS Queue
sensorData.memcpy_send(nullptr, [](void* dest, const memcpy_unique_ptr<BigDataStruct>* src) {
    return xQueueSend(sensorToStorageQueue, src, 0) == pdTRUE;
});

// After successful send, sensorData is now null; ownership is moved to the queue buffer.
```
### 2. Receiving a Unique Pointer
```cpp
memcpy_unique_ptr<BigDataStruct> receivedData;

// Claim ownership from the Queue
receivedData.memcpy_receive(nullptr, [](memcpy_unique_ptr<BigDataStruct>* dest, const void* src) {
    return xQueueReceive(sensorToStorageQueue, dest, portMAX_DELAY) == pdTRUE;
});

// Use the data safely; it is deleted automatically when receivedData goes out of scope.
```


## Technical Implementation Details
**SFINAE Signature Enforcement:**
The library ensures your transfer lambdas match the required signatures using std::is_invocable_r_v. This prevents passing incompatible functions to the memcpy hooks:

```cpp

template <typename _Function>
constexpr static bool is_memcpy_send_signature = 
    std::is_invocable_r_v<bool, _Function, void *const, const memcpy_unique_ptr<T> *const>;
```
**Reference Counting Logic:**
memcpy_shared_ptr manages reference counts across bitwise transfers:
* memcpy_send: Increments count (the bitwise copy in the buffer represents a new owner).
* memcpy_receive: Decrements the current object's count before performing a bitwise copy from the source into this.

## Testing & Validation
The library has been rigorously tested using CppUTest with Address Sanitizer (-fsanitize=address) to confirm zero leaks and zero memory corruption.

#### Targets Validated:
* x86_64 (Linux/GCC)
* ARM Cortex-M0+ (RP2040 Microcontroller)

### Unit Test Execution:


Running CppUTest...
* TEST(MAKE_MEMCPY_SHARED_PTR, MAKE_SHARED_PTR_STRING) - 0 ms
* TEST(MAKE_MEMCPY_SHARED_PTR, MAKE_SHARED_PTR_INT) - 1 ms
* TEST(MEMCPY_SHARED_PRR, Memcpy_receive_function2) - 0 ms
* TEST(MEMCPY_SHARED_PRR, Memcpy_send_receive_function) - 0 ms
* TEST(MEMCPY_SHARED_PRR, MOVE_ASSIGNMENT_sharedPtr2) - 0 ms
* TEST(MEMCPY_SHARED_PRR, MOVE_ASSIGNMENT_sharedPtr) - 0 ms
* TEST(MEMCPY_SHARED_PRR, MOVE_CONSTRUCTOR_sharedPtr) - 0 ms
* TEST(MEMCPY_SHARED_PRR, COPY_ASSIGNMENT_sharedPtr2) - 0 ms
* TEST(MEMCPY_SHARED_PRR, COPY_ASSIGNMENT_sharedPtr) - 0 ms
* TEST(MEMCPY_SHARED_PRR, COPY_CONSTRUCTOR_sharedPtr) - 0 ms
* TEST(MEMCPY_SHARED_PRR, Create_sharedPtr) - 0 ms
* TEST(MEMCPY_SHARED_PRR, Create_null_sharedPtr) - 0 ms
* TEST(MAKE_MEMCPY_UNIQUE_PTR, MAKE_UNIQUE_PTR_STRING) - 0 ms
* TEST(MAKE_MEMCPY_UNIQUE_PTR, MAKE_UNIQUE_PTR_INT) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Memcpy_receive_function) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Memcpy_SEND_function) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Bool_conversion) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Reset_uniquePtr) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Release_uniquePtr) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Move_uniquePtr) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Create_uniquePtr) - 0 ms
* TEST(MEMCPY_UNIQUE_PRR, Create_empty_uniquePtr) - 0 ms

OK (23 tests, 23 ran, 96 checks, 0 ignored, 0 filtered out, 1 ms)