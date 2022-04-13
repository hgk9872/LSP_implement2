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

// Queue를 이용한 디렉토리 탐색을 위한 노드 정의
typedef struct Node
{
    char path[PATHMAX];
    struct Node *next;
}Node;

// 디렉토리 탐색을 위한 Queue 구조체 정의
typedef struct Queue
{
    Node *front;
    Node *rear;
    int count;
}Queue;

void InitQueue(Queue *queue);
int isEmpty(Queue *queue);
void Enqueue(Queue *queue, char *path);
char *Dequeue(Queue *queue);


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
    char pathname[PATHMAX];
    struct stat statbuf;
    struct dirent** namelist;
    char *homeptr;
    int i;
    int count;


    // 입력 관련 에러 처리
    if (argc != ARGMAX) {
        printf("ERROR: Arguments error\n");
        return;
    }

    // 만약 홈디렉토리가 경로에 포함된 경우 
    if ((homeptr = strchr(argv[4], '~')) != NULL) {
        sprintf(dirname, "%s%s", getenv("HOME"), homeptr + 1);
        if(access(dirname, F_OK) < 0){
            fprintf(stderr, "access error for %s\n", dirname);
            return;
        }
    }
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

    printf("rootPath : %s\n", dirname); // 파일의 절대경로 확인

    // Queue를 사용해서 BFS 디렉토리 탐색 구현
    Queue queue;

    InitQueue(&queue);
    Enqueue(&queue, dirname); // 입력받은 경로를 root node로 시작

    // Queue가 비어있을 때까지 반복
    while (!isEmpty(&queue))
    {
        strcpy(dirname, Dequeue(&queue));
        printf("Queue Path : %s\n", dirname);

        // 꺼낸 큐의 디렉토리의 내부 파일목록을 불러오기
        if((count = scandir(dirname, &namelist, NULL, alphasort)) == -1) {
            fprintf(stderr, "scandir for %s\n", pathname);
            exit(1);
        }

        for (i = 0; i < count; i++) {
            if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
                continue;

            if (!strcmp(dirname, "/")) // 경로가 루트인 경우
                sprintf(pathname, "%s%s", dirname, namelist[i]->d_name);
            else
                sprintf(pathname, "%s/%s", dirname, namelist[i]->d_name);

            if ((lstat(pathname, &statbuf) < 0) && (!access(pathname, F_OK))) {
                fprintf(stderr, "lstat error for %s\n", pathname);
                exit(1);
            }

            if (S_ISDIR(statbuf.st_mode)) // 폴더인 경우 큐에 추가
                Enqueue(&queue, pathname);
//            else if (S_ISREG(statbuf.st_mode))
//                printf("filepath : %s\n", pathname);

        }
    }

    return;

}

// Queue 초기화하는 함수
void InitQueue(Queue *queue)
{
    queue->front = queue->rear = NULL; // 큐 맨앞과 맨끝 NULL 초기화
    queue->count = 0; // 보관 개수 0으로 초기화
}

// Queue 비어있는지 확인하는 함수
int isEmpty(Queue *queue)
{
    return queue->count == 0; // 보관 개수 0이면 1리턴
}

// Queue에 노드 추가하는 함수
void Enqueue(Queue *queue, char *path)
{
    Node *now = (Node *)malloc(sizeof(Node));
    strcpy(now->path, path);
    now->next = NULL;

    // 큐가 비어있을 때
    if (isEmpty(queue))
    {
        queue->front = now; // 맨 앞에 노드 추가
    }
    else // 비어있지 않을 때
    {
        queue->rear->next = now; // 맨 뒤에 노드 추가
    }
    queue->rear = now;
    queue->count++;
}

// 큐의 맨 앞의 원소를 뽑아 값을 리턴받음
char *Dequeue(Queue *queue)
{
    static char path[PATHMAX];  // 지역변수를 리턴해야하는데, 일단 static 사용
    Node *now;

    if (isEmpty(queue)) // 큐가 비어있는 경우
    {
        return NULL;
    }

    now = queue->front;
    strcpy(path, now->path);
    queue->front = now->next;
    free(now);
    queue->count--;
    return path;
}
    




