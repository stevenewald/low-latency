#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>

template <typename Key, typename Compare = std::less<Key>> class FastSet {
  struct Node {
    Key key;
    Node *left;
    Node *right;
    int height;
    Node(const Key &k) : key(k), left(nullptr), right(nullptr), height(1) {}
  };

  Node *root;
  Compare comp;

  static int height(Node *n) { return n ? n->height : 0; }

  static void update_height(Node *n) {
    n->height = 1 + std::max(height(n->left), height(n->right));
  }

  static int balance_factor(Node *n) {
    return height(n->left) - height(n->right);
  }

  // Right rotation
  static Node *rotate_right(Node *y) {
    Node *x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
  }

  // Left rotation
  static Node *rotate_left(Node *x) {
    Node *y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
  }

  // Rebalance subtree rooted at n
  static Node *rebalance(Node *n) {
    update_height(n);
    int bf = balance_factor(n);

    if (bf > 1) {
      // left-heavy
      if (balance_factor(n->left) < 0)
        n->left = rotate_left(n->left);
      return rotate_right(n);
    } else if (bf < -1) {
      // right-heavy
      if (balance_factor(n->right) > 0)
        n->right = rotate_right(n->right);
      return rotate_left(n);
    }
    return n;
  }

  // Insert key into subtree, return new root
  Node *insert(Node *n, const Key &key) {
    if (!n)
      return new Node(key);

    if (comp(key, n->key))
      n->left = insert(n->left, key);
    else if (comp(n->key, key))
      n->right = insert(n->right, key);
    // else: equal, do nothing

    return rebalance(n);
  }

  // Find minimum node in subtree
  static Node *find_min(Node *n) { return n->left ? find_min(n->left) : n; }

  // Erase key from subtree, return new root
  Node *erase(Node *n, const Key &key) {
    if (!n)
      return nullptr;

    if (comp(key, n->key))
      n->left = erase(n->left, key);
    else if (comp(n->key, key))
      n->right = erase(n->right, key);
    else {
      // found it
      if (!n->left || !n->right) {
        Node *child = n->left ? n->left : n->right;
        delete n;
        return child;
      } else {
        // two children: replace with in-order successor
        Node *succ = find_min(n->right);
        n->key = succ->key;
        n->right = erase(n->right, succ->key);
      }
    }
    return rebalance(n);
  }

  // Recursively delete all nodes
  static void destroy(Node *n) {
    if (!n)
      return;
    destroy(n->left);
    destroy(n->right);
    delete n;
  }

public:
  FastSet() : root(nullptr), comp(Compare()) {}
  ~FastSet() { destroy(root); }

  // Insert if not already present
  void insert(const Key &key) { root = insert(root, key); }

  // Erase by key (no-op if missing)
  void erase(const Key &key) { root = erase(root, key); }

  // Return smallest element
  std::optional<Key> get_front() const {
    if (!root)
      return std::nullopt;
    Node *cur = root;
    while (cur->left)
      cur = cur->left;
    return cur->key;
  }

  bool empty() const { return root == nullptr; }
};

