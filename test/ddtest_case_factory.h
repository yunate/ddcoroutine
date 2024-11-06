
#ifndef ddtest_case_factory_h_
#define ddtest_case_factory_h_

#include <functional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace dd {
class dditest_case {
public:
    virtual void run() = 0;
};

class ddtest_case_factory {
public:
    static ddtest_case_factory& get_instance()
    {
        static ddtest_case_factory inst;
        return inst;
    }

    inline void add_case(const std::string& name, dditest_case* testCase)
    {
        m_cases[name].push_back(testCase);
    }

    inline void insert_white_type(const std::string& name)
    {
        m_white_type.insert(name);
    }

    inline void run()
    {
        for (auto& it : m_cases) {
            if (m_white_type.find(it.first) != m_white_type.end()) {
                for (size_t i = 0; i < it.second.size(); ++i) {
                    dditest_case* testCase = it.second[i];
                    testCase->run();
                }
            }
        }
    }

private:
    std::unordered_map<std::string, std::vector<dditest_case*>> m_cases;
    std::unordered_set<std::string> m_white_type;
};

#define DDTCF ddtest_case_factory::get_instance()

#define DDTEST(ty, N) \
class dd ## ty ## N ## _test_case : public dditest_case \
{ \
public: \
    dd ## ty ## N ## _test_case() \
    { \
        m_dummy; \
        DDTCF.add_case(#ty, &m_dummy); \
    } \
    virtual void run() override; \
private: \
    static dd ## ty ## N ## _test_case m_dummy; \
}; \
dd ## ty ## N ## _test_case dd ## ty ## N ## _test_case::m_dummy; \
void dd ## ty ## N ## _test_case::run()

} // namespace


#ifdef _DEBUG
#include <assert.h>
#define DDASSERT(e) assert(e)
#include <crtdbg.h>
#define DDASSERT_FMTW(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != ::_CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))

#define DDASSERT_FMTA(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != ::_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))
#else
#define DDASSERT(x) ((void)0)
#define DDASSERT_FMTW(expr, format, ...)
#define DDASSERT_FMTA(expr, format, ...)
#endif

#ifdef _UNICODE 
#define DDASSERT_FMT    DDASSERT_FMTW
#else
#define DDASSERT_FMT    DDASSERT_FMTA
#endif

#endif // ddtest_case_factory_h_
