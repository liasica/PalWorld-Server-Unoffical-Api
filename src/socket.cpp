#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void socket_server() {
    spdlog::info("creating unix domain socket");

    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        spdlog::error("failed to create socket: {}", strerror(errno));
        return;
    }

    struct sockaddr_un serverAddr;
    memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    unlink(SOCKET_PATH);
    strncpy(serverAddr.sun_path, SOCKET_PATH, sizeof(serverAddr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_un)) == -1) {
        spdlog::error("failed to bind socket ({}): {}", SOCKET_PATH, strerror(errno));
        close(sfd);
        return;
    }

    if (listen(sfd, 5) == -1) {
        spdlog::error("failed to listen on socket: {}", strerror(errno));
        close(sfd);
        return;
    }

    spdlog::info("listening on {}", SOCKET_PATH);

    while (true) {
        spdlog::info("waiting for connection ...");
        int connfd = accept(sfd, nullptr, nullptr);
        if (connfd == -1) {
            spdlog::error("failed to accept connection: {}", strerror(errno));
            continue;
        }

        spdlog::info("accepted connection");

        int data_recv = 0;
		char recv_buf[RECEIVE_BUFFER_SIZE];
		char send_buf[SEND_BUFFER_SIZE];

        do {
            memset(recv_buf, 0, RECEIVE_BUFFER_SIZE*sizeof(char));
			memset(send_buf, 0, SEND_BUFFER_SIZE*sizeof(char));

            data_recv = recv(connfd, recv_buf, RECEIVE_BUFFER_SIZE, 0);

            if (data_recv > 0) {
                spdlog::info("received command: {}", recv_buf);

                if (strcmp(recv_buf, "state") == 0) {
                    spdlog::info("[CMD::State] Lazy, add in next version Owo");
                } else if (strcmp(recv_buf, "list") == 0) {
                    spdlog::info("[CMD::list]");
                    SDK::TArray<SDK::APalCharacter *> player_characters;

                    SDK::GetAllPlayerCharacters(&player_characters);

                    if (player_characters.IsValid()) {
                        spdlog::info("[CMD::List] current {} player online", player_characters.Num());

                        for (int i = 0; i < player_characters.Num(); i++) {
                            auto character  = static_cast<SDK::APalPlayerCharacter *>(player_characters[i]);
                            auto controller = static_cast<SDK::APlayerController *>(SDK::GetController(character));

                            SDK::FString faddress;
                            char16_t faddress_buffer[64] = { 0 };

                            faddress.Data        = faddress_buffer;
                            faddress.MaxElements = 64;
                            faddress.NumElements = 0;
                            GetPlayerNetworkAddress(&faddress, controller);
                            std::string address = faddress.ToString();

                            auto state = SDK::GetPlayerStateByPlayer(character);

                            SDK::FString fraw_name;
                            char16_t fraw_name_buffer[64] = { 0 };

                            fraw_name.Data        = fraw_name_buffer;
                            fraw_name.MaxElements = 64;
                            fraw_name.NumElements = 0;

                            SDK::GetPlayerName(state, &fraw_name);

                            auto uid               = SDK::GetPlayerUID(state);
                            auto uniqueId          = state->UniqueId;
                            auto replicationBytes  = uniqueId.ReplicationBytes;

                            // auto XID = uniqueId.ToString();

                            std::string name = utf16_to_local_codepage(fraw_name.Data, fraw_name.NumElements);
                            std::string steamId(reinterpret_cast<const char*>(replicationBytes.Data), replicationBytes.Num());  
                            // 76561198074020479

                            spdlog::info("[CMD::List] {}, {:08x}, {}, {}", name, static_cast<uint32_t>(uid->A), replicationBytes.Num(), address);
                        }
                    }
                }

                // do command
                strcpy(send_buf, "got message: ");
				strcat(send_buf, recv_buf);

                if (send(connfd, send_buf, strlen(send_buf)*sizeof(char), 0) == -1) {
                    spdlog::error("failed to send message: {}", strerror(errno));
                    close(connfd);
                    continue;
                }
            }

        } while (data_recv > 0);
    }
}