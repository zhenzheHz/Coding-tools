#ifndef ZZ_LIBRARY_PERSISTENT_QUEUE_HPP
#define ZZ_LIBRARY_PERSISTENT_QUEUE_HPP

namespace zz_library {

template <typename T>
class persistent_queue {
private:
    struct Node {
        std::size_t left = 0, right = 0;
        T value{};
    };

    struct Version {
        std::size_t root = 0;
        std::size_t front = 0, back = 0;
    };

    std::size_t capacity = 0;
    std::vector<Node> nodes;
    std::vector<Version> versions;

    std::size_t update(std::size_t old, std::size_t l, std::size_t r, std::size_t pos, const T& value) {
        std::size_t cur = nodes.size();
        nodes.push_back(nodes[old]);

        if (l == r) {
            nodes[cur].value = value;
            return cur;
        }

        std::size_t mid = (l + r) >> 1;

        if (pos <= mid) nodes[cur].left = update(nodes[cur].left, l, mid, pos, value);
        else nodes[cur].right = update(nodes[cur].right, mid + 1, r, pos, value);

        return cur;
    }

    const T& query(std::size_t cur, std::size_t l, std::size_t r, std::size_t pos) const {
        if (l == r) return nodes[cur].value;

        std::size_t mid = (l + r) >> 1;

        if (pos <= mid) return query(nodes[cur].left, l, mid, pos);
        return query(nodes[cur].right, mid + 1, r, pos);
    }

public:
    persistent_queue() {
        init(1);
    }

    explicit persistent_queue(std::size_t max_operations) {
        init(max_operations);
    }

    void init(std::size_t max_operations) {
        capacity = std::max<std::size_t>(1, max_operations);
        nodes.clear();
        versions.clear();
        nodes.push_back({});
        versions.push_back({});
    }

    std::size_t push(std::size_t version, const T& value) {
        Version cur = versions[version];
        cur.root = update(cur.root, 0, capacity - 1, cur.back++, value);
        versions.push_back(cur);
        return versions.size() - 1;
    }

    std::size_t pop(std::size_t version) {
        Version cur = versions[version];
        cur.front++;
        versions.push_back(cur);
        return versions.size() - 1;
    }

    const T& front(std::size_t version) const {
        const Version& cur = versions[version];
        return query(cur.root, 0, capacity - 1, cur.front);
    }

    const T& back(std::size_t version) const {
        const Version& cur = versions[version];
        return query(cur.root, 0, capacity - 1, cur.back - 1);
    }

    std::size_t size(std::size_t version) const {
        const Version& cur = versions[version];
        return cur.back - cur.front;
    }

    bool empty(std::size_t version) const {
        return size(version) == 0;
    }

    std::size_t version_count() const {
        return versions.size();
    }
};

}

#endif
