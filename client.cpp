#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <pthread.h>

using namespace std;
using std::vector;

const string server_ip = "172.24.15.237";
#define SERVER_PORT 9888
#define MAX_BUFSIZE 65535
#define MAX_SERVER_EVENT 256

  char maxsend[1000];
static void* sendthread(void *param)
{
  int sockid = *((int*)param);
  bzero(maxsend, sizeof(maxsend));
  while(cin.getline(maxsend, sizeof(maxsend)))
  {
    std::cout << maxsend << "  " << sizeof(maxsend) << " " << strlen(maxsend) << std::endl;
    int ret = ::send(sockid, maxsend, strlen(maxsend), 0);
    if (ret > 0)
      std::cout << "send success, ret:" << ret << std::endl;
    bzero(maxsend, sizeof(maxsend));
  }
  return 0;
}

int main()
{
  int sockid = 0;
  sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid < 0)
  {
    std::cout << "socket error" << std::endl;
    return -1;
  }

  struct timeval timeo;
  timeo.tv_sec = 0;
  timeo.tv_usec = 100000; //100 ms

  int len = sizeof(timeo);
  setsockopt(sockid, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

  sockaddr_in addr;
  bzero(&addr, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
  addr.sin_port = htons((uint16_t)SERVER_PORT);

  int ret = ::connect(sockid, (sockaddr*)&addr, sizeof(sockaddr_in));
  if (ret != 0)
  {
    std::cout << "connet error: " << ret << std::endl;
    return -1;
  }
  std::cout << "connect success!" << std::endl;

  // set send and recv
  int nBuf = MAX_BUFSIZE * 2;
  setsockopt(sockid,SOL_SOCKET,SO_RCVBUF,(const char*)&nBuf,sizeof(int));
  setsockopt(sockid,SOL_SOCKET,SO_SNDBUF,(const char*)&nBuf,sizeof(int));

  // set no block
  int flags = fcntl(sockid, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(sockid, F_SETFL, flags);

  int serverepid = 0;
  serverepid = epoll_create(MAX_SERVER_EVENT);
  epoll_event ev;
  bzero(&ev, sizeof(ev));
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
  //ev.data.ptr = this;
  epoll_ctl(serverepid, EPOLL_CTL_ADD, sockid, &ev);

  /*vector<int> testsend;
  testsend.push_back(1234);

  ret = ::send(sockid, &(testsend[0]), testsend.size() * sizeof(int), 0);
  if (ret > 0)
    std::cout << "send success, ret:" << ret << std::endl;
  */

  pthread_t pid;
  int pidret = pthread_create(&pid, NULL, &sendthread, (void*)(&sockid));
  if (pidret == 0)
  {
    std::cout << "创建线程成功, pid:" << (int)pid << std::endl;
    //sleep(1);
    //pthread_join(pid, NULL);
  }

  // listen
  struct epoll_event events[MAX_SERVER_EVENT];

  std::cout << "begin listen and send" << std::endl;
  while(true)
  {
    bzero(events, MAX_SERVER_EVENT * sizeof(epoll_event));
    int nfds = epoll_wait(serverepid, events, MAX_SERVER_EVENT, 20);
    for (int i = 0; i < nfds; ++i)
    {
      // recv data
      if (events[i].events & EPOLLIN)
      {
        char buffer[MAX_BUFSIZE];
        bzero(buffer,sizeof(buffer));

        int ret = ::recv(sockid, buffer, MAX_BUFSIZE, 0);
        std::cout << "socket recv data size: " << ret << std::endl;
        if (ret > 0 && ret < MAX_BUFSIZE)
        {
          std::cout << "data: ";
          for (int i = 0; i < ret; ++i)
            std::cout << " " << buffer[i];
          std::cout << std::endl;
        }
        std::cout << "接收字符串内容为: " << buffer << std::endl << std::endl;
      }
    }
  }

  return 0;
}

