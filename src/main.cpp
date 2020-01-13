#include "../wrapper.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

inline auto session(asio::ip::tcp::socket socket) noexcept -> asio::awaitable<void> {
  const auto endpoint = socket.remote_endpoint();
  std::cout << endpoint << " connection opened" << std::endl;
  try {
    char data[1024];
    while (true) {
      const auto size = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
      co_await asio::async_write(socket, asio::buffer(data, size), asio::use_awaitable);
    }
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unhandled exception." << std::endl;
  }
  std::error_code ec;
  socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
  std::cout << endpoint << " connection closed" << std::endl;
  co_return;
}

inline auto server(std::string address, std::string service) {
  return [address, service]() -> asio::awaitable<void> {
    auto executor = co_await asio::this_coro::executor;
    auto resolver = asio::ip::tcp::resolver{ executor };
    auto endpoint = resolver.resolve(address, service)->endpoint();
    auto acceptor = asio::ip::tcp::acceptor{ asio::make_strand(executor), endpoint, true };
    std::cout << endpoint << std::endl;
    while (true) {
      std::error_code ec;
      auto socket = co_await acceptor.async_accept(asio::redirect_error(asio::use_awaitable, ec));
      if (ec) {
        std::cerr << ec.message() << " (" << ec.value() << ")" << std::endl;
        continue;
      }
      asio::co_spawn(
        executor,
        [socket = std::move(socket)]() mutable noexcept {
          return session(std::move(socket));
        },
        asio::detached);
    }
    co_return;
  };
}

int main(int argc, char* argv[]) {
  try {
    std::string address = argc > 1 ? argv[1] : "127.0.0.1";
    std::string service = argc > 2 ? argv[2] : "9999";

    asio::io_context context{ 1 };
    asio::signal_set signals(context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) {
      context.stop();
    });

    asio::co_spawn(context.get_executor(), server(address, service), [](std::exception_ptr e) {
      if (e) {
        std::rethrow_exception(e);
      }
    });
    context.run();
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Unhandled exception." << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
