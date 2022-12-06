#pragma once
#include "intrusive_list.h"
#include <functional>
#include <iostream>
#include <list>
#include <unordered_map>

/// In order to avoid collisions with UNIX signals, the implementation has been moved to the namespace,
/// for the same reason, the file name has also been changed
namespace signals {

template <typename T>
struct signal;

struct connection_tag {};

template <typename... Args>
struct signal<void(Args...)> {
  using slot = std::function<void(Args...)>;

  struct connection : intrusive::list_element<connection_tag> {
    connection() = default;

    connection(signal* sig_, slot func_) : sig(sig_), func(std::move(func_)) {
      sig->connections.push_back(*this);
    }

    connection(connection&& other) {
      my_move(std::move(other));
    }

    connection& operator=(connection&& other) {
      if (&other != this)
        my_move(std::move(other));

      return *this;
    }

    void my_move(connection&& other) {
      sig = other.sig;
      func = std::move(other.func);
      if (sig != nullptr)
        this->link(other); /// insertion after "other"
      other.disconnect();
    }

    void disconnect() {
      if (sig != nullptr) {
        for (auto it = sig->tail; it != nullptr; it = it->prev)
          if (it->current.operator->() == this)
            it->current++;

        this->unlink();
        soft_disconnect();
      }
    }

    void soft_disconnect() {
      sig = nullptr;
    }

    void operator()(Args... args) const {
      if (sig != nullptr)
        func(args...);
    }

    ~connection() {
      disconnect();
    }

  private:
    signal* sig = nullptr;
    slot func;
  };

  using connections_list = intrusive::list<connection, connection_tag>;

  signal() = default;

  signal(signal const&) = delete;
  signal& operator=(signal const&) = delete;

  connection connect(std::function<void(Args...)> slot) noexcept {
    return connection(this, std::move(slot));
  }

  void operator()(Args&&... args) const {
    iterator_holder holder(this);
    while (holder.current != connections.end()) {
      auto copy = holder.current;
      holder.current++;
      (*copy)(args...);
      if (holder.sig == nullptr) {
        return;
      }
    }
  }

  ~signal() {
    for (auto it = tail; it != nullptr; it = it->prev)
      it->sig = nullptr;
    for (auto& connect : connections)
      connect.soft_disconnect();
  }

private:
  struct iterator_holder {
    explicit iterator_holder(const signal* sig_) : sig(sig_), current(sig->connections.begin()), prev(sig->tail) {
      sig->tail = this;
    }

    ~iterator_holder() {
      if (sig != nullptr && sig->tail != nullptr)
        sig->tail = sig->tail->prev;
    }

    const signal* sig = nullptr;
    typename connections_list::const_iterator current;
    iterator_holder* prev = nullptr;
  };

  connections_list connections;
  mutable iterator_holder* tail = nullptr;
};

} // namespace signals
