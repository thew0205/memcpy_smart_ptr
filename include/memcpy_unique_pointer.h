#include <type_traits>

template <class T>
class memcpy_unique_ptr
{

public:
    memcpy_unique_ptr() noexcept : ptr(nullptr) // default constructor
    {
    }

    explicit memcpy_unique_ptr(T *ptr) noexcept : ptr(ptr)
    {
    }

    memcpy_unique_ptr(const memcpy_unique_ptr &obj) = delete;            // copy constructor is deleted
    memcpy_unique_ptr &operator=(const memcpy_unique_ptr &obj) = delete; // copy assignment is deleted

    memcpy_unique_ptr(memcpy_unique_ptr &&dyingObj) noexcept // move constructor
    {
        // Transfer ownership of the memory pointed by dyingObj to this object
        this->ptr = dyingObj.ptr;

        dyingObj.ptr = nullptr;
    }

    memcpy_unique_ptr &operator=(memcpy_unique_ptr &&dyingObj) noexcept // move assignment
    {
        delete ptr; // cleanup any existing data
        ptr = nullptr;
        // Transfer ownership of the memory pointed by dyingObj to this object
        this->ptr = dyingObj.ptr;
        dyingObj.ptr = nullptr;

        return *this;
    }

    T *operator->() // deferencing arrow operator
    {
        return this->ptr;
    }

    T &operator*()
    {
        return *(this->ptr);
    }

    T *get()
    {
        return ptr;
    }

    ~memcpy_unique_ptr() // destructor
    {
        delete ptr;
    }

    template <typename _Function>
    constexpr static bool is_memcpy_send_signature = std::is_invocable_r_v<bool, _Function, void *const, const memcpy_unique_ptr<T> *const>;

    template <typename _Function>
    typename std::enable_if<is_memcpy_send_signature<_Function>, bool>::type memcpy_send(void *const dest, _Function copy_fn_src)
    {
        bool success = false;
        if (copy_fn_src(dest, this))
        {
            ptr = nullptr;
            success = true;
        }
        return success;
    }

    template <typename _Function>
    constexpr static bool is_memcpy_receive_signature = std::is_invocable_r_v<bool, _Function, memcpy_unique_ptr<T> *const, const void *const>;

    template <typename _Function>
    typename std::enable_if<is_memcpy_receive_signature<_Function>, bool>::type memcpy_receive(const void *const src, _Function copy_fn_dest)
    {

        uint8_t buffer[sizeof(memcpy_unique_ptr<T>)];
        bool success = false;

        if (copy_fn_dest(reinterpret_cast<memcpy_unique_ptr<T> *>(buffer), src))
        {
            delete ptr;

            memcpy(this, buffer, sizeof(memcpy_unique_ptr<T>));
            success = true;
        }
        return success;
    }

    T *release()
    {
        T *temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void reset(T *pt) noexcept
    {
        delete ptr;
        ptr = pt;
    }

    explicit operator bool() const noexcept
    {
        return ptr != nullptr;
    }

private:
    T *ptr = nullptr;
};

template <typename T, typename... ParaTypes>
typename std::enable_if<std::is_constructible<T, ParaTypes...>::value, memcpy_unique_ptr<T>>::type
make_memcpy_unique_ptr(ParaTypes &&...paras)
{
    return memcpy_unique_ptr<T>{new T(std::forward<ParaTypes...>(paras...))};
}