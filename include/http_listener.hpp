#ifndef __HTTP_LISTENER_H__
#define __HTTP_LISTENER_H__

#include <memory>

#include "http_session.hpp"


// Accepts incoming connections and launches the sessions
template <template<typename, typename> class RequestHandler>
class Listener : public std::enable_shared_from_this<Listener<RequestHandler>>
{
	tcp::acceptor acceptor_;
	tcp::socket socket_;

public:
	Listener(boost::asio::io_context& ioc,
		tcp::endpoint endpoint) : acceptor_(ioc), socket_(ioc)
	{
    boost::system::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
      fail(ec, "couldn't open endpoint");
      return;
    }

    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    if(ec)
    {
      fail(ec, "couldn't set_option reuse_address");
      return;
    }

    acceptor_.bind(endpoint, ec);
    if(ec)
    {
      fail(ec, "bind");
      return;
    }

    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if(ec)
    {
      fail(ec, "listen");
      return;
    }
	}

	// Start accepting incoming connections
	void run()
	{
    if(acceptor_.is_open())
      do_accept();
	}

	void do_accept()
	{
    acceptor_.async_accept(
      socket_,
      std::bind(&Listener::on_accept,
          this->shared_from_this(),
          std::placeholders::_1));
	}

	void on_accept(boost::system::error_code ec)
	{
    if(ec)
      fail(ec, "couldn't accept");
    else
      // Create the session and run it
      std::make_shared<Session<RequestHandler>>(std::move(socket_))->run();

    // Accept another connection
    do_accept();
	}
};

#endif
