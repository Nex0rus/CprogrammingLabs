#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <math.h>
#include <wchar.h>
#include <malloc.h>


typedef struct Frame {
    char* name;
    int size;
    char* info;
    int num_of_symbols;
}Frame;

void command_handler(int argc, char* argv[]);
char* write_bits(char* bits, char byte);
void get_version(char* array_to_write, char* bits, char byte, int* counter);
void show(FILE* mp3);
int parse_ID3_header(FILE* mp3);
void raise_error();
Frame* parse_frame(FILE* mp3, int* counter, char* buf, Frame* frame);
int skip_frame(FILE* mp3, int counter);
void get_frame(FILE* mp3, char* tag);



int main(int argc, char* argv[]) {
    command_handler(argc, argv);
    return 0;
}

void raise_error() {
    printf("Wrong parameters format\nPlease input valid parameters\nThe list of parameters:\n --filepath=\"filepath\" this is an esential parameter\n --show shows all meatinformation organised in a table\n --get=\"prop_name\" shows the value of a tag named prop_name\n --set=\"prop_name\" --value=\"value\" changes tag's named prop_name value to value parameter\n");
    exit(0);
}

void command_handler(int argc, char* argv[]) {
    if (argc < 3) {
        raise_error();
    }

    char* file_path;
    char* instruction;
    int* value;
    char* tag;
    FILE* mp3;

    strtok(argv[1], "=");
    file_path = strtok(NULL, "=");
    printf("%s\n", file_path);
    if ((mp3 = fopen(file_path, "r")) == NULL) {

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

            strtok(argv[3], "=");
            *value = atoi(strtok(NULL, "="));

        }
        
    }
    
}

char tagcmp(char* line1,char* line2) {
    for (int i = 0; i < 4; i++) {

        if (line1[i] != line2[i]) {
            return 0;
        }

    }

    return 1;
}

char* write_bits(char* bits, char byte) {

    for (int i = 0; i < 8; i++) {

        bits[i] = (byte >>(sizeof(byte)*8 -i-1)) & 1;

    }

    return bits;

}

int parse_ID3_header(FILE* mp3) {
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

    for (int i = 3; i < 8; i++) {

        if (bits[i] == '1') {
            free(bits);
            return 0;
        }

    }

    for (int i = 0; i < 4; i++) {

        byte = fgetc(mp3);
        size += byte * pow(128, 3-i);

    }

    free(bits);
    return size*10 + is_extended_header;

}

int skip_frame(FILE* mp3, int counter) {
    char* buf = malloc(4*sizeof(char));
    int size;
    fread(buf, sizeof(char), 4, mp3);

    for (int i = 0; i < 4; i++) {
        size += buf[i]*pow(256, 3-i);
    }
    counter += size + 6;
    fgetc(mp3);
    fgetc(mp3);
    for (int i = 0; i < size - 1; i++) {
        fgetc(mp3);
    }

    return counter;
    
}

void get_frame(FILE* mp3, char* tag) {
    char byte;
    int counter = 0;
    int size = parse_ID3_header(mp3);
    char is_extended_header = size % 10;
    size = size / 10;
    char* buf = malloc(4*sizeof(char));
    

    if (size == 0) {
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
    Frame* frame = malloc(sizeof(struct Frame));
   
    while (counter < size) {
        fread(buf, sizeof(char), 4, mp3);
        // for (int i = 0; i < 4; i++) {
        //     printf("%c", buf[i]);
        // }
        printf("\n");
        if (tagcmp(buf, tag)) {

            frame = parse_frame(mp3, &counter, buf, frame);
            printf("===============\n");
            for (int i = 0; i < 4; i++) {
                printf("%c", tag[i]);
            }
            printf("\n");

            for (int i = 0; i < frame->num_of_symbols; i++) {
                printf("%c", frame->info[i]);
            }
            printf("\n===============\n");

            return;
        }

        else {
            counter = skip_frame(mp3, counter);
        }
    }

    printf("No tag with ID: %s was found", tag);
    return;
}

void show(FILE* mp3) {
    char byte;
    int counter = 0;
    int size = parse_ID3_header(mp3);
    char is_extended_header = size % 10;
    size = size / 10;
    char* buf = malloc(100*sizeof(char));
    

    if (size == 0) {
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

    Frame* frame = malloc(sizeof(struct Frame));
   
    while (counter < size) {

        fread(buf, sizeof(char), 4, mp3);
        if (buf[0] == 84 || buf[0] == 87 || buf[0] == 67 || tagcmp(buf, "USLT")) {

            printf("==================\n");

            for (int i = 0; i < 4; i++) {
                printf("%c", buf[i]);
            }

            printf("\n");
            frame = parse_frame(mp3, &counter, buf, frame);

            for (int i = 0; i < frame->num_of_symbols; i++) {
                printf("%c", frame->info[i]);
            }

            printf("\n");

        }

        else {
            counter = skip_frame(mp3, counter);
        }

    }
}

Frame* parse_frame(FILE* mp3, int* counter, char* buf, Frame* frame) {
    char* tag = buf;
    int num_of_symbols = 0;
    // char* buf = malloc(4*sizeof(char));
    fread(buf, sizeof(char), 4, mp3);
    *counter += 4;
    frame->size = 0;

    for (int i = 0; i < 4; i++) {
        frame->size += (int)buf[i]*pow(256, 3-i);
    }
    num_of_symbols = frame->size;

    fread(buf, sizeof(char), 2, mp3); // Пропускаем флаги тэга
    *counter += 2;
    // if (tag[0] != 87) {
        fread(buf, sizeof(char), 1, mp3);
        if (buf[0] == 1) {
            num_of_symbols -= 3;
            fgetc(mp3);
            fgetc(mp3);
        }
        else {
            num_of_symbols -= 1;
        }
    // }

    fread(buf, sizeof(char), num_of_symbols, mp3);
    // printf("Size of the frame is %d\n", frame->size);
    frame->info = buf;
    *counter += frame->size;
    frame->num_of_symbols = num_of_symbols;
    return frame;
}

void set_value(FILE* mp3) {

}

// FILE* get_pic(FILE* mp3, int* counter, char* buf, Frame* frame) {
//     fread(buf, sizeof(char), 4, mp3);
//     *counter += 4;
//     frame->size = 0;

//     for (int i = 0; i < 4; i++) {
//         frame->size += (int)buf[i]*pow(256, 3-i);
//     } 
//     fgetc(mp3);
//     fgetc(mp3);
//     fgetc(mp3);
//     fread(buf, sizeof(char), )
// }