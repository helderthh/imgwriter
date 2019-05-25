#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "http_server.hpp"
#include "base64.hpp"


cv::Mat fromBase64(const std::string& in) {
    std::string decoded = base64::decode(in);
    std::vector<char> data(decoded.c_str(), decoded.c_str() + decoded.size());
    return cv::imdecode(cv::Mat(data), 1);
}

std::string toBase64(const cv::Mat& in) {
    std::vector<uchar> buf;
    cv::imencode(".jpeg", in, buf);
    uchar *encoded = new uchar[buf.size()];
    for(int i=0; i < buf.size(); i++)
        encoded[i] = buf[i];

    std::string ret(base64::encode(encoded, buf.size()));
    delete[] encoded;
    return ret;
}


template<class Body, class Send>
struct ReqHandler
{
  void operator()(http::request<Body>&& req, Send& send)
  {
    // Returns a bad request response
    auto const bad_request =
    [&req](boost::beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = why.to_string();
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](boost::beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + what.to_string() + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::post)
        return send(bad_request("Invalid HTTP-method"));

    // read request body
    std::stringstream ss;
    ss << req.body();

    ptree::ptree pt;
    ptree::read_json(ss, pt);

    std::string image = pt.get_child("image").data();
    std::string xPos = pt.get_child("x_pos").data();
    std::string yPos = pt.get_child("y_pos").data();
    std::string text = pt.get_child("text").data();

    std::cout << "Handling request... " << std::endl;

    cv::Mat m = fromBase64(image);
    if(m.empty()) {
        cout <<  "Couldn't read the image" << endl ;
        return send(bad_request("Couldn't read the image"));
    }

    int x = std::stoi(xPos),
        y = std::stoi(yPos);
    auto point = cv::Point(x, y);
    auto color = cv::Scalar(145, 112, 67); // BGR

    cv::putText(m, text, point, cv::FONT_HERSHEY_DUPLEX, 8, color, 13, cv::LINE_AA);
    // cv::imwrite("result.jpg", m);

    // fill response
    http::response<http::dynamic_body> resp;
    resp.version(req.version());
    resp.result(http::status::ok);
    resp.keep_alive(req.keep_alive());
    resp.set(http::field::content_type, "application/json");

    boost::beast::ostream(resp.body())
        << "{\"image\": \"" << toBase64(m)<< "\"}";
    
    std::cout << "Responding..." << std::endl;
    return send(std::move(resp));
  }
};

int main(int argc, char** argv) {
  if (argc != 4)
  {
    cerr <<
      "Usage: ./imgwritter <address> <port> <threads>" << endl
      << "Example:" << endl
      <<"    ./imgwritter 0.0.0.0 8080 1\n";
    return EXIT_FAILURE;
  }

  string address = argv[1];
  int port = std::atoi(argv[2]);
  int threads = std::max<int>(1, std::atoi(argv[3]));

  try {
    HttpServer<ReqHandler> httpServer(address, port, threads);
    httpServer.run();
  }
  catch (exception& e)
  {
    cerr << "Exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


