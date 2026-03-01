#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include "exceptions.hpp"
#include "algorithm.hpp"

#include <cstddef>

namespace sjtu {
/**
 * a data container like std::list
 * allocate random memory addresses for data and they are doubly-linked in a list.
 */
template<typename T>
class list {
protected:
    class node {
    public:
        node *prev;
        node *next;

        node(node *p = NULL, node *n = NULL) : prev(p), next(n) {}
        virtual ~node() {}
        virtual T &value() = 0;
        virtual const T &value() const = 0;
    };

    class data_node : public node {
    public:
        T data;

        data_node(const T &v, node *p = NULL, node *n = NULL) : node(p, n), data(v) {}
        ~data_node() override {}
        T &value() override { return data; }
        const T &value() const override { return data; }
    };

    class sentinel_node : public node {
    public:
        sentinel_node(node *p = NULL, node *n = NULL) : node(p, n) {}
        ~sentinel_node() override {}
        T &value() override { throw invalid_iterator(); }
        const T &value() const override { throw invalid_iterator(); }
    };

protected:
    node *head;
    node *tail;
    size_t len;

    void init_empty() {
        head = new sentinel_node();
        tail = new sentinel_node();
        head->next = tail;
        tail->prev = head;
        len = 0;
    }

    void copy_from(const list &other) {
        for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
            push_back(*it);
        }
    }

    static node *merge_two_chains(node *a, node *b) {
        if (a == NULL) return b;
        if (b == NULL) return a;
        node *new_head = NULL;
        node *new_tail = NULL;

        while (a != NULL && b != NULL) {
            node *pick;
            if (b->value() < a->value()) {
                pick = b;
                b = b->next;
            } else {
                pick = a;
                a = a->next;
            }
            pick->prev = new_tail;
            if (new_tail != NULL) new_tail->next = pick;
            else new_head = pick;
            new_tail = pick;
        }

        node *rest = (a != NULL ? a : b);
        while (rest != NULL) {
            node *pick = rest;
            rest = rest->next;
            pick->prev = new_tail;
            if (new_tail != NULL) new_tail->next = pick;
            else new_head = pick;
            new_tail = pick;
        }

        if (new_tail != NULL) new_tail->next = NULL;
        return new_head;
    }

    static node *merge_sort_chain(node *first) {
        if (first == NULL || first->next == NULL) return first;

        node *slow = first;
        node *fast = first->next;
        while (fast != NULL && fast->next != NULL) {
            slow = slow->next;
            fast = fast->next->next;
        }

        node *second = slow->next;
        slow->next = NULL;
        if (second != NULL) second->prev = NULL;

        node *left = merge_sort_chain(first);
        node *right = merge_sort_chain(second);
        return merge_two_chains(left, right);
    }

    /**
     * insert node cur before node pos
     * return the inserted node cur
     */
    node *insert(node *pos, node *cur) {
        node *l = pos->prev;
        cur->prev = l;
        cur->next = pos;
        l->next = cur;
        pos->prev = cur;
        ++len;
        return cur;
    }
    /**
     * remove node pos from list (no need to delete the node)
     * return the removed node pos
     */
    node *erase(node *pos) {
        node *l = pos->prev;
        node *r = pos->next;
        l->next = r;
        r->prev = l;
        pos->prev = NULL;
        pos->next = NULL;
        --len;
        return pos;
    }

public:
    class const_iterator;
    class iterator {
    private:
        node *ptr;
        const list *owner;

        friend class list;
        friend class const_iterator;

    public:
        iterator(node *p = NULL, const list *o = NULL) : ptr(p), owner(o) {}
        iterator(const iterator &other) : ptr(other.ptr), owner(other.owner) {}
        iterator &operator=(const iterator &other) {
            if (this == &other) return *this;
            ptr = other.ptr;
            owner = other.owner;
            return *this;
        }
        /**
         * iter++
         */
        iterator operator++(int) {
            iterator ret(*this);
            ++(*this);
            return ret;
        }
        /**
         * ++iter
         */
        iterator & operator++() {
            if (owner == NULL || ptr == NULL || ptr == owner->tail) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }
        /**
         * iter--
         */
        iterator operator--(int) {
            iterator ret(*this);
            --(*this);
            return ret;
        }
        /**
         * --iter
         */
        iterator & operator--() {
            if (owner == NULL || ptr == NULL || ptr->prev == owner->head) throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }
        /**
         * TODO *it
         * remember to throw if iterator is invalid
         */
        T & operator *() const {
            if (owner == NULL || ptr == NULL || ptr == owner->tail || ptr == owner->head) throw invalid_iterator();
            return ptr->value();
        }
        /**
         * TODO it->field
         * remember to throw if iterator is invalid
         */
        T * operator ->() const {
            return &(**this);
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr && owner == rhs.owner;
        }
        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr && owner == rhs.owner;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     * should be able to construct from an iterator.
     */
    class const_iterator {
    private:
        node *ptr;
        const list *owner;

        friend class list;
        friend class iterator;

    public:
        const_iterator(node *p = NULL, const list *o = NULL) : ptr(p), owner(o) {}
        const_iterator(const const_iterator &other) : ptr(other.ptr), owner(other.owner) {}
        const_iterator(const iterator &other) : ptr(other.ptr), owner(other.owner) {}

        const_iterator &operator=(const const_iterator &other) {
            if (this == &other) return *this;
            ptr = other.ptr;
            owner = other.owner;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator ret(*this);
            ++(*this);
            return ret;
        }

        const_iterator &operator++() {
            if (owner == NULL || ptr == NULL || ptr == owner->tail) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator ret(*this);
            --(*this);
            return ret;
        }

        const_iterator &operator--() {
            if (owner == NULL || ptr == NULL || ptr->prev == owner->head) throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }

        const T &operator*() const {
            if (owner == NULL || ptr == NULL || ptr == owner->tail || ptr == owner->head) throw invalid_iterator();
            return ptr->value();
        }

        const T *operator->() const {
            return &(**this);
        }

        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr && owner == rhs.owner;
        }

        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr && owner == rhs.owner;
        }

        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }

        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    /**
     * TODO Constructs
     * Atleast two: default constructor, copy constructor
     */
    list() {
        init_empty();
    }
    list(const list &other) {
        init_empty();
        copy_from(other);
    }
    /**
     * TODO Destructor
     */
    virtual ~list() {
        clear();
        delete head;
        delete tail;
        head = NULL;
        tail = NULL;
    }
    /**
     * TODO Assignment operator
     */
    list &operator=(const list &other) {
        if (this == &other) return *this;
        clear();
        copy_from(other);
        return *this;
    }
    /**
     * access the first / last element
     * throw container_is_empty when the container is empty.
     */
    const T & front() const {
        if (empty()) throw container_is_empty();
        return head->next->value();
    }
    const T & back() const {
        if (empty()) throw container_is_empty();
        return tail->prev->value();
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() {
        return iterator(head->next, this);
    }
    const_iterator cbegin() const {
        return const_iterator(head->next, this);
    }
    /**
     * returns an iterator to the end.
     */
    iterator end() {
        return iterator(tail, this);
    }
    const_iterator cend() const {
        return const_iterator(tail, this);
    }
    /**
     * checks whether the container is empty.
     */
    virtual bool empty() const {
        return len == 0;
    }
    /**
     * returns the number of elements
     */
    virtual size_t size() const {
        return len;
    }

    /**
     * clears the contents
     */
    virtual void clear() {
        node *cur = head->next;
        while (cur != tail) {
            node *nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head->next = tail;
        tail->prev = head;
        len = 0;
    }
    /**
     * insert value before pos (pos may be the end() iterator)
     * return an iterator pointing to the inserted value
     * throw if the iterator is invalid
     */
    virtual iterator insert(iterator pos, const T &value) {
        if (pos.owner != this || pos.ptr == NULL || pos.ptr == head) throw invalid_iterator();
        node *cur = new data_node(value);
        insert(pos.ptr, cur);
        return iterator(cur, this);
    }
    /**
     * remove the element at pos (the end() iterator is invalid)
     * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
     * throw if the container is empty, the iterator is invalid
     */
    virtual iterator erase(iterator pos) {
        if (empty()) throw container_is_empty();
        if (pos.owner != this || pos.ptr == NULL || pos.ptr == tail || pos.ptr == head) throw invalid_iterator();
        node *nxt = pos.ptr->next;
        node *rem = erase(pos.ptr);
        delete rem;
        return iterator(nxt, this);
    }
    /**
     * adds an element to the end
     */
    void push_back(const T &value) {
        insert(iterator(tail, this), value);
    }
    /**
     * removes the last element
     * throw when the container is empty.
     */
    void pop_back() {
        if (empty()) throw container_is_empty();
        erase(iterator(tail->prev, this));
    }
    /**
     * inserts an element to the beginning.
     */
    void push_front(const T &value) {
        insert(iterator(head->next, this), value);
    }
    /**
     * removes the first element.
     * throw when the container is empty.
     */
    void pop_front() {
        if (empty()) throw container_is_empty();
        erase(iterator(head->next, this));
    }
    /**
     * sort the values in ascending order with operator< of T
     */
    void sort() {
        if (len <= 1) return;

        node *first = head->next;
        node *last = tail->prev;

        first->prev = NULL;
        last->next = NULL;

        node *sorted = merge_sort_chain(first);

        node *cur = sorted;
        node *prev = head;
        while (cur != NULL) {
            cur->prev = prev;
            prev->next = cur;
            prev = cur;
            cur = cur->next;
        }
        prev->next = tail;
        tail->prev = prev;
    }
    /**
     * merge two sorted lists into one (both in ascending order)
     * compare with operator< of T
     * container other becomes empty after the operation
     * for equivalent elements in the two lists, the elements from *this shall always precede the elements from other
     * the order of equivalent elements of *this and other does not change.
     * no elements are copied or moved
     */
    void merge(list &other) {
        if (this == &other || other.empty()) return;
        node *p1 = head->next;
        node *p2 = other.head->next;

        while (p1 != tail && p2 != other.tail) {
            if (p2->value() < p1->value()) {
                node *next2 = p2->next;

                p2->prev->next = p2->next;
                p2->next->prev = p2->prev;
                --other.len;

                node *l = p1->prev;
                p2->prev = l;
                p2->next = p1;
                l->next = p2;
                p1->prev = p2;
                ++len;

                p2 = next2;
            } else {
                p1 = p1->next;
            }
        }

        if (p2 != other.tail) {
            node *first = p2;
            node *last = other.tail->prev;

            other.head->next = other.tail;
            other.tail->prev = other.head;

            node *old_last = tail->prev;
            old_last->next = first;
            first->prev = old_last;
            last->next = tail;
            tail->prev = last;

            size_t moved = other.len;
            len += moved;
            other.len = 0;
        }
    }
    /**
     * reverse the order of the elements
     * no elements are copied or moved
     */
    void reverse() {
        node *cur = head;
        while (cur != NULL) {
            node *nxt = cur->next;
            cur->next = cur->prev;
            cur->prev = nxt;
            cur = nxt;
        }
        node *tmp = head;
        head = tail;
        tail = tmp;
    }
    /**
     * remove all consecutive duplicate elements from the container
     * only the first element in each group of equal elements is left
     * use operator== of T to compare the elements.
     */
    void unique() {
        if (len <= 1) return;
        node *cur = head->next;
        while (cur != tail) {
            node *nxt = cur->next;
            while (nxt != tail && nxt->value() == cur->value()) {
                node *del = nxt;
                nxt = nxt->next;
                erase(del);
                delete del;
            }
            cur = nxt;
        }
    }
};

}

#endif //SJTU_LIST_HPP
