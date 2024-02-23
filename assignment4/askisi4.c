#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <time.h>

// #define h_addr h_addr_list[0];

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool isNumber (char buf[]) {
   for (int i = 0; i < strlen(buf); i++){
        if ((isdigit(buf[i]) || buf[i]=='\n' || buf[i]==' ') == false){
            return false; 
        }
   }
   return true;
}

bool debug_on = false;

int main(int argc, char** argv){
        //Check number of arguments
    
    if(argc = 2){
        if(strncmp(argv[1],"--debug",strlen("--debug")) == 0){
            debug_on = true;
        }
    }
    //help command for terminal
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int sd = socket(domain,type,0);

    if(sd < 0){
        perror("SOCKET FAILED");
        return 1;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    int connection_with_client = bind(sd,(struct sockaddr *)&sin,sizeof(sin));

    if(connection_with_client < 0){
            perror("bind fail");
            return 1;
    }

    char hostname[] = "iot.dslab.pub.ds.open-cloud.xyz";
    struct hostent *hostp;
    hostp = gethostbyname(hostname);

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons((u_short) 18080);
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length);
    
    int is_connected = connect(sd,(struct sockaddr *)&sa,sizeof(sa));

    if(is_connected < 0){
        perror("connect error");
        return 1;
    }
    else{
        printf("Connecting\n");
        printf("Connected to iot.dslab.pub.ds.open-cloud.xyz:18080\n");
    }

    char buf[501];
    char *original_name;

    while(true){
        fd_set inset;
        int maxfd;

        FD_ZERO(&inset);
        FD_SET(STDIN_FILENO, &inset);
        FD_SET(sd,&inset);
        maxfd = MAX(STDIN_FILENO,sd) + 1;
        int ready_fd = select(maxfd, &inset,NULL,NULL,NULL);

        if(ready_fd <= 0){
            perror("select");
            continue;
        }

        if(FD_ISSET(STDIN_FILENO, &inset)){
            int bytes_read = read(STDIN_FILENO, buf, 500);
            if(bytes_read == -1){
                perror("read");
                exit(-1);
            }

            buf[bytes_read] = '\0';
            if(bytes_read > 0 && buf[bytes_read-1] == '\n'){
                buf[bytes_read-1] = '\0';
            }

            if(bytes_read >= 4 && strncmp(buf,"exit",4) == 0){
                int shut_check = shutdown(sd,2);
                if(shut_check < 0){
                    perror("shutdown");
                }
                close(sd);
                exit(0);
            }
            else if(bytes_read >= 3 && strncmp(buf,"get",3) == 0){
                int write_bytes = write(sd,buf,3);
                if(write_bytes < 0){
                    perror("write");
                    return(-1);
                }
                if(debug_on){
                    printf("[DEBUG] sent '%s'\n",buf);
                }
            }
            else if(bytes_read >= 4 && strncmp(buf, "help", 4) == 0){
                printf("Available commands:\n");
                printf("* 'help'                    : Print this help message\n");
                printf("* 'exit'                    : Exit\n");
                printf("* 'get'                     : Retrieve sensor data\n");
                printf("* 'N name surname reason'   : Ask permission to go out\n");
            }
            else if(bytes_read >=1){
                int write_bytes = write(sd,buf,bytes_read);
                if(write_bytes < 0){
                    perror("write");
                    return(-1);
                }
                if(debug_on){
                    printf("[DEBUG] sent '%s'\n", buf);
                }
            }

        }

        if(FD_ISSET(sd,&inset)){
            char server_buf[501];
            int read_bytes = read(sd,server_buf,500);
            if(strncmp(server_buf,"try again", strlen("try again")-1) == 0){
                printf("%s",server_buf);
                memset(server_buf,0,sizeof(server_buf));
                continue;
            }
            else if(read_bytes >= 1 && isNumber(server_buf)){
                if(debug_on){
                    printf("[DEBUG] read '%s'\n", server_buf);
                }

                for(int i = 0; i < 20; ++i){
                    printf("-");
                }
                printf("\n");
                char interval[40];
                char temperature[4];
                char light[40];
                char time[40];

                original_name = server_buf;
                strncpy(interval, original_name,1);
                if(atoi(interval) == 0){
                    printf("boot (0)\n");
                }
                if(atoi(interval) == 1){
                    printf("setup (1)\n");
                }
                if(atoi(interval) == 2){
                    printf("interval (2)\n");
                }
                if(atoi(interval) == 3){
                    printf("button (3)\n");
                }
                if(atoi(interval) == 4){
                    printf("motion (4)\n");
                }
                strncpy(light,original_name + 2,3);
                printf("Light level is: '%d'\n",atoi(light));

                strncpy(temperature, original_name + 6,4);
                printf("temperature is : %.2f\n",atof(temperature)/100.0);

                strncpy(time, original_name + 11, 10);
                time_t rawtime1 = atoi(time);
                struct tm *info;
                info = localtime(&rawtime1);
                printf("Current local time and date: %s", asctime(info));
                memset(server_buf, 0, sizeof(server_buf));
            }
            else if(read_bytes >=1 && strncmp(server_buf,"ACK",strlen("ACK")-1) == 0){
                if(debug_on){
                    printf("[DEBUG] read '%s'\n", server_buf);
                }

                printf("Response: %s\n",server_buf);
                memset(server_buf, 0, sizeof(server_buf));
            }
            else if(read_bytes >=1){
                if(debug_on){
                    printf("[DEBUG] read '%s'\n", server_buf);
                }
                printf("Send verification code: '%s'\n",server_buf);
                memset(server_buf,0,sizeof(server_buf));

            }
        }
    }
    close(sd);
    return 0;
}