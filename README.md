# TCP/IP Chat Room

This project is a **multi-client chat application** built using **C, TCP sockets, and multithreading**. It allows multiple users to connect to a server, authenticate, and communicate via private or group messaging.

---

## Features

* User Authentication (Login & Registration)
* Multi-client support using threads
* Private messaging between users
* Group chat functionality
* Online user list display
* Real-time login/logout notifications
* Server crash detection handling

---

## Technologies Used

* C Programming
* POSIX Sockets (TCP/IP)
* Pthreads (Multithreading)
* File Handling
* Signal Handling

---

## Project Structure

```

├── client.c           # Client-side implementation
├── client.h           # Client header file
├── server.c           # Server entry point
├── server_fun.c       # Server logic implementation
├── server.h           # Server header file
├── users.txt          # User credentials storage
```

---

##  How It Works

### 1. Server

* Loads registered users from `users.txt`
* Listens for incoming client connections
* Creates a new thread for each client
* Manages active users and message routing

### 2. Client

* Connects to the server
* Allows user to login/register
* Sends and receives messages concurrently using threads

---

### Authentication

* Client sends `auth_info` struct
* Server validates credentials

### Messaging

* Uses `msg_frame` structure:

  * `PRIVATE_CHAT`
  * `GROUP_CHAT`
  * `USR_LIST`
  * `EXIT`

---

## How to Run

### Step 1: Compile

```bash
gcc server.c server_fun.c -o server -lpthread
gcc client.c -o client -lpthread
```

### Step 2: Run Server

```bash
./server
```

### Step 3: Run Client (multiple terminals)

```bash
./client
```

---

## Future Enhancements

* Support for remote server (dynamic IP/Port)
* Chat history storage
* GUI interface
* End-to-end encryption

---

##  Author

**Nikhil Natarj Malashett**


