#ifndef ZZ_DEBUG_HPP
#define ZZ_DEBUG_HPP

namespace debug_detail {

struct Context {
    std::ostream& out;
    std::size_t column = 0;

    explicit Context(std::ostream& stream) : out(stream) {}

    void put(char c) {
        out.put(c);
        column = (c == '\n' ? 0 : column + 1);
    }

    void write(const char* s) {
        while (*s) put(*s++);
    }

    void write(const std::string& s) {
        for (char c : s) put(c);
    }

    void newline_indent(std::size_t n) {
        put('\n');
        for (std::size_t i = 0; i < n; ++i) put(' ');
    }
};

template<typename T> struct is_pair : std::false_type {};
template<typename A, typename B> struct is_pair<std::pair<A, B>> : std::true_type {};

template<typename T> struct is_std_vector : std::false_type {};
template<typename T, typename Alloc> struct is_std_vector<std::vector<T, Alloc>> : std::true_type {};

template<typename T> struct is_vector_2d : std::false_type {};
template<typename T, typename A1, typename A2>
struct is_vector_2d<std::vector<std::vector<T, A1>, A2>> : std::bool_constant<!is_std_vector<T>::value> {};

template<typename T> struct is_vector_3d : std::false_type {};
template<typename T, typename A1, typename A2, typename A3>
struct is_vector_3d<std::vector<std::vector<std::vector<T, A1>, A2>, A3>> : std::true_type {};

template<typename T> struct is_vector_bool : std::false_type {};
template<typename Alloc> struct is_vector_bool<std::vector<bool, Alloc>> : std::true_type {};

template<typename T> struct is_bitset : std::false_type {};
template<std::size_t N> struct is_bitset<std::bitset<N>> : std::true_type {};

template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<const T&>())),
    decltype(std::end(std::declval<const T&>()))
>> : std::true_type {};

template<typename T, typename = void>
struct is_tuple_like : std::false_type {};

template<typename T>
struct is_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>> : std::true_type {};

template<typename T>
struct StructPrinter {
    static constexpr bool enabled = false;
};

template<typename T>
struct has_debug_struct : std::bool_constant<StructPrinter<T>::enabled> {};

template<typename T> struct always_false : std::false_type {};

inline void print_int128(Context& ctx, __int128_t value) {
    if (value == 0) {
        ctx.put('0');
        return;
    }

    bool negative = value < 0;
    __uint128_t x = negative ? static_cast<__uint128_t>(-(value + 1)) + 1 : static_cast<__uint128_t>(value);
    char buffer[40];
    std::size_t n = 0;

    while (x) {
        buffer[n++] = static_cast<char>('0' + x % 10);
        x /= 10;
    }

    if (negative) ctx.put('-');
    while (n) ctx.put(buffer[--n]);
}

inline void print_uint128(Context& ctx, __uint128_t value) {
    if (value == 0) {
        ctx.put('0');
        return;
    }

    char buffer[40];
    std::size_t n = 0;

    while (value) {
        buffer[n++] = static_cast<char>('0' + value % 10);
        value /= 10;
    }

    while (n) ctx.put(buffer[--n]);
}

inline void print_char(Context& ctx, char value) {
    ctx.put('\'');
    if (value == '\\' || value == '\'') {
        ctx.put('\\');
        ctx.put(value);
    } else if (value == '\n') ctx.write("\\n");
    else if (value == '\t') ctx.write("\\t");
    else if (value == '\r') ctx.write("\\r");
    else ctx.put(value);
    ctx.put('\'');
}

inline void print_string_char(Context& ctx, char c) {
    if (c == '\\' || c == '"') {
        ctx.put('\\');
        ctx.put(c);
    } else if (c == '\n') ctx.write("\\n");
    else if (c == '\t') ctx.write("\\t");
    else if (c == '\r') ctx.write("\\r");
    else ctx.put(c);
}

inline void print_string(Context& ctx, const char* value) {
    if (value == nullptr) {
        ctx.write("nullptr");
        return;
    }

    ctx.put('"');
    while (*value) print_string_char(ctx, *value++);
    ctx.put('"');
}

inline void print_string(Context& ctx, const std::string& value) {
    ctx.put('"');
    for (char c : value) print_string_char(ctx, c);
    ctx.put('"');
}

template<typename T>
void print(Context& ctx, const T& value, bool pretty_2d = true, bool top_level = false);

template<std::size_t I = 0, typename T>
void print_tuple_elements(Context& ctx, const T& value, bool pretty_2d) {
    if constexpr (I < std::tuple_size<T>::value) {
        if constexpr (I > 0) ctx.put(',');
        print(ctx, std::get<I>(value), pretty_2d, false);
        print_tuple_elements<I + 1>(ctx, value, pretty_2d);
    }
}

template<typename T>
void print_iterable(Context& ctx, const T& value, bool pretty_2d, bool top_level) {
    if constexpr (is_std_vector<T>::value) {
        constexpr bool multiline = is_vector_2d<T>::value;
        constexpr bool nested_3d = is_vector_3d<T>::value;

        if constexpr (is_vector_bool<T>::value) {
            ctx.put('{');
            bool first = true;
            for (bool item : value) {
                if (!first) ctx.put(',');
                first = false;
                print(ctx, item, false, false);
            }
            ctx.put('}');
        } else if constexpr (multiline) {
            if (pretty_2d) {
                bool first = true;
                if (!top_level) ctx.put('{');
                std::size_t indent = ctx.column;

                for (const auto& item : value) {
                    if (!first) {
                        ctx.put(',');
                        ctx.newline_indent(indent);
                    }
                    first = false;
                    print(ctx, item, false, false);
                }

                if (!top_level) ctx.put('}');
            } else {
                ctx.put('{');
                bool first = true;

                for (const auto& item : value) {
                    if (!first) ctx.put(',');
                    first = false;
                    print(ctx, item, false, false);
                }

                ctx.put('}');
            }
        } else {
            ctx.put('{');
            bool first = true;

            for (const auto& item : value) {
                if (!first) ctx.put(',');
                first = false;
                print(ctx, item, pretty_2d && !nested_3d, false);
            }

            ctx.put('}');
        }
    } else {
        ctx.put('{');
        bool first = true;

        for (const auto& item : value) {
            if (!first) ctx.put(',');
            first = false;
            print(ctx, item, pretty_2d, false);
        }

        ctx.put('}');
    }
}

template<typename T>
void print(Context& ctx, const T& value, bool pretty_2d, bool top_level) {
    using U = std::remove_cv_t<std::remove_reference_t<T>>;

    if constexpr (std::is_same_v<U, bool>) {
        ctx.write(value ? "true" : "false");
    } else if constexpr (std::is_same_v<U, char>) {
        print_char(ctx, value);
    } else if constexpr (std::is_same_v<U, signed char>) {
        std::ostringstream out;
        out << static_cast<long long>(value);
        ctx.write(out.str());
    } else if constexpr (std::is_same_v<U, unsigned char>) {
        std::ostringstream out;
        out << static_cast<unsigned long long>(value);
        ctx.write(out.str());
    } else if constexpr (std::is_same_v<U, __int128_t>) {
        print_int128(ctx, value);
    } else if constexpr (std::is_same_v<U, __uint128_t>) {
        print_uint128(ctx, value);
    } else if constexpr (std::is_same_v<U, std::string>) {
        print_string(ctx, value);
    } else if constexpr (std::is_array_v<U> && std::is_same_v<std::remove_cv_t<std::remove_extent_t<U>>, char>) {
        print_string(ctx, value);
    } else if constexpr (std::is_pointer_v<U> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<U>>, char>) {
        print_string(ctx, value);
    } else if constexpr (std::is_pointer_v<U> && (
        std::is_object_v<std::remove_pointer_t<U>> || std::is_void_v<std::remove_pointer_t<U>>
    )) {
        if (value == nullptr) ctx.write("nullptr");
        else {
            const volatile void* address = value;
            std::ostringstream out;
            out << const_cast<const void*>(address);
            ctx.write(out.str());
        }
    } else if constexpr (std::is_same_v<U, std::nullptr_t>) {
        ctx.write("nullptr");
    } else if constexpr (std::is_arithmetic_v<U>) {
        std::ostringstream out;
        out << value;
        ctx.write(out.str());
    } else if constexpr (std::is_enum_v<U>) {
        print(ctx, static_cast<std::underlying_type_t<U>>(value), pretty_2d, false);
    } else if constexpr (is_pair<U>::value) {
        ctx.put('{');
        print(ctx, value.first, pretty_2d, false);
        ctx.put(',');
        print(ctx, value.second, pretty_2d, false);
        ctx.put('}');
    } else if constexpr (is_bitset<U>::value) {
        ctx.put('{');
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (i) ctx.put(',');
            print(ctx, value[i], false, false);
        }
        ctx.put('}');
    } else if constexpr (is_tuple_like<U>::value) {
        ctx.put('{');
        print_tuple_elements(ctx, value, pretty_2d);
        ctx.put('}');
    } else if constexpr (has_debug_struct<U>::value) {
        StructPrinter<U>::print(ctx, value, pretty_2d);
    } else if constexpr (is_iterable<U>::value) {
        print_iterable(ctx, value, pretty_2d, top_level);
    } else {
        static_assert(always_false<U>::value, "debug.hpp: unsupported type; register it with DEBUG_STRUCT(Type, fields...)");
    }
}

template<typename T>
void print_args(Context& ctx, bool& first, bool top_level, const T& value) {
    if (!first) ctx.write(", ");
    first = false;
    print(ctx, value, true, top_level);
}

template<typename T, typename... Rest>
void print_args(Context& ctx, bool& first, bool top_level, const T& value, const Rest&... rest) {
    print_args(ctx, first, top_level, value);
    (print_args(ctx, first, top_level, rest), ...);
}

template<typename... T>
void debug_print(const char* names, const T&... values) {
    Context ctx(std::cerr);
    ctx.put('[');
    ctx.write(names);
    ctx.write("] = [");
    bool first = true;
    print_args(ctx, first, true, values...);
    ctx.write("]\n");
}

}

#ifndef ONLINE_JUDGE
#define debug(...) ::debug_detail::debug_print(#__VA_ARGS__, __VA_ARGS__)
#else
#define debug(...)
#endif

#endif
