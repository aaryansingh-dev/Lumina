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

}

#endif
