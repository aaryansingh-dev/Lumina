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
            explicit Iterator(const Skiplist* list);    // cannot modify skiplist, using this pointer
            bool Valid() const; 
            const Key& key() const;
            void Next();
            void Prev();
            void Seek(const Key& target);
            void SeekToFirst();
            void SeekToLast();
        
        private:
            const SkipList* list;
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

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(const Key& key, int height) {
    size_t size = sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1);
    void* mem = malloc(size);
    return new (mem) Node(key);
}


template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp)
    : head_(NewNode(Key(), kMaxHeight)),
      compare_(cmp),
      max_height_(1) {
    for (int i = 0; i < kMaxHeight; i++) {
        head_->SetNext(i, nullptr);
    }


}

}
#endif
