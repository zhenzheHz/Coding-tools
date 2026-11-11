#ifndef ZZ_LIBRARY_ORDERED_SET_HPP
#define ZZ_LIBRARY_ORDERED_SET_HPP

namespace zz_library {

template<class T>
struct ordered_set {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using const_reference = const T&;
    using const_pointer = const T*;

private:
    static constexpr size_type none = static_cast<size_type>(-1);
    static constexpr size_type block_shift = 8;
    static constexpr size_type block_size = size_type(1) << block_shift;
    static constexpr size_type block_mask = block_size - 1;

    struct Node {
        size_type left = none;
        size_type right = none;
        size_type parent = none;
        size_type subtree_size = 1;
        std::uint64_t priority = 0;
        bool alive = false;
        alignas(T) unsigned char storage[sizeof(T)];

        T* value() {
            return std::launder(reinterpret_cast<T*>(storage));
        }

        const T* value() const {
            return std::launder(reinterpret_cast<const T*>(storage));
        }
    };

    std::vector<std::unique_ptr<Node[]>> blocks_;
    size_type allocated_ = 0;
    size_type free_head_ = none;
    size_type root_ = none;
    size_type minimum_ = none;
    size_type maximum_ = none;
    size_type size_ = 0;
    std::uint64_t rng_state_ = 0;

    Node& node(size_type id) {
        return blocks_[id >> block_shift][id & block_mask];
    }

    const Node& node(size_type id) const {
        return blocks_[id >> block_shift][id & block_mask];
    }

    T& value(size_type id) {
        return *node(id).value();
    }

    const T& value(size_type id) const {
        return *node(id).value();
    }

    static bool less_than(const T& a, const T& b) {
        return std::less<T>{}(a, b);
    }

    static bool equivalent(const T& a, const T& b) {
        return !less_than(a, b) && !less_than(b, a);
    }

    void seed_rng() {
        std::uint64_t seed = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        seed ^= static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(this));
        try {
            std::random_device rd;
            seed ^= static_cast<std::uint64_t>(rd()) << 32;
            seed ^= static_cast<std::uint64_t>(rd());
        } catch (...) {
        }
        rng_state_ = seed;
    }

    std::uint64_t next_priority() {
        std::uint64_t z = (rng_state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    bool higher(size_type a, size_type b) const {
        if (node(a).priority != node(b).priority) return node(a).priority > node(b).priority;
        return a < b;
    }

    void ensure_blocks(size_type count) {
        size_type required = (count >> block_shift) + ((count & block_mask) != 0);
        while (blocks_.size() < required) blocks_.emplace_back(std::make_unique<Node[]>(block_size));
    }

    template<class... Args>
    size_type create_node(Args&&... args) {
        size_type id;
        size_type next_free = none;
        bool reused = free_head_ != none;
        if (reused) {
            id = free_head_;
            next_free = node(id).parent;
        } else {
            id = allocated_;
            ensure_blocks(allocated_ + 1);
        }

        ::new (static_cast<void*>(node(id).storage)) T(std::forward<Args>(args)...);

        if (reused) free_head_ = next_free;
        else ++allocated_;

        Node& n = node(id);
        n.left = n.right = n.parent = none;
        n.subtree_size = 1;
        n.priority = next_priority();
        n.alive = true;
        return id;
    }

    void release_node(size_type id) {
        Node& n = node(id);
        n.value()->~T();
        n.alive = false;
        n.parent = free_head_;
        free_head_ = id;
    }

    size_type subtree_size(size_type id) const {
        return id == none ? 0 : node(id).subtree_size;
    }

    void pull(size_type id) {
        Node& n = node(id);
        n.subtree_size = 1 + subtree_size(n.left) + subtree_size(n.right);
    }

    void rotate_left(size_type x) {
        size_type y = node(x).right;
        size_type b = node(y).left;
        size_type p = node(x).parent;

        if (p == none) root_ = y;
        else if (node(p).left == x) node(p).left = y;
        else node(p).right = y;

        node(y).parent = p;
        node(y).left = x;
        node(x).parent = y;
        node(x).right = b;
        if (b != none) node(b).parent = x;

        pull(x);
        pull(y);
    }

    void rotate_right(size_type x) {
        size_type y = node(x).left;
        size_type b = node(y).right;
        size_type p = node(x).parent;

        if (p == none) root_ = y;
        else if (node(p).left == x) node(p).left = y;
        else node(p).right = y;

        node(y).parent = p;
        node(y).right = x;
        node(x).parent = y;
        node(x).left = b;
        if (b != none) node(b).parent = x;

        pull(x);
        pull(y);
    }

    size_type successor_index(size_type id) const {
        if (id == none) return none;
        if (node(id).right != none) {
            id = node(id).right;
            while (node(id).left != none) id = node(id).left;
            return id;
        }
        size_type p = node(id).parent;
        while (p != none && node(p).right == id) {
            id = p;
            p = node(p).parent;
        }
        return p;
    }

    size_type predecessor_index(size_type id) const {
        if (id == none) return maximum_;
        if (node(id).left != none) {
            id = node(id).left;
            while (node(id).right != none) id = node(id).right;
            return id;
        }
        size_type p = node(id).parent;
        while (p != none && node(p).left == id) {
            id = p;
            p = node(p).parent;
        }
        return p;
    }

    size_type find_index(const T& x) const {
        size_type cur = root_;
        while (cur != none) {
            if (less_than(x, value(cur))) cur = node(cur).left;
            else if (less_than(value(cur), x)) cur = node(cur).right;
            else return cur;
        }
        return none;
    }

    void erase_index(size_type id) {
        size_type next_min = minimum_ == id ? successor_index(id) : minimum_;
        size_type next_max = maximum_ == id ? predecessor_index(id) : maximum_;

        while (node(id).left != none || node(id).right != none) {
            if (node(id).left == none) rotate_left(id);
            else if (node(id).right == none) rotate_right(id);
            else if (higher(node(id).right, node(id).left)) rotate_left(id);
            else rotate_right(id);
        }

        size_type p = node(id).parent;
        if (p == none) root_ = none;
        else if (node(p).left == id) node(p).left = none;
        else node(p).right = none;

        for (size_type cur = p; cur != none; cur = node(cur).parent) pull(cur);

        release_node(id);
        --size_;
        minimum_ = size_ ? next_min : none;
        maximum_ = size_ ? next_max : none;
    }

public:
    class const_iterator {
        friend struct ordered_set;
        const ordered_set* owner_ = nullptr;
        size_type id_ = none;

        const_iterator(const ordered_set* owner, size_type id) : owner_(owner), id_(id) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;

        reference operator*() const { return owner_->value(id_); }
        pointer operator->() const { return owner_->node(id_).value(); }

        const_iterator& operator++() {
            if (owner_ && id_ != none) id_ = owner_->successor_index(id_);
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator result = *this;
            ++*this;
            return result;
        }

        const_iterator& operator--() {
            if (owner_) id_ = owner_->predecessor_index(id_);
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator result = *this;
            --*this;
            return result;
        }

        friend bool operator==(const_iterator a, const_iterator b) {
            return a.owner_ == b.owner_ && a.id_ == b.id_;
        }

        friend bool operator!=(const_iterator a, const_iterator b) {
            return !(a == b);
        }
    };

    using iterator = const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    ordered_set() {
        seed_rng();
    }

    explicit ordered_set(const std::vector<T>& v) : ordered_set() {
        build(v);
    }

    ordered_set(const ordered_set& other) : ordered_set() {
        std::vector<T> v;
        v.reserve(other.size_);
        for (const T& x : other) v.push_back(x);
        build(v);
    }

    ordered_set(ordered_set&& other) noexcept
        : blocks_(std::move(other.blocks_)), allocated_(other.allocated_), free_head_(other.free_head_),
          root_(other.root_), minimum_(other.minimum_), maximum_(other.maximum_), size_(other.size_),
          rng_state_(other.rng_state_) {
        other.allocated_ = other.size_ = 0;
        other.free_head_ = other.root_ = other.minimum_ = other.maximum_ = none;
        other.seed_rng();
    }

    ordered_set& operator=(const ordered_set& other) {
        if (this == &other) return *this;
        std::vector<T> v;
        v.reserve(other.size_);
        for (const T& x : other) v.push_back(x);
        build(v);
        return *this;
    }

    ordered_set& operator=(ordered_set&& other) noexcept {
        if (this == &other) return *this;
        clear();
        blocks_ = std::move(other.blocks_);
        allocated_ = other.allocated_;
        free_head_ = other.free_head_;
        root_ = other.root_;
        minimum_ = other.minimum_;
        maximum_ = other.maximum_;
        size_ = other.size_;
        rng_state_ = other.rng_state_;
        other.allocated_ = other.size_ = 0;
        other.free_head_ = other.root_ = other.minimum_ = other.maximum_ = none;
        other.seed_rng();
        return *this;
    }

    ~ordered_set() {
        clear();
    }

    void reserve(size_type n) {
        ensure_blocks(n);
    }

    size_type capacity() const {
        return blocks_.size() * block_size;
    }

    void build(const std::vector<T>& v) {
        std::vector<T> a(v);
        std::sort(a.begin(), a.end(), less_than);
        a.erase(std::unique(a.begin(), a.end(), equivalent), a.end());

        std::vector<size_type> ids;
        std::vector<size_type> stack;
        ids.reserve(a.size());
        stack.reserve(a.size());

        clear();
        try {
            for (const T& x : a) ids.push_back(create_node(x));
        } catch (...) {
            for (size_type id : ids) release_node(id);
            throw;
        }

        if (ids.empty()) return;

        for (size_type id : ids) {
            size_type last = none;
            while (!stack.empty() && higher(id, stack.back())) {
                last = stack.back();
                stack.pop_back();
            }
            if (!stack.empty()) {
                node(stack.back()).right = id;
                node(id).parent = stack.back();
            }
            if (last != none) {
                node(id).left = last;
                node(last).parent = id;
            }
            stack.push_back(id);
        }

        root_ = stack.front();
        minimum_ = ids.front();
        maximum_ = ids.back();
        size_ = ids.size();

        ids.clear();
        stack.clear();
        stack.push_back(root_);
        while (!stack.empty()) {
            size_type id = stack.back();
            stack.pop_back();
            ids.push_back(id);
            if (node(id).left != none) stack.push_back(node(id).left);
            if (node(id).right != none) stack.push_back(node(id).right);
        }
        for (auto it = ids.rbegin(); it != ids.rend(); ++it) pull(*it);
    }

    std::pair<iterator, bool> insert(const T& x) {
        size_type p = none;
        size_type cur = root_;
        bool go_left = false;
        while (cur != none) {
            p = cur;
            if (less_than(x, value(cur))) {
                go_left = true;
                cur = node(cur).left;
            } else if (less_than(value(cur), x)) {
                go_left = false;
                cur = node(cur).right;
            } else {
                return {iterator(this, cur), false};
            }
        }

        size_type id = create_node(x);
        node(id).parent = p;
        if (p == none) root_ = id;
        else if (go_left) node(p).left = id;
        else node(p).right = id;

        for (size_type q = p; q != none; q = node(q).parent) ++node(q).subtree_size;

        while (node(id).parent != none && higher(id, node(id).parent)) {
            size_type q = node(id).parent;
            if (node(q).left == id) rotate_right(q);
            else rotate_left(q);
        }

        if (size_ == 0 || less_than(x, value(minimum_))) minimum_ = id;
        if (size_ == 0 || less_than(value(maximum_), x)) maximum_ = id;
        ++size_;
        return {iterator(this, id), true};
    }

    std::pair<iterator, bool> insert(T&& x) {
        size_type p = none;
        size_type cur = root_;
        bool go_left = false;
        while (cur != none) {
            p = cur;
            if (less_than(x, value(cur))) {
                go_left = true;
                cur = node(cur).left;
            } else if (less_than(value(cur), x)) {
                go_left = false;
                cur = node(cur).right;
            } else {
                return {iterator(this, cur), false};
            }
        }

        bool was_empty = size_ == 0;
        size_type id = create_node(std::move(x));
        node(id).parent = p;
        if (p == none) root_ = id;
        else if (go_left) node(p).left = id;
        else node(p).right = id;

        for (size_type q = p; q != none; q = node(q).parent) ++node(q).subtree_size;

        while (node(id).parent != none && higher(id, node(id).parent)) {
            size_type q = node(id).parent;
            if (node(q).left == id) rotate_right(q);
            else rotate_left(q);
        }

        if (was_empty || less_than(value(id), value(minimum_))) minimum_ = id;
        if (was_empty || less_than(value(maximum_), value(id))) maximum_ = id;
        ++size_;
        return {iterator(this, id), true};
    }

    size_type erase(const T& x) {
        size_type id = find_index(x);
        if (id == none) return 0;
        erase_index(id);
        return 1;
    }

    iterator erase(iterator it) {
        if (it.owner_ != this || it.id_ == none || !node(it.id_).alive) return end();
        size_type next = successor_index(it.id_);
        erase_index(it.id_);
        return iterator(this, next);
    }

    void clear() {
        if (root_ == none) return;
        size_type cur = root_;
        size_type prev = none;
        while (cur != none) {
            size_type next;
            if (prev == node(cur).parent) {
                if (node(cur).left != none) next = node(cur).left;
                else if (node(cur).right != none) next = node(cur).right;
                else {
                    next = node(cur).parent;
                    release_node(cur);
                }
            } else if (prev == node(cur).left) {
                if (node(cur).right != none) next = node(cur).right;
                else {
                    next = node(cur).parent;
                    release_node(cur);
                }
            } else {
                next = node(cur).parent;
                release_node(cur);
            }
            prev = cur;
            cur = next;
        }
        root_ = minimum_ = maximum_ = none;
        size_ = 0;
    }

    iterator find(const T& x) {
        return iterator(this, find_index(x));
    }

    const_iterator find(const T& x) const {
        return const_iterator(this, find_index(x));
    }

    bool contains(const T& x) const {
        return find_index(x) != none;
    }

    size_type count(const T& x) const {
        return contains(x) ? 1 : 0;
    }

    iterator lower_bound(const T& x) {
        size_type cur = root_, result = none;
        while (cur != none) {
            if (!less_than(value(cur), x)) {
                result = cur;
                cur = node(cur).left;
            } else cur = node(cur).right;
        }
        return iterator(this, result);
    }

    const_iterator lower_bound(const T& x) const {
        size_type cur = root_, result = none;
        while (cur != none) {
            if (!less_than(value(cur), x)) {
                result = cur;
                cur = node(cur).left;
            } else cur = node(cur).right;
        }
        return const_iterator(this, result);
    }

    iterator upper_bound(const T& x) {
        size_type cur = root_, result = none;
        while (cur != none) {
            if (less_than(x, value(cur))) {
                result = cur;
                cur = node(cur).left;
            } else cur = node(cur).right;
        }
        return iterator(this, result);
    }

    const_iterator upper_bound(const T& x) const {
        size_type cur = root_, result = none;
        while (cur != none) {
            if (less_than(x, value(cur))) {
                result = cur;
                cur = node(cur).left;
            } else cur = node(cur).right;
        }
        return const_iterator(this, result);
    }

    iterator find_by_order(size_type k) {
        if (k >= size_) return end();
        size_type cur = root_;
        while (cur != none) {
            size_type left_size = subtree_size(node(cur).left);
            if (k < left_size) cur = node(cur).left;
            else if (k == left_size) return iterator(this, cur);
            else {
                k -= left_size + 1;
                cur = node(cur).right;
            }
        }
        return end();
    }

    const_iterator find_by_order(size_type k) const {
        if (k >= size_) return end();
        size_type cur = root_;
        while (cur != none) {
            size_type left_size = subtree_size(node(cur).left);
            if (k < left_size) cur = node(cur).left;
            else if (k == left_size) return const_iterator(this, cur);
            else {
                k -= left_size + 1;
                cur = node(cur).right;
            }
        }
        return end();
    }

    size_type order_of_key(const T& x) const {
        size_type result = 0;
        size_type cur = root_;
        while (cur != none) {
            if (less_than(value(cur), x)) {
                result += subtree_size(node(cur).left) + 1;
                cur = node(cur).right;
            } else cur = node(cur).left;
        }
        return result;
    }

    iterator begin() { return iterator(this, minimum_); }
    const_iterator begin() const { return const_iterator(this, minimum_); }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return iterator(this, none); }
    const_iterator end() const { return const_iterator(this, none); }
    const_iterator cend() const { return end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return rbegin(); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return rend(); }

    size_type size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

}

#endif
