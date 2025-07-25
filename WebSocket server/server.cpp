// WebSocket server. 
// It's sends a random number to the connected client every second and
// has a log_file with info about connected clients (ip&port) 


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <iomanip>


namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

bool is_valid_ip(const std::string& ip);
void logger(const std::string& message);
void start_session(tcp::socket socket, int time_period);
std::string nowtime();

int main() {
    int default_port = 5000;
    int enter_port_number=0;
    int port_number;
    std::string default_ip = "0.0.0.0";
    std::string enter_ip_address;
    std::string ip_address;
    // Time period in seconds
    float time_period = 1;	

    std::cout << "Enter IP address (default is " << default_ip << "): ";
    std::getline(std::cin, enter_ip_address);
    ip_address = enter_ip_address.empty() ? default_ip : enter_ip_address;
    if (!is_valid_ip(ip_address)) {
        std::cerr << "Invalid IP address. Using default: " << default_ip << std::endl;
        ip_address = default_ip;
    } 

    std::cout << "Enter port number (default is " << default_port << "): ";
    std::cin >> enter_port_number;
    if (enter_port_number < 150 || enter_port_number > 65535) {
        std::cout << "Port number must be between 150 and 65535. Using default port: 5000" << std::endl;
        port_number = default_port;
    } else {
        port_number = enter_port_number; 
    }
  
    try {
        auto const address = net::ip::make_address(ip_address);
        auto const port = static_cast<unsigned short>(port_number);
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, port}};
        logger("The WEB Server started at");
        std::cout << std::endl;
        std::cout << std::string(48, '*') << std::endl;
        std::cout << "   WEB Server started at: " << nowtime() << std::endl;
        std::cout << "        on the IP-address: " << address << ":" << port << std::endl;
        std::cout << std::endl;
        std::cout << "   the WEB Server sends a random number to" << std::endl;
        std::cout << "      the connected client every second" << std::endl;
        std::cout << std::string(48, '*') << std::endl;
        std::cout << std::endl;
        while (true) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);
            auto endpoint = socket.remote_endpoint();
            unsigned short client_port = endpoint.port();
            std::string ipaddress = endpoint.address().to_string();
            std::cout << "client with IP:" << ipaddress << " port:" << client_port << "is connected" << std::endl;
          
            std::thread([socket = std::move(socket), time_period]() mutable {
                start_session(std::move(socket), time_period);
            }).detach();
        }
    } catch (const std::exception& e) {
        logger("ERROR: " + std::string(e.what()));
        return 1;
    }
}


bool is_valid_ip(const std::string& ip) {
    try {
        net::ip::make_address(ip); 
        return true; 
    } catch (const std::exception&) {
        return false;  
    }
}


void start_session(tcp::socket socket, int time_period) {
    try {
        beast::websocket::stream<tcp::socket> ws{std::move(socket)};
        ws.accept();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        while (true) {
            double res = dis(gen);
            ws.write(net::buffer(std::to_string(res)));
            std::cout << "Sent: " << res << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(time_period));
        }
    } catch (beast::system_error const& se) {
        if (se.code() != beast::websocket::error::closed) {
            std::cout << "client disconected " << std::endl;
            logger("WebSocket error: " + se.code().message());
        }
    } catch (std::exception const& e) {
        std::cout << "Error !" << std::endl;
        logger("Error: " + std::string(e.what()));
    }
}


std::string nowtime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    std::ostringstream oss; 
    oss << std::put_time(&now_tm, "%H:%M:%S %Y-%m-%d"); 
    return oss.str();
}


void logger(const std::string& message) {
    std::ofstream log_file("logg_file.log", std::ios_base::app);
    log_file << message << " " << nowtime() << std::endl;
}





