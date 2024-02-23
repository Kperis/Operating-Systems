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


void check_failure(int val, const char *msg) {
  if (val < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

bool isNumber (char buf[]) {
   for (int i = 0; i < strlen(buf); i++){
        if (isdigit(buf[i]) == false){
            return false; 
        }
   }
   if(strlen(buf) == 0){
        return false;
   }
   return true;
}

int find_max(int a, int b){
    if(a>b){
        return a;
    }
    else if(b>a){
        return b;
    }
    return a;
}

int main(int argc, char* argv[]){
    //Check number of arguments
    if(argc != 2 && argc != 3){
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }
    //help command for terminal
    if(strcmp("--help", argv[1]) == 0){
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        return 1;
    }
    int n = atoi(argv[1]);
    if(n == 0){
        printf("Please enter valid number of children\n"); //check if user entered number
    }
    int mode;
    switch (argc)
    {
        case 2:
            mode = 1; // mode 1 for round robin(default)
            break;
        case 3:
            if(strcmp("--round-robin", argv[2]) == 0){
                mode = 1;
            }
            else if(strcmp("--random", argv[2]) == 0){
                mode = 2;
            }
            else{
                printf("Please enter valid mode\n");
                return 1;
            }    
            break;
    }
    

    pid_t* children_arr;
    children_arr = (pid_t*)malloc(n*sizeof(pid_t));
    pid_t child;
    int readpd[n][2];
    int writepd[n][2];


    
    for(int  i = 0; i < n; i++){
        if(pipe(&readpd[i][0]) != 0){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if(pipe(&writepd[i][0]) != 0){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < n; i++){
        child = fork();
        check_failure(child,"fork");
        if(child == 0){      //CHILD CODE
            int val;
            
            close(writepd[i][0]);
            close(readpd[i][1]);

            while(true){
                int fd_input = read(readpd[i][0], &val,sizeof(int));
                if(fd_input == -1){
                    perror("read");
                    exit(-1);
                }
                printf("[CHILD %d]: [%d] Child received %d!\n", i, getpid(), val);
                val++;
                sleep(5);

                int fd_output = write(writepd[i][1], &val, sizeof(int));

                if(fd_output == -1){
                    perror("write");
                    exit(-1);
                }
                printf("[CHILD %d]: [%d] Child finished working hard, writing back %d!\n", i, getpid(), val);

            }

        } // END OF CHILD CODE
        children_arr[i] = child;
    }

    for(int i = 0; i < n; i++){
        close(readpd[i][0]);
        close(writepd[i][1]);
    }
    
    int counter = 0;

    while(true){
        fd_set inset;
        int maxfd;
        FD_ZERO(&inset);
        FD_SET(STDIN_FILENO, &inset);
        for(int i = 0; i<n; i++){
            FD_SET(writepd[i][0],&inset);
        }

        maxfd = STDIN_FILENO;
        for(int i = 0; i < n; i++){
            maxfd = find_max(maxfd,writepd[i][0]);
        }
        maxfd = maxfd+1;
        int ready = select(maxfd,&inset,NULL,NULL,NULL); //select(number of fd, fdset read, fdset write, error, timeout)
        if(ready <= 0){
            perror("select");
            continue;
        }
    
        if(FD_ISSET(STDIN_FILENO,&inset)){
            char buffer[101];
            int count_read = read(STDIN_FILENO,buffer,100);

            if (count_read == -1) {
                    perror("read");
                    exit(-1);
            }
            buffer[count_read] = '\0';  

            if (count_read > 0 && buffer[count_read-1] == '\n') {
                    buffer[count_read-1] = '\0';
            }

            if(count_read >= 4 && strcmp(buffer,"exit") == 0){
                for(int k = 0; k < n; k++){
                    int child_info = waitpid(children_arr[k], NULL, WNOHANG);
                    if(child_info == 0){
                        kill(children_arr[k],SIGTERM);
                        printf("[Father proccess %d]: Terminating child proccess %d with pid %d\n",getpid(), k, children_arr[k]);
                        wait(NULL);
                    }
                    close(writepd[k][0]);
                    close(readpd[k][1]);
                }
                exit(0);
            }
            else if(count_read >=1 && isNumber(buffer)){
                printf("%s",buffer);
                int num = atoi(buffer);
                if(mode == 1){ //round robin
                    int assigned_to = counter%n;
                    int written = write(readpd[assigned_to][1], &num, sizeof(int));
                    if(written == -1){
                        perror("write");
                        exit(-1);
                    }
                    printf("[Parent] Assigned %d to child %d.\n", num, assigned_to);
                    counter++;
                }
                else if(mode == 2){
                    int random = rand()%n;
                    int written = write(readpd[random][1], &num, sizeof(int));
                    if(written == -1){
                        perror("write");
                        exit(-1);
                    }
                    printf("[Parent] Assigned %d to child %d.\n", num, random);
                }   
            }
            else if(count_read >= 1){
                printf("Type a number to send job to a child!\n");
            }   
        }
    
        for(int i = 0; i < n; i++){
            if(FD_ISSET(writepd[i][0], &inset)){
                int output;
                int read_bytes = read(writepd[i][0], &output, sizeof(int));
                if(read_bytes == -1){
                    perror("read");
                    exit(-1);
                }
                printf("[Parent] Received result from child %d ----> %d.\n", i, output);

            }
        }

    }
    return 0;
    
}