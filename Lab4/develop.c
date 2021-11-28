#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>
#include <malloc.h>
#include <io.h>


typedef struct Frame {
    int size;
    char* info;
    int num_of_symbols;

}Frame;

typedef struct Picture {
    int size;
    char encoding;
    char* mime;
    char* type;
    char* data;
    char* description;

}Picture;


void command_handler(int argc, char* argv[]);
char* write_bits(char* bits, char byte);
void get_version(char* array_to_write, char* bits, char byte, int* counter);
void show(FILE* mp3);
int parse_ID3_header(FILE* mp3);
void raise_error();
Frame* parse_frame(FILE* mp3, int* counter, char* buf, Frame* frame);
int skip_frame(FILE* mp3, int counter);
void get_frame(FILE* mp3, char* tag);
void set_value(FILE* mp3, char* file_path, char* tag, char* value);
void get_pic(FILE* mp3, int* counter);


int main(int argc, char* argv[]) {
    command_handler(argc, argv);
    return 0;
}


void raise_error() { // простая alert функция вызываемая в command_handler в случае несоответствия аргументов
    printf("Wrong parameters format\nPlease input valid parameters\nThe list of parameters:\n --filepath=\"filepath\" this is an esential parameter\n --show shows all meatinformation organised in a table\n --get=\"prop_name\" shows the value of a tag named prop_name\n --set=\"prop_name\" --value=\"value\" changes tag's named prop_name value to value parameter\n");
    exit(0);
}


void command_handler(int argc, char* argv[]) { // функция обработки командлет
    if (argc < 3) {
        raise_error();
    }

    char* file_path;
    char* instruction;
    char* value = malloc(100);
    char* tag;
    FILE* mp3;

    strtok(argv[1], "=");
    file_path = strtok(NULL, "=");
    printf("====%s====\n", file_path);
    if ((mp3 = fopen(file_path, "rb")) == NULL) {

        printf("Failed to open the file. Check if the filepath is valid\n");
        exit(0);

    }

 
    instruction = strtok(argv[2], "=");
    if (strcmp(instruction, "--show") == 0) {
        show(mp3);
    }

    if (strcmp(instruction, "--get") == 0) {
        tag = strtok(NULL, "=");
        get_frame(mp3, tag);
    }

    if (strcmp(instruction, "--set") == 0) {

        if (argc < 4) {
            raise_error();
        }

        else {
            
            tag = strtok(NULL, "=");
            strtok(argv[3], "=");
            value = strtok(NULL, "\"");
            set_value(mp3, file_path, tag, value);

        }
        
    }

    free(value);   
}

char tagcmp(char* line1,char* line2) { // функция сравнения четырехсимвольных (не строковых) тэгов, заменяет здесь strcmp()
    for (int i = 0; i < 4; i++) {

        if (line1[i] != line2[i]) {
            return 0;
        }

    }

    return 1;
}

char* write_bits(char* bits, char byte) { // простая функция разложения байта по битам

    for (int i = 0; i < 8; i++) {

        bits[i] = (byte >>(sizeof(byte)*8 -i-1)) & 1;

    }

    return bits;

}

int parse_ID3_header(FILE* mp3) { // функция парсинга ID3 header - возвращает контрольную сумму из размера и флага расширенного header 
    char byte;
    char* bits = malloc(8*sizeof(char));
    int version = 0;
    int size = 0;
    char is_extended_header = 0;

    for (int i=0; i < 3; i++) { // Читает заголовок тэга, три неизменных символа ID3
        fgetc(mp3);
    }

    byte = fgetc(mp3); // Считывает версию ID3 чтобы проверить возможность парсинга
    version = byte;
    if (version == 4) {

        printf("This program doesn't support ID3v2.4 parsing\n");
        free(bits);
        return 0;

    }

    fgetc(mp3); // Пропускает байт подверсии
    byte = fgetc(mp3); // Считывает байт флагов тэга
    bits = write_bits(bits, byte); // дает побитовое представление байта 

    if (bits[0] == '1') { // если бит ансинхронизации выставлен в 1 - останавливает выполнение, парсинг такого типа файла не предусмотрен программой

        free(bits);
        return 0;

    }
    
    is_extended_header = bits[1];

    for (int i = 3; i < 8; i++) { // проверяет правильность битов всегда равных 0

        if (bits[i] == '1') {
            free(bits);
            return 0;
        }

    }

    for (int i = 0; i < 4; i++) { // считывает размер header из syncsafe байтов

        byte = fgetc(mp3);
        size += byte * pow(128, 3-i);

    }

    free(bits); // освобождает память
    return size*10 + is_extended_header; // возращает контрольную сумму - размер*10 + флаг наличия расширенного заголовка - 1 или 0

}

int skip_frame(FILE* mp3, int counter) { // функция пропуска фрейма до начала следующего 
    char* buf = malloc(4*sizeof(char));
    int size;
    fread(buf, sizeof(char), 4, mp3); // считывает размер фрейма в буфер 

    for (int i = 0; i < 4; i++) { // получает численный размер 
        size += buf[i]*pow(256, 3-i);
    }
    counter += size + 6; // увеличивает значение переменной счетчика, которую затем вернет в вызывавшую функцию
    fgetc(mp3); // пропускаем два байта флагов
    fgetc(mp3);
    for (int i = 0; i < size - 1; i++) { // пропускаем все поля структуры 
        fgetc(mp3);
    }
    free(buf); // освобождаем память 
    return counter;
    
}

void get_frame(FILE* mp3, char* tag) { // функция реализующая опцию --get
    char byte;
    int counter = 0;
    int size = parse_ID3_header(mp3); // получим информацию о размере заголовка 
    char is_extended_header = size % 10; // считаем наличие ext.header'a 
    size = size / 10;
    char* buf = malloc(4*sizeof(char)); // создадим буфер
    

    if (size == 0) { // если размер тэга равен нулю - файл не имеет тэга ID3 и не может быть распаршен
        printf("Some error occured while reading the ID3 header.\nPerharps some of the flags is invalid or ID3 version is not supported");
    }
    
    if (is_extended_header) { // При наличии расширенного заголовка, он пропускается 

        int ext_header_size = 0;

        for (int i = 0; i < 4; i++) {

            byte = fgetc(mp3);
            ext_header_size += pow(256, 3-i)*byte;

        }

        for (int i = 0; i < ext_header_size + 6; i++) {
            fgetc(mp3);
        }

        size = size - ext_header_size;

    }

    Frame* frame = malloc(sizeof(struct Frame)); // создадим структуру фрейма, которую будем передавать в функцию парсинга 
    printf("Started looking throw frames:\n"); // просматриваем фреймы 
    while (counter < size) {
        fread(buf, sizeof(char), 4, mp3); // будем считывать новый ID фрейма и сверять с ID искомого
        counter += 4;

        for (int i = 0; i < 4; i++) { // для наглядности будем выводить просмотренные тэги
            printf("%c", buf[i]);
        }

        printf("\n");

        if (tagcmp(buf, tag)) { // если нашли совпадение по названию тэга с искомым выведем информацию о нем

            frame = parse_frame(mp3, &counter, buf, frame); // получим заполненную структуру фрейма из функции парсинга 
            printf("===============\n");

            for (int i = 0; i < 4; i++) {
                printf("%c", tag[i]); // выведем название 
            }

            printf("\n");

            for (int i = 0; i < frame->num_of_symbols; i++) { // выведем информацию на экран
                printf("%c", frame->info[i]);
            }

            printf("\n===============\n");
            
            return;
        }

        else if (tagcmp(buf, "APIC")) {
    
        }

        else if (buf[0] != '\0') { // если получили неподдерживаемый фрейм - пропускаем его 
            counter = skip_frame(mp3, counter);
        }

        else { // иначе, такой фрейм найти не удалось - его нет в тэге

        printf("No tag with ID: %s was found", tag);
        free(frame);
        free(buf);

        return;

        }

    }

    free(buf);
    free(frame);
    
}

void show(FILE* mp3) { // функция реализующая опцию --show
    char byte;
    int counter = 0;
    int size = parse_ID3_header(mp3);
    char is_extended_header = size % 10; // parse ID3 head возвращает контрольную сумму последняя цифра которой 0 или 1 говорит о наличиии ext. заголовка
    size = size / 10; // считаем сам размер
    char* buf = malloc(size*sizeof(char)); // создадим буфер
    

    if (size == 0) { // если размер ID3 тэга равен нулю значит файл не имеет ID3 тэга 

        free(buf);
        printf("Some error occured while reading the ID3 header.\nPerharps some of the flags is invalid or ID3 version is not supported");

    }
    
    if (is_extended_header) { // При наличии расширенного заголовка, он пропускается 

        int ext_header_size = 0;

        for (int i = 0; i < 4; i++) {

            byte = fgetc(mp3);
            ext_header_size += pow(256, 3-i)*byte;

        }

        for (int i = 0; i < ext_header_size + 6; i++) {
            fgetc(mp3);
        }

        size = size - ext_header_size - 10;

    }

    Frame* frame = malloc(sizeof(struct Frame)); // создадим структуру фрейма
   
    while (counter < size) { // пройдемся до конца тэга в поисках фрейма 

        fread(buf, sizeof(char), 4, mp3); // каждый раз будем брать 4 байта и получать новый ID фрейма
        if (buf[0] == 'T' || buf[0] == 'W' || buf[0] == 'C' || tagcmp(buf, "USLT")) { // проверим что полученный фрейм поддерживается программой 

            printf("==================\n");
            for (int i = 0; i < 4; i++) {
                printf("%c", buf[i]); // выведем ID тэга
            }
            printf("\n");

            frame = parse_frame(mp3, &counter, buf, frame); // получим информацию о фрейме функцией 

            for (int i = 0; i < frame->num_of_symbols; i++) { // выведем на экран текстовое поле фрейма
                printf("%c", frame->info[i]);
            }

            printf("\n");

        }

        else if (tagcmp(buf, "APIC")) {
            get_pic(mp3, &counter);
            return;
            
        }

        else if (buf[0] != '\0') { // если встретилось значение не на 0, необходимо пропустить тэг, просто пролистав символы 
            counter = skip_frame(mp3, counter);
        }

        else { // если значение равно 0, все тэги уже были пройдены - необходимо выйти из функции

            free(buf);
            free(frame);

            return;

        }

    }
 
    free(buf);
    free(frame);
}

Frame* parse_frame(FILE* mp3, int* counter, char* buf, Frame* frame) { // промежуточная функция получающая значение частей фрейма
    char tag = buf[0];
    int num_of_symbols = 0;
    fread(buf, sizeof(char), 4, mp3);
    *counter += 4;
    frame->size = 0;

    for (int i = 0; i < 4; i++) {
        frame->size += (int)buf[i]*pow(256, 3-i);
    }
    num_of_symbols = frame->size;

    fread(buf, sizeof(char), 2, mp3); // Пропускаем флаги тэга
    *counter += 2;

    if (tag != 'W') {
        fread(buf, sizeof(char), 1, mp3);
        if (buf[0] == 1) {
            num_of_symbols -= 3;
            fgetc(mp3);
            fgetc(mp3);
        }
        else {
            num_of_symbols -= 1;
        }
    }
 

    fread(buf, sizeof(char), num_of_symbols, mp3);
    frame->info = buf;
    *counter += frame->size;
    frame->num_of_symbols = num_of_symbols;
    return frame;
}

void set_value(FILE* mp3, char* file_path, char* tag, char* value) { // функция реализующая опцию --set
    
    char* bits = malloc(8*sizeof(char)); // массив символов для разложения числа побитово
    char is_ext_header = 0;              // Переменная принимающая значение 0 или 1 в зависимости от того есть ли расширенный заголовок
    int ext_size = 0;                    // переменная размера расширенного заголовка (от 10 до 20 байт)
    int num_of_symbols = 0;              // переменная количества символов фрейма
    int size = 0;                        // размер ID3 тэга 
    int start_of_tag = -2;               // индекс начала искомого тэга в буфере
    int end_of_tag = -2;                 // индекс конца искомого тэга в буфере
    int tag_size = 0;                    // размер тэга включая заголовок 
    char* tag_towrite_value;             // информация содержащаяся в value, необходимо записать на место фрейма
    int tag_towrite_size = 0;            // размер фрейма который будет записан на место замененного
    
    FILE* mp3_out = fopen("temp.mp3", "wb"); // открываем временный файл куда будет скопировано все из текущего за исключением заменяемого тэга
    if (tag[0] == 'T' || tag[0] == 'W' || tag[0] == 'C') { // проверяем что изменяются поддерживаемый программой тэги
        tag_towrite_size += (int)strlen(value) + 1; // считываем размер введенной строки из value
        tag_towrite_value = malloc((tag_towrite_size+10)*sizeof(char)); // зная что общий размер фрейма на 10 больше выделяем память под перменную с текстом фрейма
        int tmp = tag_towrite_size; // создаем копию размера, чтобы не изменять сам размер а лишь работать со значением

        for (int i = 0; i < 4; i++) { // записываем в фрейм название тэга
            tag_towrite_value[i] = tag[i];
        }

        for (int i = 7; i > 3; i--) { // высчитываем размер записываемого тэга зная размер поля информации, переводя в 4 байта 16ричной системы
            tag_towrite_value[i] = (int)tmp%256;
            tmp = tmp/256;
        }

        tag_towrite_value[8] = '\0'; // обнуляем байты флагов, так как нет необходимости в их выставлении
        tag_towrite_value[9] = '\0'; //
        if (tag[0] != 'W') { // фрейм ссылки не имеет байта кодировки перед текстом, остальные - имеют, необходимо обнулить его чтобы выставить ASCII
            tag_towrite_value[10] = '\0';

            for (int i = 11; i < tag_towrite_size + 10; i++) { // переписываем из value сам текст
                tag_towrite_value[i] = value[i - 11];
            }
        }

        else { // если же фрейм хранит ссылку занулять байт перед текстом не нужно
            for (int i = 10; i < tag_towrite_size + 10; i++) { // переписываем значение 
                tag_towrite_value[i] = value[i - 10];
            }
        }

        tag_towrite_size += 10; // увеличиваем размер фрейма на 10 байт так как учитываем header фрейма

    }

    else { // если фрейм не является текстовым то выходим из программы
        printf("This program only supports changing Text, URL and COMM frame's values\n");
        return;
    }



    char* header = malloc(10*sizeof(char)); // запишем header ID3 в переменную
    fread(header, sizeof(char), 10, mp3);
    int power = 1; // функция степени числа пригодится для вычисления размера 
    for (int i = 6; i < 10; i++) { // функция рассчета размера всего ID3 тэга, байты содержать synchsafe int вида %0xxxxxxx 
        power = pow(128, 9 - i); // следовательно умножать нужно на 128 а не на 256 
        size += header[i]*power; // эта переменная будет хранить размер всего ID3 тэга
    }

    char* buf = malloc(sizeof(char)); // создадим буфер куда попадет весь тэг без ID3 header
    bits = write_bits(bits, header[5]); // разложим побитово 6ой байт ID3 header чтобы узнать не нарушены ли флаги
    
    if (bits[1] == 1) { // если первый бит установлен в 1 - присутствует extended header

        for (int i = 0; i < 10; i++) { // прочитаем его размер
            buf[i] = fgetc(mp3);
        }

        for (int i = 0; i < 4; i++) { // рассчитаем количество байт 
            ext_size += buf[i]*pow(256, 3-i);
        }

        buf = realloc(buf, ext_size); // пропустим extended header
        for (int i = 0; i < ext_size; i++) {
            fgetc(mp3);
        }
        ext_size += 10; // сам header занимает 10 байт так что нужно учесть его далее в size 
    } 

    size -= ext_size; // вычтем чтобы получить количество байт во всех фреймах
    buf = realloc(buf, size * sizeof(char)); // изменим размер буфера на равный количеству байт фреймов

    fread(buf, sizeof(char), size, mp3); // прочитаем все фреймы до конца тэга
    power = 1; // восстановим значение степенной переменной до 1
    char byte_symbol = 0; // переменная для считывания одного байта 
    for (int i = 3; i < size; i++) { // пока не индекс не дойдет до конца буфера будем искать необходимый фрейм

        if (buf[i] == tag[3] && buf[i - 1] == tag[2] && buf[i - 2] == tag[1] && buf[i - 3] == tag[0]) { // если  фрейм найден

            printf("Found the tag\n"); 
            start_of_tag = i - 4; // укажем индекс перед началом фрейма

            for (int j = i + 1; j < i + 5; j++) { // считаем размер текстового поля фрейма

                byte_symbol = buf[j];
                power = pow(256, 4 - (j - i)); // переведем в 256 ричную систему 
                tag_size += (int)byte_symbol * power; // на каждом шаге увеличиваем размер всего фрейма

            }

            tag_size += 10; // допишем к нему фиксированный размер заголовка в 10 байт
            end_of_tag = start_of_tag + tag_size; // очевидно вычислим индекс последнего символа фрейма

            break; 
        }
    }
   
    int tmp = size - (end_of_tag - start_of_tag) + tag_towrite_size; // запишем во временную переменную количество байт без старого тэга но с новым тэгом 
    header[5] = '\0'; // установим флаги header в 0
    for (int i = 9; i > 6; i--)  // паресчитаем новый размер ID3 header с добавленным новым и убранным старым фреймом
    {
        header[i] = tmp % 128; 
        tmp = tmp / 128;

    }

    if (start_of_tag == -2) // если фрейм не был найден то его нет в тэге и переменная останется равна начальному значению
    {
        for (int i = 0; i < 10; i++) // перепишем весь подготовленный ранее header в временный файл 
        {
            fputc(header[i], mp3_out);
        }

        for (int i = 0; i < tag_towrite_size; i++) // сразу после запишем новый фрейм
        {
            fputc(tag_towrite_value[i], mp3_out);
        }

        for (int i = 0; i < size; i++) // просто дозапишем весь ID3 тэг так как не будет конфликтов одинаковых фреймов
        {
            fputc(buf[i], mp3_out);
        }
    }

    else // если же фрейм был найден его необходимо пропустить 
    {

        for (int i = 0; i < 10; i++) // аналогично запишем header 
        {
            fputc(header[i], mp3_out);
        }

        for (int i = 0; i < tag_towrite_size; i++) // запишем новый фрейм в файл
        {
            fputc(tag_towrite_value[i], mp3_out);
        }

        for (int i = 0; i <= start_of_tag; i++) // запишем все что идет до старой версии данного фрейма в тэге
        {
            fputc(buf[i], mp3_out);
        }

        for (int i = end_of_tag + 1; i < size; i++) // запишем все что идет после 
        {
            fputc(buf[i], mp3_out);
        }
    }

    long cur_pos = ftell(mp3); // узнаем текущую позицию курсора относительно начала файла - она должна быть на конце ID3 header'a
    fseek(mp3, 0, SEEK_END); // вычислим сколько всего байт в файле
    long end_of_file = ftell(mp3);
    fseek(mp3, cur_pos, SEEK_SET); // вернемся на раннюю позицию, тем самым узнав количество байт до конца файла
    cur_pos = ftell(mp3);

    buf = realloc(buf, end_of_file - cur_pos + 1); // считаем это количество в буфер 
    fread(buf, sizeof(char), (end_of_file - cur_pos + 1), mp3);
    int counter = 0;
    for (int i = 0; i < (end_of_file - cur_pos); i++) { // запишем все символы в файл
        fputc(buf[i], mp3_out);
        counter += 1;
    }

    fclose(mp3); // закроем файлы 
    fclose(mp3_out);
    remove(file_path);
    rename("temp.mp3", file_path); // подменим оригинальный изменненым
    free(bits); // освободим память
    free(header);
    free(buf);
}

void get_pic(FILE* mp3, int* counter) {
    Picture* pic;
    char* buf = malloc(10*sizeof(char));
    unsigned char* size_buf = malloc(4*sizeof(char));
    fread(size_buf, sizeof(char), 4, mp3);
    int size = 0;
    int power = 1;
    char byte = 1;
    
    for (int i = 0; i < 4; i++) {

        power = pow(256, 3 - i);
        size += size_buf[i]*power;
        printf("%d\n", size);

    }
    printf(" 609 Prev size is %d\n", size);
    
    pic = malloc(size*sizeof(char) + sizeof(int) + 100);
    pic->size = size;
    fgetc(mp3);
    fgetc(mp3);
    printf("Position after header  %d\n", ftell(mp3));
    pic->encoding = fgetc(mp3);
    size -= 1;

    // for (int i = 0; i < 6; i++) {
    //     size -= 1;
    // } 


    int i = 0;
    byte = 1;
    buf[0] = 1;

    while (byte != '\0') {

        byte = fgetc(mp3);
        buf[i] = byte;
        size -= 1;
        i++;

    }
    
    pic->mime = buf;
    pic->mime = strtok(pic->mime, "/");
    pic->mime = strtok(NULL, "/");

    fgetc(mp3);
    size -= 1;
    byte = 1;
    i = 0;

    
    while (byte != '\0') {

        byte = fgetc(mp3);
        printf("Last byte is %d\n", byte);
        size -= 1;
        i++;

    }

    printf("Position before picture  %d\n", ftell(mp3));
    printf("SIZE is %d\n", size);
    char* image_path = calloc(10, sizeof(char));
    strcat(image_path, "image.");
    strcat(image_path, pic->mime);
    FILE* pic_out = fopen(image_path, "wb");
    if (pic_out != NULL) {
        printf("Opened image\n");
    }

    char* data_buf = malloc(size*sizeof(char));
    if (data_buf != NULL) {
        printf("YES\n");
    }
    char code = 1;
    for (int i = 0; i < size; i++) {
        code = fgetc(mp3);
        fputc(code, pic_out);
    }
    // fread(data_buf, sizeof(int), size, mp3);
    // fwrite(data_buf, sizeof(char), size, pic_out);
    // pic->data = data_buf;
    // for (int i = 0; i < size; i++) {
    //     printf("i is %d    %d \n", i, fgetc(mp3));
    // }
    // i = 0;
    // while (i < size) {
    //     printf("%d\n", (int)fgetc(mp3));
    //     i++;
    // }
    // printf("%s\n", data_buf);
    // for (int i = 0; i < size; i++) {
    //     code = fgetc(mp3);
    //     printf("%d ", code);
    //     fputc(code, pic_out);
    // }
    fclose(pic_out);
    *counter += pic->size + 10;
 
    return;

}
