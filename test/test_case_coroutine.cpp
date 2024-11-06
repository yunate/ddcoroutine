
#include "../include/ddcoroutine.h"
#include "ddtest_case_factory.h"
#include <thread>
#include <windows.h>

using namespace dd;

void asyncio(const std::function<bool(bool)>& callback)
{
    // callback(false);
    std::thread([callback]() {
        //::Sleep(1000);
        callback(false);
    }).detach();
}

ddcoroutine<void> co_io()
{
    auto raw_callback = [](bool) { return true; };
    co_await ddawaitable([raw_callback](const ddresume_helper& resumer) {
        asyncio([resumer, raw_callback](bool v) {
            bool ret = raw_callback(v);
            resumer.lazy_resume();
            return ret;
        });
    });

    co_return;
}

ddcoroutine<int> counter()
{
    int ii = 0;
    ++ii;
    int i = 0;
    ++i;
    co_return 1;
}

ddcoroutine<std::string> counter1()
{
    int ii = 0;
    ++ii;
    int i = 0;
    ++i;
    co_return "abd";
}

ddco_task create_sleep_async(int ms)
{
    return [ms](const ddresume_helper& resumer) {
        std::thread([ms, resumer]() {
            ::Sleep(ms);
            resumer.lazy_resume();
        }).detach();
    };
}

void func_callbackex(const std::function<void()>& callback)
{
    callback();
}

static void async_caller(const std::function<void()>& task)
{
    std::thread([task]() {
        task();
    }).detach();
}

void func_callback(const std::function<void()>& callback)
{
    async_caller(callback);
}

ddcoroutine<void> test_stackoverflow()
{
    for (int i = 0; i < 3000; ++i) {
        co_await ddawaitable([](const ddresume_helper& resumer) {
            func_callback([resumer]() {
                resumer.lazy_resume();
            });
        });
    }

    for (int i = 0; i < 3000; ++i) {
        co_await ddawaitable_ex([](const ddresume_helper& resumer) {
            func_callbackex([resumer]() {
                resumer.lazy_resume();
            });
        });
    }

    for (int i = 0; i < 1000; ++i) {
        co_await counter();
    }

    for (int i = 0; i < 3000; ++i) {
        co_await ddcoroutine_all({
            ddcoroutine_from(counter()),
            ddcoroutine_from(counter1()),
            ddcoroutine_from(counter1()),
            ddcoroutine_from(counter1()),
        });
    }
}

ddcoroutine<void> test()
{
    co_await test_stackoverflow();
    auto value = co_await counter(); value;
    std::string value1 = co_await counter1(); value1;
    co_await co_io();
    co_await co_io();
    auto x = ddcoroutine_from(counter());
    co_await x;
    auto y = ddcoroutine_all({
        co_io(),
        ddcoroutine_from(counter()),
        ddcoroutine_from(counter1()),
        ddcoroutine_from(create_sleep_async(3000)),
        ddcoroutine_from(create_sleep_async(1000)),
        ddcoroutine_from(create_sleep_async(1000)),
        ddcoroutine_from(create_sleep_async(1000)),
        ddcoroutine_from(create_sleep_async(1000))
    });
    co_await y;
    co_return ;
}

ddcoroutine<void> wait_test(std::condition_variable& cv)
{
    co_await test();
    cv.notify_one();
    co_return;
}

ddcoroutine<int> tt()
{
    co_return 1;
}

namespace dd {
DDTEST(test_case_coroutine, 1)
{
    std::condition_variable cv;
    ddcoroutine_run(wait_test(cv));
    {
        std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock);
    }
}
} // namespace dd
