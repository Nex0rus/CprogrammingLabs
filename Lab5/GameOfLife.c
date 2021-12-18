#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <dir.h>
#include <windows.h>
#define HEADER_SIZE 62
struct Header { 
    int offset;
    unsigned int height;
    unsigned int width;
    int bit_depth;
    char* header;
    char* pixel_bytes;
};

char* get_bits(char byte);
void game(char* file_path, char*  dir_name, int max_iter, int freq, int to_display, int to_create);
unsigned char* set_map(struct Header* header);
unsigned char* set_cell(int x, int y, int height, int width, unsigned char* map);
void display(int height, int width, unsigned char* map);
void output(unsigned char* map, int map_size, int generation, struct Header* header);
unsigned char* create(unsigned char* map, int map_height, int map_width);
unsigned char* add_object(unsigned char* map, char* src, int x, int y, int map_height, int map_width);
void show_object(char* obj_name);

static void inline print_object_list(void) {
    printf("======================\n Following objects can be created\n===Stable===\n 1. loaf\n 2. beehive\n 3. block\n 5. boat\n 6. tub\n===Oscillators===\n 1. blinker\n 2. toad\n 3. beacon\n 4. pulsar\n 5. i-column\n===Spaceships===\n 1. glider_left\n 2. glider_right\n 3. light_left\n 4. light_right\n 5. middle_left\n 6. middle_right\n 7. heavy_left\n 8. heavy_right\n======================\n");

}

void input_error(void) {
    printf(
        "Wrong input format, please check if you had passed valid arguements:\n--input <input_file.bmp> input_file.bmp is the name of the bmp setting first generation\n--output <dir_name> dir_name is the name of the directory to store generations\n"
    );
}

void set_console(void)
{   
    DWORD outMode = 0;
    HANDLE stdoutHandle;
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(stdoutHandle, &outMode);
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(stdoutHandle, outMode);
}

void restoreConsole(void) {
    printf("\x1b[0m"); 	
}

void display(int height, int width, unsigned char* map) {
    printf("\x1b%d", 7);
    for (int y = 0; y < height - 1; y++) {

        for (int x = 0; x < width; x++) {

                if ((map[y * width + x] & 0x01) == 1) {
                    printf("*");
                }

                else {
                    printf("");
                }

            }

            printf("\n");
        }
        printf("\x1b%d", 8);
        Sleep(2000);
         

}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        input_error();
    }

    char* file_path;
    char* dir_name;
    int max_iter = -1;
    int freq = 1;
    int to_display = 0;
    int to_create = 0;
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--input") == 0) {
            file_path = argv[i + 1];
        }

        if (strcmp(argv[i], "--output") == 0) {
            dir_name = argv[i + 1];
        }

        if (strcmp(argv[i], "--max_iter") == 0) {
            max_iter = atoi(argv[i + 1]);
        }

        if (strcmp(argv[i], "--dump_freq") == 0) {
            freq = atoi(argv[i + 1]);
        }

        if (strcmp(argv[i], "--display") == 0) {
            to_display = 1;
        }

        if (strcmp(argv[i], "--create") == 0) {
            to_create = 1;
        }

    }

    game(file_path, dir_name, max_iter, freq, to_display, to_create);

    return 0;
} 

struct Header* parse_file(char* file_path) {
    struct Header* header = malloc(sizeof(struct Header));
    header->offset = 0;
    header->bit_depth = 0;
    FILE* file = fopen(file_path, "rb");

    if (file == NULL) {
        return NULL;
    }
    
    unsigned char* buf = malloc(HEADER_SIZE * sizeof(unsigned char));
    fread(buf, sizeof(unsigned char), HEADER_SIZE, file); //считывам HEADER и INFOHEADER
    header->header = buf; 
    int size = 0;
    if (buf[0] != 'B' && buf[0] != 'M') { // проверяем что картинка bmp
        return NULL;
    }

    if ((buf[28] + buf[29] * 256) != 1) { //проверяем что глубина кодировки 1 бит на пиксель
        return NULL;
    }

    header->bit_depth = 1;
    header->offset = 0;

    for (int i = 10; i < 14; i++) {
        header->offset += pow(256, i - 10) * buf[i];
    }

    header->width = 0;
    for (int i = 18; i < 22; i++) {
        header->width += buf[i] * pow(256, i - 18);
    }

    header->height = 0;
    for (int i = 22; i < 26; i++) {
        header->height += buf[i] * pow(256, i - 22);
    }

    int num_of_bytes = header->height * header->width / 8;
    unsigned char* pixel_bytes = malloc(num_of_bytes);
    fseek(file, header->offset, SEEK_SET);
    fread(pixel_bytes, sizeof(unsigned char), num_of_bytes, file);
    header->pixel_bytes = pixel_bytes;

    return header;

}

char* get_bits(char byte) {

    char* bits = malloc(8 * sizeof(char));

    for (int i = 0; i < 8; i++) {
        
        bits[i] = (byte >> (sizeof(byte) * 8 - i - 1)) & 1;

    }

    return bits;

}

unsigned char* set_map(struct Header* header) {
    int w = header->width;
    int h = header->height;
    char byte;
    char* bits;
    unsigned char* map = calloc(w * h, sizeof(char));

    for (int i = 0; i < w * h / 8; i++) {

        byte = header->pixel_bytes[i];
        bits = get_bits(byte); // раскладываем байт на 8 бит (8 пикселей по 1 биту)

        for (int j = 0; j < 8; j++) {

            if (bits[j] == 1) { // проверяем если пиксель в bmp обозначен единицей - активируем соотвествующую клетку в битмап
                int x = (8 * i + j) % w; 
                int y = (8 * i + j) / w;
                set_cell(x, y, h, w, map);
            }

        }

    }

    free(bits);
    return map;

}

unsigned char* set_cell(int x, int y, int height, int width, unsigned char* map) {

    int cleft, cright, cabove, cbelow;
    int position = y * width + x; 
    map[position] |= 0x01;

    if (x == 0) {
        cleft = width - 1;
    }
    else {
        cleft = -1;
    }

    if (x == width - 1) {
        cright = -(width - 1);
    }
    else {
        cright = 1;
    }

    if (y == 0) {
        cabove = height - 1;
    }
    else {
        cabove = -1;
    }

    if (y == height - 1) {
        cbelow = -(height - 1);
    }
    else {
        cbelow = 1;
    }

    map[y * width + x + cleft] += 0x02;
    map[y * width + x + cright] += 0x02;
    map[(y + cabove) * width + x] += 0x02;
    map[(y + cbelow) * width + x] += 0x02;
    map[(y + cabove) * width + x + cleft] += 0x02;
    map[(y + cabove) * width + x + cright] += 2;
    map[(y + cbelow) * width + x + cleft] += 2;
    map[(y + cbelow) * width + x + cright] += 2;

    return map;
}

void output(unsigned char* map, int map_size, int generation, struct Header* header)  {
    
    char* filename = calloc(25, sizeof(char));
    char* gen = malloc(5 * sizeof(char));
    strcat(filename, "generation");
    itoa(generation, gen, 10);
    strcat(filename, gen);
    strcat(filename, ".bmp");
    FILE* file_out = fopen(filename, "wb");
    unsigned char byte = 0;
    fwrite(header->header, sizeof(char), HEADER_SIZE, file_out); // Пишем в файл неизменный header

    for (int i = 0; i < map_size / 8; i++) {

        for (int j = 0; j < 7; j++){

            byte += (map[i * 8 + j] & 0x01); //берем младщий бит (жива/мертва клетка) собираем в байт -> пишем в файл
            byte <<= 1;

        }

        byte += (map[(i * 8) + 7] & 0x01);
        fputc(byte, file_out);
        byte = 0;

    }
    // printf("Closing file\n");
    fclose(file_out);
    free(filename);
    free(gen);
}

void game(char* file_path, char*  dir_name, int max_iter, int freq, int to_display, int to_create) {
    set_console();
    struct Header* header = NULL;
    header = parse_file(file_path);
    if (header == NULL) {
        printf("Failed to read the file setting the first generation\n");
        return;
    }
    
    const int width = header->width;
    const int height = header->height;
    unsigned char* map;
    unsigned char* next_map = calloc(height * width, sizeof(unsigned char));
    map = set_map(header);
    int generation = 0; 

    if (to_create) {
        map = create(map, height, width);
    }

    if (opendir(dir_name) == NULL) {
        printf("Creating new directory\n");
        mkdir(dir_name);
        chdir(dir_name);
    }
    else {
        chdir(dir_name);
    }

    output(map, height * width, generation, header);
    int position = 0;
    int changed = 1;
    
    while (generation - max_iter != 0) {

        set_console();

        if (changed == 0) {
            printf("The life has looped - eternal meaningless of existence makes no point in keeping the civilization alive\n");
            return;
        }

        changed = 0;
        generation += 1;

        for (int y = 0; y < height; y++) {

            for (int x = 0; x < width; x++) {

                position = y * width + x;
                if (map[position] != 0) {
            
                    if (((map[position] & 1) == 0) && ((map[position] >> 1) == 0x03)) {

                        changed = 1;
                        next_map = set_cell(x, y, height, width, next_map);

                    }

                    else if ((map[position] & 1 == 1) && ((map[position] >> 1) < 4) && ((map[position] >> 1) > 1)) {

                        changed = 1;
                        next_map = set_cell(x, y, height, width, next_map);

                    }

                }

            }
        }

        memcpy(map, next_map, height * width);
        memset(next_map, 0, height * width);

        if (generation % freq == 0) {

            if (to_display) {
                display(width, height, map);
            }

            output(map, height * width, generation, header);

        }

        
    }
    
    restoreConsole();
    free(header->header);
    free(header->pixel_bytes);
    free(header);
    free(map);
    free(next_map);

}

unsigned char* create(unsigned char* map, int map_height, int map_width) {
    char* name;
    memset(name, 0, 20); 
    int x = 0;
    int y = 0;
    char* src = calloc(30, sizeof(unsigned char));
    print_object_list();
    while (1) {
    
        scanf("%s", name);
        if (strcmp(name, "done") == 0) {
            return map;
        }
        
        if (strcmp(name, "show") == 0) {
            scanf("%s", name);
            show_object(name);
            memset(name, 0, 20);
            continue;
        }

        strcat(src, "patterns/");
        strcat(src, name);
        strcat(src, ".bmp");

        FILE* src_file;
        if ((src_file = fopen(src, "rb")) == NULL) {
            printf("No such pattern in list, please choose existing pattern\n");
            continue;
        }

        fclose(src_file);

        scanf("%d%d", &x, &y);
        y = map_height - y;
        add_object(map, src, x, y, map_height, map_width);
        memset(src, 0, 30);
        memset(name, 0, 20);

    }

    free(name);
    free(src);
    return map;

}

unsigned char* add_object(unsigned char* map, char* src, int x, int y, int map_height, int map_width) {

    struct Header* header = NULL;
    header = parse_file(src);
    int pattern_height = header->height;
    int pattern_width = header->width;
    unsigned char* pattern_map = calloc(pattern_width * pattern_height, sizeof(unsigned char));
    pattern_map = set_map(header);

    for (int y_coord = 0; y_coord < pattern_height; y_coord++) {
        for (int x_coord = 0; x_coord < pattern_width; x_coord++) {
            if (((pattern_map[y_coord * pattern_width + x_coord] & 1) == 1) && ((map[(y + y_coord) * map_width + x + x_coord] & 1) == 0)) {
                set_cell(x + x_coord, y + y_coord, map_height, map_width, map);
            } 
        }
    }

    free(pattern_map);
    return map;

}

void show_object(char* obj_name) {
    char* src = calloc(30, sizeof(unsigned char));
    strcat(src, "patterns/");
    strcat(src, obj_name);
    strcat(src, ".bmp");
    struct Header* header = parse_file(src);
    unsigned char* obj_map = calloc(header->width * header->height, sizeof(char));
    obj_map = set_map(header);
    set_console();
    display(header->height, header->width, obj_map);
    Sleep(10);
    printf("\x1b[2J");
    restoreConsole();
    print_object_list();


}