#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assignment1.h"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

People applicants[MAX_APPLICANTS];
int total_applic = 0;
int day_counts[MAX_APPLIC_PER_DAY] = {0};

int get_index(char *day) {
    if (strcmp(day, "monday") == 0) {
        return 0;
    } else if (strcmp(day, "tuesday") == 0) {
        return 1;
    } else if (strcmp(day, "wednesday") == 0) {
        return 2;
    } else if (strcmp(day, "thursday") == 0) {
        return 3;
    } else if (strcmp(day, "friday") == 0) {
        return 4;
    } else {
        return -1;
    }
}

void add_applicant(){
    printf("Enter your name: ");
    scanf("%s", applicants[total_applic].name);
    getchar();


    printf("Enter the days when you will work: ");
    fgets(applicants[total_applic].avail, MAX_AVAIL_LEN + 1, stdin);
    int len = strlen(applicants[total_applic].avail);
    if (len > 0 && applicants[total_applic].avail[len - 1] == '\n') {
        applicants[total_applic].avail[len - 1] = '\0'; 
    } 

    char avail_copy[MAX_AVAIL_LEN + 1]; 
    strcpy(avail_copy, applicants[total_applic].avail);

    char *day = strtok(avail_copy, " ");
    bool accepted = true;
    while (day != NULL) {
        int day_index = get_index(day);
        if (day_index == -1) {
            printf("Invalid day: %s\n", day);
            return;
        }

        if (day_counts[day_index] >= MAX_APPLIC_PER_DAY) {
            printf("Cannot accept more applications for %s.\n\n", day);
            accepted = false;
        } else {
            day_counts[day_index]++;
            
        }
        day = strtok(NULL, " ");
    }

    if (accepted){
        total_applic++;
        FILE *fp = fopen("data.txt", "a");
        fprintf(fp, "%-20s,%-10s\n", applicants[total_applic-1].name, applicants[total_applic-1].avail);
        fclose(fp);

        printf("Your application is submitted\n");
    } else {
        printf("One of the days was full, try to enter other days");
    }     
}

void delete_applicant(){
    if (total_applic == 0){
        printf("There are no applications to delete\n");
    } else {
        char name[MAX_NAME_LEN+1];
        printf("Enter the name of an applicant you want to delete: ");
        scanf("%s", name);
        getchar();
        int i;
        for(i = 0; i < total_applic; i++){
            if(strcmp(applicants[i].name, name) == 0){
                break;
            }
        }

        char *day = strtok(applicants[i].avail, " ");
        while (day != NULL) {
            int day_index = get_index(day);
            if (day_index != -1) {
                day_counts[day_index]--;
            }
            day = strtok(NULL, " ");
        }

        for(int j = i; j < total_applic-1; j++){
            applicants[j] = applicants[j+1];
        }

        total_applic -= 1;


        FILE *fp = fopen("data.txt", "w");
        for (int i = 0; i < total_applic; i++){
            fprintf(fp, "%-20s,%-10s\n", applicants[i].name, applicants[i].avail);
        }
        fclose(fp);
        printf("Your application is deleted\n");

    } 
}

#include <ctype.h>

char* trim(char* str) {
    char* end;

    // Trim leading whitespaces
    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0') 
        return str;

    // Trim trailing whitespaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Null-terminate the trimmed string
    *(end + 1) = '\0';
    return str;
}

void modify_applicant() {
    if (total_applic == 0) {
        printf("There are no applications to modify\n");
        return;
    }

    char name[MAX_NAME_LEN + 1];
    printf("Enter the name of an applicant you want to modify: ");
    scanf("%s", name);
    getchar();

    bool found = false;
    People temp_applicants[MAX_APPLICANTS];
    int temp_total_applic = 0;

    for (int i = 0; i < total_applic; i++) {
        if (strcmp(trim(applicants[i].name), trim(name)) == 0) {
            found = true;

            printf("Enter the new name: ");
            scanf("%s", temp_applicants[temp_total_applic].name);
            getchar();

            printf("Enter the new days when you would like to work: ");
            fgets(temp_applicants[temp_total_applic].avail, MAX_AVAIL_LEN + 1, stdin);
            int len = strlen(temp_applicants[temp_total_applic].avail);
            if (len > 0 && temp_applicants[temp_total_applic].avail[len - 1] == '\n') {
                temp_applicants[temp_total_applic].avail[len - 1] = '\0';
            }

            temp_total_applic++;
        } else {
            temp_applicants[temp_total_applic] = applicants[i];
            temp_total_applic++;
        }
    }

    if (!found) {
        printf("Applicant with name %s not found.\n", name);
        return;
    }

    FILE *fp = fopen("data.txt", "w");
    if (fp != NULL) {
        for (int i = 0; i < temp_total_applic; i++) {
            fprintf(fp, "%-20s,%-10s\n", temp_applicants[i].name, temp_applicants[i].avail);
        }
        fclose(fp);

        // Update the original array with the modified data
        for (int i = 0; i < temp_total_applic; i++) {
            applicants[i] = temp_applicants[i];
        }
        total_applic = temp_total_applic;

        printf("Your application is modified\n");
    } else {
        printf("Unable to open file for writing.\n");
        return;
    }
}



void print_applicants(){
    printf("%-20s %-10s\n", "Names", "Assigned Day");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < total_applic; i++) {
        printf("%-20s,%-10s\n", applicants[i].name, applicants[i].avail);
    }
}

struct Message { 
    long mtype;//this is a free value e.g for the address of the message
    char mtext [1024]; //this is the message itself
    int counter;
};

int send(int mqueue, const char* message_text, int counter) {
    struct Message message;
    message.mtype = 5;
    strncpy(message.mtext, message_text, sizeof(message.mtext) - 1);
    message.mtext[sizeof(message.mtext) - 1] = '\0';
    message.counter = counter;

    int status = msgsnd(mqueue, &message, sizeof(message) - sizeof(long), 0);
    if (status < 0) {
        perror("msgsnd error");
    }
    return 0; 
} 

int receive(int mqueue) {
    struct Message message;

    int status = msgrcv(mqueue, &message, sizeof(message) - sizeof(long), 5, 0);
    if (status < 0) {
        perror("msgrcv error");
    } else {
        printf("The %s has arrived with %d applicants\n", message.mtext, message.counter);
    }
    return 0;
}

void bus_handler(int signum) {
    if (signum == SIGUSR1) {
        printf("Signal for the first bus has arrived with number %i\n", signum);
    } else if (signum == SIGUSR2){
        printf("Signal for the second bus has arrived with number %i\n", signum);
    }
}


void start_bus() {
    char day[MAX_AVAIL_LEN];
    printf("Input the day to prepare the bus: ");
    scanf("%s", day);
    getchar();

    char applicants_names[MAX_APPLICANTS][MAX_NAME_LEN + 1];
    int num_applicants = 0; 
    for (int i = 0; i < total_applic; i++) {
        if (strstr(applicants[i].avail, day) != 0) {
            strncpy(applicants_names[num_applicants], applicants[i].name, MAX_NAME_LEN);
            applicants_names[num_applicants][MAX_NAME_LEN] = '\0';  // Add the null terminator
            num_applicants++;
        }
    }

    struct sigaction sigact;
    sigact.sa_handler = bus_handler; 
    sigemptyset(&sigact.sa_mask); 
    sigact.sa_flags = 0; 
    sigaction(SIGUSR1,&sigact, NULL);
    sigaction(SIGUSR2,&sigact, NULL);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    key_t key = ftok(".", 1);  // Generate a key using ftok
    if (key < 0) {
        perror("ftok error");
        exit(1);
    }

    int msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid < 0) {
        perror("msgget error");
        exit(1);
    }

    if (num_applicants == 0) {
        printf("There are no applicants for this day\n");
    }
    else if (num_applicants <= 5) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process 1
            printf("\nChild process 1 created!\n");

            // Send signal to parent process
            kill(getppid(), SIGUSR1);

            // Receive worker list from parent process
            close(pipe_fd[1]); // Close the write end of the pipe
            char worker_names[MAX_APPLICANTS][MAX_NAME_LEN + 1];
            int i = 0;
            while (read(pipe_fd[0], worker_names[i], MAX_NAME_LEN) > 0) {
                worker_names[i][MAX_NAME_LEN] = '\0'; // Add the null terminator
                i++;
            }
            close(pipe_fd[0]); 

            // Print worker list
            printf("Workers list received from parent process for the first bus:\n");
            for (int j = 0; j < i; j++) {
                printf("%s\n", worker_names[j]);
            }

            send(msgqid, "first bus", num_applicants);

            exit(1);
        } else if (pid > 0) {
            // Parent process

            sigset_t sigset;
            sigfillset(&sigset);
            sigdelset(&sigset,SIGUSR1);
            sigdelset(&sigset,SIGUSR2);
            sigsuspend(&sigset);

            close(pipe_fd[0]); //close the read end
            for (int i = 0; i < num_applicants; i++) {
                write(pipe_fd[1], applicants_names[i], MAX_NAME_LEN);
            }
            close(pipe_fd[1]);
            printf("The parent process has sent %d applicants to the child\n", num_applicants);

            receive(msgqid);

            waitpid(pid, NULL, 0);

            int status = msgctl(msgqid, IPC_RMID, NULL);
            if (status < 0) {
                perror("msgctl error");
            }
        } else {
            printf("Error creating child process.\n");
        }
    } else {
        pid_t pid1 = fork();
        pid_t pid2;

        if (pid1 == 0) {
            // Child process 1
            printf("\nChild process 1 created!\n");

            // Send signal to parent process
            kill(getppid(), SIGUSR1);

            // Receive worker list from parent process
            close(pipe_fd[1]); // Close the write end of the pipe
            char worker_names[MAX_APPLICANTS][MAX_NAME_LEN + 1];
            int i = 0;
            while (read(pipe_fd[0], worker_names[i], MAX_NAME_LEN) > 0) {
                worker_names[i][MAX_NAME_LEN] = '\0'; // Add the null terminator
                i++;
            }
            close(pipe_fd[0]); // Close the read end of the pipe

            // Print worker list
            printf("Workers list received from parent process:\n");
            for (int j = 0; j < i; j++) {
                printf("%s\n", worker_names[j]);
                
            }

            send(msgqid, "first bus", 5);

            exit(0);
        } else if (pid1 > 0) {
            // Parent process
            sigset_t sigset;
            sigfillset(&sigset);
            sigdelset(&sigset,SIGUSR1);
            sigdelset(&sigset,SIGUSR2);
            sigsuspend(&sigset);

            close(pipe_fd[0]); //close the read end
            for (int i = 0; i < 5; i++) {
                write(pipe_fd[1], applicants_names[i], MAX_NAME_LEN);
            }
            close(pipe_fd[1]);
            printf("The parent process has sent %d applicants to the child\n", 5);

            receive(msgqid);
            waitpid(pid1, NULL, 0);

            // Assign remaining applicants to the second bus
            int remaining_applicants = num_applicants - 5;
            if (remaining_applicants > 0) {
                pid2 = fork();

                if (pid2 == 0) {
                    // Child process 2
                    sleep(2);
                    printf("\nChild process 2 created!\n");
                    //sleep(1);
                    // Send signal to parent process
                    kill(getppid(), SIGUSR2);
                    sleep(1);

                    // Receive worker list from parent process
                    close(pipe_fd[1]); // Close the write end of the pipe
                    char second_worker_names[MAX_APPLICANTS][MAX_NAME_LEN + 1];
                    int i = 0;
                    while (read(pipe_fd[0], second_worker_names[i], MAX_NAME_LEN) > 0) {
                        second_worker_names[i][MAX_NAME_LEN] = '\0'; // Add the null terminator
                        i++;
                    }
                    close(pipe_fd[0]);

                    printf("Workers list received from parent process for the second bus:\n");
                    for (int j = 5; j < num_applicants; j++) {
                        printf("%s\n", applicants_names[j]);
                    }

                    send(msgqid, "second bus", remaining_applicants);
                    
                    exit(0);
            } else if (pid2 > 0) {
                // Parent process
                sigset_t sigset;
                sigfillset(&sigset);
                sigdelset(&sigset,SIGUSR2);
                sigsuspend(&sigset);

                // Read worker names from the pipe
                close(pipe_fd[0]); // Close the write end of the pipe

                for (int i = 5; i < remaining_applicants + 5; i++) {
                    write(pipe_fd[1], applicants_names[i], MAX_NAME_LEN);
                }
                close(pipe_fd[1]);
                printf("The parent process has sent %d applicants to the child\n", remaining_applicants);

                receive(msgqid);

                waitpid(pid2, NULL, 0);

                int status = msgctl(msgqid, IPC_RMID, NULL);
                    if (status < 0) {
                    perror("msgctl error");
                }


            } else {
                printf("Error creating child process.\n");
            } 
            }  
            
        } else {
            printf("Error creating child process.\n");
        }
    }

}


int main(int argc, char *argv[])
{
    FILE *fp = fopen("data.txt", "r");
    if (fp != NULL) {
        char line[MAX_NAME_LEN + MAX_AVAIL_LEN + 2];
        while (fgets(line, MAX_NAME_LEN + MAX_AVAIL_LEN + 2, fp) != NULL) {
            char *delim = strchr(line, ',');
            if (delim != NULL) {
                *delim = '\0';
                char *name = line;
                char *days = delim + 1;
                strncpy(applicants[total_applic].name, name, MAX_NAME_LEN);
                strncpy(applicants[total_applic].avail, days, MAX_AVAIL_LEN);
                total_applic++;
            }
        }
        fclose(fp);
    }

    int choice = 0;
    do {
        printf("\n\n -------------Program Menu--------------");
        printf("\n0.Exit");
        printf("\n1.Add applicant");
        printf("\n2.Delete applicant");
        printf("\n3.Modify applicant");
        printf("\n4.Print applicants");
        printf("\n5.Start the bus");
        printf("\n\nInput your choice: ");

        scanf("%d",&choice);
        if(choice > 5 || choice < 0) {
            printf("There is no such operation. Choose between 0-4");
        } else {
            switch (choice)
            {
            case 0:
                fp = fopen("data.txt", "w");
                for (int i = 0; i < total_applic; i++)
                {
                    fprintf(fp, "%-20s,%-10s\n", applicants[i].name, applicants[i].avail);
                }
                fclose(fp);
                return 0;
            case 1:
                system("clear");
                add_applicant();
                print_applicants();
                break;
            case 2:
                system("clear");
                delete_applicant();
                break;
            case 3:
                system("clear");
                modify_applicant();
                break;
            case 4:
                system("clear");
                print_applicants();
                break;
            case 5:
                system("clear");
                start_bus();
                break;
            default:
                break;
            }
        }
        
    }while(choice != 0);

    return 0;
    
}