#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <iostream>
#include <thread>

// #include <boost/asio.hpp>
// #include <boost/bind.hpp>

#include "http_listener.hpp"


using namespace std;


template <template<typename, typename> class RequestHandler>
class HttpServer
{
  const int threads_;
  boost::asio::io_context ioc_;

public:
  HttpServer(const string address, const int port, const int threads)
    : threads_(threads), ioc_(threads)
  {
    // Create and launch a listening port
    auto auxAddress = boost::asio::ip::make_address(address);
    auto auxPort = static_cast<unsigned short>(port);

    std::make_shared<Listener<RequestHandler>>(ioc_, tcp::endpoint{auxAddress, auxPort})->run();
  }

  // Run the I/O service on the requested number of threads
  void run()
  {
    std::vector<std::thread> v;
    v.reserve(threads_ - 1);

    for(auto i = threads_ - 1; i > 0; --i)
      v.emplace_back([this]{
          this->ioc_.run();
          }
      );

    ioc_.run(); // use the current thread
  }
};

#endif