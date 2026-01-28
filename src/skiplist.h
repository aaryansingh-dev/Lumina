#ifndef LUMINA_SKIPLIST_H
#define LUMINA_SKIPLIST_H

#include <atomic>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cassert>

namespace lumina{

const int seed_multiplier = 1664525;
const int seed_add = 1013904223;


/**
 * @brief A thread-safe, probabilistic SkipList implementation.
 * 
 * SkipList maintains sorted elements and allows fast search, insertion, and iteration.
 * It uses a layered linked list with randomized heights to achieve average O(log n) complexity.
 * 
 * @tparam Key Type of keys stored in the SkipList.
 * @tparam Comparator Functor for key comparison. Should implement operator()(a, b) returning:
 *         -1 if a < b, 1 if a > b, 0 if a == b.
 */
template <typename Key, class Comparator>
class SkipList{

private:
    struct Node;

public:

   /**
     * @brief Construct a new SkipList with the given comparator.
     * @param cmp Comparator instance to order the keys.
     */
    explicit SkipList(Comparator cmp);

    /**
     * @brief Destructor. Cleans up all nodes.
     */
    ~SkipList();

    /**
     * @brief Insert a new key into the SkipList.
     * @param key Key to insert.
     * @note Duplicate keys are not allowed; assert will fail if key already exists.
     */
    void Insert(const Key& key);

    /**
     * @brief Check if a key exists in the SkipList.
     * @param key Key to search for.
     * @return true if the key exists, false otherwise.
     */
    bool Contains(const Key& key) const;    // this function will not change the current state of the skiplist.


    /**
     * @brief Iterator to traverse SkipList in order.
     */
    class Iterator{
        public:
            explicit Iterator(const SkipList* list) : list_(list), node_(nullptr) {}

            /**
             * @brief Check if iterator is pointing to a valid node.
             * @return true if valid, false if at end.
             */
            bool Valid() const { return node_ != nullptr; }

            /**
             * @brief Return the key at current iterator position.
             * @return const Key& Current key.
             * @note Call Valid() before key().
             */
            const Key& key() const {
                assert(Valid());
                return node_->key;
            }
            
            /**
             * @brief Move iterator to the next node.
             */
            void Next() {
                assert(Valid());
                node_ = node_->Next(0);
            }
            
            /**
             * @brief Move iterator to the previous node.
             */
            void Prev() {
                assert(Valid());
                node_ = list_->FindLessThan(node_->key);
                if (node_ == list_->head_) {
                    node_ = nullptr;
                }
            }
            
            /**
             * @brief Seek iterator to the first node >= target key.
             * @param target Key to seek.
             */
            void Seek(const Key& target) {
                node_ = list_->FindGreaterOrEqual(target, nullptr);
            }

            /**
             * @brief Seek iterator to the first element in the list.
             */
            void SeekToFirst() {
                node_ = list_->head_->Next(0);
            }

            /**
             * @brief Seek iterator to the last element in the list.
             */            void SeekToLast() {
                node_ = list_->FindLast();
                if (node_ == list_->head_) {
                    node_ = nullptr;
                }
            }  
        
        private:
            const SkipList* list_;
            Node* node_;
    };

private:

    enum {kMaxHeight = 12 };
    constexpr int kBranchingFactor = 4;

    Node* const head_;
    Comparator const compare_;
    std::atomic<int> max_height_;   // atomic to handle multi-threading and race conditions

    Node* NewNode(const Key& key, int height);
    int RandomHeight(); // Randomly generate a node height
    bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;    // Find first node >= key 
    Node* FindLessThan(const Key& key) const;   // Find last node < key
    Node* FindLast() const; //Find last node in the list

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

};


/**
 * @brief Node structure used internally by SkipList.
 */
template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
    Key const key;

    explicit Node(const Key& k) : key(k){}

    Node* Next(int n){
        assert(n >= 0);
        // we used load with an order memory_order_acquire to avoid race conditions and load all changes before store was called.
        return next_[n].load(std::memory_order_acquire);    
    }

    void SetNext(int n, Node* x){
        assert(n>=0);
        next_[n].store(x, std::memory_order_release);
    }

private:
    std::atomic<Node*> next_[1];
};

// NewNode()
template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
    size_t size = sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1);
    void* mem = malloc(size);
    return new (mem) Node(key);
}

// SkipList()
template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp)
    : head_(NewNode(Key(), kMaxHeight)),
      compare_(cmp),
      max_height_(1) {
    for (int i = 0; i < kMaxHeight; i++) {
        head_->SetNext(i, nullptr);
    }
}

// Destructor definition
template <typename Key, class Comparator>
SkipList<Key, Comparator>::~SkipList() {
    Node* node = head_;
    while (node != nullptr) {
        Node* next = node->Next(0);
        node->~Node();
        free(node);
        node = next;
    }
}


template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
    Node* cur = head_;
    int level = max_height_.load(std::memory_order_relaxed) - 1;
    while (true) {
        Node* next = cur->Next(level);
        if (next != nullptr && compare_(next->key, key) < 0) {
            cur = next;
        } else {
            if (level == 0) return cur;
            else level--;
        }
    }
}


template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
    Node* cur = head_;
    int level = max_height_.load(std::memory_order_relaxed) - 1;
    while (true) {
        Node* next = cur->Next(level);
        if (next != nullptr) {
            cur = next;
        } else {
            if (level == 0) return cur;
            else level--;
        }
    }
}


template <typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
    // Linear Congruential Generator (LCG) - Faster and thread-local
    // rand() is not used because it has a global lock which can slow down the process, and also introduce bias because
    // sharing of the seed.
    static thread_local uint32_t seed = static_cast<uint32_t>(time(nullptr));
    
    // Increase height with probability 1 in 4
    int height = 1;
    while (height < kMaxHeight) {
        // Simple LCG step
        seed = seed * seed_multiplier + seed_add;
        if ((seed % kBranchingFactor) == 0) {
            height++;
        } else {
            break;
        }
    }
    return height;
}


template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key){
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev); // the next element in the skiplist(at the very bottom layer)

    // we dont want duplicate keys, so we do this
    assert(x == nullptr || !Equal(key, x->key));

    int height = RandomHeight();
    int current_max = max_height_.load(std::memory_order_acquire);
    if (height > current_max){
        // max_height_ = 3, max_level = 2... because levels are 0 indexed. This is why we start with current_max level
        for(int i = current_max; i < height; i++){
            prev[i] = head_;
        }
        max_height_.store(height, std::memory_order_release);
    }

    Node* newNode = NewNode(key, height);
    // Now we will set the prevs with level below or equal to the height
    for (int level = 0; level < height; level++){
        newNode->SetNext(level, prev[level]->Next(level));
        prev[level]->SetNext(level, newNode);
    }
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key, Node** prev) const {

    Node* cur = head_;
    int level = max_height_.load(std::memory_order_acquire)-1;
    while (true){
        Node* next = cur->Next(level);
        if (next != nullptr && compare_(next->key, key) < 0){
            cur = next;
        }else{
            if (prev != nullptr) prev[level] = cur;
            if (level == 0) return next;
            else level--;
        }
    }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) const {
    Node* x = FindGreaterOrEqual(key, nullptr);
    return (x != nullptr && Equal(key, x->key));
}

}
#endif
