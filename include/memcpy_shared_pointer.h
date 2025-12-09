#include <type_traits>

class memcpy_shared_ptr_count
{
public:
    size_t count;
    memcpy_shared_ptr_count() : count(1)
    {
    }

    void increment()
    {
        count++;
    }

    void decrement()
    {
        count--;
    }
};

class memcpy_shared_ptr_object_count
{

public:
    memcpy_shared_ptr_count *count;

    memcpy_shared_ptr_object_count() : count(nullptr)
    {
    }

    template <typename Ptr>
    explicit memcpy_shared_ptr_object_count(Ptr p) : count(nullptr)
    {
        count = new memcpy_shared_ptr_count{};
    }

    memcpy_shared_ptr_object_count(const memcpy_shared_ptr_object_count &obj) : count(obj.count)
    {
        if (count != nullptr)
        {
            count->increment();
        }
    }

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

    memcpy_shared_ptr_object_count(memcpy_shared_ptr_object_count &&obj) : count(obj.count)
    {
        obj.count = nullptr;
    }

    memcpy_shared_ptr_object_count &operator=(memcpy_shared_ptr_object_count &&obj)
    {

        count = std::move(obj.count);
        obj.count = nullptr;
        return *this;
    }

    ~memcpy_shared_ptr_object_count()
    {
        if (count != nullptr)
        {
            count->decrement();
        }
    }
};

template <class T>
class memcpy_shared_ptr
{
public:
    memcpy_shared_ptr_object_count refCount;

    memcpy_shared_ptr() : ptr(nullptr), refCount{} // default constructor
    {
    }

    memcpy_shared_ptr(T *ptr) : ptr(ptr), refCount(ptr) // constructor
    {
    }

    /*** Copy Semantics ***/
    memcpy_shared_ptr(const memcpy_shared_ptr &obj) : ptr{obj.ptr}, refCount{obj.refCount} // copy constructor
    {
    }

    memcpy_shared_ptr &operator=(const memcpy_shared_ptr &obj) // copy assignment
    {

        if (refCount.count != obj.refCount.count)
        {
            __cleanup__(); // cleanup any existing data

            // Assign incoming object's data to this object
            this->ptr = obj.ptr; // share the underlying pointer
            this->refCount = obj.refCount;
        }
        return *this;
        // __cleanup__(); // cleanup any existing data

        // // Assign incoming object's data to this object
        // this->ptr = obj.ptr; // share the underlying pointer
        // this->refCount = obj.refCount;
        // if (nullptr != obj.ptr)
        // {
        //     (*this->refCount)++; // if the pointer is not null, increment the refCount
        // }
    }

    /*** Move Semantics ***/
    memcpy_shared_ptr(memcpy_shared_ptr &&dyingObj) : ptr{std::move(dyingObj.ptr)}, refCount{std::move(dyingObj.refCount)} // move constructor
    {
        dyingObj.ptr = nullptr;
    }

    memcpy_shared_ptr &operator=(memcpy_shared_ptr &&obj) // copy assignment
    {

        if (refCount.count != obj.refCount.count)
        {
            __cleanup__(); // cleanup any existing data

            // Assign incoming object's data to this object
            this->ptr = obj.ptr; // share the underlying pointer
            this->refCount = std::move(obj.refCount);
            obj.ptr = nullptr;
        }
        return *this;
    }

    size_t get_count() const
    {
        return refCount.count == nullptr ? 0 : refCount.count->count; // *this->refCount
    }

    T *get() const
    {
        return this->ptr;
    }

    T *operator->() const
    {
        return this->ptr;
    }

    T &operator*() const
    {
        return *this->ptr;
    }

    template <typename _Function>
    constexpr static bool is_memcpy_send_signature = std::is_invocable_r_v<bool, _Function, void *const, const memcpy_shared_ptr<T> *const>;

    template <typename _Function>
    typename std::enable_if<is_memcpy_send_signature<_Function>, bool>::type memcpy_send(void *const dest, _Function copy_fn)
    {

        bool success = false;
        if (copy_fn(dest, this))
        {

            if (nullptr != refCount.count)
            {
                refCount.count->increment(); // if the pointer is not null, increment the refCount
            }
            success = true;
        }
        return success;
    }

    template <typename _Function>
    constexpr static bool is_memcpy_receive_signature = std::is_invocable_r_v<bool, _Function, memcpy_shared_ptr<T> *const, const void *const>;

    template <typename _Function>
    typename std::enable_if<is_memcpy_receive_signature<_Function>, bool>::type memcpy_receive(const void *const src, _Function copy_fn)
    {

        uint8_t buffer[sizeof(memcpy_shared_ptr<T>)];
        bool success = false;
        if (copy_fn(reinterpret_cast<memcpy_shared_ptr<T> *>(buffer), src))
        {

            if (nullptr != refCount.count)
            {
                refCount.count->decrement(); // if the pointer is not null, increment the refCount
            }
            __cleanup__();
            memcpy(this, buffer, sizeof(memcpy_shared_ptr<T>));
            success = true;
        }
        return success;
    }

    ~memcpy_shared_ptr() // destructor
    {
        __cleanup__();
    }

private:
    T *ptr = nullptr;

    void __cleanup__()
    {
        if (refCount.count != nullptr)
        {
            // Making it ==1 should be safe because it means only this object is using it
            // but if it < 1 then the ptr is null which is still safe to delete
            if ((refCount.count->count) <= 1)
            {
                delete refCount.count;
                refCount.count = nullptr;
                delete ptr;
                ptr = nullptr;
            }
        }
        else
        {
            delete ptr;
        }
    }
};

template <typename T, typename... ParaTypes>
typename std::enable_if<std::is_constructible<T, ParaTypes...>::value, memcpy_shared_ptr<T>>::type
make_memcpy_shared_ptr(ParaTypes &&...paras)
{
    return memcpy_shared_ptr<T>{new T(std::forward<ParaTypes...>(paras...))};
}