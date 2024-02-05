#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/libpal-plugin-loader.socket"

/// @doc https://blog.csdn.net/Dontla/article/details/131114439
int main()
{
    // 创建套接字
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 连接到服务器
    struct sockaddr_un serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    strncpy(serverAddr.sun_path, SOCKET_PATH, sizeof(serverAddr.sun_path) - 1);

    while (true)
    {
        while (true)
        {
            if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_un)) == -1)
            {
                // std::cerr << "Failed to connect to server" << std::endl;
                std::cerr << "Failed to connect to server, sleep 1s and reconnect..." << std::endl;
                sleep(1);
                continue;
                // close(sockfd);
                // return 1;
            }
            break;
        }

        std::cout << "Connected to server!" << std::endl;

        while (true)
        {
            try // 貌似这里try多余了，如果服务端是被强行关闭的，客户端这里是捕获不到的，也会强行结束
            {

                // 输入要发送的消息
                std::cout << "Enter message to send: ";
                std::string message;
                // std::string message = "client auto send a message";
                std::getline(std::cin, message);

                if (message == "q")
                {
                    break;
                }

                // 发送消息给服务器
                ssize_t bytesWritten = send(sockfd, message.c_str(), message.length(), 0);
                if (bytesWritten == -1)
                {
                    std::cerr << "Failed to send message to server" << std::endl;
                    break;
                    // close(sockfd);
                    // return 1;
                }

                sleep(1);

                // 接收服务器的回复
                char buffer[1024];
                ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
                if (bytesRead == -1)
                {
                    std::cerr << "Failed to receive response from server" << std::endl;
                    break;
                    // close(sockfd);
                    // return 1;
                }

                buffer[bytesRead] = '\0';
                std::cout << "Received response from server: " << buffer << std::endl;
            }
            catch(std::exception& e)
            {
                std::cout << "catch(std::exception& e): " << e.what() << std::endl;
                break;
            }
        }

        // std::cout << "Disconnecting from server" << std::endl;
        // close(sockfd);
        // return 0;
    }
}
