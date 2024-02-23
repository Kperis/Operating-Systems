#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

time_t start,finish;
int timer = 0;
int child_num = 0;
bool gstatus;



void sig_alarm_handler(int signum){
    if(gstatus == true){
        printf("[GATE=%d/PID=%d/TIME=%ds] The gates are open\n", child_num, getpid(), timer);
        timer += 15;
        alarm(15);
    }
    else{
        printf("[GATE=%d/PID=%d/TIME=%ds] The gates are closed\n", child_num, getpid(), timer);
        timer += 15;
        alarm(15);
    }
    return;
}



void sig_flip(int signum){
    gstatus = !gstatus;
    finish = time(NULL);
    time_t total_time = finish-start;
    char* temp;
    if(gstatus){
        temp = "open";
    }
    else{
        temp = "closed";
    }
    printf("[GATE=%d/PID=%d/TIME=%lds] The gates are %s\n", child_num, getpid(), total_time, temp);
    return;
    
}



void sig_print_state(int signum){
    finish = time(NULL);
    time_t total_time = finish-start;
    char* temp;
    if(gstatus){
        temp = "open";
    }
    else{
        temp = "closed";
    }
    printf("[GATE=%d/PID=%d/TIME=%lds] The gates are %s\n", child_num, getpid(), total_time, temp);
    return;
}



void sig_term(int signum){
    exit(0);
}



int main(int argc, char *argv[]){
    sleep(0.05);
    start = time(NULL);
    signal(SIGALRM,sig_alarm_handler);
    signal(SIGUSR1, sig_flip);
    signal(SIGUSR2, sig_print_state);
    signal(SIGTERM,sig_term);
    int n = atoi(argv[1]);
    child_num = n;
    char gate = argv[2][n];       
    if(gate == 't'){
        gstatus = true;
    }
    else{
        gstatus = false;
    }
    if(gstatus == true){
        printf("[GATE=%d/PID=%d/TIME=%ds] The gates are open\n", child_num, getpid(), timer);
        timer += 15;
    }
    else{
        printf("[GATE=%d/PID=%d/TIME=%ds] The gates are closed\n", child_num, getpid(), timer);
        timer += 15;
    }
    alarm(15);
    while(true){
        
    }
}
