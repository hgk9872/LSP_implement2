#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define BUFMAX 1024
#define ARGMAX 5

int split(char *input, char *delimiter, char* argv[]);

int main(void)
{
    int argc = 0;
    char input[BUFMAX];
    char *argv[ARGMAX];
    pid_t pid;
    int status;

    //프롬프트 시작
    while (1) {
        printf("20192209> ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        argc = split(input, " ", argv);

        if (argc == 0)
            continue;

        else if (strcmp(argv[0], "exit") == 0) {
            printf("Prompt End\n");
            break;
        }

        if((pid = fork()) < 0) {
            fprintf(stderr, "fork error");
        }

        else if(pid == 0) { //자식의 경우

            if (strcmp(argv[0], "fmd5") == 0) {
                char *args[] = {"./ssu_find-md5", argv[0], argv[1], argv[2], argv[3], argv[4], NULL};
                execv(args[0], args);
                exit(0);
            }

            else if (strcmp(argv[0], "fsha1") == 0) {
                char *args[] = {"./ssu_find-sha1", argv[0], argv[1], argv[2], argv[3], argv[4], NULL};
                execv(args[0], args);
                exit(0);
            }

            else {
                char *args[] = {"./ssu_help", NULL};
                execv(args[0], args);
                exit(0);
            }
        }

        if((pid = waitpid(pid, &status, 0)) < 0)
            fprintf(stderr, "waitpid error");
    }

    exit(0);
}

// 입력받은 문자열 분리하여 argv[]에 인자로 입력하는 함수
int split(char *input, char *delimiter, char *argv[]) 
{
    int argc = 0;
    char *ptr = NULL;

    ptr = strtok(input, delimiter);
    while (ptr != NULL) {
        argv[argc++] = ptr;
        ptr = strtok(NULL, " ");
    }

    return argc;
}
