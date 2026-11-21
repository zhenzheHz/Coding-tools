#ifndef ZZ_STRUCT_DEBUG_HPP
#define ZZ_STRUCT_DEBUG_HPP

#include "debug.hpp"

#define DEBUG_DETAIL_CAT_IMPL(A, B) A##B
#define DEBUG_DETAIL_CAT(A, B) DEBUG_DETAIL_CAT_IMPL(A, B)

#define DEBUG_DETAIL_NARG_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define DEBUG_DETAIL_NARG(...) DEBUG_DETAIL_NARG_IMPL(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define DEBUG_DETAIL_FE_1(M, A, B, C, D, X) M(A, B, C, D, X)
#define DEBUG_DETAIL_FE_2(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_1(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_3(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_2(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_4(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_3(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_5(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_4(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_6(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_5(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_7(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_6(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_8(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_7(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_9(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_8(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_10(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_9(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_11(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_10(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_12(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_11(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_13(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_12(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_14(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_13(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_15(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_14(M, A, B, C, D, __VA_ARGS__)
#define DEBUG_DETAIL_FE_16(M, A, B, C, D, X, ...) M(A, B, C, D, X) DEBUG_DETAIL_FE_15(M, A, B, C, D, __VA_ARGS__)

#define DEBUG_DETAIL_FOR_EACH(M, A, B, C, D, ...) \
    DEBUG_DETAIL_CAT(DEBUG_DETAIL_FE_, DEBUG_DETAIL_NARG(__VA_ARGS__))(M, A, B, C, D, __VA_ARGS__)

#define DEBUG_DETAIL_PRINT_MEMBER(CTX, VALUE, ALLOW, FIRST, MEMBER) \
    do { \
        if (!(FIRST)) (CTX).write(", "); \
        (FIRST) = false; \
        (CTX).write(#MEMBER); \
        (CTX).write(": "); \
        ::debug_detail::print((CTX), (VALUE).MEMBER, (ALLOW), false); \
    } while (false);

#define DEBUG_STRUCT(TYPE, ...) \
    template<> struct debug_detail::StructPrinter<TYPE> { \
        static constexpr bool enabled = true; \
        static void print(::debug_detail::Context& zz_debug_ctx, const TYPE& zz_debug_value, bool zz_debug_allow_2d) { \
            zz_debug_ctx.put('{'); \
            bool zz_debug_first = true; \
            DEBUG_DETAIL_FOR_EACH(DEBUG_DETAIL_PRINT_MEMBER, zz_debug_ctx, zz_debug_value, zz_debug_allow_2d, zz_debug_first, __VA_ARGS__) \
            zz_debug_ctx.put('}'); \
        } \
    };

#endif
