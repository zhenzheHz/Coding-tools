#ifndef ZZ_LIBRARY_HEAP_HPP
#define ZZ_LIBRARY_HEAP_HPP

namespace zz_library {

struct minimum {};
struct maximum {};
struct leftist {};
struct binomial {};
struct fibonacci {};
struct pairing {};
struct SMMH {};
struct interval {};

namespace detail {

template<class T, class Compare, bool IsMinimum>
struct heap_order {
    Compare compare{};
    bool before(const T& a, const T& b) const {
        if constexpr (IsMinimum) return compare(a, b);
        else return compare(b, a);
    }
};

template<class T, class Type, class Compare>
class heap_impl;

template<class T, class Compare, bool IsMinimum>
class binary_heap {
    struct priority_compare {
        Compare compare{};
        bool operator()(const T& a, const T& b) const {
            if constexpr (IsMinimum) return compare(b, a);
            else return compare(a, b);
        }
    };
    using storage_type = std::priority_queue<T, std::vector<T>, priority_compare>;
    storage_type data_;
public:
    using value_type = T;
    using size_type = std::size_t;
    binary_heap() = default;
    explicit binary_heap(const Compare& compare) : data_(priority_compare{compare}) {}
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    const T& top() const { return data_.top(); }
    void push(const T& value) { data_.push(value); }
    void push(T&& value) { data_.push(std::move(value)); }
    template<class... Args> void emplace(Args&&... args) { data_.emplace(std::forward<Args>(args)...); }
    void pop() { data_.pop(); }
    void clear() { storage_type().swap(data_); }
};

template<class T, class Compare>
class heap_impl<T, maximum, Compare> : public binary_heap<T, Compare, false> {
    using base = binary_heap<T, Compare, false>;
public:
    using base::base;
};

template<class T, class Compare>
class heap_impl<T, minimum, Compare> : public binary_heap<T, Compare, true> {
    using base = binary_heap<T, Compare, true>;
public:
    using base::base;
};

template<class T, class Compare, bool IsMinimum>
class leftist_heap {
    struct node {
        T value;
        node* left = nullptr;
        node* right = nullptr;
        std::size_t rank = 1;
        template<class... Args> explicit node(Args&&... args) : value(std::forward<Args>(args)...) {}
    };
    using order_type = heap_order<T, Compare, IsMinimum>;
    node* root_ = nullptr;
    std::size_t size_ = 0;
    order_type order_;
    static std::size_t rank(node* p) { return p ? p->rank : 0; }
    node* meld(node* a, node* b) {
        if (!a) return b;
        if (!b) return a;
        if (order_.before(b->value, a->value)) std::swap(a, b);
        a->right = meld(a->right, b);
        if (rank(a->left) < rank(a->right)) std::swap(a->left, a->right);
        a->rank = rank(a->right) + 1;
        return a;
    }
    static void destroy(node* p) {
        if (!p) return;
        std::vector<node*> st{p};
        while (!st.empty()) {
            node* q = st.back(); st.pop_back();
            if (q->left) st.push_back(q->left);
            if (q->right) st.push_back(q->right);
            delete q;
        }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    leftist_heap() = default;
    explicit leftist_heap(const Compare& compare) { order_.compare = compare; }
    leftist_heap(const leftist_heap&) = delete;
    leftist_heap& operator=(const leftist_heap&) = delete;
    leftist_heap(leftist_heap&& other) noexcept : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0)), order_(std::move(other.order_)) {}
    leftist_heap& operator=(leftist_heap&& other) noexcept {
        if (this != &other) { clear(); root_ = std::exchange(other.root_, nullptr); size_ = std::exchange(other.size_, 0); order_ = std::move(other.order_); }
        return *this;
    }
    ~leftist_heap() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { return root_->value; }
    void push(const T& value) { root_ = meld(root_, new node(value)); ++size_; }
    void push(T&& value) { root_ = meld(root_, new node(std::move(value))); ++size_; }
    template<class... Args> void emplace(Args&&... args) { root_ = meld(root_, new node(std::forward<Args>(args)...)); ++size_; }
    void pop() { node* old = root_; root_ = meld(old->left, old->right); delete old; --size_; }
    void merge(leftist_heap& other) {
        if (this == &other || other.empty()) return;
        root_ = meld(root_, other.root_); size_ += other.size_; other.root_ = nullptr; other.size_ = 0;
    }
    void clear() { destroy(root_); root_ = nullptr; size_ = 0; }
};

template<class T, class Compare>
class heap_impl<T, leftist, Compare> : public leftist_heap<T, Compare, false> {
    using base = leftist_heap<T, Compare, false>;
public:
    using base::base;
};

template<class T, class Compare>
class binomial_heap {
    struct node {
        T value;
        std::size_t degree = 0;
        node* parent = nullptr;
        node* child = nullptr;
        node* sibling = nullptr;
        template<class... Args> explicit node(Args&&... args) : value(std::forward<Args>(args)...) {}
    };
    using order_type = heap_order<T, Compare, false>;
    std::vector<node*> roots_;
    std::size_t size_ = 0;
    order_type order_;
    void link(node* child, node* parent) {
        child->parent = parent; child->sibling = parent->child; parent->child = child; ++parent->degree;
    }
    void merge_roots(std::vector<node*>& other) {
        std::vector<node*> merged;
        merged.reserve(roots_.size() + other.size());
        std::size_t i = 0, j = 0;
        while (i < roots_.size() || j < other.size()) {
            if (j == other.size() || (i < roots_.size() && roots_[i]->degree < other[j]->degree)) merged.push_back(roots_[i++]);
            else merged.push_back(other[j++]);
        }
        roots_.swap(merged);
        other.clear();
        if (roots_.size() < 2) return;
        std::size_t prev = 0, cur = 0;
        while (cur + 1 < roots_.size()) {
            if (roots_[cur]->degree != roots_[cur + 1]->degree || (cur + 2 < roots_.size() && roots_[cur + 2]->degree == roots_[cur]->degree)) { ++cur; continue; }
            if (order_.before(roots_[cur + 1]->value, roots_[cur]->value)) {
                node* a = roots_[cur]; node* b = roots_[cur + 1]; link(a, b); roots_[cur] = b; roots_.erase(roots_.begin() + cur + 1);
            } else {
                link(roots_[cur + 1], roots_[cur]); roots_.erase(roots_.begin() + cur + 1);
            }
            prev = cur;
            (void)prev;
        }
    }
    static void destroy(node* p) {
        while (p) { node* next = p->sibling; destroy(p->child); delete p; p = next; }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    binomial_heap() = default;
    explicit binomial_heap(const Compare& compare) { order_.compare = compare; }
    binomial_heap(const binomial_heap&) = delete;
    binomial_heap& operator=(const binomial_heap&) = delete;
    binomial_heap(binomial_heap&& other) noexcept : roots_(std::move(other.roots_)), size_(std::exchange(other.size_, 0)), order_(std::move(other.order_)) { other.roots_.clear(); }
    binomial_heap& operator=(binomial_heap&& other) noexcept {
        if (this != &other) { clear(); roots_ = std::move(other.roots_); size_ = std::exchange(other.size_, 0); order_ = std::move(other.order_); other.roots_.clear(); }
        return *this;
    }
    ~binomial_heap() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { node* best = roots_[0]; for (node* p : roots_) if (order_.before(p->value, best->value)) best = p; return best->value; }
    void push(const T& value) { binomial_heap tmp; tmp.order_ = order_; tmp.roots_.push_back(new node(value)); tmp.size_ = 1; merge(tmp); }
    void push(T&& value) { binomial_heap tmp; tmp.order_ = order_; tmp.roots_.push_back(new node(std::move(value))); tmp.size_ = 1; merge(tmp); }
    template<class... Args> void emplace(Args&&... args) { binomial_heap tmp; tmp.order_ = order_; tmp.roots_.push_back(new node(std::forward<Args>(args)...)); tmp.size_ = 1; merge(tmp); }
    void pop() {
        std::size_t index = 0;
        for (std::size_t i = 1; i < roots_.size(); ++i) if (order_.before(roots_[i]->value, roots_[index]->value)) index = i;
        node* old = roots_[index]; roots_.erase(roots_.begin() + index);
        std::vector<node*> children;
        for (node* p = old->child; p;) { node* next = p->sibling; p->parent = nullptr; p->sibling = nullptr; children.push_back(p); p = next; }
        std::reverse(children.begin(), children.end());
        delete old; --size_; merge_roots(children);
    }
    void merge(binomial_heap& other) {
        if (this == &other || other.empty()) return;
        merge_roots(other.roots_); size_ += other.size_; other.size_ = 0;
    }
    void clear() { for (node* p : roots_) destroy(p); roots_.clear(); size_ = 0; }
};

template<class T, class Compare>
class heap_impl<T, binomial, Compare> : public binomial_heap<T, Compare> {
    using base = binomial_heap<T, Compare>;
public:
    using base::base;
};

template<class T, class Compare>
class pairing_heap {
    struct node {
        T value;
        node* child = nullptr;
        node* sibling = nullptr;
        template<class... Args> explicit node(Args&&... args) : value(std::forward<Args>(args)...) {}
    };
    using order_type = heap_order<T, Compare, false>;
    node* root_ = nullptr;
    std::size_t size_ = 0;
    order_type order_;
    node* meld(node* a, node* b) {
        if (!a) return b;
        if (!b) return a;
        if (order_.before(b->value, a->value)) std::swap(a, b);
        b->sibling = a->child; a->child = b;
        return a;
    }
    node* merge_children(node* first) {
        if (!first || !first->sibling) return first;
        std::vector<node*> paired;
        while (first) {
            node* a = first; node* b = a->sibling;
            first = b ? b->sibling : nullptr;
            a->sibling = nullptr;
            if (b) { b->sibling = nullptr; paired.push_back(meld(a, b)); }
            else paired.push_back(a);
        }
        node* result = nullptr;
        for (std::size_t i = paired.size(); i-- > 0;) result = meld(paired[i], result);
        return result;
    }
    static void destroy(node* p) {
        if (!p) return;
        std::vector<node*> st{p};
        while (!st.empty()) { node* q = st.back(); st.pop_back(); if (q->child) st.push_back(q->child); if (q->sibling) st.push_back(q->sibling); delete q; }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    pairing_heap() = default;
    explicit pairing_heap(const Compare& compare) { order_.compare = compare; }
    pairing_heap(const pairing_heap&) = delete;
    pairing_heap& operator=(const pairing_heap&) = delete;
    pairing_heap(pairing_heap&& other) noexcept : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0)), order_(std::move(other.order_)) {}
    pairing_heap& operator=(pairing_heap&& other) noexcept { if (this != &other) { clear(); root_ = std::exchange(other.root_, nullptr); size_ = std::exchange(other.size_, 0); order_ = std::move(other.order_); } return *this; }
    ~pairing_heap() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { return root_->value; }
    void push(const T& value) { root_ = meld(root_, new node(value)); ++size_; }
    void push(T&& value) { root_ = meld(root_, new node(std::move(value))); ++size_; }
    template<class... Args> void emplace(Args&&... args) { root_ = meld(root_, new node(std::forward<Args>(args)...)); ++size_; }
    void pop() { node* old = root_; root_ = merge_children(old->child); delete old; --size_; }
    void merge(pairing_heap& other) { if (this == &other || other.empty()) return; root_ = meld(root_, other.root_); size_ += other.size_; other.root_ = nullptr; other.size_ = 0; }
    void clear() { destroy(root_); root_ = nullptr; size_ = 0; }
};

template<class T, class Compare>
class heap_impl<T, pairing, Compare> : public pairing_heap<T, Compare> {
    using base = pairing_heap<T, Compare>;
public:
    using base::base;
};

template<class T, class Compare>
class fibonacci_heap {
    struct node {
        T value;
        node* parent = nullptr;
        std::list<node*> children;
        typename std::list<node*>::iterator sibling;
        typename std::list<node*>::iterator root_position;
        std::size_t degree = 0;
        bool mark = false;
        template<class... Args> explicit node(Args&&... args) : value(std::forward<Args>(args)...) {}
    };
    using order_type = heap_order<T, Compare, false>;
public:
    using value_type = T;
    using size_type = std::size_t;
    using handle_type = void*;
private:
    std::list<node*> roots_;
    node* top_ = nullptr;
    std::size_t size_ = 0;
    order_type order_;
    void add_root(node* p) { p->parent = nullptr; p->mark = false; roots_.push_back(p); p->root_position = std::prev(roots_.end()); }
    void link(node* child, node* parent) { child->parent = parent; child->mark = false; parent->children.push_back(child); child->sibling = std::prev(parent->children.end()); ++parent->degree; }
    void consolidate() {
        std::vector<node*> old(roots_.begin(), roots_.end()); roots_.clear();
        std::vector<node*> degree;
        for (node* x : old) {
            std::size_t d = x->degree;
            while (true) {
                if (d >= degree.size()) degree.resize(d + 1, nullptr);
                if (!degree[d]) { degree[d] = x; break; }
                node* y = degree[d]; degree[d] = nullptr;
                if (order_.before(y->value, x->value)) std::swap(x, y);
                link(y, x); d = x->degree;
            }
        }
        top_ = nullptr;
        for (node* p : degree) if (p) { roots_.push_back(p); p->root_position = std::prev(roots_.end()); if (!top_ || order_.before(p->value, top_->value)) top_ = p; }
    }
    void cut(node* x, node* p) { p->children.erase(x->sibling); --p->degree; add_root(x); }
    void cascading_cut(node* p) { node* parent = p->parent; if (!parent) return; if (!p->mark) p->mark = true; else { cut(p, parent); cascading_cut(parent); } }
    static void destroy_tree(node* p) { if (!p) return; for (node* child : p->children) destroy_tree(child); delete p; }
    bool contains(node* target) const {
        std::vector<node*> st(roots_.begin(), roots_.end());
        while (!st.empty()) { node* p = st.back(); st.pop_back(); if (p == target) return true; for (node* child : p->children) st.push_back(child); }
        return false;
    }
public:
    fibonacci_heap() = default;
    explicit fibonacci_heap(const Compare& compare) { order_.compare = compare; }
    fibonacci_heap(const fibonacci_heap&) = delete;
    fibonacci_heap& operator=(const fibonacci_heap&) = delete;
    fibonacci_heap(fibonacci_heap&& other) noexcept : roots_(std::move(other.roots_)), top_(std::exchange(other.top_, nullptr)), size_(std::exchange(other.size_, 0)), order_(std::move(other.order_)) { other.roots_.clear(); }
    fibonacci_heap& operator=(fibonacci_heap&& other) noexcept { if (this != &other) { clear(); roots_ = std::move(other.roots_); top_ = std::exchange(other.top_, nullptr); size_ = std::exchange(other.size_, 0); order_ = std::move(other.order_); other.roots_.clear(); } return *this; }
    ~fibonacci_heap() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { return top_->value; }
    handle_type push(const T& value) { return emplace(value); }
    handle_type push(T&& value) { return emplace(std::move(value)); }
    template<class... Args> handle_type emplace(Args&&... args) { node* p = new node(std::forward<Args>(args)...); add_root(p); if (!top_ || order_.before(p->value, top_->value)) top_ = p; ++size_; return static_cast<handle_type>(p); }
    void pop() {
        node* old = top_;
        for (node* child : old->children) add_root(child);
        roots_.erase(old->root_position); delete old; --size_;
        if (roots_.empty()) top_ = nullptr; else consolidate();
    }
    void decrease_key(handle_type handle, const T& value) {
        node* target = static_cast<node*>(handle);
        if (!target || !contains(target)) throw std::invalid_argument("zz_library::heap<fibonacci>::decrease_key: invalid handle");
        if (order_.before(value, target->value)) throw std::invalid_argument("zz_library::heap<fibonacci>::decrease_key: new value has higher priority");
        target->value = value;
        node* parent = target->parent;
        if (parent && order_.before(target->value, parent->value)) { cut(target, parent); cascading_cut(parent); }
        if (!top_ || order_.before(target->value, top_->value)) top_ = target;
        else if (top_ == target) { top_ = nullptr; for (node* p : roots_) if (!top_ || order_.before(p->value, top_->value)) top_ = p; }
    }
    void merge(fibonacci_heap& other) {
        if (this == &other || other.empty()) return;
        roots_.splice(roots_.end(), other.roots_);
        if (!top_ || (other.top_ && order_.before(other.top_->value, top_->value))) top_ = other.top_;
        size_ += other.size_; other.top_ = nullptr; other.size_ = 0;
    }
    void clear() { for (node* p : roots_) destroy_tree(p); roots_.clear(); top_ = nullptr; size_ = 0; }
};

template<class T, class Compare>
class heap_impl<T, fibonacci, Compare> : public fibonacci_heap<T, Compare> {
    using base = fibonacci_heap<T, Compare>;
public:
    using base::base;
};


template<class T, class Compare>
class interval_heap {
    std::vector<T> data_;
    Compare compare_{};
    bool less(const T& a, const T& b) const { return compare_(a, b); }
    std::size_t high(std::size_t node) const { return 2 * node + 1 < data_.size() ? 2 * node + 1 : 2 * node; }
    void sift_up_min(std::size_t node) {
        while (node) {
            std::size_t parent = (node - 1) / 2;
            if (!less(data_[2 * node], data_[2 * parent])) break;
            std::swap(data_[2 * node], data_[2 * parent]);
            node = parent;
        }
    }
    void sift_up_max(std::size_t node) {
        while (node) {
            std::size_t parent = (node - 1) / 2;
            std::size_t h = high(node), ph = 2 * parent + 1;
            if (!less(data_[ph], data_[h])) break;
            std::swap(data_[h], data_[ph]);
            node = parent;
        }
    }
    void sift_down_min(std::size_t node) {
        while (true) {
            std::size_t left = 2 * node + 1;
            if (left >= (data_.size() + 1) / 2) break;
            std::size_t child = left, right = left + 1;
            if (right < (data_.size() + 1) / 2 && less(data_[2 * right], data_[2 * left])) child = right;
            if (!less(data_[2 * child], data_[2 * node])) break;
            std::swap(data_[2 * child], data_[2 * node]);
            std::size_t h = high(child);
            if (less(data_[h], data_[2 * child])) std::swap(data_[h], data_[2 * child]);
            node = child;
        }
    }
    void sift_down_max(std::size_t node) {
        while (true) {
            std::size_t left = 2 * node + 1;
            if (left >= (data_.size() + 1) / 2) break;
            std::size_t child = left, right = left + 1;
            if (right < (data_.size() + 1) / 2 && less(data_[high(left)], data_[high(right)])) child = right;
            std::size_t h = high(node), ch = high(child);
            if (!less(data_[h], data_[ch])) break;
            std::swap(data_[h], data_[ch]);
            if (less(data_[high(child)], data_[2 * child])) std::swap(data_[high(child)], data_[2 * child]);
            node = child;
        }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    interval_heap() = default;
    explicit interval_heap(const Compare& compare) : compare_(compare) {}
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    const T& min() const { return data_[0]; }
    const T& max() const { return data_[data_.size() == 1 ? 0 : 1]; }
    const T& top_min() const { return min(); }
    const T& top_max() const { return max(); }
    const T& top() const { return max(); }
    void push(const T& value) {
        data_.push_back(value);
        std::size_t k = data_.size() - 1, node = k / 2;
        if ((k & 1) && less(data_[k], data_[k - 1])) std::swap(data_[k], data_[k - 1]);
        sift_up_min(node);
        sift_up_max(node);
    }
    void push(T&& value) {
        data_.push_back(std::move(value));
        std::size_t k = data_.size() - 1, node = k / 2;
        if ((k & 1) && less(data_[k], data_[k - 1])) std::swap(data_[k], data_[k - 1]);
        sift_up_min(node);
        sift_up_max(node);
    }
    template<class... Args> void emplace(Args&&... args) { push(T(std::forward<Args>(args)...)); }
    void pop_min() {
        if (data_.empty()) return;
        if (data_.size() == 1) { data_.pop_back(); return; }
        T value = std::move(data_.back());
        data_.pop_back();
        if (data_.empty()) return;
        data_[0] = std::move(value);
        sift_down_min(0);
    }
    void pop_max() {
        if (data_.empty()) return;
        if (data_.size() <= 2) { data_.pop_back(); return; }
        T value = std::move(data_.back());
        data_.pop_back();
        data_[1] = std::move(value);
        sift_down_max(0);
    }
    void pop() { pop_max(); }
    void clear() { data_.clear(); }
};

template<class T, class Compare>
class heap_impl<T, interval, Compare> : public interval_heap<T, Compare> {
    using base = interval_heap<T, Compare>;
public:
    using base::base;
};

template<class T, class Compare>
class smmh_heap {
    std::vector<T> data_;
    Compare compare_{};
    bool less(const T& a, const T& b) const { return compare_(a, b); }
    void sift_up(std::size_t i) {
        while (i >= 2) {
            std::size_t gp = (i + 2) / 4, left = 2 * gp - 2, right = 2 * gp - 1;
            if (less(data_[i], data_[left])) { std::swap(data_[i], data_[left]); i = left; }
            else if (less(data_[right], data_[i])) { std::swap(data_[i], data_[right]); i = right; }
            else break;
        }
    }
    void sift_down_min(std::size_t hole, const T& value) {
        while (true) {
            std::size_t child = 2 * hole + 2, sibling_child = 2 * hole + 4;
            if (child >= size() && sibling_child >= size()) break;
            if (child >= size()) child = sibling_child;
            else if (sibling_child < size() && less(data_[sibling_child], data_[child])) child = sibling_child;
            if (!less(data_[child], value)) break;
            data_[hole] = std::move(data_[child]);
            hole = child;
        }
        data_[hole] = value;
        if ((hole & 1) == 0 && hole + 1 < size() && less(data_[hole + 1], data_[hole])) {
            std::swap(data_[hole], data_[hole + 1]);
            sift_up(hole + 1);
        }
    }
    void sift_down_max(std::size_t hole, const T& value) {
        while (true) {
            std::size_t child = 2 * hole + 3, sibling_child = 2 * hole + 1;
            if (child >= size() && sibling_child >= size()) break;
            if (child >= size()) child = sibling_child;
            else if (sibling_child < size() && less(data_[child], data_[sibling_child])) child = sibling_child;
            if (!less(value, data_[child])) break;
            data_[hole] = std::move(data_[child]);
            hole = child;
        }
        data_[hole] = value;
        if ((hole & 1) == 1 && hole - 1 < size() && less(data_[hole], data_[hole - 1])) {
            std::swap(data_[hole], data_[hole - 1]);
            sift_up(hole - 1);
        }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    smmh_heap() = default;
    explicit smmh_heap(const Compare& compare) : compare_(compare) {}
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    const T& min() const { return data_[0]; }
    const T& max() const { return data_[size() == 1 ? 0 : 1]; }
    const T& top_min() const { return min(); }
    const T& top_max() const { return max(); }
    const T& top() const { return max(); }
    void push(const T& value) {
        data_.push_back(value);
        std::size_t i = size() - 1;
        if ((i & 1) && less(data_[i], data_[i - 1])) { std::swap(data_[i], data_[i - 1]); --i; }
        sift_up(i);
    }
    void push(T&& value) {
        data_.push_back(std::move(value));
        std::size_t i = size() - 1;
        if ((i & 1) && less(data_[i], data_[i - 1])) { std::swap(data_[i], data_[i - 1]); --i; }
        sift_up(i);
    }
    template<class... Args> void emplace(Args&&... args) { push(T(std::forward<Args>(args)...)); }
    void pop_min() {
        if (empty()) return;
        if (size() == 1) { data_.pop_back(); return; }
        T value = std::move(data_.back());
        data_.pop_back();
        sift_down_min(0, value);
    }
    void pop_max() {
        if (empty()) return;
        if (size() <= 2) { data_.pop_back(); return; }
        T value = std::move(data_.back());
        data_.pop_back();
        sift_down_max(1, value);
    }
    void pop() { pop_max(); }
    void clear() { data_.clear(); }
};

template<class T, class Compare>
class heap_impl<T, SMMH, Compare> : public smmh_heap<T, Compare> {
    using base = smmh_heap<T, Compare>;
public:
    using base::base;
};

} // namespace detail

template<class T, class Type = maximum, class Compare = std::less<T>>
class heap : public detail::heap_impl<T, Type, Compare> {
    using base = detail::heap_impl<T, Type, Compare>;
public:
    using base::base;
};

} // namespace zz_library

#endif // ZZ_LIBRARY_HEAP_HPP
