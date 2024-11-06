# This is a single header file library for CPP coroutine
This repo is a part of: https://github.com/yunate/dd/

Tests and demos: https://github.com/yunate/ddcoroutine/blob/main/test/test_case_coroutine.cpp

corountine + iocp + windows socket components(TCP/UDP/HTTP (s) server/HTTP (s) client): https://github.com/yunate/dd/tree/main/projects/ddbase/network

corountine + iocp + windows http(s) server demo: https://github.com/yunate/dd/blob/main/projects/test/ddbase/network/test_case_http_server.cpp

corountine + iocp + windows http(s) client demo: https://github.com/yunate/dd/blob/main/projects/test/ddbase/network/test_case_http_client.cpp

corountine + iocp + windows http(s) request demo: https://github.com/yunate/dd/blob/main/projects/test/ddbase/network/test_case_http_requester.cpp

## License
Do WTF you want to but report the issue you met to this repo.

## Useage:
### Import to your project:
Copy the `ddcoroutine.h` file to your project and include it.
```
#include "include/ddcoroutine.h"
```

### Define a co-function
Just use `ddcroutine<T>` as a function return, the template `T` is the truly return type:
```
ddcoroutine<void> co_test_void() {
    co_return;
}

ddcoroutine<int> co_test_int() {
    co_return 1;
}
```

### Wrap a async function to co-function
Assuming we have an asynchronous function:
```
void async_io(const std::function<void(void)>& callback) {
    std::thread([callback]() {
        callback();
    }).detach();
}
```
Change it to a co-function:

- `auto ddawaitable(const ddco_task& task)`

This function can wrap a async function to an awaitable object:
```
ddcoroutine<void> co_io() {
    co_await ddawaitable([](const ddresume_helper& resumer) {
        async_io([resumer]() {
            resumer.lazy_resume();
        });
    });
}
```
**Note that the wrapped function must be async(whose callback function will be called in another thread or the next loop). If the wrapped function is uncertain whether it is asynchronous or not, use `auto ddawaitable_ex(const ddco_task& task)` or `ddcoroutine<void> ddcoroutine_from(ddco_task task)` instead.**

- `auto ddawaitable_ex(const ddco_task& task)`

This is similar to `ddawaitable`, but the difference is that `ddawaitable_ex` does not require the wrapped function to be asynchronous. The cost is some additional judgment processing. Therefore, if you can ensure that the wrapped function is always asynchronous, use `ddawaitable` to get a better performance. Otherwise, use `ddawaitable_ex` function.
```
ddcoroutine<void> co_io() {
    co_await ddawaitable_ex([](const ddresume_helper& resumer) {
        async_io([resumer]() {
            resumer.lazy_resume();
        });
    });
}
```

- `ddcoroutine<void> ddcoroutine_from(ddco_task task)`

This function implements the same function as `auto ddawaitable_ex(const ddco_task& task)`, but the difference is that its return value is `ddcoroutine<void>`, which will be very useful because most of the input parameters of the other utility functions below are `ddcoroutine<void>`.
```
ddcoroutine<void> co_io() {
    co_await ddcoroutine_from([](const ddresume_helper& resumer) {
        async_io([resumer]() {
            resumer.lazy_resume();
        });
    });
}
```

### Call a co-function in a co-function
Use `co_await` to suspend current co-function until the called co-function run to end.
```
void test_normal() {}
ddcoroutine<void> test() {
    // call a normal function.
    test_normal();

    // `co_test_void` will end directly, and co_await will be immediately resumed.
    co_await co_test_void();

    // `co_test_int` will end directly, and co_await will be immediately resumed, and retrieve the return value.
    int x = co_await co_test_int();

    // `co_io` will not end directly, co_await will be suspended and will be resumed when `async_io`'s callback is called.
    co_await co_io();
    co_return;
}
```

### Call a co-function in a normal function
Just call a co-function as a normal function, the caller function will not be suspended:
```
int main() {
    ddcoroutine<void> co_ctx = co_io();

    // loop
    while (true) {
        std::sleep(1000);
    }
}
```
**Note that, before the coroutine funtion run to the end, the co-function's return object(co_ctx) cannot be release. if you do not want to manage its lifecycle, use `void ddcoroutine_run(const ddcoroutine<void>& r)`:**
```
int main() {
    // run co_io
    ddcoroutine_run(co_io());

    // loop
    while (true) {
        std::sleep(1000);
    }
}
```

### Wait for multiple co-functions
`ddcoroutine_all`
```
ddcoroutine<void> test() {
    // the co_await will be suspend until `co_io`, `co_test_void`, `co_test_int` are all resumed.
    co_await ddcoroutine_all({
        co_io(),
        co_test_void(),
        ddcoroutine_from(co_test_int())
    });
    co_return;
}
```

### Wait for any co-functions
`ddcoroutine_any`
```
ddcoroutine<void> test() {
    // the co_await will be suspend until any co-functions in `co_io`, `co_test_void`, `co_test_int` is resumed.
    int index = co_await ddcoroutine_any({
        co_io(),
        co_test_void(),
        ddcoroutine_from(co_test_int())
    }, true);
    co_return;
}
```
