#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>


FILE *fp = NULL;

void *createSocket(void *ptr) {
    pthread_detach(pthread_self());    

    struct sockaddr_un client_addr;
    socklen_t client_len;
    static const char *socket_path = "/data/run/ios_mirroring";     
    int client_fd;

RETRY:
    usleep(1000);
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        printf("socket() failed: %s \n", strerror(errno));
        goto RETRY;
	//abort();
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags < 0) {
        printf("ERROR: Could not get flags for socket\n");
    } else {
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            printf("ERROR: Could not set socket to non-blocking\n");
        }
    }

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, socket_path);
    if (connect(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) != 0) {
        printf("connect() failed: %s", strerror(errno));
        close(client_fd);
	goto RETRY;
	//abort();
    }

    ssize_t recv_length = 0;
    char recv_buff[4096] = {0};
    int exit_sign = 1;
    while (1) {
            memset(recv_buff, 0, sizeof(recv_buff));
            printf("recv start\n");
            recv_length = recv(client_fd, recv_buff, sizeof(recv_buff) ,0);
            printf("recv start________________recv_length: %d, recv_total: %d, ret: %s \n", recv_length, sizeof(recv_buff), strerror(errno));
            if (recv_length < 0) {
                    //no data read
                    if (errno == EAGAIN) {
                            printf("no data read");
                            exit_sign = 1;
                            continue;
                            //fclose(fp);
                            //break;
                    } else {
                            printf("ERROR recv failed cause: %s\n", strerror(errno));
                            close(client_fd);
                            goto RETRY;
                            //fclose(fp);
                            //return NULL;
                    }
            } else if (0 == recv_length) {
                    //server close fd
                    printf("recv() failed: %s\n", strerror(errno));
                    close(client_fd);
                    goto RETRY;
                    //close(client_fd);
                    //fclose(fp);
                    //return NULL;
            } else if (recv_length == sizeof(recv_buff)) {
                    printf("read again\n");
                    fwrite(recv_buff, recv_length, 1, fp);
            } else if (recv_length > sizeof(recv_buff)) {//maybe not happend
                    printf("ERROR too much data recv buff over\n");
                    close(client_fd);
                    goto RETRY;
                    //fclose(fp);
                    //return NULL;
            } else {
                    fwrite(recv_buff, recv_length, 1, fp);
            }
    }

    return NULL;
}


int main(int agrc, char	** agrv) {

        int ret = 0;
        pthread_t socket_thread;

   fp = fopen("/data/ios_usb/wuli_ios_buffer.h264", "a+");
   if (!fp) {
	printf("open stream file error\n");
	abort();
   }   
 
   ret = pthread_create(&socket_thread, NULL, createSocket, NULL);
   printf("create socket thread: %d\n", ret);
   while (getchar() != 27) {
  	;  
   }
   //createSocket();   
   return 1;
}


