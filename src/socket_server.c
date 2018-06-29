#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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


FILE *fp_video = NULL;
FILE *fp_audio = NULL;

void *createVideoSocket(void *ptr) {
	pthread_detach(pthread_self());    
	struct sockaddr_un bind_addr;
	struct sockaddr_un client_addr;
	socklen_t client_len;
	int client_fd = -1;
	int listenfd;
	static const char *socket_path = "/data/run/ios_mirroring_video"; 

	if(unlink(socket_path) == -1 && errno != ENOENT) {
		printf("H264 unlink(%s) failed: %s\n", socket_path, strerror(errno));
		abort();
	}

	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd == -1) {
		printf("H264 socket() failed: %s\n", strerror(errno));
		abort();
	}

	int flags = fcntl(listenfd, F_GETFL, 0);
	if (flags < 0) {
		printf("H264 ERROR: Could not get flags for socket\n");
	} else {
		if (fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) < 0) {
			printf("H264 ERROR: Could not set socket to non-blocking\n");
		}
	}

	bzero(&bind_addr, sizeof(bind_addr));
	bind_addr.sun_family = AF_UNIX;
	strcpy(bind_addr.sun_path, socket_path);
	if (bind(listenfd, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
		printf("H264 bind() failed: %s\n", strerror(errno));
		abort();
	}

	// Start listening
	if (listen(listenfd, 5) != 0) {
		printf("H264 listen() failed: %s\n", strerror(errno));
		abort();
	}

	chmod(socket_path, 0666);

	ssize_t recv_length = 0;
	//char recv_buff[1024] = {0};
    char *recv_buff = (char*) malloc(1024*1024);
	memset(recv_buff, 0, sizeof(1024*1024));
	while (true) {
		//accept client connect
		if (client_fd < 0) {
			printf("H264 accept client connect\n");
            bzero(&client_addr, sizeof(client_addr));
			client_len = sizeof(client_addr);
			client_fd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
		} else {
			//memset(recv_buff, 0, sizeof(recv_buff));
			printf("H264 recv start\n");
			recv_length = recv(client_fd, recv_buff, 1024*1024 ,0);
			printf("H264 recv start________________recv_length: %d, recv_total: %d, ret: %s \n", recv_length, 1024*1024, strerror(errno));
			if (recv_length < 0) {
				//no data read
				if (errno == EAGAIN || errno == EINTR) {
					printf("H264 no data read");
					continue;
				} else {
					printf("H264 ERROR recv failed cause: %s\n", strerror(errno));//for detail message need analyse
                    client_fd = -1;
                    continue;
				}
			} else if (0 == recv_length) {
				//client close fd
                client_fd = -1;
				printf("H264 recv() failed: %s\n", strerror(errno));
                
			} else if (recv_length == 1024*1024) {
				printf("H264 read again\n");
                abort();
				fwrite(recv_buff, recv_length, 1, fp_video);
			} else if (recv_length > 1024*1024) {//maybe not happend
				printf("H264 ERROR too much data recv buff over\n");
                abort();
			    client_fd = -1;
            } else {
            	fwrite(recv_buff, recv_length, 1, fp_video);
                memset(recv_buff, 0, sizeof(1024*1024)); 
                // if ( 0 == strncmp(recv_buff, "video", 5) ) {
                //     printf("write video file\n");
                //     fwrite(recv_buff + 12, recv_length, 1, fp_video);
                //     memset(recv_buff, 0, sizeof(1024*1024)); 
                // } else if ( 0 == strncmp(recv_buff, "audio", 5) ) {
                //     printf("write audio file\n");
                //     fwrite(recv_buff + 12, recv_length, 1, fp_audio);
                //     memset(recv_buff, 0, sizeof(1024*1024));
                // } else {
                //     printf("other type :\n");
                    
                //     for (int i=0; i<8; i++) {
                //         printf("0x%02x ", recv_buff[i]);
                //     }
                //     printf("\n");
                //     abort();
                // }
                //fwrite(recv_buff, recv_length, 1, fp_video);
			}

		}
		usleep(100);
	}

ABORT_CASE:
    if (recv_buff) {
        free(recv_buff);
        recv_buff = NULL;
    }
    abort();
	return NULL;
}


void *createAudioSocket(void *ptr) {
	pthread_detach(pthread_self());    
	struct sockaddr_un bind_addr;
	struct sockaddr_un client_addr;
	socklen_t client_len;
	int client_fd = -1;
	int listenfd;
	static const char *socket_path = "/data/run/ios_mirroring_audio"; 

	if(unlink(socket_path) == -1 && errno != ENOENT) {
		printf("LPCM unlink(%s) failed: %s\n", socket_path, strerror(errno));
		abort();
	}

	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd == -1) {
		printf("LPCM socket() failed: %s\n", strerror(errno));
		abort();
	}
/*
	int flags = fcntl(listenfd, F_GETFL, 0);
	if (flags < 0) {
		printf("ERROR: Could not get flags for socket\n");
	} else {
		if (fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) < 0) {
			printf("ERROR: Could not set socket to non-blocking\n");
		}
	}
*/
	bzero(&bind_addr, sizeof(bind_addr));
	bind_addr.sun_family = AF_UNIX;
	strcpy(bind_addr.sun_path, socket_path);
	if (bind(listenfd, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
		printf("LPCM bind() failed: %s\n", strerror(errno));
		abort();
	}

	// Start listening
	if (listen(listenfd, 5) != 0) {
		printf("LPCM listen() failed: %s\n", strerror(errno));
		abort();
	}

	chmod(socket_path, 0666);

	ssize_t recv_length = 0;
	//char recv_buff[1024] = {0};
    char *recv_buff = (char*) malloc(1024*1024);
	memset(recv_buff, 0, sizeof(1024*1024));
	while (true) {
		//accept client connect
		if (client_fd < 0) {
			printf("LPCM accept client connect\n");
            bzero(&client_addr, sizeof(client_addr));
			client_len = sizeof(client_addr);
			client_fd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
		} else {
			//memset(recv_buff, 0, sizeof(recv_buff));
			printf("LPCM recv start\n");
			recv_length = recv(client_fd, recv_buff, 1024*1024 ,0);
			printf("LPCM recv start________________recv_length: %d, recv_total: %d, ret: %s \n", recv_length, 1024*1024, strerror(errno));
			if (recv_length < 0) {
				//no data read
				if (errno == EAGAIN || errno == EINTR) {
					printf("LPCM no data read");
					continue;
				} else {
					printf("LPCM ERROR recv failed cause: %s\n", strerror(errno));//for detail message need analyse
                    client_fd = -1;
                    continue;
				}
			} else if (0 == recv_length) {
				//client close fd
                client_fd = -1;
				printf("LPCM recv() failed: %s\n", strerror(errno));
                
			} else if (recv_length == 1024*1024) {
				printf("LPCM read again\n");
                abort();
				fwrite(recv_buff, recv_length, 1, fp_audio);
			} else if (recv_length > 1024*1024) {//maybe not happend
				printf("LPCM ERROR too much data recv buff over\n");
                abort();
			    client_fd = -1;
            } else {
            	fwrite(recv_buff, recv_length, 1, fp_audio);
                memset(recv_buff, 0, sizeof(1024*1024)); 
			}

		}
		usleep(100);
	}

ABORT_CASE:
    if (recv_buff) {
        free(recv_buff);
        recv_buff = NULL;
    }
    abort();
	return NULL;
}


int main(int agrc, char	** agrv) {

	int ret = 0;
	pthread_t video_socket_thread;
	pthread_t audio_socket_thread;

	fp_video = fopen("/data/ios_usb/wuli_ios_buffer.h264", "a+");
    fp_audio = fopen("/data/ios_usb/wuli_ios_buffer.lpcm", "a+");
	if (!fp_video) {
		printf("open video stream file error\n");
		abort();
	}   
    if (!fp_audio) {
        printf("open audio stream file error\n");
        abort();
    }

	ret = pthread_create(&video_socket_thread, NULL, createVideoSocket, NULL);
    printf("create video socket thread: %d\n", ret);
	ret = pthread_create(&audio_socket_thread, NULL, createAudioSocket, NULL);
	printf("create audio socket thread: %d\n", ret);
	while (getchar() != 27) {
		;  
	}
	//createSocket();   
	return 1;
}


