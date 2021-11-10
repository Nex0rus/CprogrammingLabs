#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#define _XOPEN_SOURCE 700
#define MAXLENGTH 400
int check_status(char* line, FILE* file_out);

int main() {
 
    FILE* file_in = fopen("access_log_Jul95", "r");
    FILE* file_out = fopen("status_info.txt", "w");
    char* line;
    line = calloc(MAXLENGTH, sizeof(char));
    printf("0\n");

    while (!feof(file_in)) {

        fgets(line, MAXLENGTH, file_in);
        check_status(line, file_out);

    }

    fclose(file_in);
    fclose(file_out);

    return 0;

}

int check_status(char* line, FILE* file_out) {
    int index = 1;
    int i = 0;
    char str_status[4] = {0};
    char request[MAXLENGTH] = {0};

    while (line[index] !='\n') {   // Проходимся до начала запроса вида ... "&REQUEST"

        if (line[index] == '"' && line[index-1] == ' ') {
            index++;
            break;
        }

        index++;
        

    }

    while (!(line[index] == '"' && isdigit(line[index + 2]))) { // Проходим до конца запроса проверяя что еще не дошли до status code который состоит из цифр

        request[i] = line[index]; // Переписываем запрос в переменную для вывода в файл
        i++;
        index++;

    }

    request[i] = '\0'; // Нуль терминатор добавлен чтобы обозначить конец запроса для вывод в файл
    i = 0;
    index += 2; // После запроса через пробел следует status code, таким образом "&REQUEST" &STATUS_CODE поэтому дойдя до закрывающей ковычки: i+=2

    while (line[index] != ' ') { // Идем до пробела пока не закончится status code

        str_status[i] = line[index]; // Переписываем status code в временную переменную 
        i++;
        index++;

    }

    if (str_status[0] == '5') { // Если строка status code начинается на 5 тогда это код ошибки вида 5** и его запрос нужно вывести в файл

        fprintf(file_out, "%s\n", request);

    }

}
