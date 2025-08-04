//
// Created by paull on 2023-12-17.
//

#pragma once

/**
 * Modeling this after Themaister's intrusive list design!
 *
 * I'm pretty late to the party when it comes to these intrusive lists, apparently they're pretty handy for
 * other things too.
 */

namespace pEngine::util {
    template<class ElementType>
    struct IntrusiveListNode {
        IntrusiveListNode<ElementType> *next = nullptr;
        IntrusiveListNode<ElementType> *prev = nullptr;

        IntrusiveListNode() = default;
    };

    template<typename ElementType>
    class IntrusiveList {
    public:
        /**
         * Basically just wraps a pointer to an intrusive list element in some particular intrusive list
         */
        struct Iterator {
            friend class IntrusiveList;

            explicit Iterator(IntrusiveListNode<ElementType> *node) : node(node) {
                // TODO - make sure the node is not nullptr
            }

            Iterator() = default;

            explicit operator bool() {
                return node != nullptr;
            }

            bool operator==(const Iterator &other) const {
                return node == other.node;
            }

            bool operator!=(const Iterator &other) const {
                return !(*this == other);
            }

            ElementType &operator*() {
                return *reinterpret_cast<ElementType *>(node);
            }

            const ElementType &operator*() const {
                return *reinterpret_cast<ElementType *>(node);
            }

            ElementType *get() {
                return reinterpret_cast<ElementType *>(node);
            }

            const ElementType *get() const {
                return reinterpret_cast<const ElementType *>(node);
            }

            ElementType *operator->() {
                return reinterpret_cast<ElementType *>(node);
            }

            const ElementType *operator->() const {
                return reinterpret_cast<ElementType *>(node);
            }

            Iterator &operator++() {
                if (!node || !node->next) {
                    // does this work?
                    *this = Iterator();
                } else {
                    node = node->next;
                }
                return *this;
            }

            Iterator &operator--() {
                if (!node) {
                    *this = Iterator();
                } else {
                    node = node->prev;
                }
                return *this;
            }

        private:
            IntrusiveListNode<ElementType> *node = nullptr;
        };

        // Intrusive List Methods
        void clear() {
            head = nullptr;
            tail = nullptr;
            size = 0;
        }

        Iterator begin() const {
            return Iterator(head);
        }

        Iterator rbegin() const {
            return Iterator(tail);
        }

        static Iterator end() {
            return Iterator();
        }

        Iterator erase(Iterator erasedItr) {
            auto *erasedNode = erasedItr.get();
            auto *erasedNext = erasedNode->next;
            auto *erasedPrev = erasedNode->prev;

            if (erasedPrev) {
                // the erased iterator is not the first in the list
                erasedPrev->next = erasedNext;
            } else {
                // the erased iterator is the first in the list
                head = erasedNext;
            }

            if (erasedNext) {
                // the erased iterator is not the last element in the list
                erasedNext->prev = erasedPrev;
            } else {
                // the erased iterator is the last element in the list
                tail = erasedPrev;
            }

            size--;
            return Iterator(erasedNext);
        }

        // this is causing problems! after insertion, the backing allocation becomes NULL
        void insertFront(Iterator insertedNodeItr) {
            auto *node = insertedNodeItr.get();
            if (head) {
                // if there is an existing head element, put this one in front of it
                head->prev = node;
            } else {
                // else we don't have an existing head, so this becomes the new tail
                tail = node;
            }

            // set inserted node so that it's before the current head and has no previous element
            node->next = head;
            node->prev = nullptr;

            head = node;
            size++;
        }

        void insertBack(Iterator insertedNodeItr) {
            auto *node = insertedNodeItr.get();
            if (tail) {
                // if we have a current tail, put this node at the end of it
                tail->next = node;
            } else {
                // in this case the list must have been empty?
                head = node;
            }

            // put the new node at the end
            node->prev = tail;
            node->next = nullptr;

            tail = node;
            size++;
        }

        void moveToFront(IntrusiveList<ElementType> &other, const Iterator itr) {
            other.erase(itr);
            insertFront(itr);
        }

        void moveToBack(IntrusiveList<ElementType> &other, const Iterator itr) {
            other.erase(itr);
            insertBack(itr);
        }

        [[nodiscard]] bool empty() const {
            return head == nullptr;
        }

        [[nodiscard]] unsigned int getSize() const {
            return size;
        }

        IntrusiveListNode<ElementType> *front() const {
            return head;
        }

        IntrusiveListNode<ElementType> *back() const {
            return tail;
        }

        void popFront() {
            if (empty()) {
                if (size != 0) {
                    // TODO - error handling/log!
                }
                return;
            }
            if (size == 1) {
                head = nullptr;
            }
            erase(Iterator(front()));
        }

        void popBack() {
            if (empty()) {
                if (size != 0) {
                    // TODO - error handling/log!
                }
                return;
            }
            if (size == 1) {
                head = nullptr;
            }
            erase(Iterator(back()));
        }

    private:
        IntrusiveListNode<ElementType> *head = nullptr;
        IntrusiveListNode<ElementType> *tail = nullptr;

        unsigned size = 0u; // TODO - evaluate whether this is a valid way of tracking the size of the list...
        // i'm worried about edge cases I haven't thought of
    };
} // util
