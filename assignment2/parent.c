#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <stdbool.h>


pid_t * children_arr;
char *states;
int num_children;
int remaining_c;
bool final_terminate = false;



void check_failure(int val, const char *msg) {
  if (val < 0) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}



void sig_child_handler(int signum){
    int status;
    int index;
    if(!final_terminate){
        pid_t childid = waitpid(-1, &status, WUNTRACED);
        for(int i = 0; i<num_children; i++){
            if(children_arr[i] == childid){
                index = i;
            }
        }
        if(WIFSTOPPED(status)){
            printf("Child with PID %d stopped. Attempting to restart it\n" , children_arr[index]);
            kill(children_arr[index], SIGCONT);
        }
        else{
            printf("[PARENT/PID=%d] Child %d with PID=%d exited\n", getpid(),index ,childid);
            pid_t child = fork();
            check_failure(child,"fork");
            if(child == 0){
                char buffer[50];
                sprintf(buffer," %d",index);
                char* const arr[] = {"/child", buffer, states, NULL};
                execv("/home/kaladin/SXOLI/Leitourgika/askisi2/child",arr);
            }
            else{
                children_arr[index] = child; 
                printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), index, child, states[index]);
            }
        }
    }
    else{
        return;
    }
    return;
}



void sig_flip(int signum){
    for(int i = 0; i<num_children; i++){
        kill(children_arr[i], SIGUSR1);
    }
}



void sig_print(int signum){
    for(int i = 0; i<num_children; i++){
        kill(children_arr[i], SIGUSR2);
    }
}



void sig_term(int signum){
    remaining_c = num_children;
    final_terminate = true;
    int status;
    for(int i = 0; i<num_children; i++){
        kill(children_arr[i], SIGTERM);
        printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), remaining_c);
        pid_t childid = waitpid(-1, &status, 0);
        if(WIFEXITED(status)){
            printf("[PARENT/PID=%d] Child with PID=%d terminated successfully with exit status code %d\n", getpid(), children_arr[i], WEXITSTATUS(status));
        }
        else{
            perror("Child did not exit properly\n");
            exit(EXIT_FAILURE);
        }
        remaining_c--;
    }
    printf("[PARENT/PID=%d] All children exited, terminating as well\n", getpid());
    exit(0);

}



int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: %s with only one argument(for example ttf)\n", argv[0]);
        return 1;
    }
    if(strcmp("--help", argv[1]) == 0){
        printf("Usage: %s with only one argument(for example ttf)\n", argv[0]);
        return 1;
    }
    signal(SIGCHLD, sig_child_handler);
    signal(SIGUSR1, sig_flip);
    signal(SIGUSR2,sig_print);
    signal(SIGTERM, sig_term);
    num_children = strlen(argv[1]);
    for (int i = 0; i < num_children; i++){            // does string contains only f or t 
        if (argv[1][i] != 'f' && argv[1][i] != 't'){
            printf("Insert only 'f' and 't' characters \n");
            return 1;
        }
    }
    children_arr = (pid_t*)malloc(num_children*sizeof(pid_t));
    states = (char*)malloc(num_children*sizeof(char));
    pid_t child;
    int status;
    for(int i = 0; i < num_children; i++){
        sleep(0.009);
        child = fork();
        check_failure(child,"fork");
        if(child == 0){
            char buffer[50];
            sprintf(buffer," %d",i);
            char* const arr[] = {"/child", buffer, argv[1], NULL};
            execv("/home/kaladin/SXOLI/Leitourgika/askisi2/child",arr);
        }
        else{
            children_arr[i] = child;
            states[i] = argv[1][i];
            printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, child, argv[1][i]);
        }
       
    }
    while(true){
        
    }
}

