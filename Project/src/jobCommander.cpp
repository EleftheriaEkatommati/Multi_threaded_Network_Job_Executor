#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>
#include <fstream> 
#include <fcntl.h>

#define BUFFER_SIZE 1024

using namespace std;

void perror_exit(char const *message);


void receiveFile(int socket)  // διαβάζουμε το αρχείο 
{
     
    int totalSize;
    if (recv(socket, &totalSize, sizeof(totalSize), 0) <= 0)  // αρχικά διαβάζουμε το μέγεθος του αρχείου
    {
        cerr << "Failed to receive total size.\n";
        return;
    }
   
    char buffer[totalSize];
    ssize_t bytesReceived;
    int64_t totalBytesReceived = 0;
    const std::string terminationMessage = "SERVER TERMINATED BEFORE EXECUTION";
    string accumulatedData;
    cout<<"\n"<<endl;
    cout<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;


    while (totalBytesReceived < totalSize)  // ένα loop μέσω του οποίου διβάζουμε όλο το αρχείο που στένει ο σερβερ
    {
        bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) 
        {
            cerr << "Failed to receive file data.\n";    
        }

        accumulatedData.append(buffer, bytesReceived);
        totalBytesReceived += bytesReceived;

        // Check if the accumulated data contains the termination message
        if (accumulatedData.find(terminationMessage) != std::string::npos)  // ωστόσο αν εστάλη το termination message τότε κάνει break
        {
            // cout << "SERVER TERMINATED BEFORE EXECUTION\n";
            cout<<accumulatedData<<endl;
            break;
        }

        // Print the received data
        cout.write(buffer, bytesReceived);  // eεκτυπώνεται στο terminal
        cout.flush();
        
    }

    if (totalBytesReceived == totalSize)
    {
        cout << "\nContent received successfully.\n";
    } 
    else
    {
        cerr << "Error: Received bytes do not match the expected total size.\n";
    }
    cout<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;
    cout<<"\n"<<endl;
    

    


}   



















int main(int argc, char *argv[]) {
    int port, sock;

    char *jobCommanderInputCommand;


    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    // δημιουργία socket 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror_exit("socket");
    }

    // resolve το seerver adress 
    if ((rem = gethostbyname(argv[1])) == NULL)
    {
        perror_exit("gethostbyname");
    }

    port = atoi(argv[2]);                        // port number to integer 
    server.sin_family = AF_INET;                   // Internet domain 
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);                  // Server port 

    // Connect to server
    if (connect(sock, serverptr, sizeof(server)) < 0)
    {
        perror_exit("connect");
    }

    printf("Connecting to %s port %d\n", argv[1], port);

   
    string arguments;
    for (int i = 3; i < argc; i++)  // φτιάχνουμε όλα σε ένα string τα ορίσμτα της main
    {
        arguments += argv[i];
        if (i < argc - 1) 
        {
            arguments += " "; // Add space 
        }
    }

    int len = arguments.length() +1;
    jobCommanderInputCommand = new char[len];
    strcpy(jobCommanderInputCommand, arguments.c_str());


    send(sock, &len, sizeof(len), 0);
    send(sock, jobCommanderInputCommand, len, 0);
    

    if(strcmp("poll", argv[3]) == 0)
    {
        int queu_size;
        recv(sock, &queu_size, sizeof(queu_size), 0);           // λαμβάνουμε πρώτα τα queuesize 
        if(queu_size == 0)
        {
            cout<<"NO JOB WAS FOUND WAITTING"<<endl;
        }
        while (queu_size > 0 )     // εκτυπώνουμε ένα ένα τα watting process 
        {   
            int message_length;
            recv(sock, &message_length, sizeof(message_length), 0);
        
            
            char* message = new char[message_length +1]; // +1  null terminator
            recv(sock, message, message_length, 0);
           
            message[message_length] = '\0';
            cout << message <<endl;
            queu_size --;
        }

    }
    else
    {
        int message_length;                                                         // στάδιο επιβεβαιώσης λήψης διεργασίας  + αναγωριστικό id
        recv(sock, &message_length, sizeof(message_length), 0);
    
       
        char* message = new char[message_length +1]; // +1 null terminator

        recv(sock, message, message_length, 0);
        
        message[message_length] = '\0';
        cout << message <<endl;

        
    }

    

    if(strcmp("issueJob", argv[3]) == 0) // μόνο για το issuejob
    {
        receiveFile(sock);
    }
    

    //Close the socket
    close(sock);

    return 0;
}

void perror_exit(char const *message) {
    perror(message);
    exit(EXIT_FAILURE);
}
