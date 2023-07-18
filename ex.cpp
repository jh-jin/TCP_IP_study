#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUFFER_SIZE = 1024;
const int PORT = 8888;
const int MAX_CLIENTS = 15;

class EchoServer {

private:
    int serverSocket;
    std::vector<int> clientSockets;

    bool createSocket() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "서버 소켓을 생성하는데 실패했습니다." << std::endl;
            return false;
        }
        return true;
    }

    bool bindSocket() {
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(PORT);

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            std::cerr << "바인딩하는데 실패했습니다." << std::endl;
            return false;
        }
        return true;
    }

    bool listenForClients() {
        listen(serverSocket, MAX_CLIENTS);
        std::cout << "클라이언
트 연결을 대기합니다..." << std::endl;
        return true;
    }

    void acceptClient() {
        struct sockaddr_in clientAddres;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int newClientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (newClientSocket < 0) {
            std::cerr << "클라이언트 연결을 수락하는데 실패했습니다." << std::endl;
            return;
        }

        std::cout << "새로운 클라이언트가 연결되었습니다." << std::endl;

        // 클라이언트 소켓 목록에 추가
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clientSockets[i] == -1) {
                clientSockets[i] = newClientSocket;
                break;
            }
        }
    }

    void closeSocket(int socket) {
        close(socket);

        // 클라이언트 소켓 목록에서 제거
        auto it = std::find(clientSockets.begin(), clientSockets.end(), socket);
        if (it != clientSockets.end()) {
            *it = -1;
        }
    }

    void closeSocket() {
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clientSockets[i] != -1) {
                close(clientSockets[i]);
            }
        }
        if (serverSocket != -1) {
            close(serverSocket);
            serverSocket = -1;
        }
    }

public:
    EchoServer() : serverSocket(-1) {
        clientSockets.resize(MAX_CLIENTS, -1);
    }

    bool start() {
        if (!createSocket()) {
            return false;
        }

        if (!bindSocket()) {
            closeSocket();
            return false;
        }

        if (!listenForClients()) {
            closeSocket();
            return false;
        }

        return true;
    }

    void run() {
        fd_set readfds;
        int maxSocket = serverSocket;
        int activity, i, valread, clientSocket, sd;
        char buffer[BUFFER_SIZE];

        while (true) {
            // 소켓 목록 초기화
            FD_ZERO(&readfds); 

            // 서버 소켓 추가
            FD_SET(serverSocket, &readfds);

            // 클라이언트 소켓 추가
            for (i = 0; i < MAX_CLIENTS; ++i) {
                clientSocket = clientSockets[i];
                if (clientSocket != -1) {
                    FD_SET(clientSocket, &readfds);
                    if (clientSocket > maxSocket) {
                        maxSocket = clientSocket;
                    }
                }
            }

            // 소켓 감시
            activity = select(maxSocket + 1, &readfds, NULL, NULL, NULL);
            if (activity < 0) {
                std::cerr << "소켓 감시 중 오류가 발생했습니다." << std::endl;
                break;
            }

            // 클라이언트 연결 처리
            if (FD_ISSET(serverSocket, &readfds)) {
                acceptClient();
            }

            // 클라이언트로부터 데이터 수신 및 에코
            for (i = 0; i < MAX_CLIENTS; ++i) {
                clientSocket = clientSockets[i];
                if (FD_ISSET(clientSocket, &readfds)) {
                    memset(buffer, 0, BUFFER_SIZE);
                    valread = read(clientSocket, buffer, BUFFER_SIZE - 1);
                    if (valread < 0) {
                        std::cerr << "데이터를 읽는데 실패했습니다." << std::endl;
                        closeSocket(clientSocket);
                        break;
                    }
                    if (valread == 0) {
                        std::cout << "클라이언트와의 연결이 종료되었습니다." << std::endl;
                        closeSocket(clientSocket);
                        break;
                    }

                    std::cout << "수신한 메시지: " << buffer << std::endl;

                    if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                        std::cerr << "데이터를 전송하는데 실패했습니다." << std::endl;
                        closeSocket(clientSocket);
                        break;
                    }
                }
            }
        }
        closeSocket();
    }
};

int main() {
    EchoServer server;
    if (server.start()) {
        server.run();
    }
    return 0;
}