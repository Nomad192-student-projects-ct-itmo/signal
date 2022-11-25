#pragma once
#include <functional>

// Чтобы не было коллизий с UNIX-сигналами реализация вынесена в неймспейс, по
// той же причине изменено и название файла
namespace signals {

template <typename T>
struct signal;

template <typename... Args>
struct signal<void(Args...)> {
  struct connection;

  signal();

  signal(signal const&) = delete;
  signal& operator=(signal const&) = delete;

  ~signal();

  connection connect(std::function<void(Args...)> slot) noexcept;

  void operator()(Args...) const;
};

} // namespace signals
