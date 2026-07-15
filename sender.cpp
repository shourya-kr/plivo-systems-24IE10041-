#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>
#include <vector>

using namespace std;

int main() {
    int recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_sock < 0) return 1;
    
    int opt = 1;
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in source_addr = {0};
    source_addr.sin_family = AF_INET;
    source_addr.sin_port = htons(47010);
    source_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (::bind(recv_sock, (struct sockaddr *)&source_addr, sizeof(source_addr)) < 0) {
        perror("sender failed to bind port 47010");
        return 1;
    }

    int send_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in relay_addr = {0};
    relay_addr.sin_family = AF_INET;
    relay_addr.sin_port = htons(47001);
    relay_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    map<uint32_t, vector<uint8_t>> frame_history;
    unsigned char buffer[2048];
    
    while (true) {
        ssize_t bytes_read = recvfrom(recv_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (bytes_read < 164) continue;

        uint32_t current_seq;
        memcpy(&current_seq, buffer, 4);
        current_seq = ntohl(current_seq);

        frame_history[current_seq] = vector<uint8_t>(buffer + 4, buffer + 164);

        bool send_fec = (current_seq % 10 != 0);
        int pkt_size = send_fec ? 324 : 164;
        
        unsigned char outgoing_packet[324];
        memcpy(outgoing_packet, buffer, 164); 

        if (send_fec) {
            for (int i = 0; i < 160; i++) {
                uint8_t prev1 = 0;
                uint8_t prev3 = 0;
                
                if (current_seq >= 1 && frame_history.count(current_seq - 1)) {
                    prev1 = frame_history[current_seq - 1][i];
                }
                if (current_seq >= 3 && frame_history.count(current_seq - 3)) {
                    prev3 = frame_history[current_seq - 3][i];
                }
                
                outgoing_packet[164 + i] = prev1 ^ prev3;
            }
        }

        sendto(send_sock, outgoing_packet, pkt_size, 0, (struct sockaddr *)&relay_addr, sizeof(relay_addr));

        if (current_seq >= 2000) {
            frame_history.erase(current_seq - 2000);
        }
    }
    return 0;
}
