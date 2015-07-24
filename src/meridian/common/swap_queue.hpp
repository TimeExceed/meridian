#ifndef MERIDIAN_COMMON_SWAP_QUEUE_HPP
#define MERIDIAN_COMMON_SWAP_QUEUE_HPP

#include <deque>
#include <atomic>

namespace meridian {
namespace common {

namespace details {

struct Node
{
    std::atomic::atomic<Node<T>*> mNext;
    T mValue;

    Node()
      : mNext(NULL), mValue()
    {}
    explicit Node(const T& v)
      : mNext(NULL), mValue(v)
    {}
};

} // namespace details

template<typename T>
class SingleEndList
{
public:
    Node<T> mSentinel;

    ~SingleEndList()
    {
        for(Node<T>* cur = mSentinel.mNext.load(std::memory_order_relaxed);
            cur != NULL;
            cur = cur->mNext.load(std::memory_order_relaxed))
        {
            delete cur;
        }
    }

    /**
     * @note thread-safe
     */
    void push(const T& v)
    {
    }

    /**
     * @note not thread-safe
     */
    std::unique_ptr<Node<T>> pop()
    {
        Node<T>* head = mSentinel.mNext.load(std::memory_order_relaxed);
        if (head != NULL) {
            Node<T>* next = head->mNext.load(std::memory_order_relaxed);
            mSentinel.mNext.save(next, std::memory_order_relaxed);
            head->mNext.save(NULL, std::memory_order_relaxed);
            return std::unique_ptr<Node<T>>(head);
        } else {
            return std::unique_ptr<Node<T>>();
        }
    }

    /**
     * @note not thread-safe
     */
    void reverse()
    {
    }
};

template<typename T>
class PoppableQueue
{
    details::Node<T>* mList;

public:
    PoppableQueue(details::Node<T>* lst)
      : mList(NULL)
    {
        reverse(lst);
    }
    PoppableQueue(const PoppableQueue<T>&) =delete;
    PoppableQueue<T>& operator=(const PoppableQueue<T>&) =delete;
    PoppableQueue(PoppableQueue<T>&& ano)
    {
        mList = ano.mList;
        ano.mList = NULL;
    }
    ~PoppableQueue()
    {
        for(details::Node<T>* cur = mList; cur != NULL;) {
            details::Node<T>* nxt = cur->mNext.load(std::memory_order_relaxed);
            delete cur;
            cur = nxt;
        }
    }
    
    /**
     * @note not thread-safe
     */
    std::unique_ptr<Node<T>> pop()
    {
        if (mList != NULL) {
            Node<T>* head = mList;
            Node<T>* next = mList->mNext.load(std::memory_order_relaxed);
            mList = next;
            head->mNext.save(NULL, std::memory_order_relaxed);
            return std::unique_ptr<Node<T>>(head);
        } else {
            return std::unique_ptr<Node<T>>();
        }
    }
    
private:
    void reverse(details::Node<T>* lst)
    {
        std::deque<Node<T>*> nodes;
        for(Node<T>* cur = lst;
            cur != NULL;
            cur = cur->mNext.load(std::memory_order_relaxed))
        {
            nodes.push_back(cur);
        }
        for(; !nodes.empty(); nodes.pop_back()) {
            Node<T>* nxt = mList;
            mList = nodes.back();
            mList->mNext.save(nxt, std::memory_order_relaxed);
        }
    }
};

template<typename T>
class PushableQueue
{
    std::atomic::atomic<details::Node<T>*> mList;

public:
    PushableQueue()
      : mList(NULL)
    {}
    PushableQueue(const PushableQueue<T>&) =delete;
    PushableQueue<T>& operator=(const PushableQueue&) =delete;
    PushableQueue(PushableQueue<T>&& ano)
      : mList(NULL)
    {
        details::Node<T>* lst = ano.mList.exchange(NULL, std::memory_order_relaxed);
        mList.save(lst, std::memory_order_relaxed);
    }
    ~PushableQueue()
    {
        for(details::Node<T>* cur = mList.load(std::memory_order_relaxed);
            cur != NULL;)
        {
            details::Node<T>* nxt = cur->mNext.load(std::memory_order_relaxed);
            delete cur;
            cur = nxt;
        }
    }

    /**
     * @note thread-safe
     */
    void push(const T& v)
    {
        std::unique_ptr<Node<T>> newNode(new Node<T>(v));
        for (;;) {
            Node<T>* head = mList.load(std::memory_order_acq_rel);
            newNode->mNext.save(head, std::memory_order_acq_rel);
            if (mList.compare_exchange_weak(
                    head, newNode.get(), std::memory_order_acq_rel)) {
                newNode.release();
                break;
            }
            // FIXME: shall I back off?
        }
    }

    /**
     * @note thread-safe
     */
    PoppableQueue<T> reset()
    {
        details::Node<T>* lst = mList.exchange(NULL, std::memory_order_acq_req);
        return PoppableQueue(lst);
    }
};

} // namespace common
} // namespace meridian

#endif /* MERIDIAN_COMMON_SWAP_QUEUE_HPP */
