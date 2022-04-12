#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define PATHMAX 4096
#define NAMEMAX 255
#define BUFMAX 1024
#define ARGMAX 5

int split(char *input, char *delimiter, char* argv[]);
void command_help(void);
void fmd5(int argc, char *argv[]);

int main(void)
{

    int argc = 0;
    char input[BUFMAX];
    char *argv[ARGMAX];

    //프롬프트 시작
    while (1) {
        printf("20192209> ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        argc = split(input, " ", argv);

        if (argc == 0)
            continue;

        if (strcmp(argv[0], "fmd5") == 0)
            fmd5(argc, argv); // fmd5 함수 구현중...

        else if (strcmp(argv[0], "fsha1") == 0)
            printf("fsha1 호출\n"); // fsha1 함수 구현
    
        else if (strcmp(argv[0], "help") == 0)
            command_help();

        else if (strcmp(argv[0], "exit") == 0) {
            printf("Prompt End\n");
            break;
        }

        else 
            command_help();
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

void command_help(void)
{
    printf("Usage:\n");
    printf("    > fmd5/fsha1 [FILE_EXTENSION] [MINSIZE] [MAXSIZE] [TARGET_DIRECTORY]\n");
    printf("        >> [SET_INDEX] [OPTION ... ]\n");
    printf("           [OPTION ... ]\n");
    printf("           d [LIST_IDX] : delete [LIST_IDX] file\n");
    printf("           i : ask for comfirmation before delete\n");
    printf("           f : force delete except the recently modified file\n");
    printf("           t : force move to Trash except the recently modified file\n");
    printf("    > help\n");
    printf("    > exit\n\n");
}

// fmd5 해쉬값으로 파일 찾아서 중복파일 출력하는 함수
void fmd5(int argc, char *argv[]) 
{
    char dirname[PATHMAX];
    struct stat statbuf;
    char *homeptr;


    // 입력 관련 에러 처리
    if (argc != ARGMAX) {
        printf("ERROR: Arguments error\n");
        return;
    }

    // 만약 홈디렉토리가 경로에 포함된 경우 
    if ((homeptr = strchr(argv[4], '~')) != NULL) 
        sprintf(dirname, "%s%s", getenv("HOME"), homeptr + 1);
    else {  // 경로에 홈 디렉토리가 포함되지 않는 경우
        if (realpath(argv[4], dirname) == NULL) {
            printf("ERROR: Path exist error\n");
            return;
        }
    }

    // 폴더 stat 처리 에러
    if (lstat(dirname, &statbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", dirname);
        exit(1);
    }

    // 만약 폴더가 아니라면
    if (!S_ISDIR(statbuf.st_mode)) {
        printf("ERROR: Path must be directory\n");
        return;
    }

//    printf("%s\n", getenv("HOME"));
//    homeptr = strchr(argv[4], '~');
//    sprintf(dirname, "%s%s", getenv("HOME"), homeptr+1); 
    printf("AbsPath : %s\n", dirname);

    return;

}
