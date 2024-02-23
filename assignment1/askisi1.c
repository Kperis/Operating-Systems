#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    //check how many arguments were passed. If different than 1 then error.
    if(argc == 2){
        if(strcmp(argv[1],"--help") == 0){
        printf("Usage: %s filename\n", argv[0]);
        return 0;
        }

        struct stat buffer;
        int does_exist = stat(argv[1],&buffer);  //using stat to determine if file exists
                                                //does exist becomes 0 if file exists otherwise -1
        
        if(!does_exist){
            printf("Error: %s already exists\n", argv[1]);
            return 1;
        }

        int status;
        pid_t child;
        int fd = open(argv[1], O_CREAT| O_APPEND| O_WRONLY,0644);  //mode 0abc-> a permission for owner
                                                                   //b permission for the group
                                                                   //c permission to others
                                                                   //6 rw, 4 read only 
        if(fd == -1){
            perror("Error\n");
            return 1;
        }

        child = fork();
        char buf[50];
        if(child < 0){
            perror("Error\n");
            return 1;
        }
         
        //code for child (because fork returns 0 for child)
        if(child == 0){
            sprintf(buf,"[CHILD] getpid() = %d, getppid() = %d\n", getpid(), getppid()); 
            if(write(fd, buf, strlen(buf)) < strlen(buf)){
                perror("write\n");
                return 1;
            }
            exit(0);
        }

        //code for father
        else{
            wait(&status);
            sprintf(buf,"[PARENT] getpid() = %d, getppid() = %d\n", getpid(), getppid()); 
            if(write(fd, buf, strlen(buf)) < strlen(buf)){
                perror("write\n");
                return 1;
            }
            exit(0);
        }
        close(fd);
    }   
    else{
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }
    return 0;
}