#include <map>
#include <set>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "../common/IConnectionClient.hpp"

#define MAX_CLIENTS 100

char check(const char * message, int err, char is_exit)
{
    if (err != 0)
    {
        perror(message);
        
        if (is_exit)
            exit(err);
        else
        {
            errno = 0;
            return 0;
        }
    }
    
    return 1;
}

struct SocketsConnectionClient;
void listener(SocketsConnectionClient*);
void sender(SocketsConnectionClient*);

class RecvRequest: public IRequest
{
    int tag;
    char * buf;
    int size;
    int cur_size;
    SocketsConnectionClient * connection;
    
    RecvRequest();
    
    char * ready_buf;
    
    static const int KB;
public:
    RecvRequest(int tag, char * buf, int size, SocketsConnectionClient * connection);
    
    bool test();
    
    void wait();
    
    ~RecvRequest();
};
const int RecvRequest::KB = 1024;

class SendRequest: public IRequest
{
    int tag;
    SocketsConnectionClient * connection;
    
    SendRequest();
public:
    SendRequest(int, SocketsConnectionClient *);
    
    bool test();
    void wait();
};

struct ConnectionIn
{
    int tag;
    int cur_tag_size;
    
    char * buf;
    int cur_size;
    int expected_size;
    int cur_expected_size;
    
    ConnectionIn()
    : tag(-1), cur_tag_size(0), buf(0), cur_size(0), expected_size(-1), cur_expected_size(0)
    {
    }
    
    void clear()
    {
        tag = -1;
        cur_tag_size = 0;
        buf = 0;
        cur_size = 0;
        expected_size = -1;
        cur_expected_size = 0;
    }
};

struct DataForSend
{
    char * data;
    int expected_size;
    int cur_expected_size;
    int cur_size;
    
    int worker_id;
    
    int tag;
    int cur_tag_size;
    
    DataForSend(char * data, int expected_size, int worker_id, int tag)
    : data(data), expected_size(expected_size), cur_expected_size(0), cur_size(0), 
      worker_id(worker_id), tag(tag), cur_tag_size(0)
    {
    }
    
    bool is_ready() const
    {
        return (cur_tag_size == sizeof(int) && cur_expected_size == sizeof(int) && cur_size == expected_size);
    }
};

struct SocketsConnectionClient: public IConnectionClient
{
    int num_workers;
    int worker_id;
    int port;
    char * addr;
    
    std::map<int, ConnectionIn*> in_connections; // access only for listener
    std::map<int, int> out_connections; // access only for sender
    
    std::map<int, char*> recv_ready; // access for worker and for listener
    std::mutex recv_ready_m;
    
    std::queue<DataForSend*> send_queue; // access for worker and for sender
    std::mutex send_queue_m;
    std::condition_variable cv;
    
    std::set<int> send_ready; // access for worker and for sender
    std::mutex send_ready_m;
    
    void init(int argc, char ** argv)
    {
        if (argc != 6)
        {
            fprintf(stderr, "I can't start, wrong argc!\n");
            exit(1);
        }
        // argv[2] - number of workers
        num_workers = atoi(argv[2]);
        // argv[3] - id of this worker
        worker_id = atoi(argv[3]);
        // argv[4] - addres
        addr = argv[4];
        // argv[5] - port
        port = atoi(argv[5]);
        
        std::thread l_thread(listener, this);
        std::thread s_thread(sender, this);
        
        l_thread.detach();
        s_thread.detach();
        
        sleep(2);
        
        printf("it started\n");
    }
    
    int get_num_threads()
    {
        return num_workers;
    }
    
    int get_thread_id()
    {
        return worker_id;
    }
    
    void send(int * data, int size, int thread_id, int tag)
    {
        IRequest * request = async_send(data, size, thread_id, tag);
        request->wait();
    }
    
    void recv(int * buf, int size, int thread_id, int tag)
    {
        IRequest * request = async_recv(buf, size, thread_id, tag);
        request->wait();
    }
    
    IRequest * async_recv(int * buf, int size, int thread_id, int tag)
    {
        assert(thread_id < num_workers);
        assert(thread_id >= 0);
        RecvRequest * request = new RecvRequest(tag, (char*)buf, size, this);
        return request;
    }
    
    IRequest * async_send(int * buf, int size, int thread_id, int tag)
    {
        DataForSend * data = new DataForSend((char*)buf, size, thread_id, tag);
        
        send_queue_m.lock();
        send_queue.push(data);
        send_queue_m.unlock();
        cv.notify_all();
        
        SendRequest * request = new SendRequest(tag, this);
        return request;
    }
    
    void finalize()
    {
        
    }
};


// recv and listener

RecvRequest::RecvRequest(int tag, char * buf, int size, SocketsConnectionClient * connection)
: tag(tag), buf(buf), size(size), cur_size(0), connection(connection), ready_buf(0)
{
}
    
bool RecvRequest::test()
{
    connection->recv_ready_m.lock();
    if (connection->recv_ready.find(tag) == connection->recv_ready.end())
    {
        connection->recv_ready_m.unlock();
        return 0;
    }
    
    connection->recv_ready_m.unlock();
    
    if (!ready_buf)
        ready_buf = connection->recv_ready[tag];
    
    if (cur_size < size)
    {
        int len = std::min(KB, size - cur_size);
        memcpy(buf + cur_size, ready_buf + cur_size, len);
        cur_size += len;
    }
    
    if (cur_size == size)
        return 1;
    
    return 0;
}
    
void RecvRequest::wait()
{
    while (!test())
        pthread_yield();
}

RecvRequest::~RecvRequest()
{
    delete ready_buf;
}

// listener
void add_client(int c_fd, int e_fd, SocketsConnectionClient * connection)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = c_fd;
    epoll_ctl(e_fd, EPOLL_CTL_ADD, c_fd, &ev);
    
    connection->in_connections[c_fd] = new ConnectionIn();
}

void finalize_recv(ConnectionIn * input, SocketsConnectionClient * connection)
{
    connection->recv_ready_m.lock();
    connection->recv_ready[input->tag] = input->buf;
    connection->recv_ready_m.unlock();
    
    fprintf(stderr, "recv %d finalized\n", input->tag);
    input->clear();
}

void disconnect_client(int c_fd, int e_fd, SocketsConnectionClient * connection)
{
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = c_fd;
    epoll_ctl(e_fd, EPOLL_CTL_DEL, c_fd, &ev);
    
    close(c_fd);
    
    delete connection->in_connections[c_fd];
    connection->in_connections.erase(c_fd);
}

bool read_int(int fd, int &cur_size, int * start)
{
    cur_size += recv(fd, start + cur_size, sizeof(int) - cur_size, MSG_DONTWAIT);
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        errno = 0;
        cur_size++;
        return 0;
    }
    check("recv int", errno, 1);
    
    if (cur_size < (int)sizeof(int))
        return 0;
    
    return 1;
}

void receive_message(int c_fd, SocketsConnectionClient * connection)
{
    ConnectionIn * input = connection->in_connections[c_fd];
    if (input->cur_tag_size < (int)sizeof(int))
    {
        if (!read_int(c_fd, input->cur_tag_size, &(input->tag)))
            return;
    }
    
    if (input->cur_expected_size < (int)sizeof(int))
    {
        if (!read_int(c_fd, input->cur_expected_size, &(input->expected_size)))
            return;
    }
    
    if (!(input->buf))
        input->buf = new char[input->expected_size];
    
    input->cur_size += recv(c_fd, input->buf + input->cur_size, input->expected_size - input->cur_size, MSG_DONTWAIT);
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        errno = 0;
        input->cur_size++;
    }
    check("recv int", errno, 1);
    
    if (input->cur_size == input->expected_size)
    {
        finalize_recv(input, connection);
    }
}

void listener(SocketsConnectionClient * connection)
{
    //~ SocketsConnectionClient * connection = (SocketsConnectionClient *)(arg);
    
    int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in listener_addr;
    listener_addr.sin_family = AF_INET;
    listener_addr.sin_addr.s_addr = inet_addr(connection->addr);
    listener_addr.sin_port = htons(connection->port + connection->worker_id);
    
    printf("%d\n", connection->port + connection->worker_id);
    
    int so_reuseaddr = 1;

    setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int));
    check("setsockopt", errno, 1);
    
    bind(listener_socket, (struct sockaddr*)(&listener_addr), sizeof(struct sockaddr_in));
    check("bind", errno, 1);
    
    listen(listener_socket, MAX_CLIENTS);
    check("listen", errno, 1);
    
    struct epoll_event ev, events[MAX_CLIENTS];
    
    int e_fd = epoll_create(MAX_CLIENTS);
    check("epoll_create", errno, 1);
    
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = listener_socket;
    epoll_ctl(e_fd, EPOLL_CTL_ADD, listener_socket, &ev);
    check("epoll_ctl", errno, 1);
    
    while (1)
    {
        int num = epoll_wait(e_fd, events, MAX_CLIENTS, 1);
        check("epoll_wait", errno, 1);
        
        for (int i = 0; i < num; ++i)
        {
            //~ printf("smth happened %d\n", num);
            //~ usleep(500);
            if (events[i].data.fd == listener_socket)
            {
                printf("%d: client connected\n", connection->worker_id);
                int c_fd = accept(listener_socket, NULL, NULL);
                check("accept", errno, 1);
                
                add_client(c_fd, e_fd, connection);
            }
            else if (events[i].events == EPOLLIN)
            {
                receive_message(events[i].data.fd, connection);
            }
            else
            {
                disconnect_client(events[i].data.fd, e_fd, connection);
                printf("client disconnected\n");
            }
        }
    }
}

// sender

SendRequest::SendRequest()
{
}

SendRequest::SendRequest(int tag, SocketsConnectionClient * connection)
: tag(tag), connection(connection)
{
}

bool SendRequest::test()
{
    return connection->send_ready.find(tag) != connection->send_ready.end();
}

void SendRequest::wait()
{
    while (!test())
    {
        pthread_yield();
    }
}

void init_connection(int worker_id, SocketsConnectionClient * connection)
{
    int c_fd = socket(AF_INET, SOCK_STREAM, 0);
        
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(connection->addr);
    addr.sin_port = htons(connection->port + worker_id);
    
    connect(c_fd, (struct sockaddr*)(&addr), sizeof(sockaddr_in));
    check("connect", errno, 1);
    
    connection->out_connections[worker_id] = c_fd;
}

bool send_int(int fd, int &cur_size, int * start)
{
    cur_size += send(fd, start + cur_size, sizeof(int) - cur_size, MSG_DONTWAIT);
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        errno = 0;
        cur_size++;
        return 0;
    }
    check("send int", errno, 1);
    
    if (cur_size < (int)sizeof(int))
        return 0;
    
    return 1;
}

void send_part(DataForSend * data, SocketsConnectionClient * connection)
{
    int fd = connection->out_connections[data->worker_id];
    
    if (data->cur_tag_size < (int)sizeof(int))
        if (!send_int(fd, data->cur_tag_size, &(data->tag)))
            return;
    
    if (data->cur_expected_size < (int)sizeof(int))
        if (!send_int(fd, data->cur_expected_size, &(data->expected_size)))
            return;
    
    data->cur_size += send(fd, data->data + data->cur_size, data->expected_size - data->cur_size, MSG_DONTWAIT);
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        errno = 0;
        data->cur_size++;
    }
    check("send", errno, 1);
}

void sender(SocketsConnectionClient * connection)
{
    while (1) 
    {
        while (1)
        {
            DataForSend * data;
            {
                std::lock_guard<std::mutex> lg(connection->send_queue_m);
                if (connection->send_queue.empty())
                    break;
                
                
                data = connection->send_queue.front();
            }
            
            if (connection->out_connections.find(data->worker_id) == connection->out_connections.end())
                init_connection(data->worker_id, connection);
            
            send_part(data, connection);
            
            if (data->is_ready())
            {
                connection->send_queue_m.lock();
                connection->send_queue.pop();
                connection->send_queue_m.unlock();
                
                connection->send_ready_m.lock();
                connection->send_ready.insert(data->tag);
                connection->send_ready_m.unlock();
            }
        }
        
        std::unique_lock<std::mutex> ul(connection->send_queue_m);
        while (connection->send_queue.empty())
            connection->cv.wait(ul);
        ul.unlock();
    }
}
