#ifndef ZZ_LIBRARY_DISJOINT_SET_CORE_HPP
#define ZZ_LIBRARY_DISJOINT_SET_CORE_HPP

#if defined(LOCAL) && !defined(ZZ_LIBRARY_DEBUG)
#define ZZ_LIBRARY_DEBUG
#endif
namespace zz_library {

struct basic {};
struct almost {};
struct rollback {};

template<class T = int, class Type = basic>
struct disjoint_set;

}

#endif
