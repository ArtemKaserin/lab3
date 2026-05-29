#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "12341"));

        std::cout << "Connected to server. Enter message: ";
        std::string msg;
        std::getline(std::cin, msg);
        msg += "\n";
        boost::asio::write(socket, boost::asio::buffer(msg));

        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, '\n');
        std::istream is(&buf);
        std::string response;
        std::getline(is, response);
        std::cout << "Server reply: " << response << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
    return 0;
} 