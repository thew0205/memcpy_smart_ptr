#include <type_traits>
#include <ext/atomicity.h> // For _Atomic_word
#include <ext/concurrence.h> // For __gnu_cxx lock policies
#include <atomic>

/**
 * Base template for the reference counter. 
 * Uses __gnu_cxx lock policies to differentiate between atomic and non-atomic counting.
 */
template <__gnu_cxx::_Lock_policy _Lp = __gnu_cxx::__default_lock_policy>
class memcpy_shared_ptr_count;

// Specialization for single-threaded environments: No atomic overhead.
template <>
class memcpy_shared_ptr_count<__gnu_cxx::_S_single>
{
public:
    memcpy_shared_ptr_count() : count(1) {}

    void increment() { count++; }
    void decrement() { count--; }

    _Atomic_word get_count() const { return count; }

private:
    _Atomic_word count;
};

// Specialization for multi-threaded environments: Uses std::atomic for thread safety.
template <>
class memcpy_shared_ptr_count<__gnu_cxx::_S_atomic>
{
public:
    memcpy_shared_ptr_count() : count(1) {}

    void increment() { count++; }
    void decrement() { count--; }

    _Atomic_word get_count() const { return count.load(); }

private:
    std::atomic<_Atomic_word> count;
};

/**
 * RAII Management class for the reference counter object.
 * This handles the allocation and life-cycle of the counter itself, 
 * but does NOT manage the user's data pointer (that's handled by memcpy_shared_ptr).
 */
template <__gnu_cxx::_Lock_policy _Lp>
class memcpy_shared_ptr_object_count
{
public:
    memcpy_shared_ptr_object_count() : count(nullptr) {}

    // Initialize counter for a new raw pointer
    template <typename Ptr>
    explicit memcpy_shared_ptr_object_count(Ptr p) : count(nullptr)
    {
        count = new memcpy_shared_ptr_count<_Lp>{};
    }

    // Copy Constructor: Increments reference count
    memcpy_shared_ptr_object_count(const memcpy_shared_ptr_object_count &obj) : count(obj.count)
    {
        if (count != nullptr)
        {
            count->increment();
        }
    }

    // Copy Assignment: Decrements old count and increments new count
    memcpy_shared_ptr_object_count &operator=(const memcpy_shared_ptr_object_count &obj)
    {
        if (count != nullptr)
        {
            count->decrement();
        }
        count = obj.count;
        if (count != nullptr)
        {
            count->increment();
        }
        return *this;
    }

    // Move Constructor: Transfers ownership of the counter without changing the count
    memcpy_shared_ptr_object_count(memcpy_shared_ptr_object_count &&obj) : count(obj.count)
    {
        obj.count = nullptr;
    }

    // Move Assignment: Cleans up current counter and takes over the new one
    memcpy_shared_ptr_object_count &operator=(memcpy_shared_ptr_object_count &&obj)
    {
        count = std::move(obj.count);
        obj.count = nullptr;
        return *this;
    }

    // Destructor: Decrements count (Cleanup of the counter memory is handled in memcpy_shared_ptr)
    ~memcpy_shared_ptr_object_count()
    {
        if (count != nullptr)
        {
            count->decrement();
        }
    }

private:
    memcpy_shared_ptr_count<_Lp> *count;

    // Grant access to the main smart pointer class
    template <class T>
    friend class memcpy_shared_ptr;
};

/**
 * A shared pointer compatible with C-style memcpy functions.
 * Designed for RTOS Queues and buffers where the memory is eventually 
 * copied back out into a class instance to maintain ownership tracking.
 */
template <class T>
class memcpy_shared_ptr
{
public:
    memcpy_shared_ptr() : ptr(nullptr), refCount{} {}

    memcpy_shared_ptr(T *ptr) : ptr(ptr), refCount(ptr) {}

    // Copy logic
    memcpy_shared_ptr(const memcpy_shared_ptr &obj) : ptr{obj.ptr}, refCount{obj.refCount} {}

    memcpy_shared_ptr &operator=(const memcpy_shared_ptr &obj)
    {
        if (refCount.count != obj.refCount.count)
        {
            __cleanup__(); 
            this->ptr = obj.ptr;
            this->refCount = obj.refCount;
        }
        return *this;
    }

    // Move logic
    memcpy_shared_ptr(memcpy_shared_ptr &&dyingObj) : ptr{std::move(dyingObj.ptr)}, refCount{std::move(dyingObj.refCount)}
    {
        dyingObj.ptr = nullptr;
    }

    memcpy_shared_ptr &operator=(memcpy_shared_ptr &&obj)
    {
        if (refCount.count != obj.refCount.count)
        {
            __cleanup__();
            this->ptr = obj.ptr;
            this->refCount = std::move(obj.refCount);
            obj.ptr = nullptr;
        }
        return *this;
    }

    // Accessors
    _Atomic_word get_count() const { return refCount.count == nullptr ? 0 : refCount.count->get_count(); }
    T *get() const { return this->ptr; }
    T *operator->() const { return this->ptr; }
    T &operator*() const { return *this->ptr; }

    // SFINAE checks for C-API compatibility
    template <typename _Function>
    constexpr static bool is_memcpy_send_signature = std::is_invocable_r_v<bool, _Function, void *const, const memcpy_shared_ptr<T> *const>;

    template <typename _Function>
    constexpr static bool is_memcpy_receive_signature = std::is_invocable_r_v<bool, _Function, memcpy_shared_ptr<T> *const, const void *const>;

    /**
     * Bridges C++ ownership to C-style Send.
     * Increments refCount because the bitwise copy in 'dest' becomes a new 'owner'.
     */
    template <typename _Function>
    typename std::enable_if<is_memcpy_send_signature<_Function>, bool>::type memcpy_send(void *const dest, _Function copy_fn)
    {
        bool success = false;
        if (copy_fn(dest, this))
        {
            if (nullptr != refCount.count)
            {
                refCount.count->increment(); // Register the new bitwise owner
            }
            success = true;
        }
        return success;
    }

    /**
     * Bridges C-style Receive to C++ ownership.
     * Decrements the count of the current instance before taking over the bitwise-received data.
     */
    template <typename _Function>
    typename std::enable_if<is_memcpy_receive_signature<_Function>, bool>::type memcpy_receive(const void *const src, _Function copy_fn)
    {
        uint8_t buffer[sizeof(memcpy_shared_ptr<T>)];
        bool success = false;
        if (copy_fn(reinterpret_cast<memcpy_shared_ptr<T> *>(buffer), src))
        {
            if (nullptr != refCount.count)
            {
                refCount.count->decrement(); // Relinquish current reference
            }
            __cleanup__();
            // Perform the bitwise transfer into 'this' instance
            memcpy(this, buffer, sizeof(memcpy_shared_ptr<T>));
            success = true;
        }
        return success;
    }

    ~memcpy_shared_ptr() { __cleanup__(); }

public:
    T *ptr = nullptr;
    memcpy_shared_ptr_object_count<__gnu_cxx::__default_lock_policy> refCount;

    /**
     * Internal cleanup: Checks if this is the last owner.
     * If count <= 1, it deletes both the counter and the raw data.
     */
    void __cleanup__()
    {
        if (refCount.count != nullptr)
        {
            if ((refCount.count->get_count()) <= 1)
            {
                delete refCount.count;
                refCount.count = nullptr;
                delete ptr;
                ptr = nullptr;
            }
        }
        else
        {
            // Edge case: Pointer exists but no count object (e.g. error in allocation)
            delete ptr;
            ptr = nullptr;
        }
    }
};

/**
 * Helper function to create a memcpy_shared_ptr with a new object.
 */
template <typename T, typename... ParaTypes>
typename std::enable_if<std::is_constructible<T, ParaTypes...>::value, memcpy_shared_ptr<T>>::type
make_memcpy_shared_ptr(ParaTypes &&...paras)
{
    return memcpy_shared_ptr<T>{new T(std::forward<ParaTypes...>(paras...))};
}