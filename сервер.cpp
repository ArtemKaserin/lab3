#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

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
        // Parse numbers
        std::istringstream iss(request);
        std::vector<int> nums;
        int num;
        while (iss >> num) nums.push_back(num);
        if (nums.empty()) {
            do_write("Error: no numbers\n");
            return;
        }

        // Post computation to avoid blocking the io_context
        auto self = shared_from_this();
        boost::asio::post(socket_.get_executor(), [this, self, nums]() {
            int max_val = *std::max_element(nums.begin(), nums.end());
            std::string response = "Max: " + std::to_string(max_val) + "\n";
            do_write(response);
            });
    }

    void do_write(const std::string& response) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(response),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    // Close connection after response
                    socket_.close();
                }
            });
    }

    tcp::socket socket_;
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
    try {
        boost::asio::io_context io;
        Server server(io, 12342);
        std::cout << "Server 2 (async max) listening on port 12342\n";
        io.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}