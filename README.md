# Multi-threaded Network Job Executor

## Overview
This project involves creating a multi-threaded network application in a Unix environment. The application will manage and execute jobs submitted by users over the network. The primary goals are to familiarize with system programming, thread creation, and network communication.

## Components
The project consists of two main components:
1. **jobCommander**: A client process that allows users to interact with the server by submitting commands.
2. **jobExecutorServer**: A server process that manages and executes the jobs submitted by jobCommander.

## Features
- **Multi-threaded Execution**: The server uses multiple worker threads to execute jobs concurrently.
- **Network Communication**: The client and server communicate over the network using sockets.
- **Flow Control**: The server controls the flow of job execution based on the current concurrency level.
- **Dynamic Concurrency**: The concurrency level can be adjusted dynamically without stopping the server.

## Commands
The `jobCommander` supports the following commands:
- `issueJob <job>`: Submits a job to the server for execution.
- `setConcurrency <N>`: Sets the maximum number of concurrent worker threads.
- `stop <jobID>`: Removes a job from the queue.
- `poll`: Lists all queued jobs.
- `exit`: Terminates the server.

## Usage
### jobCommander
```sh
./jobCommander [serverName] [portNum] [jobCommanderInputCommand]
```
## Server Configuration
- **serverName**: The hostname of the machine running the `jobExecutorServer`.
- **portNum**: The port number on which the `jobExecutorServer` is listening.
- **jobCommanderInputCommand**: One of the supported commands.

```sh
jobExecutorServer [portnum] [bufferSize] [threadPoolSize]
```
- **serverName**: The hostname of the machine running the `jobExecutorServer`.
- **portNum**: The port number on which the `jobExecutorServer` is listening.
- **command**: One of the supported commands.

## Example Execution

```sh
jobCommander serverName 2035 issueJob ls -l /path/to/directory1
jobCommander serverName 2035 setConcurrency 2
jobCommander serverName 2035 issueJob wget aUrl
jobCommander serverName 2035 issueJob grep "keyword" /path/to/file1
jobCommander serverName 2035 issueJob cat /path/to/file2
jobCommander serverName 2035 issueJob ./progDelay 20
jobCommander serverName 2035 issueJob ./executable arg1 arg2
jobCommander serverName 2035 poll
jobCommander serverName 2035 stop job_1
jobCommander serverName 2035 stop job_15
jobCommander serverName 2035 exit
```

## Implementation Details
## Main Thread
- Creates worker threads.
- Listens for connections from jobCommander clients.
- Creates a controller thread for each connection.
## Controller Threads
- Handle client commands.
- Manage job queue and concurrency level.
- Communicate with clients.
## Worker Threads
- Execute jobs from the queue.
- Redirect job output to files.
- Send job output to clients.
## Synchronization
- Uses condition variables to synchronize access to the shared job queue.
- Ensures no busy-waiting for resource access.
