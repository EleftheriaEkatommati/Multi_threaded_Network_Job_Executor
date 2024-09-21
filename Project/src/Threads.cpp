
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <queue>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream> 
#include <cstdlib>
#include "../include/thread_functions.h"
#include <csignal>
#define BUFFER_SIZE 1024

int flag_unblock = 0; //  το flag που σηματοδοτεί τον τερματιμός τον running_workers


struct data_client
{
    char* jobID; // αναγνωριστικό id κάθε καταχωρημένης εργασίας
    char *job; // το string της εργασίας
    int clientSocket; //  το socket επικοινωνίας με τον client
};


queue<data_client*> jobQueue; // buffer που κρατάει τις διεργασίες
int concurrencyLevel = 1; // βαθμός παραλληλίας
int running_workers = 0; // πόσοι running_wokers είναι active
int id_job = 0;  // το μοναδικό νούμερο κάθε καταχωρημένης διεργασίας , με την σειρά που εισάχθηκε 





// Φτιάχνουμε τα στοιχεία κάθε εργασίας και τα αποθηκεύουμε σε ενα struct
data_client* issuejob_func(string job, int num ) 
{

    data_client * ptr = new data_client;
    //job
    int len = job.length() + 1;
    ptr ->job =  new char [len];
    strcpy(ptr ->job, job.c_str());
    cout << "The job to be executed "<< ptr ->job << endl;
    //jobID
    id_job ++;
    string make_id_job = string("job_") + to_string(id_job);
    len = make_id_job.length() + 1;
    ptr ->jobID =  new char [len];
    strcpy(ptr ->jobID, make_id_job.c_str());
    //client socket
    ptr -> clientSocket = num ;
    return ptr ;
}





vector<string> splitString(const string& input) // Παίρνει ένα string και το  φτιάχνει ένα πίνακα με σειρές όσες οι λέξεις που χωρίζονται με κενά
{  
    
    vector<string> words;
    stringstream ss(input);
    string word;

    while (ss >> word) 
    {
        words.push_back(word);
    }

    return words;
}







/*Αυτη η συνάρτηση στέλνει στον client τα περιεχόμενα του αρχείου που αποτελούν 
το αποτέλεσμα της εντολής που έστειλε ο client. Προσέχουμε για μεγάλα αρχεία, 
δηλαδή στέλνουμε το αρχείο σε chunks, και με ένα while loop ο client το διαβάζει.*/

void sendFile(const char* fileName, int socket, data_client* job_pop) 
{  

    ifstream file(fileName, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open file.\n";
        return;
    }
    

    // Get the file size
    file.seekg(0, ios::end);
    streamsize fileSize = file.tellg();
    file.seekg(0, ios::beg);

    string startMarker = "-----" + string(job_pop->jobID) + " output start-----\n";

    string endMarker = "-----" + string(job_pop->jobID) + " output end-----\n";
    // cout<< startMarker <<endl;
    // cout<< endMarker<<endl;
  
    streamsize totalSize = fileSize + startMarker.size() + endMarker.size();

    // Send the total size
    if (send(socket, &totalSize, sizeof(totalSize), 0) == -1) {
        cerr << "Failed to send total size.\n";
    }
    
   
    if (send(socket, startMarker.c_str(), startMarker.size(), 0) == -1) {
        cerr << "Failed to send start marker.\n";
    }

    
    char buffer[BUFFER_SIZE];
    while (file.read(buffer, sizeof(buffer))) {
        if (send(socket, buffer, sizeof(buffer), 0) == -1) {
            cerr << "Failed to send file data.\n";
            file.close();
        }
    }

    // Send any remaining bytes if the file size is not a multiple of BUFFER_SIZE
    if (file.gcount() > 0) {
        if (send(socket, buffer, file.gcount(), 0) == -1) {
            cerr << "Failed to send remaining file data.\n";
        }
    }

    file.close();

    
    if (send(socket, endMarker.c_str(), endMarker.size(), 0) == -1) {
        cerr << "Failed to send end marker.\n";
    }

    cout << "File sent successfully.\n";
}













void* workerThreadFunction(void* arg) 
{
    while (flag_unblock == 0)  
    { 

        pthread_mutex_lock(&queueMutex);//μία διεργασία την φορά , ακολουθεί critical section
        while (jobQueue.empty() || (running_workers >= concurrencyLevel)) /*πρέπει να σιγουρευτούμε ότι δεν η ουρά δεν είναι άδεια και ότi οι running workers δεν ξεπερνουντ το concurrency level*/
        {
            pthread_cond_wait(&notEmpty, &queueMutex);
        }
       
        data_client *run_worker ; // ένα νήμα την φορά θα εξάγει από την ουρά διεργασία
        run_worker = jobQueue.front();
        jobQueue.pop();
        running_workers++;        
        
        // cout<< "Going to execute:"<<endl;
        // cout<<concurrencyLevel<<endl;
        // cout<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<running_workers<<endl;
        // cout<<"->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<run_worker->job <<"----"<<endl;
        pthread_mutex_unlock(&queueMutex);
        // cout <<run_worker->jobID <<endl;
        // cout<< run_worker-> clientSocket <<endl;            
        vector <string>command;                                    // φτιάχνουμε τα arguments της execvp
        command = splitString(run_worker->job);
        char** args = new char*[command.size() + 1]; 
        for (size_t i = 0; i < command.size(); ++i) 
        {
            args[i] = new char[command[i].size() + 1]; // +1  null terminator
            strcpy(args[i], command[i].c_str());
        }

        
        args[command.size()] = nullptr;
        pid_t pid = fork();

        if(pid == 0){
            
            string filename = "../build/" + to_string(getpid()) + ".output";              // δημιουργία του αρχείο στο /build
            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("File open failed");
                
            }
            
            
            if (dup2(fd, STDOUT_FILENO) == -1) { // ανακατεύθυνση του stdout στο fd
                perror("dup2 failed");
                close(fd);
               
            }

            close(fd);



            execvp(args[0], args);

  

          
        }
        if(pid != 0)
        {
            waitpid(pid, nullptr, 0);                   //αναμονή για την εκτέλση του παιδιού

            for (size_t i = 0; i <command.size(); ++i) {    // free space allocated fot the args
                delete[] args[i];
            }
            delete[] args;
            // string filename = to_string(pid) + string(".output");
            string filename = "../build/" + to_string(pid) + ".output";
            sendFile(filename.c_str(), run_worker->clientSocket, run_worker); 
            close(run_worker-> clientSocket); 
            delete [] run_worker->job;
            delete [] run_worker->jobID;
            delete run_worker;
            running_workers--; 
            //cout<<"FINISH THE JOB : "<< running_workers<< endl;
            
            if (remove(filename.c_str()) != 0)  // αφαιρούμε το αρχείο από τον φάκελο /build
            {
                perror("Error removing file"); 
            }
            else
            {
                cout << "The file "<< filename <<" was removed"<<endl;
            }
            


        }
        
    //    if(jobQueue.size() < bufferSize)
    //    {
    //         
    //    }
        pthread_cond_signal(&notFull);  // ενημερώνουμε τον contoller ότι υπάρχει χώρος γιακαταχώρηση νέας εργασίας 

    }
    return nullptr;
}















void* controllerThreadFunction(void* arg) 
{
    
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int clientSocket = args->client_socket;
     int process_id = args->process_id;
  
    //free(arg);

    pthread_mutex_lock(&queueMutex);
    while (jobQueue.size() >= bufferSize) {                                // αν το jobQueue είναι γεμάτο τότε πρέπει να γίνεί wait, μέχρι να έρθει το signal notfull
        pthread_cond_wait(&notFull, &queueMutex);
    }
    int message_length;                                                     //  reiceive το Message του client
    recv(clientSocket, &message_length, sizeof(message_length), 0);

    
    char* message = new char[message_length + 1]; // +1  null terminator

    
    recv(clientSocket, message, message_length, 0);
    
   
    message[message_length] = '\0';
    cout<<message<<endl;
   
    vector <string> words ;                                                     // χωρίζουμε το message σε strings με βάση τα spaces
    words = splitString(message);
    
    if (strcmp(words[0].c_str(),"issueJob")==0)
    {
        string command ;
        for(int i= 1; i < words.size(); i++ )
        {
            command = command + words[i];
            if(i != words.size() - 1 )
            {
                command = command + " "; 
            }
            
        }
        
        data_client * job ;
        job = issuejob_func(command, clientSocket);
        jobQueue.push(job);
        // cout<<"SIZE OF QUEUE: "<< jobQueue.size()<< endl;
        string message;
        message = string("JOB") + string("<") + string(job->jobID) + string(",") + string(job->job) + string(">") + string("SUBMITTED");     //στένουμε το αναγνωριτστικό ότι εχει λειφθεί το message
        int len  = message.length() +1;
       
        send(clientSocket, &len, sizeof(len), 0);
        send(clientSocket, message.c_str(), len, 0);

        pthread_cond_signal(&notEmpty);                             //έγινε push άρα υπα΄ρχει δουλεία μέσα στο jobQueue

    }
    else if(strcmp(words[0].c_str(),"setConcurrency")==0)
    { 

        concurrencyLevel = stoi(words[1].c_str());
        string message = string("CONCURRENCY SET AT ") + to_string(concurrencyLevel);
        int len  = message.length() +1;

        send(clientSocket, &len, sizeof(len), 0);
        send(clientSocket, message.c_str(), len, 0);


    }
    else if(strcmp(words[0].c_str(),"stop")==0)
    {

        data_client *last_element = jobQueue.back();
        data_client * element = jobQueue.front();
        int one_element_on_queue = 0;

        if(element == last_element)                     // περίπτωση ύπαρξης μόνο ενός στοιχείου στην ουρά
        {
            one_element_on_queue = 1;
        }
        int flag_found =0;
        while (element != last_element || (one_element_on_queue == 1) )
        {   
           
            jobQueue.pop();
            if(strcmp(words[1].c_str(), element->jobID) == 0)                                                                       // βρεθεί τότε στέλνουμε το ανάλογο μήνυμα 
            {
                
                string message = string("JOB") + string("<") + string(element->jobID) + string(">") + string("REMOVED");
                int len  = message.length() +1;
                send(clientSocket, &len, sizeof(len), 0);
                send(clientSocket, message.c_str(), len, 0);
                flag_found = 1;
                break;
            }
            else
            {
                jobQueue.push(element);
                element =  jobQueue.front();
                
            }

        }
        if(flag_found != 1 )                                                                                                             // αν δεν βρεθεί 
        {
            string message = string("JOB") + string("<") + string(words[1].c_str()) + string(">") + string("NOTFOUND");
            int len  = message.length() +1;
            send(clientSocket, &len, sizeof(len), 0);
            send(clientSocket, message.c_str(), len, 0);
        } 


    }
    else if(strcmp(words[0].c_str(),"poll")==0)
    {
        
        data_client * element = jobQueue.front();            
        int size_queue = jobQueue.size();
        send(clientSocket, &size_queue, sizeof(size_queue), 0);                                             // χρήση του μεγέθεους της ουράς για την εύρεση τον διεργασιών σε κατάσταση αναμονής
        while (size_queue > 0 )
        {   
            size_queue --;
            
            
            jobQueue.pop();
            string message = string("<") + string(element->jobID) + string(",") + string(element->job) + string(">") ;
            int len  = message.length() +1;

            send(clientSocket, &len, sizeof(len), 0);
            send(clientSocket, message.c_str(), len, 0);
 
            jobQueue.push(element);
            element =  jobQueue.front();
        }


    }
    else if(strcmp(words[0].c_str(),"exit")==0)
    {

        string message = string("SERVER TERMINATED") ;      // αρχικά στέλνετε το μύνημα τερματισμού του server
        int len  = message.length() +1;
        send(clientSocket, &len, sizeof(len), 0);
        send(clientSocket, message.c_str(), len, 0);
        flag_unblock = 1;                                   //θέτουμε τους workers να βγουν από την loop , να τερματίσουν όσοι περιμένουν
  
        int queue_size = jobQueue.size();
        
        while(queue_size > 0)                                       // όσες διεργασίες είναι ακομά σε κατάσταση αναμονής στην ουρά , στέλνουμε στον πελάτη το κατάλληλο μύνημα
        {
            
            queue_size --;
            data_client * element = jobQueue.front();
            jobQueue.pop();
            string message = string("SERVER TERMINATED BEFORE EXECUTION") ;
            int len  = message.length() +1;
           
            send(element->clientSocket, &len, sizeof(len), 0);
            send(element->clientSocket, message.c_str(), len, 0);
            delete [] element->job;                 
            delete [] element->jobID;
            delete element;
        }   

        while(true)
        {
            if(running_workers == 0)                             // εφόσον πλέον δεν υπάρχει active worker threads κάνε shutdown για να τερματίσει και ο jobExecutorserver
            {
                int ret = shutdown(args->mainsocket, SHUT_RDWR);
            
                if (ret == 0) 
                {
                    cout << "Shutdown both parts of the socket successfully." << endl;
                    
                }
                else 
                {
                    perror("shutdown");                
                }
                break;
            }
        }
        delete [] workers;

    }
 
   
    
    pthread_mutex_unlock(&queueMutex);

    return nullptr;
}
