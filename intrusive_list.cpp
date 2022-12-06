#include "intrusive_list.h"

intrusive::list_base::list_base(const list_base&) {}

intrusive::list_base::list_base(list_base&& old)
    : prev(old.prev), next(old.next) {
  if (old.prev != nullptr) {
    old.prev->next = this;
  }
  if (old.next != nullptr) {
    old.next->prev = this;
  }

  old.prev = old.next = nullptr;
}

void intrusive::list_base::unlink() {
  if (prev != nullptr) {
    prev->next = next;
  }

  if (next != nullptr) {
    next->prev = prev;
  }

  prev = nullptr;
  next = nullptr;
}

void intrusive::list_base::link(list_base& cur) {
  if (&cur == this || cur.next == this)
    return;

  unlink();

  next = cur.next;
  prev = &cur;
  cur.next = this;

  if (next != nullptr) {
    next->prev = this;
  }
}

intrusive::list_base::~list_base() {
  unlink();
}

bool intrusive::list_base::operator==(list_base const& other) const {
  return (prev == other.prev) && (next == other.next);
}
bool intrusive::list_base::operator!=(list_base const& other) const {
  return !(*this == other);
}