#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;

bool isValidIPv4(const string& ip) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip.c_str(), &addr) == 1;
}

void* sock_err(const char* msg, int sockfd) {
    perror(msg);
    if (sockfd >= 0) close(sockfd);
    return nullptr;
}

struct ThreadArgs{

        string ip;
        string port;
        string nickname;

};

void* listen(void* arg){

        ThreadArgs* args = static_cast<ThreadArgs*>(arg);
        char buffer[1024];
        sockaddr_in addr{};
        socklen_t addr_len = sizeof(addr); 

        //create UDP-socket
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
                return sock_err("socket", s);

        int reuse = 1;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
                 return sock_err("setsockopt(SO_REUSEADDR)", s);
         }


        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET; //ipv4
        addr.sin_port = htons(static_cast<uint16_t>(stoi(args->port))); //port
        addr.sin_addr.s_addr = htonl(INADDR_ANY); //incoming ip's

        // linking our socket with given ip
        if (bind(s, (struct sockaddr*) &addr, sizeof(addr)) < 0)
                return sock_err("bind", s);

        while (true){

                memset(buffer, 0, sizeof(buffer));
                if (recvfrom(s, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&addr, &addr_len) < 0){

                        perror("recvfrom error");

                }

                char* colon = strchr(buffer, ':');
                if (colon){
			cout << "\r\033[K";	
			cout << "> ";
                        *colon = '\0';
                        cout << "[" << inet_ntoa(addr.sin_addr) << "] "
                                << buffer << ": " << (colon+1) << endl;
		}

        }

        close(s);
        return nullptr;
}

void* send(void* arg){

        ThreadArgs* args = static_cast<ThreadArgs*>(arg);
        char message[1000];

        sockaddr_in addr;
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0) 
                return sock_err("socket", s);

        // enable socket to broadcast
        int broadcast_enable = 1;
        if (setsockopt(s, SOL_SOCKET, SO_BROADCAST,
                                &broadcast_enable, sizeof(broadcast_enable))){
                perror("setsockopt(SO_BROADCAST)");
                close(s);
                return nullptr;
         }

        //set address
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(stoi(args->port));
        addr.sin_addr.s_addr = inet_addr("255.255.255.255");

        while (true){
		
		cout << "\r\033[K";	
		cout << "> ";
                cin.getline(message, sizeof(message));
		if (strcmp(message, "exit") == 0) {
			close(s);
  			exit(0);
		}

                string full_msg = args->nickname + ":" + message;
                if (sendto(s, full_msg.c_str(), full_msg.size(), 0, (sockaddr*)&addr, sizeof(addr)) < 0){

                        perror("sendto fail");
                }
        }
        close(s);

        return nullptr;
}

void threads_work(ThreadArgs *args){

        pthread_t tids[2];
        pthread_create(&tids[0], 0, listen, args);
        pthread_create(&tids[1], 0, send, args);

        pthread_join(tids[0], 0);
        pthread_join(tids[1], 0);

}

int main(int argc, char* argv[]){

        if (argc!=3){
        	cout << "Usage: \"./udpchat <ip> <port>\"" << endl;
                 return 1;
        } 

	string ip = argv[1];
	if (!isValidIPv4(ip)){	
		cerr << "err: Invalid IPv4 addr" << endl;
		return 1;	
	}

	string port = argv[2];
	int port_valid = stoi(port);
	if (port_valid < 1 || port_valid > 65535){
		cout << "err: Invalid port. Must be between 1 and 65535" << endl;
		return 1;	
	}

	string nickname;
        cout << "Enter nickname: "; 
        getline (cin, nickname);

	ThreadArgs args{ip, port, nickname};
        threads_work(&args);

        return 0;

}
