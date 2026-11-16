#ifndef ZZ_LIBRARY_MONOTONIC_STACK_HPP
#define ZZ_LIBRARY_MONOTONIC_STACK_HPP

namespace zz_library {

template<class T>
class monotonic_stack {
private:
    std::vector<T> values_;
    std::vector<int> next_less_;
    std::vector<int> next_leq_;
    std::vector<int> next_greater_;
    std::vector<int> next_geq_;
    std::vector<int> prev_less_;
    std::vector<int> prev_leq_;
    std::vector<int> prev_greater_;
    std::vector<int> prev_geq_;

    static std::vector<int> build_next(const std::vector<T>& values, bool less, bool equal_allowed) {
        const int n = static_cast<int>(values.size());
        std::vector<int> result(static_cast<std::size_t>(n), -1);
        std::vector<int> stack;
        stack.reserve(static_cast<std::size_t>(n));

        for (int i = n - 1; i >= 0; --i) {
            while (!stack.empty()) {
                const T& candidate = values[static_cast<std::size_t>(stack.back())];
                const T& current = values[static_cast<std::size_t>(i)];
                const bool qualifies = less
                    ? (equal_allowed ? candidate <= current : candidate < current)
                    : (equal_allowed ? candidate >= current : candidate > current);
                if (qualifies) break;
                stack.pop_back();
            }
            if (!stack.empty()) result[static_cast<std::size_t>(i)] = stack.back();
            stack.push_back(i);
        }
        return result;
    }

    static std::vector<int> build_prev(const std::vector<T>& values, bool less, bool equal_allowed) {
        const int n = static_cast<int>(values.size());
        std::vector<int> result(static_cast<std::size_t>(n), -1);
        std::vector<int> stack;
        stack.reserve(static_cast<std::size_t>(n));

        for (int i = 0; i < n; ++i) {
            while (!stack.empty()) {
                const T& candidate = values[static_cast<std::size_t>(stack.back())];
                const T& current = values[static_cast<std::size_t>(i)];
                const bool qualifies = less
                    ? (equal_allowed ? candidate <= current : candidate < current)
                    : (equal_allowed ? candidate >= current : candidate > current);
                if (qualifies) break;
                stack.pop_back();
            }
            if (!stack.empty()) result[static_cast<std::size_t>(i)] = stack.back();
            stack.push_back(i);
        }
        return result;
    }

public:
    class reference {
        friend class monotonic_stack;

        const monotonic_stack* owner_;
        int index_;

        reference(const monotonic_stack* owner, int index) : owner_(owner), index_(index) {}

    public:
        int next_less() const { return owner_->next_less_[static_cast<std::size_t>(index_)]; }
        int next_leq() const { return owner_->next_leq_[static_cast<std::size_t>(index_)]; }
        int next_greater() const { return owner_->next_greater_[static_cast<std::size_t>(index_)]; }
        int next_geq() const { return owner_->next_geq_[static_cast<std::size_t>(index_)]; }
        int prev_less() const { return owner_->prev_less_[static_cast<std::size_t>(index_)]; }
        int prev_leq() const { return owner_->prev_leq_[static_cast<std::size_t>(index_)]; }
        int prev_greater() const { return owner_->prev_greater_[static_cast<std::size_t>(index_)]; }
        int prev_geq() const { return owner_->prev_geq_[static_cast<std::size_t>(index_)]; }
    };

    monotonic_stack() = default;

    explicit monotonic_stack(const std::vector<T>& values) {
        init(values);
    }

    void init(const std::vector<T>& values) {
        values_ = values;
        next_less_ = build_next(values_, true, false);
        next_leq_ = build_next(values_, true, true);
        next_greater_ = build_next(values_, false, false);
        next_geq_ = build_next(values_, false, true);
        prev_less_ = build_prev(values_, true, false);
        prev_leq_ = build_prev(values_, true, true);
        prev_greater_ = build_prev(values_, false, false);
        prev_geq_ = build_prev(values_, false, true);
    }

    reference operator[](int index) const {
        return reference(this, index);
    }

    reference at(int index) const {
        if (index < 0 || static_cast<std::size_t>(index) >= values_.size()) {
            throw std::out_of_range("zz_library::monotonic_stack: index out of range");
        }
        return reference(this, index);
    }

    std::size_t size() const { return values_.size(); }
    bool empty() const { return values_.empty(); }
};

} // namespace zz_library

#endif // ZZ_LIBRARY_MONOTONIC_STACK_HPP
