#ifndef LUMINA_SKIPLIST_H
#define LUMINA_SKIPLIST_H

#include <atomic>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cassert>

namespace lumina{
    
template <typename Key, class Comparator>
class SkipList{

private:
    struct Node;

public:

    explicit SkipList(Comparator cmp);

    // Destructor 
    ~SkipList();

    void Insert(const Key& key);
    bool Contains(const Key& key) const;    // this function cannot modify value key is pointing to, and cannot modify Skiplist.

    class Iterator{
        public:
            explicit Iterator(const SkipList* list) : list_(list), node_(nullptr) {}

            bool Valid() const { return node_ != nullptr; }

            const Key& key() const {
                assert(Valid());
                return node_->key;
            }

            void Next() {
                assert(Valid());
                node_ = node_->Next(0);
            }

            void Prev() {
                assert(Valid());
                node_ = list_->FindLessThan(node_->key);
                if (node_ == list_->head_) {
                    node_ = nullptr;
                }
            }

            void Seek(const Key& target) {
                node_ = list_->FindGreaterOrEqual(target, nullptr);
            }

            void SeekToFirst() {
                node_ = list_->head_->Next(0);
            }

            void SeekToLast() {
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

    Node* const head_;
    Comparator const compare_;
    std::atomic<int> max_height_;   // atomic to handle multi-threading and race conditions

    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
    Node* FindLessThan(const Key& key) const;
    Node* FindLast() const;

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

};

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
    Node* x = head_;
    int level = max_height_.load(std::memory_order_relaxed) - 1;
    while (true) {
        Node* next = x->Next(level);
        if (next != nullptr && compare_(next->key, key) < 0) {
            x = next;
        } else {
            if (level == 0) return x;
            else level--;
        }
    }
}


template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
    Node* x = head_;
    int level = max_height_.load(std::memory_order_relaxed) - 1;
    while (true) {
        Node* next = x->Next(level);
        if (next != nullptr) {
            x = next;
        } else {
            if (level == 0) return x;
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
        seed = seed * 1664525 + 1013904223;
        if ((seed % 4) == 0) {
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
