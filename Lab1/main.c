#include <stdio.h>
#include <string.h>


int collectline(FILE *file); //Функция чтения строки в массив символов из файла, возвращяет длину строки

int main(int argc, char *argv[]) {

    int current_length; // Длина текущей прочитаной строки в символах
//char line[MAXLENGTH];// Массив символов, куда будет записываться прочитанная строка
    FILE *file = fopen(argv[2], "r");


    if (argc != 3) { //Проверка случая несоответсвия количества параметров,
        printf("--------------------------------------------\n"
               "Invalid input format. The file name and options are passed through\n"
               "command line arguments in the following format:\n"
               "WordCont.exe [OPTION] filename\n"
               "--------------------------------------------\n");

    }
    else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) { //Реализация опции --help, выводящей на экран доступные для ввода опции
        printf("--------------------------------------------\n"
               "WordCount supports following options:\n"
               "-h, --help available options\n"
               "-l, --lines displays the number of lines\n"
               "-c, --bytes displays the file size in bytes \n"
               "-w, --words displays the number of words\n"
               "--------------------------------------------\n");

    }
    else if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--lines") == 0) { // Реализация опции --lines, выводящей на экран количество строк в файле
        int line_counter = 0; //Счетчик количества прочитанных строк
        int symbol;
        if ((symbol = fgetc(file)) != EOF) line_counter++; // Отрабатывает случай наличия в файле единственной строки без перехода на следующую строку
        if (symbol == '\n') line_counter++; // Обрабатывает случай наличия единственной строки с переходом на следующую, но пустую(EOF) строку
        while ((symbol = fgetc(file)) != EOF) // Считывание всех строк файла(проверка что новая строка не является EOF)
            if (symbol == '\n') line_counter++; // Увеличение счетчика с каждой новой строкой


        printf("Number of lines : %d", line_counter);

    }
    else if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--bytes") == 0) { // Реализация опции --bytes, выводящей на экран размер файла в байтах
        int bytes_counter = 0; // Счетчик кол-ва байт

        while ((current_length = collectline(file)) > 0) {
            bytes_counter += current_length;// При прочтении каждой новой строки счетчик увеличивается на кол-во всех символов в строке т.к каждый символ кодируется 1 байтом
            }


        printf("File size in bytes : %d", bytes_counter);

    }
    else if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--words") == 0) { // Реализация опции --words, выводящей на экран количество слов в файле
        int key = 0; // Перменная фиксирующая нахождение внутри (key = 1) или снаружи слова (key = 0)
        int symbol;
        int words_counter = 0; // Счетчик кол-ва слов в файле

        while ((symbol = fgetc(file)) != EOF) {

                if (symbol == ' ' || symbol == '\n' || symbol == '\t' || symbol == '\0') { // Проверка выхода из слова
                    if (key == 1) words_counter++; // Приращение счетчика слов
                    key = 0; // Изменение метки нахождения снаружи слова
                }
                else if (key == 0) key = 1;// Проверка входа в слово и изменение метки на соотвествующее значение

            }

            if (key == 1) words_counter += 1; //Если после прохода всего файла метка указыввет на нахлждение внутри слова, значит сразу после последнего слова стоял знак '\0'        }



        printf("Number of words : %d", words_counter);

    }

    return 0;
}

int collectline(FILE *file) { // читает строку и возвращает ее длину ( учитвает в длину символ \r при наличии \n)
    int i = 0;
    int symbol;

    while ((symbol=fgetc(file))!=EOF && symbol !='\n') ++i; // Проходит посимвольно до \n
    if (symbol == '\n') i+=2; // индекс i увеличивается на 2 а не на 1 т.к за каждым символом '\n' в системе Windows следует символ '\r' перевода каретки, который учитывается здесь в длину строки

    return i;
}


