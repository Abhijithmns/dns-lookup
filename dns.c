#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


int main() {

    printf("Performing DNS lookup....\n");

    //DNS lookup logic

    struct addrinfo *result;
    const char *node = "google.com";
    const char *service = "http";
   struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = 0; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int res = getaddrinfo(node,service,&hints,&result);
    // ignoring the value of res for now
    (void)res;

    // printing the results of DNS lookup
    struct addrinfo *rp;
    for(rp = result; rp!=NULL; rp = rp->ai_next){
        printf("Family : %s\n",rp->ai_family == AF_INET ? "IPv4" : "IPv6");
    }

    freeaddrinfo(result);

    return 0;
}
