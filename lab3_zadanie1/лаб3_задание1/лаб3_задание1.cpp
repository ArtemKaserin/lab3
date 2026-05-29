#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

using boost::asio::ip::tcp;

std::string process(const std::string& msg) {
    std::string upper = msg;
    for (char& c : upper) c = std::toupper(static_cast<unsigned char>(c));
    return std::to_string(msg.size()) + ": " + upper;
}

int main() {
    try {
        boost::asio::io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12341));
        std::cout << "Server 1 (sync echo) listening on port 12341\n";

        while (true) {
            tcp::socket socket(io);
            acceptor.accept(socket);
            std::cout << "Client connected\n";

            boost::asio::streambuf buf;
            boost::asio::read_until(socket, buf, '\n');
            std::istream is(&buf);
            std::string line;
            std::getline(is, line);
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::string response = process(line) + "\n";
            boost::asio::write(socket, boost::asio::buffer(response));
            std::cout << "Sent: " << response;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}