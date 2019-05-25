#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#include <memory>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/property_tree/json_parser.hpp>
 
#include "utils.hpp"


using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;
namespace ptree = boost::property_tree;

template<class Body, class Allocator, class Send>
using RequestHandlerType = 
void (*) (http::request<Body, http::basic_fields<Allocator>>&& req,
          Send&& send);
  

// Handles an HTTP server connection
template <template<typename, typename> class RequestHandler>
class Session : public std::enable_shared_from_this<Session<RequestHandler>>
{
  struct SendLambda
  {
    Session& session_;

    explicit SendLambda(Session& self) : session_(self) { /* */ }

    template<bool isRequest, class MsgBody, class Fields>
    void operator()(http::message<isRequest, MsgBody, Fields>&& msg) const 
    {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto msgSp = 
        std::make_shared<http::message<isRequest, MsgBody, Fields>>(std::move(msg));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      session_.res_ = msgSp;

      // Write the response
      http::async_write(
        session_.socket_,
        *msgSp,
        boost::asio::bind_executor(
          session_.strand_,
          std::bind(
              &Session::on_write,
              this->session_.shared_from_this(),
              std::placeholders::_1,
              std::placeholders::_2,
              msgSp->need_eof())));
    }
  };

  tcp::socket socket_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  boost::beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  SendLambda sendLambda_;

public:
  // Take ownership of the socket
  explicit
  Session(tcp::socket socket) : socket_(std::move(socket)),
    strand_(socket_.get_executor()), sendLambda_(*this)
  {
  }

  void run() { do_read(); }

  void do_read()
  {
    req_ = {}; // Clean the request

    // And then read it
    http::async_read(socket_, buffer_, req_,
      boost::asio::bind_executor(
        strand_,
        std::bind(
          &Session::on_read,
          this->shared_from_this(),
          std::placeholders::_1,
          std::placeholders::_2
        )
      )
    );
  }

  void do_close()
  {
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);
  }

  void on_read(boost::system::error_code ec, std::size_t bytes_transferred)
  {
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
      return do_close();

    if(ec)
      return fail(ec, "couldn't read");

    // Send the response
    RequestHandler<http::string_body, SendLambda>()(std::move(req_), sendLambda_);
  }

  void on_write(boost::system::error_code ec,
                std::size_t bytes_transferred,
                bool close)
  {
    boost::ignore_unused(bytes_transferred);

    if(ec)
      return fail(ec, "write");

    if(close)
    {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return do_close();
    }

    // We're done with the response so delete it
    res_ = nullptr;

    // Read another request
    do_read();
  }
};

#endif