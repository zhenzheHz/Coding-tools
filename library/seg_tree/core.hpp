#ifndef ZZ_LIBRARY_SEG_TREE_CORE_HPP
#define ZZ_LIBRARY_SEG_TREE_CORE_HPP

#if defined(LOCAL) && !defined(ZZ_LIBRARY_DEBUG)
#define ZZ_LIBRARY_DEBUG
#endif

namespace zz_library {

struct zkw {};
struct lazy {};
struct domain {};
struct persistent {};
struct kth {};
struct beats {};
struct tree2d {};

namespace detail {

template <class>
inline constexpr bool dependent_false_v = false;

} // namespace detail

template <class T, class Type = zkw>
class Segment_tree {
    static_assert(detail::dependent_false_v<Type>, "zz_library::Segment_tree: unsupported segment tree type");
};

} // namespace zz_library

#endif // ZZ_LIBRARY_SEG_TREE_CORE_HPP
