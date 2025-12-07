#include "CppUTest/TestHarness.h"
#include "memcpy_smart_pointer.h"

TEST_GROUP(MEMCPY_UNIQUE_PRR){};

TEST(MEMCPY_UNIQUE_PRR, Create_empty_uniquePtr)
{
    memcpy_unique_ptr<int> ptr1{};
    CHECK(ptr1.get() == nullptr);
}

TEST(MEMCPY_UNIQUE_PRR, Create_uniquePtr)
{
    memcpy_unique_ptr<int> ptr1{new int(5)};
    CHECK(*ptr1 == 5);
}

TEST(MEMCPY_UNIQUE_PRR, Move_uniquePtr)
{
    memcpy_unique_ptr<int> ptr1{new int(10)};
    memcpy_unique_ptr<int> ptr2 = std::move(ptr1);
    CHECK(*ptr2 == 10);
    CHECK(ptr1.get() == nullptr);
}

TEST(MEMCPY_UNIQUE_PRR, Release_uniquePtr)
{
    memcpy_unique_ptr<int> ptr1{new int(20)};
    int *rawPtr = ptr1.release();
    CHECK(*rawPtr == 20);
    CHECK(ptr1.get() == nullptr);
    delete rawPtr; // Manually delete since ownership is released
}

TEST(MEMCPY_UNIQUE_PRR, Reset_uniquePtr)
{
    memcpy_unique_ptr<int> ptr1{new int(30)};
    ptr1.reset(new int(40));
    CHECK(*ptr1 == 40);
}

TEST(MEMCPY_UNIQUE_PRR, Bool_conversion)
{
    memcpy_unique_ptr<int> ptr1;
    CHECK_FALSE(static_cast<bool>(ptr1));

    memcpy_unique_ptr<int> ptr2{new int(50)};
    CHECK(static_cast<bool>(ptr2));
}

TEST(MEMCPY_UNIQUE_PRR, Memcpy_SEND_function)
{
    memcpy_unique_ptr<int> ptr1{new int(60)};
    uint8_t buffer[sizeof(memcpy_unique_ptr<int>)];
    bool copied = ptr1.memcpy_send(buffer, [](void *dest, memcpy_unique_ptr<int> *src)
                                   { return memcpy(dest, src, sizeof(memcpy_unique_ptr<int>)); });
    CHECK(copied);
    CHECK(ptr1.get() == nullptr);
    int *ptr = nullptr;
    if constexpr (sizeof(int *) == sizeof(uint64_t))
    {
        ptr = reinterpret_cast<int *>(*reinterpret_cast<uint64_t *>(buffer));
    }
    else if constexpr (sizeof(int *) == sizeof(uint32_t))
    {
        ptr = reinterpret_cast<int *>(*reinterpret_cast<uint32_t *>(buffer));
    }
    delete ptr;
    ptr = nullptr;
}

TEST(MEMCPY_UNIQUE_PRR, Memcpy_receive_function)
{
    memcpy_unique_ptr<int> ptr1{new int(70)};
    uint8_t buffer[sizeof(memcpy_unique_ptr<int>)];

    bool sent = ptr1.memcpy_send(buffer, [](void *dest, memcpy_unique_ptr<int> *src)
                                 { return memcpy(dest, src, sizeof(memcpy_unique_ptr<int>)); });
    CHECK(sent);
    CHECK(ptr1.get() == nullptr);

    memcpy_unique_ptr<int> ptr2;
    bool received = ptr2.memcpy_receive(buffer, [](memcpy_unique_ptr<int> *dest, void *src)
                                        { return memcpy(dest, src, sizeof(memcpy_unique_ptr<int>)); });
    CHECK(received);
    CHECK(*ptr2 == 70);
}


TEST_GROUP(MAKE_MEMCPY_UNIQUE_PTR){};

TEST(MAKE_MEMCPY_UNIQUE_PTR, MAKE_UNIQUE_PTR_INT){
    memcpy_unique_ptr<int> ptr= make_memcpy_unique_ptr<int>(6);
}


TEST(MAKE_MEMCPY_UNIQUE_PTR, MAKE_UNIQUE_PTR_STRING){
    memcpy_unique_ptr<std::string> ptr= make_memcpy_unique_ptr<std::string>("This is Tolulope Matthew Busoye");
}