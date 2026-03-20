#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

struct DNS_HEADER {
    unsigned short id;  // unique identifier for each query
    unsigned short flags; //contains multiple flags
    unsigned short qdcount; //no of questions
    unsigned short anscount; // no of ans records
    unsigned short nscount; //number of authority records
    unsigned short arcount;  // number of additional records
};

struct QUESTION {
    unsigned short qtype;
    unsigned short qclass;
};

#pragma pack(push,1) // using just pack without stacks can affect other structures defined after this
struct R_DATA {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)





void encode_input(unsigned char *dst, char *src) {
    int j = 0, start = 0;
    int len = strlen(src);

    for(int i = 0; i <= len ;i++) {
        if(src[i] == '.' || src[i] == '\0') {
            dst[j++] = i - start;
            for(int k = start; k < i; k++) {
                dst[j++] = src[k];
            }
            start = i + 1;
        }
    }
    dst[j] = 0;
}

int main() {
    unsigned char buffer[512];
    memset(buffer, 0 , sizeof(buffer));

    struct DNS_HEADER *dns = (struct DNS_HEADER*)buffer;
    dns->id = htons(7564); // random number but you should set this to a new random number for every request
    dns-> flags = htons(0x0100);
    dns->qdcount = htons(1); //one question
    dns->anscount = 0;
    dns->nscount = 0;
    dns->arcount = 0;

    unsigned char *qname = buffer + sizeof(struct DNS_HEADER);
    char *domain = "www.pesuacademy.com";
    encode_input(qname,domain);

    struct QUESTION *qinfo = (struct QUESTION *)(qname + strlen((char *)qname) + 1);
    qinfo->qclass = htons(1); //IN  (internet)
    qinfo->qtype = htons(1); // A record
    
    int pkt_len = sizeof(struct DNS_HEADER) + strlen((char*)qname) + 1 + sizeof(struct QUESTION);

    int sockFD = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);

    if(sockFD < 0) {
        perror("Socket Failed\n");
        return -1;
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);  //default
    dest.sin_addr.s_addr = inet_addr("1.1.1.1"); // Google's DNS

    if(sendto(sockFD,buffer,pkt_len,0,(struct sockaddr *)&dest,sizeof(dest)) < 0) {
        perror("sendtp failed!\n");
        return -1;
    }
    printf("Query Sent!!\n");

    // Recieving the response

    socklen_t dest_len = sizeof(dest);
    int res = recvfrom(sockFD, buffer , sizeof(buffer) , 0 , (struct sockaddr *)&dest , &dest_len);

    if(res < 0) {
        perror("recvfrom failed\n");
        return -1;
    }

    close(sockFD);

    // re-read the header from response
    dns = (struct DNS_HEADER *) buffer;
    int ans_count = ntohs(dns->anscount);
    printf("Got %d answers:\n", ans_count);

    //skip past the header + question sections to reach ans section
    unsigned char *reader = buffer + sizeof(struct DNS_HEADER) + strlen((char *)qname) + 1 + sizeof(struct QUESTION);
    
    struct sockaddr_in ans;
    for(int i = 0; i < ans_count; i++) {
        reader += 2; // skip name its a compression pointer
        struct R_DATA *resource = (struct R_DATA *)reader;
        reader += sizeof(struct R_DATA);

        if(ntohs(resource->type) == 1) { // A record
            memcpy(&ans.sin_addr, reader , 4);
            printf("IP: %s\n", inet_ntoa(ans.sin_addr));
        } 
        reader += ntohs(resource->data_len);
    }
    return 0;
}
