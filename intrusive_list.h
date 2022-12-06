#pragma once

#include <cstddef>
#include <iterator>

namespace intrusive {
struct default_tag;

template <typename T, typename Tag>
class list;

class list_base {
public:
  list_base* prev = nullptr;
  list_base* next = nullptr;
  template <typename T, typename lTag>
  friend class list;

public:
  list_base() = default;
  list_base(const list_base&); // copy
  list_base(list_base&&);      // move

  list_base& operator=(list_base const& old) = default; // copy

  bool operator==(list_base const& other) const;
  bool operator!=(list_base const& other) const;

  void link(list_base&);
  void unlink();
  ~list_base();
};

template <typename T, typename Tag>
class list;

template <typename Tag = default_tag>
struct list_element : public list_base {};

template <typename T, typename Tag = default_tag>
class list {
  list_element<Tag> sentinel;
  void reset() {
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
  }
  static list_element<Tag>& make_ref(T& old_ref) {
    return static_cast<list_element<Tag>&>(old_ref);
  }
  static list_element<Tag>& make_ref(list_base& old_ref) {
    return static_cast<list_element<Tag>&>(old_ref);
  }
  static list_element<Tag>* make_p(list_base* old_p) {
    return static_cast<list_element<Tag>*>(old_p);
  }

public:
  list() {
    reset();
  }

  // move constructor default?
  list(list<T>&& old) {
    if (old.empty()) {
      reset();
    } else {
      list_base& end = *old.sentinel.prev;
      old.sentinel.unlink();
      old.reset();
      sentinel.link(end);
    }
  }

  // move operator default?
  list& operator=(list<T>&& old) {
    if (this == &old)
      return *this;

    this->sentinel.unlink();

    if (old.empty()) {
      reset();
    } else {
      list_base& end = *old.sentinel.prev;
      old.sentinel.unlink();
      old.reset();
      sentinel.link(end);
    }

    return *this;
  }

  void push_back(T& data) {
    make_ref(data).link(make_ref(*sentinel.prev));
  }

  void pop_back() {
    sentinel.prev->unlink();
  }

  void push_front(T& data) {
    data.link(sentinel);
  }

  void pop_front() {
    sentinel.next->unlink();
  }

  T const& front() const {
    return static_cast<T&>(make_ref(*sentinel.next));
  }

  T const& back() const {
    return static_cast<T&>(make_ref(*sentinel.prev));
  }

  T& front() {
    return static_cast<T&>(make_ref(*sentinel.next));
  }

  T& back() {
    return static_cast<T&>(make_ref(*sentinel.prev));
  }

  bool empty() {
    return sentinel.next == &sentinel;
  }

  ~list() {
    while (!empty()) {
      pop_back();
    }
  }

private:
  template <typename iT>
  class list_iterator {
  private:
    list_element<Tag>* cur = nullptr;
    list_iterator(decltype(cur) cur) : cur(cur) {}
    template <typename lT, typename lTag>
    friend class list;

  public:
    using difference_type = ptrdiff_t;
    using value_type = iT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    list_iterator() = default;

    template <typename eq_iT>
    bool operator==(list_iterator<eq_iT> const& other) const {
      return *cur == *other.cur;
    }

    template <typename eq_iT>
    bool operator!=(list_iterator<eq_iT> const& other) const {
      return *cur != *other.cur;
    }

    reference operator*() const {
      return static_cast<iT&>(*cur);
    }

    pointer operator->() const {
      return static_cast<iT*>(cur);
    }

    list_iterator& operator++() {
      cur = make_p(cur->next);
      return *this;
    }
    list_iterator& operator--() {
      cur = make_p(cur->prev);
      return *this;
    }
    list_iterator operator++(int) {
      list_iterator res(*this);
      cur = make_p(cur->next);
      return res;
    }
    list_iterator operator--(int) {
      list_iterator res(*this);
      cur = make_p(cur->prev);
      return res;
    }

    operator list_iterator<const iT>() const {
      return {cur};
    }
  };

public:
  using iterator = list_iterator<T>;
  using const_iterator = list_iterator<const T>;

  iterator begin() {
    return {make_p(sentinel.next)};
  }

  const_iterator begin() const {
    return {make_p(sentinel.next)};
  }

  iterator end() {
    return {&sentinel};
  }

  const_iterator end() const {
    return {const_cast<list_element<Tag>*>(&sentinel)};
  }

  iterator insert(iterator const& it, T& data) {
    data.link(*(it.cur->prev));
    return iterator(make_p(it.cur->prev));
  }

  iterator erase(iterator const& it) {
    list_base* next = it.cur->next;
    it.cur->unlink();
    return iterator(make_p(next));
  }

private:
  void link(list_base* a, list_base* b) {
    a->next = b;
    b->prev = a;
  }

public:
  void splice(const_iterator pos, list& other, const_iterator first,
              const_iterator last) {
    if (last == pos) // this == &other &&? why then other?
      return;

    if (first == last)
      return;

    auto pos_prev = pos.cur->prev;
    auto first_prev = first.cur->prev;
    auto last_prev = last.cur->prev;

    link(pos_prev, first.cur);
    link(last_prev, pos.cur);
    link(first_prev, last.cur);
  }
};
} // namespace intrusive
