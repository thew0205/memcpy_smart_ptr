#include "CppUTest/TestHarness.h"
#include "memcpy_shared_pointer.h"

#include <string>
#include <cstring>

TEST_GROUP(MEMCPY_SHARED_PRR){};

TEST(MEMCPY_SHARED_PRR, Create_null_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{};
    LONGS_EQUAL(0, ptr1.get_count());
    POINTERS_EQUAL(nullptr, ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, Create_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{new int(5)};
    LONGS_EQUAL(1, ptr1.get_count());
    LONGS_EQUAL(5, *ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, COPY_CONSTRUCTOR_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{new int(5)};
    {

        memcpy_shared_ptr<int> ptr2{ptr1};
        LONGS_EQUAL(2, ptr1.get_count());
        LONGS_EQUAL(5, *ptr1.get());
        LONGS_EQUAL(2, ptr2.get_count());
        LONGS_EQUAL(5, *ptr2.get());
    }
    LONGS_EQUAL(1, ptr1.get_count());
    LONGS_EQUAL(5, *ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, COPY_ASSIGNMENT_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{new int(7)};
    {

        memcpy_shared_ptr<int> ptr2;
        ptr2 = ptr1;
        LONGS_EQUAL(2, ptr1.get_count());
        LONGS_EQUAL(7, *ptr1.get());
        LONGS_EQUAL(2, ptr2.get_count());
        LONGS_EQUAL(7, *ptr2.get());
    }
    LONGS_EQUAL(1, ptr1.get_count());
    LONGS_EQUAL(7, *ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, COPY_ASSIGNMENT_sharedPtr2)
{
    memcpy_shared_ptr<int> ptr1{new int(7)};
    {

        memcpy_shared_ptr<int> ptr2 = ptr1;

        LONGS_EQUAL(2, ptr1.get_count());
        LONGS_EQUAL(7, *ptr1.get());

        memcpy_shared_ptr<int> ptr3{new int(10)};
        ptr2 = ptr3;
        LONGS_EQUAL(1, ptr1.get_count());
        LONGS_EQUAL(7, *ptr1.get());

        LONGS_EQUAL(2, ptr2.get_count());
        LONGS_EQUAL(10, *ptr2.get());

        LONGS_EQUAL(2, ptr3.get_count());
        LONGS_EQUAL(10, *ptr3.get());

        ptr1 = ptr3;
    }
    LONGS_EQUAL(1, ptr1.get_count());
    LONGS_EQUAL(10, *ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, MOVE_CONSTRUCTOR_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{new int(5)};
    {

        memcpy_shared_ptr<int> ptr2{std::move(ptr1)};
        LONGS_EQUAL(0, ptr1.get_count());
        POINTERS_EQUAL(nullptr, ptr1.get());
        LONGS_EQUAL(5, *ptr2.get());
        LONGS_EQUAL(1, ptr2.get_count());
    }
    LONGS_EQUAL(0, ptr1.get_count());
    POINTERS_EQUAL(nullptr, ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, MOVE_ASSIGNMENT_sharedPtr)
{
    memcpy_shared_ptr<int> ptr1{new int(7)};
    {

        memcpy_shared_ptr<int> ptr2;
        ptr2 = std::move(ptr1);
        LONGS_EQUAL(0, ptr1.get_count());
        POINTERS_EQUAL(nullptr, ptr1.get());
        LONGS_EQUAL(1, ptr2.get_count());
        LONGS_EQUAL(7, *ptr2.get());
    }
    LONGS_EQUAL(0, ptr1.get_count());
    POINTERS_EQUAL(nullptr, ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, MOVE_ASSIGNMENT_sharedPtr2)
{
    memcpy_shared_ptr<int> ptr1{new int(7)};
    {

        memcpy_shared_ptr<int> ptr2 = std::move(ptr1);

        LONGS_EQUAL(0, ptr1.get_count());
        POINTERS_EQUAL(nullptr, ptr1.get());

        LONGS_EQUAL(1, ptr2.get_count());
        LONGS_EQUAL(7, *ptr2.get());

        memcpy_shared_ptr<int> ptr3{new int(8)};
        ptr2 = std::move(ptr3);

        LONGS_EQUAL(0, ptr1.get_count());
        POINTERS_EQUAL(nullptr, ptr1.get());

        LONGS_EQUAL(1, ptr2.get_count());
        LONGS_EQUAL(8, *ptr2.get());

        LONGS_EQUAL(0, ptr3.get_count());
        POINTERS_EQUAL(nullptr, ptr3.get());

        ptr1 = ptr3;

        LONGS_EQUAL(0, ptr1.get_count());
        POINTERS_EQUAL(nullptr, ptr1.get());

        LONGS_EQUAL(1, ptr2.get_count());
        LONGS_EQUAL(8, *ptr2.get());

        LONGS_EQUAL(0, ptr3.get_count());
        POINTERS_EQUAL(nullptr, ptr3.get());

        ptr1 = ptr2;

        LONGS_EQUAL(2, ptr1.get_count());
        LONGS_EQUAL(8, *ptr1.get());

        LONGS_EQUAL(2, ptr2.get_count());
        LONGS_EQUAL(8, *ptr2.get());

        LONGS_EQUAL(0, ptr3.get_count());
        POINTERS_EQUAL(nullptr, ptr3.get());
    }
    LONGS_EQUAL(1, ptr1.get_count());
    LONGS_EQUAL(8, *ptr1.get());
}

TEST(MEMCPY_SHARED_PRR, Memcpy_SEND_function)
{
    memcpy_shared_ptr<int> ptr1{new int(60)};
    uint8_t buffer[sizeof(memcpy_shared_ptr<int>)];
    bool copied = ptr1.memcpy_send(buffer, [](void *const dest, const memcpy_shared_ptr<int> *src)
                                   { return memcpy(dest, src, sizeof(memcpy_shared_ptr<int>)); });
    CHECK(copied);
    LONGS_EQUAL(2, ptr1.get_count());
    LONGS_EQUAL(60, *ptr1.get());

    ptr1.refCount.count->decrement(); // Manually decrement to avoid double free in test
}

TEST(MEMCPY_SHARED_PRR, Memcpy_receive_function)
{
    memcpy_shared_ptr<int> ptr1{new int(70)};
    uint8_t buffer[sizeof(memcpy_shared_ptr<int>)];

    bool sent = ptr1.memcpy_send(buffer, [](void *const dest, const memcpy_shared_ptr<int> *src)
                                 { return memcpy(dest, src, sizeof(memcpy_shared_ptr<int>)); });
    CHECK(sent);
    LONGS_EQUAL(2, ptr1.get_count());
    LONGS_EQUAL(70, *ptr1.get());

    memcpy_shared_ptr<int> ptr2;
    bool received = ptr2.memcpy_receive(buffer, [](memcpy_shared_ptr<int> *dest, const void *const src)
                                        { return memcpy(dest, src, sizeof(memcpy_shared_ptr<int>)); });
    CHECK(received);
    LONGS_EQUAL(2, ptr1.get_count());
    LONGS_EQUAL(70, *ptr1.get());

    LONGS_EQUAL(2, ptr2.get_count());
    LONGS_EQUAL(70, *ptr2.get());
}

TEST(MEMCPY_SHARED_PRR, Memcpy_receive_function2)
{
    memcpy_shared_ptr<int> ptr1{new int(70)};
    uint8_t buffer[sizeof(memcpy_shared_ptr<int>)];

    bool sent = ptr1.memcpy_send(buffer, [](void *const dest, const memcpy_shared_ptr<int> *src)
                                 { return memcpy(dest, src, sizeof(memcpy_shared_ptr<int>)); });
    CHECK(sent);
    LONGS_EQUAL(2, ptr1.get_count());
    LONGS_EQUAL(70, *ptr1.get());

    memcpy_shared_ptr<int> ptr2(new int(100));
    bool received = ptr2.memcpy_receive(buffer, [](memcpy_shared_ptr<int> *dest, const void *const src)
                                        { return memcpy(dest, src, sizeof(memcpy_shared_ptr<int>)); });
    CHECK(received);
    LONGS_EQUAL(2, ptr1.get_count());
    LONGS_EQUAL(70, *ptr1.get());

    LONGS_EQUAL(2, ptr2.get_count());
    LONGS_EQUAL(70, *ptr2.get());
}

TEST_GROUP(MAKE_MEMCPY_SHARED_PTR){};

TEST(MAKE_MEMCPY_SHARED_PTR, MAKE_SHARED_PTR_INT)
{
    memcpy_shared_ptr<int> ptr = make_memcpy_shared_ptr<int>(6);
}

TEST(MAKE_MEMCPY_SHARED_PTR, MAKE_SHARED_PTR_STRING)
{
    memcpy_shared_ptr<std::string> ptr = make_memcpy_shared_ptr<std::string>("This is Tolulope Matthew Busoye");
}