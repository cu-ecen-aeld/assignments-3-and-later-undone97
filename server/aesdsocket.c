#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>


#define PORT    9000

char* output_file_path = "/var/tmp/aesdsocketdata";

sig_atomic_t volatile stop = 1;

void sig_handler(int signum)
{
    fprintf(stderr,"Got Interrupted by signal");
    if (signum == SIGINT || signum == SIGTERM)
        stop = 0;
        syslog(LOG_NOTICE, "Caught signal, exiting");
        if (remove(output_file_path) < 0) {
            syslog(LOG_ERR, "Failed to remove the file at %s upon termination, error: %s", output_file_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
        /*TODO: Gracefully close all fp and sockets*/

}




void read_socket_to_file(int socketd, int fd) {
    char* ptr = NULL;
    char* tmp_ptr = NULL;
    int chunk_size = 512;
    int allocated_memory=0;
    int bytes_read=0;
    int total_read=0;
   
    while(1) {
        if (allocated_memory-total_read < chunk_size >> 2) {
            tmp_ptr = realloc(ptr,allocated_memory+chunk_size);
            if (tmp_ptr==NULL) {
                syslog(LOG_ERR, "Failed to malloc, error %d", errno);
                exit(EXIT_FAILURE);
            }
            ptr=tmp_ptr;
            allocated_memory+=chunk_size;
        }

        bytes_read=read(socketd,ptr+total_read,allocated_memory-total_read);
        if (bytes_read<0) {
            syslog(LOG_ERR, "Error while reading from socket, error %d", errno);
            exit(EXIT_FAILURE);
        }
        total_read+=bytes_read;     
    
     if (ptr[total_read-1] == '\n') {
            int bytes_wrote;
            int bytes_wrote_total=0;
            int bytes_to_write=total_read;
            fprintf(stderr,ptr);
            while((bytes_wrote=write(fd,ptr+bytes_wrote_total,bytes_to_write))<bytes_to_write){
                if (bytes_wrote < 0) {
                    syslog(LOG_ERR, "Failure to write to file, error %d", errno);
                    exit(EXIT_FAILURE);
                }
                bytes_to_write-=bytes_wrote;
                bytes_wrote_total+=bytes_wrote;
            }
            break;
        }
    }
    free(ptr);   
}


void write_file_to_socket(int conn_socket,int fd) {

    off_t current_file_offset = lseek(fd, 0, SEEK_CUR);
    if (current_file_offset < 0) {
        syslog(LOG_ERR, "cant find current offset error %d", errno);
        exit(EXIT_FAILURE);
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        syslog(LOG_ERR, "Failed to set file pointer to beginning error %d", errno);
        exit(EXIT_FAILURE);
    }

    char buffer[512] = {0};
    int bytes_read = 0;
    while((bytes_read = read(fd, buffer, 512)) > 0) {
        int current_chunk_size = bytes_read;
        int write_ptr = 0;

        while (current_chunk_size > 0) {
            int bytes_wrote = write(conn_socket, buffer + write_ptr, current_chunk_size);
            if (bytes_wrote < 0) {
                syslog(LOG_ERR, "Failed to write to socket, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            write_ptr += bytes_wrote;
            current_chunk_size -= bytes_wrote;
        }

    }
    if (bytes_read < 0) {
        syslog(LOG_ERR, "Failed reading output file, error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, current_file_offset, SEEK_SET) < 0) {
        syslog(LOG_ERR, "Could not set output file offset to its original value, error %d", errno);
        exit(EXIT_FAILURE);
    }
}




int main(int argc, char* argv[]) {
    openlog("aesdsocket", LOG_CONS | LOG_PERROR | LOG_PID, LOG_USER);
    int run_as_daemon=false;
    if (argc == 2 && strcmp(argv[1],"-d")==0) { /*Run as Daemon*/
        run_as_daemon=true;
    }

    signal(SIGINT, &sig_handler);
    signal(SIGTERM,&sig_handler);

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) { 
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(socket_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address))< 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(run_as_daemon) {
        pid_t sid = 0;
        pid_t pid=fork();

        if (pid<0){
            syslog(LOG_ERR, "Failed to spawn a child process, errno %d",errno);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) {
            syslog(LOG_NOTICE, "Spawn daemon process with PID %d, EXITING", pid);
            exit(EXIT_SUCCESS);
        }
        else { /*need to close on daemon: stdin,stdou,stderr so it doesnt spam console*/
            if (close(0) < 0) {
                syslog(LOG_ERR, "Failed to close stdin on daemon process, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (close(1) < 0) {
                syslog(LOG_ERR, "Failed to close stdout on daemon process, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (close(2) < 0) {
                syslog(LOG_ERR, "Failed to close stderr on daemon process, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            /*need to redirect stdin,out,err to /dev/null*/
            int null_fd = open("/dev/null", O_APPEND|O_RDWR);
            if (null_fd < 0) {
                syslog(LOG_ERR, "Could not open /dev/null to redirect std streams, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(null_fd, 0) < 0) {
                syslog(LOG_ERR, "Failed redirect stdin, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(null_fd, 1) < 0) {
                syslog(LOG_ERR, "Failed redirect stdout, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (dup2(null_fd, 2) < 0) {
                syslog(LOG_ERR, "Failed redirect stderr, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }

            sid = setsid();
            if (sid < 0) {
                syslog(LOG_ERR, "Failed to set SID daemon, error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            chdir("/");
        }

    }
    if (listen(socket_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int conn_socket=0;
    while ((conn_socket = accept(socket_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) > 0) {
        int fd = open(output_file_path, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd < 0) {
            perror("Could not open/create output file");
            exit(EXIT_FAILURE);
        }

        read_socket_to_file(conn_socket,fd);
        if (fsync(fd) < 0) {
            syslog(LOG_ERR, "Failed to fsync file");
            exit(EXIT_FAILURE);
        }
        write_file_to_socket(conn_socket,fd);
        close(conn_socket);
        if (fsync(fd) < 0) {
            syslog(LOG_ERR, "Failed to fsync file");
            exit(EXIT_FAILURE);
        }
        close(fd);
        close(conn_socket);
    }
    close(socket_fd);
    if (remove(output_file_path) < 0) {
        syslog(LOG_ERR, "Failed to remove the file, error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}