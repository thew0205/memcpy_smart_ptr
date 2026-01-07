#include <type_traits>
#include <cstring> // For memcpy

/**
 * A unique-ownership smart pointer designed for bitwise transfer compatibility.
 * * Unlike std::unique_ptr, this class provides specific hooks (memcpy_send/receive)
 * to allow the internal pointer state to be moved through C-style APIs like 
 * RTOS queues or circular buffers while maintaining RAII safety.
 */
template <class T>
class memcpy_unique_ptr
{
public:
    // Default constructor: creates an empty manager
    memcpy_unique_ptr() noexcept : ptr(nullptr) 
    {
    }

    // Explicit constructor: takes ownership of a raw pointer
    explicit memcpy_unique_ptr(T *ptr) noexcept : ptr(ptr)
    {
    }

    // --- Ownership Rules ---
    // Unique pointers cannot be copied because there can only be one owner.
    memcpy_unique_ptr(const memcpy_unique_ptr &obj) = delete;            
    memcpy_unique_ptr &operator=(const memcpy_unique_ptr &obj) = delete; 

    // Move constructor: Transfers ownership from a dying object to this one.
    memcpy_unique_ptr(memcpy_unique_ptr &&dyingObj) noexcept 
    {
        this->ptr = dyingObj.ptr;
        dyingObj.ptr = nullptr; // Dying object is now empty
    }

    // Move assignment: Cleans up current resource before taking the new one.
    memcpy_unique_ptr &operator=(memcpy_unique_ptr &&dyingObj) noexcept 
    {
        if (this != &dyingObj) // Protect against self-assignment
        {
            delete ptr; // Release existing resource
            ptr = dyingObj.ptr;
            dyingObj.ptr = nullptr;
        }
        return *this;
    }

    // --- Pointer Accessors ---
    T *operator->() { return this->ptr; }
    T &operator*()  { return *(this->ptr); }
    T *get()        { return ptr; }

    // Destructor: Automatically deletes the managed object
    ~memcpy_unique_ptr() 
    {
        delete ptr;
    }

    // --- C-API Bridge Interface (SFINAE Guarded) ---

    // Signature Requirement: bool func(void* dest, const memcpy_unique_ptr* src)
    template <typename _Function>
    constexpr static bool is_memcpy_send_signature = std::is_invocable_r_v<bool, _Function, void *const, const memcpy_unique_ptr<T> *const>;

    /**
     * Prepares the pointer for a bitwise send (e.g., xQueueSend).
     * If the user-provided copy function succeeds, this object relinquishes 
     * ownership immediately to prevent double-deletion.
     */
    template <typename _Function>
    typename std::enable_if<is_memcpy_send_signature<_Function>, bool>::type 
    memcpy_send(void *const dest, _Function copy_fn_src)
    {
        bool success = false;
        if (copy_fn_src(dest, this))
        {
            ptr = nullptr; // Successfully "moved" into the buffer
            success = true;
        }
        return success;
    }

    // Signature Requirement: bool func(memcpy_unique_ptr* dest, const void* src)
    template <typename _Function>
    constexpr static bool is_memcpy_receive_signature = std::is_invocable_r_v<bool, _Function, memcpy_unique_ptr<T> *const, const void *const>;

    /**
     * Claims ownership from a bitwise source (e.g., xQueueReceive).
     * Uses a temporary buffer to safely receive the bits before updating 'this'.
     */
    template <typename _Function>
    typename std::enable_if<is_memcpy_receive_signature<_Function>, bool>::type 
    memcpy_receive(const void *const src, _Function copy_fn_dest)
    {
        uint8_t buffer[sizeof(memcpy_unique_ptr<T>)];
        bool success = false;

        // Perform the bitwise copy into a temporary buffer first
        if (copy_fn_dest(reinterpret_cast<memcpy_unique_ptr<T> *>(buffer), src))
        {
            delete ptr; // Clean up current data before accepting new ownership

            // Bitwise move: The object in the buffer is now managed by 'this'
            memcpy(this, buffer, sizeof(memcpy_unique_ptr<T>));
            success = true;
        }
        return success;
    }

    // --- Utility Functions ---

    // Releases ownership and returns the raw pointer (caller must delete it)
    T *release()
    {
        T *temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Replaces the managed object with a new one
    void reset(T *pt) noexcept
    {
        delete ptr;
        ptr = pt;
    }

    // Contextual conversion to bool (check if ptr is not null)
    explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }

private:
    T *ptr = nullptr;
};

/**
 * Factory function: Ensures the managed object is constructible with provided arguments.
 * Provides a cleaner syntax: auto p = make_memcpy_unique_ptr<MyClass>(args...);
 */
template <typename T, typename... ParaTypes>
typename std::enable_if<std::is_constructible<T, ParaTypes...>::value, memcpy_unique_ptr<T>>::type
make_memcpy_unique_ptr(ParaTypes &&...paras)
{
    return memcpy_unique_ptr<T>{new T(std::forward<ParaTypes...>(paras...))};
}