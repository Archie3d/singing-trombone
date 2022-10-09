#include <cassert>

namespace core {

template <class Item> struct ListItem;
template <class Item> struct List;

template <typename Item>
struct ListNode
{
private:
    Item* _prev{ nullptr };
    Item* _next{ nullptr };

    template <typename I> friend struct ListItem;
    template <typename I> friend struct List;
};

template <class Item>
struct ListItem : public ListNode<ListItem<Item>>
{
    void appendAfter(ListItem* item) noexcept
    {
        assert(item != nullptr);
        this->_prev = item;
        this->_next = item->_next;
        item->_next = this;
    }

    void remove() noexcept
    {
        if (this->_prev != nullptr)
            this->_prev->_next = this->_next;

        if (this->_next != nullptr)
            this->_next->_prev = this->_prev;

        this->_prev = this->_next = nullptr;
    }

    Item* next() noexcept { return static_cast<Item*>(this->_next); }
    Item* prev() noexcept { return static_cast<Item*>(this->_prev); }
};

template <class Item>
struct List
{
    Item* first() noexcept { return _head; }
    Item* last() noexcept { return _tail; }

    void append(Item* item) noexcept
    {
        assert(item != nullptr);

        if (_head == nullptr) {
            _head = item;
            _tail = item;
        } else {
            item->appendAfter(_tail);
            _tail = item;
        }
    }

    void prepend(Item* item) noexcept
    {
        assert(item != nullptr);

        item->_next = _head;
        _head = item;

        if (_tail == nullptr)
            _tail = item;
    }

    void remove(Item* item) noexcept
    {
        assert(item != nullptr);

        if (_head == item)
            _head = item->next();

        if (_tail == item)
            _tail = item->prev();

        item->remove();
    }

    bool contains(Item* item) const noexcept
    {
        auto* it{ _head };

        while (it != nullptr) {
            if (it == item)
                return true;

            it = it->next();
        }

        return false;
    }

    Item* removeAndReturnNext(Item* item) noexcept
    {
        Item* nextItem{ item->next() };
        remove(item);
        return nextItem;
    }

    bool isEmpty() const noexcept
    {
        return _head == nullptr;
    }

    Item* operator[] (int index) const noexcept
    {
        Item* it{ _head };

        if (index >= 0) {
            while (index > 0 && it != nullptr) {
                --index;
                it = it->next();
            }
        } else {
            it = _tail;
            ++index;

            while (index < 0 && it != nullptr) {
                ++index;
                it = it->prev();
            }
        }

        return it;
    }

private:
    using ItemBase = struct ListItem<Item>;
    static_assert(std::derived_from<Item, ItemBase>, "List item must inherit from ListItem<>");

    Item* _head{ nullptr };
    Item* _tail{ nullptr };
};

} // namespace core
