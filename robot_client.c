#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

struct mesg {
    int label;
    double position[6];
    struct timeval timeSent;
};

#define ERROR (-1)

int main(int argc, char *argv[]) {
    int client, server, results,xx,addr;
    struct sockaddr_in serverAddr;
    struct mesg receivedMesg, sendMesg;
    receivedMesg.label = 1;
    ssize_t bytes_received;
    long int Te;

    Te=20000; // Te=20ms

    // Create socket for server
    server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(2200); // Assuming port 3000 for server
    //serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Update IP address as needed
    addr=sizeof(serverAddr);
    // Bind server socket
    int bind_result = bind(server, (struct sockaddr*)&serverAddr, addr);
    if (bind_result == -1) {
        perror("bind failed");
        // Handle error (e.g., exit or continue)
    } else {
        // Binding successful, continue with the program
        printf("Binding successful\n");
    }

    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);
    fcntl(server,F_SETFL,fcntl(server,F_GETFL) | O_NONBLOCK);

    while (receivedMesg.label<300) {
      // Receive message from the server
        bytes_received = recvfrom(server, &receivedMesg, sizeof(receivedMesg), 0, (struct sockaddr*)&serverAddr, &addr);
        if (bytes_received == -1) {
            //perror("rlabel=1.000000,ecvfrom failed");
            xx++;
            // Handle error (e.g., exit or continue)
        } else //if(receivedMesg.label != 0) 
        {
        // Multiply the received position by 2
        sendMesg = receivedMesg;
        sendMesg.position[0] = sendMesg.position[0]*= 2;
        //sendMesg.position[0] = receivedMesg.position[0] *= 2;


  
        // Print the sent position for verification
        printf("Received position from server: label=%d, position=%lf\n", receivedMesg.label, receivedMesg.position[0]);

        // Send the modified message back to the server
        results=sendto(server, &sendMesg, sizeof(sendMesg), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }
        //usleep(Te); //Delay
    }

    close(server);

    return 0;
}
