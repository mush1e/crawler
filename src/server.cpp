#include "server.hpp"

namespace index_stream {

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ create socket and bind to port ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
    HTTP_Server::HTTP_Server(int port) : port(port) {
        if((this->server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error: Failed to create socket!\n";
            exit(1);
        }

        this->server_address.sin_addr.s_addr = INADDR_ANY;
        this->server_address.sin_family = AF_INET;
        this->server_address.sin_port = htons(this->port);

        if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
            std::cerr << "Error: Failed to bind socket to port!\n";
            exit(1);
        }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ close socket on deletion ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
    HTTP_Server::~HTTP_Server() {
        close(this->server_socket);
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ check if new webpages scraped and update db ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
    auto HTTP_Server::recurring_db_update() -> void {
        for(;;) {
            int file_count {};
            auto& idxr = indexer::Indexer::get_instance();

            for (const auto& entry : std::filesystem::directory_iterator("../raw_dump")) {
                if (std::filesystem::is_regular_file(entry.status())) 
                    file_count++;

                if (file_count > 1) {
                    idxr.update_db();
                    thread_pool.pause_task_queue();

                    if (thread_pool.await_pending_tasks())
                        idxr.merge_db();
                        
                    thread_pool.resume_task_queue();
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::minutes(60));
        }
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ start listening for connections and handle client when connected ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
    auto HTTP_Server::start() -> void {
        if(listen(server_socket, 10) < 0) {
            std::cerr << "Error: Failed to listen for connections!\n";
            exit(1);
        }

        std::cout << "Server Started! Listening on port: " << this->port << std::endl;
        std::thread t(&HTTP_Server::recurring_db_update, this);
        auto& idxr = indexer::Indexer::get_instance();

        for(;;) {

            int client_socket = accept(this->server_socket, nullptr, nullptr);

            if(client_socket < 0) {
                std::cerr << "Error: Failed to accept connection!\n";
                continue;
            }

            this->thread_pool.enqueue([client_socket] {
                handle_client(client_socket);
            });
        }

        if (t.joinable())
            t.join();
    }
}