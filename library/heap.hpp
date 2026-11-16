#ifndef ZZ_LIBRARY_HEAP_HPP
#define ZZ_LIBRARY_HEAP_HPP

namespace zz_library {

struct maximum {};
struct minimum {};
struct leftist_tree {};
struct double_ended {};
struct f_heap {};

namespace detail {

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
    void clear() { storage_type empty; data_.swap(empty); }
};

template<class T, class Compare>
class heap_impl<T, maximum, Compare> : public binary_heap<T, Compare, false> {};

template<class T, class Compare>
class heap_impl<T, minimum, Compare> : public binary_heap<T, Compare, true> {};

template<class T, class Compare>
class heap_impl<T, leftist_tree, Compare> {
    struct node {
        T value;
        node* left = nullptr;
        node* right = nullptr;
        std::size_t rank = 1;
        template<class... Args> explicit node(Args&&... args) : value(std::forward<Args>(args)...) {}
    };
    node* root_ = nullptr;
    std::size_t size_ = 0;
    Compare compare_{};

    node* meld(node* a, node* b) {
        if (!a) return b;
        if (!b) return a;
        if (compare_(a->value, b->value)) std::swap(a, b);
        a->right = meld(a->right, b);
        if (!a->left || a->left->rank < a->right->rank) std::swap(a->left, a->right);
        a->rank = a->right ? a->right->rank + 1 : 1;
        return a;
    }
    static void destroy(node* p) {
        if (!p) return;
        std::vector<node*> stack{p};
        while (!stack.empty()) {
            node* q = stack.back(); stack.pop_back();
            if (q->left) stack.push_back(q->left);
            if (q->right) stack.push_back(q->right);
            delete q;
        }
    }
public:
    using value_type = T;
    using size_type = std::size_t;
    heap_impl() = default;
    explicit heap_impl(const Compare& compare) : compare_(compare) {}
    heap_impl(const heap_impl&) = delete;
    heap_impl& operator=(const heap_impl&) = delete;
    heap_impl(heap_impl&& other) noexcept : root_(std::exchange(other.root_, nullptr)), size_(std::exchange(other.size_, 0)), compare_(std::move(other.compare_)) {}
    heap_impl& operator=(heap_impl&& other) noexcept {
        if (this != &other) { clear(); root_ = std::exchange(other.root_, nullptr); size_ = std::exchange(other.size_, 0); compare_ = std::move(other.compare_); }
        return *this;
    }
    ~heap_impl() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { return root_->value; }
    void push(const T& value) { root_ = meld(root_, new node(value)); ++size_; }
    void push(T&& value) { root_ = meld(root_, new node(std::move(value))); ++size_; }
    template<class... Args> void emplace(Args&&... args) { root_ = meld(root_, new node(std::forward<Args>(args)...)); ++size_; }
    void pop() { node* old = root_; root_ = meld(old->left, old->right); delete old; --size_; }
    void clear() { destroy(root_); root_ = nullptr; size_ = 0; }
    void merge(heap_impl& other) {
        if (this == &other || other.empty()) return;
        root_ = meld(root_, other.root_); size_ += other.size_;
        other.root_ = nullptr; other.size_ = 0;
    }
};

template<class T, class Compare>
class heap_impl<T, double_ended, Compare> {
    std::multiset<T, Compare> data_;
public:
    using value_type = T;
    using size_type = std::size_t;
    heap_impl() = default;
    explicit heap_impl(const Compare& compare) : data_(compare) {}
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    const T& min() const { return *data_.begin(); }
    const T& max() const { return *data_.rbegin(); }
    const T& top_min() const { return min(); }
    const T& top_max() const { return max(); }
    const T& top() const { return max(); }
    void push(const T& value) { data_.insert(value); }
    void push(T&& value) { data_.insert(std::move(value)); }
    template<class... Args> void emplace(Args&&... args) { data_.emplace(std::forward<Args>(args)...); }
    void pop_min() { data_.erase(data_.begin()); }
    void pop_max() { auto it = data_.end(); data_.erase(--it); }
    void pop() { pop_max(); }
    void clear() { data_.clear(); }
};

template<class T, class Compare>
class heap_impl<T, f_heap, Compare> {
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
public:
    using value_type = T;
    using size_type = std::size_t;
    using handle_type = void*;
private:
    std::list<node*> roots_;
    node* minimum_ = nullptr;
    std::size_t size_ = 0;
    Compare compare_{};
    bool before(const T& a, const T& b) const { return compare_(a, b); }
    void add_root(node* p) {
        p->parent = nullptr; p->mark = false;
        roots_.push_back(p); p->root_position = std::prev(roots_.end());
    }
    void link(node* child, node* parent) {
        child->parent = parent; child->mark = false;
        parent->children.push_back(child); child->sibling = std::prev(parent->children.end()); ++parent->degree;
    }
    void consolidate() {
        std::vector<node*> roots(roots_.begin(), roots_.end());
        roots_.clear();
        std::vector<node*> by_degree;
        for (node* x : roots) {
            std::size_t d = x->degree;
            while (true) {
                if (d >= by_degree.size()) by_degree.resize(d + 1, nullptr);
                if (!by_degree[d]) { by_degree[d] = x; break; }
                node* y = by_degree[d]; by_degree[d] = nullptr;
                if (before(y->value, x->value)) std::swap(x, y);
                link(y, x); d = x->degree;
            }
        }
        minimum_ = nullptr;
        for (node* p : by_degree) if (p) {
            roots_.push_back(p);
            p->root_position = std::prev(roots_.end());
            if (!minimum_ || before(p->value, minimum_->value)) minimum_ = p;
        }
    }
    void cut(node* x, node* p) {
        p->children.erase(x->sibling); --p->degree;
        add_root(x);
    }
    void cascading_cut(node* p) {
        node* parent = p->parent;
        if (!parent) return;
        if (!p->mark) p->mark = true;
        else { cut(p, parent); cascading_cut(parent); }
    }
    static void destroy_tree(node* p) {
        if (!p) return;
        for (node* child : p->children) destroy_tree(child);
        delete p;
    }
public:
    heap_impl() = default;
    explicit heap_impl(const Compare& compare) : compare_(compare) {}
    heap_impl(const heap_impl&) = delete;
    heap_impl& operator=(const heap_impl&) = delete;
    heap_impl(heap_impl&& other) noexcept : roots_(std::move(other.roots_)), minimum_(std::exchange(other.minimum_, nullptr)), size_(std::exchange(other.size_, 0)), compare_(std::move(other.compare_)) { other.roots_.clear(); }
    heap_impl& operator=(heap_impl&& other) noexcept {
        if (this != &other) { clear(); roots_ = std::move(other.roots_); minimum_ = std::exchange(other.minimum_, nullptr); size_ = std::exchange(other.size_, 0); compare_ = std::move(other.compare_); other.roots_.clear(); }
        return *this;
    }
    ~heap_impl() { clear(); }
    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }
    const T& top() const { return minimum_->value; }
    handle_type push(const T& value) { return emplace(value); }
    handle_type push(T&& value) { return emplace(std::move(value)); }
    template<class... Args> handle_type emplace(Args&&... args) {
        node* p = new node(std::forward<Args>(args)...); add_root(p);
        if (!minimum_ || before(p->value, minimum_->value)) minimum_ = p;
        ++size_; return static_cast<handle_type>(p);
    }
    void pop() {
        node* old = minimum_;
        for (node* child : old->children) add_root(child);
        roots_.erase(old->root_position); delete old; --size_;
        if (roots_.empty()) minimum_ = nullptr; else consolidate();
    }
    void decrease_key(handle_type h, const T& value) {
        node* target = static_cast<node*>(h);
        if (before(target->value, value)) throw std::invalid_argument("zz_library::heap<f_heap>::decrease_key: new value is greater");
        target->value = value;
        node* p = target->parent;
        if (p && before(target->value, p->value)) { cut(target, p); cascading_cut(p); }
        if (!minimum_ || before(target->value, minimum_->value)) minimum_ = target;
    }
    void merge(heap_impl& other) {
        if (this == &other || other.empty()) return;
        roots_.splice(roots_.end(), other.roots_);
        if (!minimum_ || (other.minimum_ && before(other.minimum_->value, minimum_->value))) minimum_ = other.minimum_;
        size_ += other.size_; other.minimum_ = nullptr; other.size_ = 0;
    }
    void clear() {
        for (node* p : roots_) destroy_tree(p);
        roots_.clear(); minimum_ = nullptr; size_ = 0;
    }
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
