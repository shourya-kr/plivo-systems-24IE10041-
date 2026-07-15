#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <queue>

using namespace std;

map<uint32_t, vector<uint8_t>> raw_data;
map<uint32_t, vector<uint8_t>> parity_data;
int player_sock;
struct sockaddr_in player_addr;

void push_frame(uint32_t seq, const uint8_t* data) {
    unsigned char packet[164];
    uint32_t net_seq = htonl(seq);
    memcpy(packet, &net_seq, 4);
    memcpy(packet + 4, data, 160);
    sendto(player_sock, packet, 164, 0, (struct sockaddr *)&player_addr, sizeof(player_addr));
}

int main() {
    int listen_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sock < 0) return 1;

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(47002);
    local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (::bind(listen_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("receiver failed to bind port 47002");
        return 1;
    }

    player_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&player_addr, 0, sizeof(player_addr));
    player_addr.sin_family = AF_INET;
    player_addr.sin_port = htons(47020);
    player_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    unsigned char buffer[2048];
    uint32_t top_seq = 0;

    while (true) {
        ssize_t bytes = recvfrom(listen_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (bytes < 164) continue;

        uint32_t seq;
        memcpy(&seq, buffer, 4);
        seq = ntohl(seq);

        if (seq > top_seq) top_seq = seq;

        queue<uint32_t> dirty_nodes;

        if (!raw_data.count(seq)) {
            raw_data[seq] = vector<uint8_t>(buffer + 4, buffer + 164);
            push_frame(seq, buffer + 4);
            dirty_nodes.push(seq + 1);
            dirty_nodes.push(seq + 3);
        }
        if (bytes == 324 && !parity_data.count(seq)) {
            parity_data[seq] = vector<uint8_t>(buffer + 164, buffer + 324);
            dirty_nodes.push(seq);
        }

        while (!dirty_nodes.empty()) {
            uint32_t curr = dirty_nodes.front();
            dirty_nodes.pop();

            if (!parity_data.count(curr)) continue;

            bool got_minus_1 = (curr < 1) || raw_data.count(curr - 1);
            bool got_minus_3 = (curr < 3) || raw_data.count(curr - 3);

            if (got_minus_1 && !got_minus_3) {
                vector<uint8_t> recovered(160, 0);
                for (int i = 0; i < 160; i++) {
                    uint8_t p1 = (curr < 1) ? 0 : raw_data[curr - 1][i];
                    recovered[i] = parity_data[curr][i] ^ p1;
                }
                raw_data[curr - 3] = recovered;
                push_frame(curr - 3, recovered.data());
                dirty_nodes.push((curr - 3) + 1);
                dirty_nodes.push((curr - 3) + 3);
            }
            else if (got_minus_3 && !got_minus_1) {
                vector<uint8_t> recovered(160, 0);
                for (int i = 0; i < 160; i++) {
                    uint8_t p3 = (curr < 3) ? 0 : raw_data[curr - 3][i];
                    recovered[i] = parity_data[curr][i] ^ p3;
                }
                raw_data[curr - 1] = recovered;
                push_frame(curr - 1, recovered.data());
                dirty_nodes.push((curr - 1) + 1);
                dirty_nodes.push((curr - 1) + 3);
            }
        }

        if (top_seq > 2000) {
            uint32_t limit = top_seq - 2000;
            while (!raw_data.empty() && raw_data.begin()->first < limit) {
                raw_data.erase(raw_data.begin());
            }
            while (!parity_data.empty() && parity_data.begin()->first < limit) {
                parity_data.erase(parity_data.begin());
            }
        }
    }
    return 0;
}
