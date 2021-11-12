#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#define MAXLENGTH 400
int check_status(char* line, FILE* file_out, char* status);
void get_timeshift(FILE* file_in, long timeshift);
long parse_time(char* line);
void command_handler(char* argv[], FILE* file_in, FILE* file_out);


int main(int argc, char* argv[]) {
    clock_t tic = clock();
    FILE* file_in = fopen("access_log_Jul95", "r");
    FILE* file_out = fopen("status_info.txt", "w");
    command_handler(argv, file_in, file_out);
    fclose(file_in);
    fclose(file_out);
    clock_t toc = clock();
    printf("Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);
    return 0;

}

int check_status(char* line, FILE* file_out, char* status) {
    char* request;
    char counter = 1;
    char result[4];
    char* tmp;
    char* pch = strtok(line, "\"");
    while (pch != NULL) {

        tmp = pch;
        pch = strtok(NULL, "\"");
        if (counter == 1) {
            request = pch;
            counter++;
        }

        if (pch == NULL) {
            break;
        }

    }
    for (int i = 1; i < 4; i++) {
        result[i-1] = tmp[i]; 
    }
    result[3] = '\0';

    if (result[0] == status[0]) {
        fprintf(file_out, "%s   status code: %s\n", request, result);
        return 1;
    }

    // int index = 1;
    // int i = 0;
    // char str_status[4] = {0};
    // char request[MAXLENGTH] = {0};

    // while (line[index] !='\n') {   // Проходимся до начала запроса вида ... "&REQUEST"

    //     if (line[index] == '"' && line[index-1] == ' ') {
    //         index++;
    //         break;
    //     }

    //     index++;
        

    // }

    // while (!(line[index] == '"' && isdigit(line[index + 2]))) { // Проходим до конца запроса проверяя что еще не дошли до status code который состоит из цифр

    //     request[i] = line[index]; // Переписываем запрос в переменную для вывода в файл
    //     i++;
    //     index++;

    // }

    // request[i] = '\0'; // Нуль терминатор добавлен чтобы обозначить конец запроса для вывод в файл
    // i = 0;
    // index += 2; // После запроса через пробел следует status code, таким образом "&REQUEST" &STATUS_CODE поэтому дойдя до закрывающей ковычки: i+=2

    // while (line[index] != ' ') { // Идем до пробела пока не закончится status code

    //     str_status[i] = line[index]; // Переписываем status code в временную переменную 
    //     i++;
    //     index++;

    // }

    // if (str_status[0] == '5') { // Если строка status code начинается на 5 тогда это код ошибки вида 5** и его запрос нужно вывести в файл

    //     fprintf(file_out, "%s\n", request);

    // }
    return 0;

}

void get_timeshift(FILE* file_in, long timeshift) {
    char* line;
    line = calloc(MAXLENGTH, sizeof(char));
    long line_time;
    long* times_queue;
    times_queue = malloc(2000000*sizeof(long));
    int head = 0;
    int tail = -1;
    time_t* max_head;
    max_head = malloc(sizeof(time_t));
    time_t* max_tail;
    max_tail = malloc(sizeof(time_t));
    long curr_time = timeshift;
    long delta;
    int max_requests = 0;
    int counter = 0;

    while (!feof(file_in)) {
        counter += 1;
        fgets(line, MAXLENGTH, file_in);
        line_time = parse_time(line);
        if (tail == -1) {

            tail += 1;
            times_queue[tail] = line_time;
            continue;

        }

        delta = line_time - times_queue[tail];

        if (delta <= curr_time) {

            curr_time -= delta;
            tail += 1;
            times_queue[tail] = line_time;

        }  

        else {

            if ((tail - head + 1) > max_requests) {

                max_requests = tail - head + 1;
                *max_head = (time_t)times_queue[head];
                *max_tail = (time_t)times_queue[tail];

            }

            while (delta > curr_time && head < tail) {

                curr_time += times_queue[head+1] - times_queue[head];
                head +=1;

            }
            if (delta > curr_time) {
                
                tail += 1;
                times_queue[tail] = line_time;
                curr_time = timeshift;
                head = tail;

            }
            else {

                curr_time -= delta;
                tail += 1;
                times_queue[tail] = line_time;
            }
 
        }
    
    }
    // printf("%lu   %lu\n", ctime(max_head), ctime(max_tail));
    // char* window_start = malloc(32*sizeof(char));
    // window_start = ctime(max_head);
    // printf("%ld, %ld\n", *max_head, *max_tail);
    printf("The max number of requests is %ld, starting from %s\r", max_requests, ctime(max_head));
    printf("ending with %s\n", ctime(max_tail));
    free(times_queue);
    free(max_tail);
    free(max_head);
    
    }


long parse_time(char* line) {
    char string[3];
    struct tm* time;
    time = malloc(64);
    char* split;
    split = strtok(line, "[");
    split = strtok(NULL, "]");
    time->tm_year = 95;
    time->tm_mon = 6;
    split = strtok(split, "/");
    time->tm_mday = atoi(split);
    strtok(NULL, ":");
    split = strtok(NULL, ":");
    time->tm_hour = atoi(split);
    split = strtok(NULL, ":");
    time->tm_min = atoi(split);
    split = strtok(NULL, " ");
    for (int i = 0; i < 2; i++) {
        string[i] = split[i];
    }
    string[2] = '\0';
    time->tm_sec = atoi(string);
    return mktime(time);

}

void command_handler(char* argv[], FILE* file_in, FILE* file_out) {
    char* status;
    long timeshift;
    int counter = 0;
    int max_requests;
    if (strcmp(argv[1], "--help") == 0) {
        printf("===========================\nThis is log parsing programm\nThe list of features:\n --help Shows available options\n --status X Creates a output file status_info.txt\n   with all requests of status code X** where X from 1 to 5\n --timeshift X Shows the maximum number of requests\n   that were made in X seconds\n===========================\n");
    }

    if (strcmp(argv[1], "--status") == 0) {
        status = argv[2];
        printf("%s\n", status);
        char* line;
        line = calloc(MAXLENGTH, sizeof(char));

        while (!feof(file_in)) {
            
            fgets(line, MAXLENGTH, file_in);
            counter += check_status(line, file_out, status);

        }
        fprintf(file_out, "The number of rquests with status code %s** is %d\n", status, counter);
        
    }
     
    if (strcmp(argv[1], "--timeshift") == 0) {

        timeshift = atoi(argv[2]);
        get_timeshift(file_in, timeshift);

    }
    
}