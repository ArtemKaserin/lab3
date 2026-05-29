#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket)
        : socket_(std::move(socket)), timer_(socket_.get_executor()) {
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    std::istream is(&buffer_);
                    std::string line;
                    std::getline(is, line);
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    handle_request(line);
                }
                else {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            });
    }

    void handle_request(const std::string& request) {
        if (request.rfind("timer", 0) != 0) {
            do_write("Invalid command. Use: timer N\n", true);
            return;
        }
        std::istringstream iss(request);
        std::string cmd;
        int seconds;
        iss >> cmd >> seconds;
        if (seconds <= 0) {
            do_write("Invalid seconds\n", true);
            return;
        }

       
        std::string ack = "Ready in " + std::to_string(seconds) + " sec\n";
        do_write(ack, false);

       
        auto self = shared_from_this();
        timer_.expires_after(boost::asio::chrono::seconds(seconds));
        timer_.async_wait([this, self](boost::system::error_code ec) {
            if (!ec) {
                do_write("Done!\n", true);
            }
            });
    }

    void do_write(const std::string& response, bool close_after) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self, close_after](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec && close_after) {
                    socket_.close();
                }
            });
    }

    tcp::socket socket_;
    boost::asio::steady_timer timer_;
    boost::asio::streambuf buffer_;
};

class Server {
public:
    Server(boost::asio::io_context& io, short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket))->start();
            }
            do_accept();
            });
    }
    tcp::acceptor acceptor_;
};

int main() {

    setlocale(LC_ALL, "Russian");

    try {
        boost::asio::io_context io;
        Server server(io, 12343);
        std::cout << "Server 3 (timer) listening on port 12343\n";
        io.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}