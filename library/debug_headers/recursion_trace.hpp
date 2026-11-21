#ifndef ZZ_RECURSION_TRACE_HPP
#define ZZ_RECURSION_TRACE_HPP

#include "debug.hpp"

namespace recursion_trace {

inline std::size_t& depth_ref() {
    static thread_local std::size_t value = 0;
    return value;
}

inline std::size_t depth() {
    return depth_ref();
}

inline void print_indent(std::size_t level) {
    for (std::size_t i = 0; i < level * 2; ++i) std::cerr.put(' ');
}

struct Scope {
    std::size_t level;

    explicit Scope(const char* function) : level(depth_ref()++) {
        print_indent(level);
        std::cerr << function << '(';
    }

    template<typename... T>
    void enter(const T&... args) {
        if constexpr (sizeof...(T) > 0) {
            debug_detail::Context ctx(std::cerr);
            bool first = true;
            debug_detail::print_args(ctx, first, false, args...);
        }
        std::cerr << ") {\n";
    }

    void return_value() const {
        print_indent(level + 1);
        std::cerr << "return\n";
    }

    template<typename T>
    void return_value(const T& value) const {
        print_indent(level + 1);
        debug_detail::Context ctx(std::cerr);
        ctx.write("return ");
        debug_detail::print(ctx, value, true, false);
        std::cerr.put('\n');
    }

    ~Scope() {
        --depth_ref();
        print_indent(level);
        std::cerr << "}\n";
    }
};

}

#ifndef ONLINE_JUDGE
#define TRACE_ENTER(...) ::recursion_trace::Scope zz_trace_scope(__func__); zz_trace_scope.enter(__VA_ARGS__)
#define TRACE_SCOPE(...) TRACE_ENTER(__VA_ARGS__)
#define TRACE_RETURN(...) zz_trace_scope.return_value(__VA_ARGS__)
#define TRACE_DEPTH() ::recursion_trace::depth()
#else
#define TRACE_ENTER(...)
#define TRACE_SCOPE(...)
#define TRACE_RETURN(...)
#define TRACE_DEPTH() 0
#endif

#endif
