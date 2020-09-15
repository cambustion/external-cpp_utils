// Hey Emacs, this is -*- coding: utf-8; mode: c++ -*-

// This is a light-weight implementation of boost::signals2
// it has not been tested yet.

// The code in this file was adopted from:
//   https://dreamdota.com/c-17-signals/
//   https://github.com/TheWisp/signals

#pragma once
#include <cstdlib>
#include <vector>

namespace rh {

namespace signals {

namespace details {

struct ConnectionBase;

struct SignalBase {
  struct Call {
    void* object;
    void* function;
  };

  SignalBase() = default;
  ~SignalBase();
  SignalBase(const SignalBase&) = delete;
  SignalBase& operator=(const SignalBase&) = delete;
  SignalBase(SignalBase&& other) noexcept;
  SignalBase& operator=(SignalBase&& other) noexcept;

  // space can be optimized by using "struct of array"
  // containers since both always have the same size
  mutable std::vector<Call> calls;
  mutable std::vector<ConnectionBase*> connections;

  // space can be optimized by stealing 2 unused bits from the vector size
  mutable bool calling{false};
  mutable bool dirty{false};
};

struct BlockedConnection {
  const SignalBase* signal{nullptr};
  SignalBase::Call call{nullptr, nullptr};
};

struct ConnectionBase {
  union {
    const SignalBase* signal;
    BlockedConnection* blockedConnection;
  };

  size_t index;

  // space can be optimized by stealing bits from index as
  // it's impossible to support max uint64 number of slots
  bool blocked = false;
  bool owned = false;

  ConnectionBase(const SignalBase* signal, size_t index)
      : signal{signal}, index{index} {}

  virtual ~ConnectionBase() {
    if(!blocked) {
      if(signal) {
        signal->calls[index].object = nullptr;
        signal->calls[index].function = nullptr;
        signal->connections[index] = nullptr;
        signal->dirty = 1;
      }
    }
    else {
      delete blockedConnection;
    }
  }

  void setSignal(const SignalBase* signal) {
    if(blocked) {
      this->blockedConnection->signal = signal;
    }
    else {
      this->signal = signal;
    }
  }

  void block() {
    if(!blocked) {
      blocked = 1;
      const SignalBase* orig_sig = signal;
      signal = nullptr;
      blockedConnection = new BlockedConnection;
      blockedConnection->signal = orig_sig;
      std::swap(blockedConnection->call, orig_sig->calls[index]);
    }
  }

  void unblock() {
    if(blocked) {
      const SignalBase* orig_sig = blockedConnection->signal;
      std::swap(blockedConnection->call, orig_sig->calls[index]);
      delete blockedConnection;
      blockedConnection = nullptr;
      signal = orig_sig;
      blocked = 0;
    }
  }
};

template <typename T>
struct ConnectionNonTrivial : public ConnectionBase {
  using ConnectionBase::ConnectionBase;

  virtual ~ConnectionNonTrivial() {
    if(signal) {
      reinterpret_cast<T*>(&signal->calls[index].object)->~T();
    }
  }
};

inline SignalBase::~SignalBase() {
  for(ConnectionBase* c : connections) {
    if (c) {
      if (c->owned) {
        c->setSignal(nullptr);
      }
      else { delete c; }
    }
  }
}

inline SignalBase::SignalBase(SignalBase&& other) noexcept
    : calls{std::move(other.calls)},
      connections{std::move(other.connections)},
      calling{other.calling},
      dirty{other.dirty}
{
  for(ConnectionBase* c : connections) {
    if(c) c->setSignal(this);
  }
}

inline SignalBase& SignalBase::operator=(SignalBase&& other) noexcept {
  calls = std::move(other.calls);
  connections = std::move(other.connections);
  calling = other.calling;
  dirty = other.dirty;
  for(ConnectionBase* c : connections)
    if(c) c->setSignal(this);
  return *this;
}

} // namespace details

template<typename F> struct Signal;

// A connection without auto disconnection
struct ConnectionRaw {
  details::ConnectionBase* ptr = nullptr;
};

struct connection {
  details::ConnectionBase* ptr = nullptr;

  void disconnect() {
    delete ptr;
    ptr = nullptr;
  }

  void block() {
    ptr->block();
  }

  void unblock() {
    ptr->unblock();
  }

  connection() = default;

  ~connection() {
    disconnect();
  }

  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;

  connection(connection&& other) noexcept
			: ptr (other.ptr)
  {
    other.ptr = nullptr;
  }

  connection& operator=(connection&& other) noexcept {
    disconnect();
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  connection(ConnectionRaw conn) : ptr(conn.ptr) {
    ptr->owned = true;
  }
};

template <typename... A>
struct Signal<void(A...)> : details::SignalBase {
  template <typename... ActualArgsT>
  void operator()(ActualArgsT&&... args) const {
    bool recursion = calling;
    if(!calling) calling = 1;
    for(size_t i = 0, n = calls.size(); i < n; ++i) {
      auto& cb = calls[i];
      if(cb.func) {
        if(cb.object == cb.func)
          reinterpret_cast<void (*)(A...)>(cb.func)(
            std::forward<ActualArgsT>(args)...);
        else
          reinterpret_cast<void (*)(void*, A...)>(cb.func)(
            &cb.object, std::forward<ActualArgsT>(args)...);
      }
    }

    if(!recursion) {
      calling = 0;

      if(dirty) {
        dirty = 0;
        // remove all empty slots while patching the stored index in the
        // connection
        size_t sz = 0;
        for(size_t i = 0, n = connections.size(); i < n; ++i) {
          if(connections[i]) {
            connections[sz] = connections[i];
            calls[sz] = calls[i];
            connections[sz]->idx = sz;
            ++sz;
          }
        }
        connections.resize(sz);
        calls.resize(sz);
      }
    }
  }

  template <auto PMF, class C>
  ConnectionRaw connect(C* object) const {
    size_t idx = connections.size();
    auto& call = calls.emplace_back();
    call.object = object;
    call.func = reinterpret_cast<void*>(+[](void* obj, A... args) {
      ((*reinterpret_cast<C**>(obj))->*PMF)(args...);
    });
    details::ConnectionBase* conn = new details::ConnectionBase(
      this, idx);
    connections.emplace_back(conn);
    return {conn};
  }

  template <auto func>
  ConnectionRaw connect() const {
    return connect(func);
  }

  ConnectionRaw connect(void (*function)(A...)) const {
    size_t idx = connections.size();
    auto& call = calls.emplace_back();
    call.func = call.object = reinterpret_cast<void*>(function);
    details::ConnectionBase* conn = new details::ConnectionBase(
      this, idx);
    connections.emplace_back(conn);
    return {conn};
  }

  template <typename F>
  ConnectionRaw connect(F&& functor) const {
    using f_type = std::remove_pointer_t<std::remove_reference_t<F>>;
    if constexpr(std::is_convertible_v<f_type, void (*)(A...)>) {
      return connect(+functor);
    }
    else if constexpr(std::is_lvalue_reference_v<F>) {
      size_t idx = connections.size();
      auto& call = calls.emplace_back();
      call.func = reinterpret_cast<void*>(+[](void* obj, A... args) {
        (*reinterpret_cast<f_type**>(obj))->operator()(args...);
      });
      call.object = &functor;
      details::ConnectionBase* conn = new details::ConnectionBase(
        this, idx);
      connections.emplace_back(conn);
      return {conn};
    }
    else if constexpr(
      sizeof(std::remove_pointer_t<f_type>) <= sizeof(void*)) {
      // copy the functor.
      size_t idx = connections.size();
      auto& call = calls.emplace_back();
      call.func = reinterpret_cast<void*>(+[](void* obj, A... args) {
        reinterpret_cast<f_type*>(obj)->operator()(args...);
      });
      new(&call.object) f_type(std::move(functor));
      using conn_t = std::conditional_t<
        std::is_trivially_destructible_v<F>,
        details::ConnectionBase,
        details::ConnectionNonTrivial<F>>;
      details::ConnectionBase* conn = new conn_t(this, idx);
      connections.emplace_back(conn);
      return {conn};
    }
    else {
      struct unique {
        f_type* ptr;

        unique(f_type* ptr) : ptr(ptr) {}
        unique(const unique&) = delete;
        unique(unique&&) = delete;

        ~unique() {
          delete ptr;
        }
      };

      size_t idx = connections.size();
      auto& call = calls.emplace_back();
      call.func = reinterpret_cast<void*>(+[](void* obj, A... args) {
        reinterpret_cast<unique*>(obj)->ptr->operator()(args...);
      });
      new(&call.object) unique{new f_type(std::move(functor))};
      details::ConnectionBase* conn =
        new details::ConnectionNonTrivial<unique>(this, idx);
      connections.emplace_back(conn);
      return {conn};
    }
  }
};

} // namespace signals

} // namespace rh
