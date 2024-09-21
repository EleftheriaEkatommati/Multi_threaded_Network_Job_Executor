#include <iostream>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "../include/thread_functions.h"

#define MAX_BUFFER_SIZE 1024

using namespace std;
queue < struct ThreadArgs*> Args;                       // ουρά κράτησης στοιχείων των οποιών είχε γίνει δθναμική δέσμευση μνήμης
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t notEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t notFull = PTHREAD_COND_INITIALIZER;

int bufferSize;    // μέγιστο μέγεθος του jobQueue
pthread_t * workers; // extern  τα threads των workers 
int threadPoolSize; // πόσοι workers θα δημιουργηθούν 




int main(int argc, char* argv[]) 
{
    
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " [portnum] [bufferSize] [threadPoolSize]" << endl;
        return EXIT_FAILURE;
    }
    
    int portnum = stoi(argv[1]);
    bufferSize = stoi(argv[2]);
    int threadPoolSize = stoi(argv[3]);

    if (bufferSize <= 0)
    {
        cerr << "Buffer size must be > 0" << endl;
        return EXIT_FAILURE;
    }

    workers = new pthread_t[threadPoolSize];                                // creation of pthread  workers
    for (int i = 0; i < threadPoolSize; ++i)
    {
        pthread_create(&workers[i], nullptr, workerThreadFunction, nullptr);
        pthread_detach(workers[i]);                                         //είναι ανεξάρτητα από το main thread
    }

    int serverSocket;
    struct sockaddr_in serverAddr;

    // Create the server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Prepare the sockaddr_in structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;                                    // Internet domain
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);                    // Any IP address
    serverAddr.sin_port = htons(portnum);                           // Port number

    // Bind the server socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Listen on the server socket
    if (listen(serverSocket, 100) < 0)              // τόσες συνδέσεις ακούει ο server
    {
        perror("listen failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    pthread_t controllerThread;
    while (true) {                                  //διαρκής έλεγχος για νεα request 
       
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
  
        if (clientSocket < 0)  // αν γίνει shutdown to serversocket
        {
            break;
        }
       
        
        ThreadArgs * args = new struct ThreadArgs;
        args ->client_socket = clientSocket;
        args->process_id = getpid();
        args->mainsocket =serverSocket;
        Args.push(args);
        cout<<args->process_id <<endl;
        pthread_create(&controllerThread, nullptr, controllerThreadFunction, args);       // creation of pthread  controllers
        pthread_detach(controllerThread);


    }

    while (!Args.empty()) {
        ThreadArgs * ptr = Args.front();
        Args.pop();
        delete ptr;
    }

    close(serverSocket);
    return 0;
}
