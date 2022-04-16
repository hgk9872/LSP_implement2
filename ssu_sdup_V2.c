#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <openssl/md5.h>

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

// fileinfo structure
typedef struct fileinfo {
    int count;
    size_t size;
    char hash[PATHMAX];
    char path[PATHMAX];
}fileinfo;

// 링크드리스트 구조체
typedef struct listNode {
    fileinfo data;
    struct listNode *next;
}listNode;

listNode *head = NULL;


void InitQueue(Queue *queue);
int isEmpty(Queue *queue);
void Enqueue(Queue *queue, char *path);
char *Dequeue(Queue *queue);

listNode* add_list(listNode *head, char *pathname);
void add_node(listNode **head, fileinfo tmp); 
void print_list(listNode *head);
char *get_time(time_t stime);
int split(char *input, char *delimiter, char* argv[]);
void command_help(void);
void find_hash(int argc, char *argv[]);
//listNode* search_size(listNode *head, int size);
//listNode* update_node(listNode *p, char *pathname);
//listNode* delete_node(listNode* head);
void sort_node(listNode* head);
void swap_node(listNode* node1, listNode* node2);

char* fmd5(char* pathname);

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
            find_hash(argc, argv); // fmd5 함수 구현중...

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

// fmd5 해쉬값으로 파일 찾아서 중복파일 출력하는 함수
void find_hash(int argc, char *argv[]) 
{
    char dirname[PATHMAX];
    char pathname[PATHMAX];
    struct timeval begin_t, end_t;
    struct stat statbuf;
    struct dirent** namelist;
    char *homeptr;
    int i;
    int count;

    //
    gettimeofday(&begin_t, NULL);

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

            if (!strcmp(namelist[i]->d_name, "proc") || !strcmp(namelist[i]->d_name, "run") || !strcmp(namelist[i]->d_name, "sys"))
                    continue;

            if (!strcmp(dirname, "/")) { // 경로가 루트인 경우
                sprintf(pathname, "%s%s", dirname, namelist[i]->d_name);
            }
            else
                sprintf(pathname, "%s/%s", dirname, namelist[i]->d_name);

            if ((lstat(pathname, &statbuf) < 0) && (!access(pathname, F_OK))) {
                fprintf(stderr, "lstat error for %s\n", pathname);
                exit(1);
            }

            if (S_ISDIR(statbuf.st_mode)) // 폴더인 경우 큐에 추가
                Enqueue(&queue, pathname);
            else if (S_ISREG(statbuf.st_mode))
                head = add_list(head, pathname);

        }
    }

//    head = delete_node(head);
    sort_node(head);
    print_list(head);

    gettimeofday(&end_t, NULL);

    //
    end_t.tv_sec -= begin_t.tv_sec;
    if (end_t.tv_sec < begin_t.tv_usec) {
        end_t.tv_usec += 1000000;
    }
    end_t.tv_usec -= begin_t.tv_usec;
    printf("Runtime: %ld:%06ld(sec:usec)\n", end_t.tv_sec, end_t.tv_usec);

    return;

}

// 파일을 연결리스트에 추가하여 중복파일 관리
listNode* add_list(listNode* head, char *pathname)
{
    fileinfo tmp;
    struct stat statbuf;
    int size;
    listNode* p;

    // 에러 처리
    if (lstat(pathname, &statbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", pathname);
        exit(1);
    }

    size = statbuf.st_size;

    // 만약 파일이 있는 경우 count++, pathlist++
//    if ((p = search_size(head, size)) != NULL) { // 만약 파일 크기가 동일한 경우 
//        if (!strcmp(fmd5(p->data.pathlist[0]), fmd5(pathname)))    // 해쉬값도 같다면 
//            printf("%s \n", fmd5(p->data.pathlist[0]));
//            printf("%s \n", fmd5(pathname));
//            update_node(p, pathname);     // 중복 파일 리스트에 추가
//    }

    // 파일이 기존 링크드리스트에 존재하지 않는 경우 
//    else {
        tmp.count = 1;
        strcpy(tmp.hash, fmd5(pathname));
        strcpy(tmp.path, pathname);
        tmp.size = statbuf.st_size;
        add_node(&head, tmp);
//    }
    
    return head;
}

//aos dkvdp
void add_node(listNode** head, fileinfo tmp)
{
    listNode *p = (listNode*)malloc(sizeof(listNode));
    p->data = tmp;

    //만약 리스트가 비어있는 경우
    if ((*head) == NULL)
       (*head) = p; 
    else {
        p->next = *head;
        (*head) = p;
    }
}

// 파일의 크기가 동일한 파일을 연결리스트에서 찾는 함수
listNode* search_size(listNode* head, int size)
{
    listNode *p = head;

    while (p != NULL) {
        if(p->data.size == size)
            return p;
        p = p->next;
    }

    return NULL;
}

// wjdfuf
void sort_node(listNode* head)
{
    listNode *p;
    int num = 0;

    //
    if (head == NULL)
        return;

    for (p = head; p != NULL; p = p->next)
        num++;

    p = head;

    for (int i = 0; i < num-1; i++)
    {
        for (int j = 0; j < num-1-i; j++) {
            if (p->data.size > p->next->data.size)
                swap_node(p, p->next);
            p = p->next;
        }
        p = head;
    }
}

//swap
void swap_node(listNode* node1, listNode* node2)
{
    fileinfo tmp;
    tmp = node1->data;
    node1->data = node2->data;
    node2->data = tmp;
}


// 연결리스트의 각 노드의 정보(파일)들 출력
void print_list(listNode* head)
{
    listNode* p = head;
    int num = p->data.count;

    while(1) {
        printf("size : %ld bytes ", p->data.size);
        printf("hash : %s ", p->data.hash);
        for (int i = 0; i < num; i++) {
            printf("path : %s \n", p->data.path);
            p = p->next;
        }
        if (p == NULL) break;
        num = p->data.count;
    }
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

//fmd5
char* fmd5(char* pathname)
{
    int i;
    FILE *IN;
    static char hash[100];
    MD5_CTX c;

    IN = fopen(pathname, "r");
    if (IN == NULL) {
        fprintf(stderr, "hash error %s\n", pathname);
        exit(1);
    }

    unsigned char md[MD5_DIGEST_LENGTH];
    int fd;
    static unsigned char buf[BUFMAX*16];

    fd = fileno(IN);
    MD5_Init(&c);
    for (;;)
    {
        i = read(fd, buf, BUFMAX*16);
        if (i <= 0) break;
        MD5_Update(&c, buf, (unsigned long)i);
    }
    MD5_Final(&(md[0]), &c);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&hash[i*2], "%02x", md[i]);

    fclose(IN);

    return hash;
}

//
char *get_time(time_t stime)
{
    char *time = (char*)malloc(sizeof(char) * BUFMAX);
    struct tm *tm;

    tm = localtime(&stime);
    sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    return time;
}
