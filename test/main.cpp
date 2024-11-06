#include "ddtest_case_factory.h"

#include <process.h>
#include <Windows.h>
#include <ConsoleApi2.h>
#include <locale.h>


namespace dd {

int test_main()
{
    DDTCF.insert_white_type("test_case_coroutine");
    DDTCF.run();
    return 0;
}
} // namespace NSP_DD

bool set_current_locale(const std::string& name)
{
    int config = ::_configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
    if (config == -1) {
        return false;
    }

    if (::setlocale(LC_ALL, name.c_str()) == NULL) {
        return false;
    }
    (void)::_configthreadlocale(config);
    return true;
}

int get_io_codepage()
{
    return int(::GetConsoleOutputCP());
}

bool set_io_codepage(int code_page)
{
    if (!::SetConsoleOutputCP(code_page)) {
        return false;
    }
    return true;
}

bool set_utf8_locale_and_io_codepage()
{
    auto raw_codepage = get_io_codepage();
    if (!set_io_codepage(65001)) {
        return false;
    }
    if (set_current_locale(".UTF-8") == NULL) {
        (void)set_io_codepage(raw_codepage);
        return false;
    }
    return true;
}

int main()
{
    // ::_CrtSetBreakAlloc(421);
    set_utf8_locale_and_io_codepage();
    int result = dd::test_main();

#ifdef _DEBUG
    _cexit();
    DDASSERT_FMT(!::_CrtDumpMemoryLeaks(), L"Memory leak!!! Check the output to find the log.");
    ::system("pause");
    ::_exit(result);
#else
    ::system("pause");
    ::exit(result);
#endif
    return 0;
}

