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
#include <openssl/sha.h>
#include <ctype.h>

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

double input_to_byte(char *strsize);
listNode* add_list(listNode *head, char *pathname);
void add_node(listNode **head, fileinfo tmp); 
int print_list(listNode *head);
char *get_time(time_t stime);
int split(char *input, char *delimiter, char* argv[]);
void command_help(void);
void find_hash(int argc, char *argv[]);
listNode* search_size(listNode *head, int size);
//listNode* update_node(listNode *p, char *pathname);
void append_node(listNode* head, char* pathname);
void sort_node(listNode* head);
void swap_node(listNode* node1, listNode* node2);
void delete_node(listNode** head, listNode* delete);
void index_option(void);
void delete_list(listNode** head);
void delete_option(listNode *head, int sindex, int fno);
void trash_option(listNode *head, int sindex);
const char *min_time_path(listNode *p);

const char* comma(long size);
char* fmd5(char* pathname);
char* sha1(char* pathname);

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
    char extension[NAMEMAX];
    char inputsize[NAMEMAX];
    char *ptr;
    int i;
    int count;
    double minsize;
    double maxsize;

    //
    gettimeofday(&begin_t, NULL);

    // 입력 관련 에러 처리
    if (argc != ARGMAX) {
        printf("ERROR: Arguments error\n");
        return;
    }

    // argv[1] error 
    if(strcmp(argv[1], "*") && strncmp(argv[1], "*.", 2)) {
        printf("ERROR: filename error\n");
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

    // minsize 
    if (!strcmp(argv[2], "~"))
        minsize = 1; 
    else if ((minsize = input_to_byte(argv[2])) == 0) {
        printf("ERROR : MINSIZE error\n");
        return;
    }

    // maxsiz
    if (!strcmp(argv[3], "~"))
        maxsize = -1;
    else if ((maxsize = input_to_byte(argv[3])) == 0) {
        printf("ERROR : MAXSIZE error\n");
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
            else if (S_ISREG(statbuf.st_mode)) { // 정규파일인 경우
                //
                if (maxsize == -1) {
                    if (statbuf.st_size < minsize)
                        continue;
                }
                else {
                    if (statbuf.st_size < minsize || statbuf.st_size > maxsize)
                        continue;
                }
                
                if (!strcmp(argv[1], "*")) // 만약 파일이 "*"인 경우
                     head = add_list(head, pathname);
                else {                                // "*.~" 형태인 경우
                    strcpy(extension, strstr(argv[1], "."));  // 확장자 추출
                    ptr = strstr(strrchr(pathname, '/'), extension); // 확장자가 포함되어 있는 문자열 찾기
                    if(ptr != NULL) {
                        if (strlen(ptr) == strlen(extension)) 
                            head = add_list(head, pathname);
                    }
                    else
                        continue;
                }
            }
        }
    }

    // 오름차순으로 링크드리스트 정렬
    sort_node(head);
    // 중복파일리스트를 출력하고, 만약 중복리스트가 없는 경우 flag = 0
    int flag = print_list(head);

    gettimeofday(&end_t, NULL);

    // 종료시간 측정
    end_t.tv_sec -= begin_t.tv_sec;
    if (end_t.tv_sec < begin_t.tv_usec) {
        end_t.tv_usec += 1000000;
    }
    end_t.tv_usec -= begin_t.tv_usec;
    printf("Searching time: %ld:%06ld(sec:usec)\n\n", end_t.tv_sec, end_t.tv_usec);

    // 만약 중복리스트가 존재하는 경우 옵션 프롬프트 시작
    if (flag)
        index_option();

    // 링크드리스트 데이터 제거
    delete_list(&head);

    return;

}

// 입력받은 인자를 byte단위로 변환시키는 함수
double input_to_byte(char *strsize)
{
    double inputsize;
    int pointnum = 0;

    // KB, MB, GB 입력받을 때, byte로 변환하여 리턴
    if (strstr(strsize, "KB") != NULL || strstr(strsize, "kb") != NULL) {
        inputsize = atof(strsize);
        return inputsize * 1024;
    }
    if (strstr(strsize, "MB") != NULL || strstr(strsize, "mb") != NULL) {
        inputsize = atof(strsize);
        return inputsize * 1024 * 1024;
    }
    if (strstr(strsize, "GB") != NULL || strstr(strsize, "gb") != NULL) {
        inputsize = atof(strsize);
        return inputsize * 1024 * 1024 * 1024;
    }

    // 숫자만 입력받았는지 체크
    for(int i = 0; strsize[i] != '\0'; i++)
    {
        if (strsize[i] == '.') { // floating point도 입력받을 수 있음
            pointnum++;
            continue;
        }
        if (isdigit(strsize[i]) == 0) // .을 제외한 다른 문자가 들어오는 경우
            return 0;
    }
    // point가 두개 이상인 경우 실수가 아님
    if (pointnum > 1)
        return 0;

    inputsize = atof(strsize);
    return inputsize;
}

// 파일을 연결리스트에 추가하여 중복파일 관리
listNode* add_list(listNode* head, char *pathname)
{
    fileinfo tmp;
    struct stat statbuf;
    int size;
    listNode* p = head;

    // 에러 처리
    if (lstat(pathname, &statbuf) < 0) {
        fprintf(stderr, "lstat error for %s\n", pathname);
        exit(1);
    }

    size = statbuf.st_size;

    // 리스트의 모든 노드를 순회하면서
    while (p != NULL) {
        if (p->data.size == size) {      // 만약 사이즈가 같고,
            char* originhash = fmd5(p->data.path);
            char* newhash = fmd5(pathname);
            if (!strcmp(originhash, newhash)) {  // 해시값도 같다면
                free(originhash);
                free(newhash);
                append_node(p, pathname);       // 파일 세트의 맨 끝에 해당 파일 추가
                break;
            }
        }
        p = p->next;
    }

    // 모든 노드를 순회했을 때, 리스트에 존재하지 않는 경우
    if (p == NULL) {
        tmp.count = 1;
        strcpy(tmp.path, pathname);
        tmp.size = statbuf.st_size;
        add_node(&head, tmp);
    }
    
    return head;
}

//링크드리스트의 맨 앞에 노드를 추가하는 함수
void add_node(listNode** head, fileinfo tmp)
{
    listNode *p = (listNode*)malloc(sizeof(listNode));
    p->next = NULL;
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

// 노드의 사이즈를 기준으로 오름차순 정렬하는 함수
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

// 두 노드의 데이터를 바꾸는 함수
void swap_node(listNode* node1, listNode* node2)
{
    fileinfo tmp;
    tmp = node1->data;
    node1->data = node2->data;
    node2->data = tmp;
}

// 파일 세트의 맨 끝에 파일을 추가하는 함수
void append_node(listNode* p, char* pathname)
{
    listNode *newNode = (listNode*)malloc(sizeof(listNode));
    int count = p->data.count;

    // 카운트를 증가시키고, 새로운 노드를 만들고 데이터 복사
    p->data.count++;
    newNode->data = p->data;

    // 해당 파일 세트의 가장 끝에 있는 파일로 이동
    for (int i = 0; i < count-1; i++) {
        p = p->next;
        p->data.count++;
    }

    // 노드 경로 업데이트, 해당 노드를 마지막 위치에 삽입
    strcpy(newNode->data.path, pathname);
    newNode->next = p->next;
    p->next = newNode;
}

// 연결리스트의 각 노드의 정보(파일)들 출력
int print_list(listNode* head)
{
    listNode* p = head;
    struct stat statbuf;
    int num;
    int index = 0;

    while(1) {
        if (p == NULL) break;
        if (p->data.count == 1) {
            p = p->next;
            continue;
        }
        index++;
        num = p->data.count;
        char *hash = fmd5(p->data.path);
        printf("---- Identical files #%d (%s bytes - %s) ----\n", index, comma(p->data.size), hash);
        free(hash);
        for (int i = 0; i < num; i++) {
            printf("count %d -----", p->data.count);
            lstat(p->data.path, &statbuf);
            printf("[%d] %s (mtime : %s) (atime : %s)\n", i + 1, p->data.path, get_time(statbuf.st_mtime), get_time(statbuf.st_atime));
            p = p->next;
        }
        printf("\n");
        num = p->data.count;
    }

    if (index != 0)
        return 1;

    if (index == 0) {
        printf("No duplicates in \n");
        return 0;
    }
}

// 특정노드 삭제..... 
void delete_node(listNode** head, listNode* delete)
{
    // 헤드가 삭제노드면
    if ((*head) == delete)
        (*head) = (*head)->next;
    else {
        // 삭제노드가 헤드가 아닌 경우 
        listNode* p = (*head);

        while ((p->next != delete) && (p != NULL)) {
            p = p->next;
        }

        p->next = delete->next;
    }
    free(delete);
}

// 옵션 프롬프트 출력하여 옵션 입력받는 함수
void index_option(void)
{
    int argc = 0;
    char input[BUFMAX];
    char *argv[3];

    // 프롬프트 시작
    while(1) {
        printf(">> ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        argc = split(input, " ", argv);

        if (argc == 0)
            continue;

        // exit 입력받으면 프롬프트로 이동
        if ((strcmp(argv[0], "exit") == 0)) {
            printf(">> Back to Prompt\n");
            return;
        }
        // d 옵션
        else if ((strcmp(argv[1], "d") == 0) && (argc == 3)) {
            delete_option(head, atoi(argv[0]), atoi(argv[2]));
            print_list(head); 
            continue;
        }
        else if ((strcmp(argv[1], "t") == 0) && (argc == 2)) {
            trash_option(head, atoi(argv[0]));
            print_list(head);
            continue;
        }
        else
            continue;

        
        
    }

}

void delete_option(listNode *head, int sindex, int fno)
{
    listNode *p = head;
    listNode *delete;
    int index = 0;
    int count;

    while (1) {
        // count = 1인 파일세트는 패스
        if (p->data.count == 1) {
            p = p->next;
            continue;
        }

        index++;
        // 입력받은 파일세트 인덱스로 이동
        if (index == sindex) {
            count = p->data.count;
            for (int i = 1; i <= count; i++) {
                p->data.count--; // 해당 파일세트의 count 하나씩 감소
                if (i == fno) { // 삭제하려는 파일 차례인 경우
                    delete = p; // 삭제할 노드로 지정
                }
                p = p->next;
            }
            printf("\"%s\" has been deleted in #%d\n\n", delete->data.path, sindex);
            remove(delete->data.path);
            delete_node(&head, delete);
            return;
        }
        else {
            count = p->data.count;
            for (int i = 1; i <= count; i++)
                p = p->next;
        }

        if (p == NULL)
            return;
    }
}

// 가장 짧은 수정시간을 갖는 파일을 제외하고 휴지통으로 이동
void trash_option(listNode* head, int sindex)
{
    listNode *p = head;
    listNode *delete;
    int index = 0;
    int count;
    char *newpath[PATHMAX];

    while (1) {
        if (p == NULL)
            return;

        // count = 1인 파일세트는 패스
        if (p->data.count == 1) {
            p = p->next;
            continue;
        }

        index++;
        // 입력받은 파일세트 인덱스로 이동
        if (index == sindex) {
            count = p->data.count;
            strcpy(p->data.path, min_time_path(p)); // 첫 번째 노드를 가장 짧은 수정시간의 pathf로 변경
            p->data.count = 1; // 첫 번째 노드의 count = 1로 변경
            p = p->next; //다음 노드로 이동
            for (int i = 2; i <= count; i++) { // 첫 노드 제외하고 순회
                // 휴지통으로 이동시키기
                delete = p;
                p = p->next;
                delete_node(&head, delete); // 순회하면서 각 노드 리스트에서 제거
            }
            return;
        }
        else {
            count = p->data.count;
            for (int i = 1; i <= count; i++)
                p = p->next;
        }
    }
}

// 가장 접근시간이 짧은 path 찾기
const char* min_time_path(listNode *p)
{
    int count;
    time_t mintime;
    struct stat statbuf;
    static char path[PATHMAX];

    count = p->data.count;
    lstat(p->data.path, &statbuf);
    mintime = statbuf.st_mtime;

    for (int i = 1; i <= count; i++) {
        lstat(p->data.path, &statbuf);
        if(mintime > statbuf.st_mtime)
            strcpy(path, p->data.path);
        p = p->next;
    }

    return path;
}


// 헤드부터 순회하여 모든 노드 삭제
void delete_list(listNode** head)
{
    listNode *p, *iterator;
    iterator = *head;
    while(iterator)
    {
        p = iterator->next;
        free(iterator);
        iterator = p;
    }

    *head = NULL;
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
    char *path = (char*)malloc(sizeof(char) * PATHMAX);
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
char* sha1(char* pathname)
{
    int i;
    FILE *IN;
    MD5_CTX c;
    char *hash = (char*)malloc(sizeof(char) * BUFMAX);

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

char* fmd5(char* pathname)
{
    int i;
    FILE *IN;
    SHA_CTX c;
    char *hash = (char*)malloc(sizeof(char) * BUFMAX);

    IN = fopen(pathname, "r");
    if (IN == NULL) {
        fprintf(stderr, "hash error %s\n", pathname);
        exit(1);
    }

    unsigned char md[SHA_DIGEST_LENGTH];
    int fd;
    static unsigned char buf[BUFMAX*16];

    fd = fileno(IN);
    SHA1_Init(&c);
    for (;;)
    {
        i = read(fd, buf, BUFMAX*16);
        if (i <= 0) break;
        SHA1_Update(&c, buf, (unsigned long)i);
    }
    SHA1_Final(&(md[0]), &c);

    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
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

const char *comma(long size)
{
    static char comma_str[64];
    char str[64];
    int idx, len, cidx = 0, mod;

    sprintf(str, "%ld", size);
    len = strlen(str);
    mod = len % 3;
    for (idx = 0; idx < len; idx++) {
        if (idx != 0 && (idx) % 3 == mod) {
            comma_str[cidx++] = ',';
        }
        comma_str[cidx++] = str[idx];
    }
    comma_str[cidx] = 0x00;

    return comma_str;
}
