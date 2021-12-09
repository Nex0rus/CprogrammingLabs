#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#define HEADER_SIZE 62

struct Header { 
    int offset;
    int height;
    int width;
    int bit_depth;
    char* header;
    char* pixel_bytes;
};

void input_error() {
    printf(
        "Wrong input format, please check if you had passed valid arguements:\n--input <input_file.bmp> input_file.bmp is the name of the bmp setting first generation\n--output <dir_name> dir_name is the name of the directory to store generations\n"
    );
}
void game(char* file_path, char*  dir_name, int max_iter, int freq);
struct Header* parse_file(char* file_path);
unsigned char* set_map(struct  Header* header);
char* get_bits(char byte);
void output(unsigned char* map, int map_size, int generation, struct Header* header);
unsigned char* set_cell(int x, int y, int height, int width, unsigned char* map);

int main(int argc, char* argv[]) {
    if (argc < 5) {
        input_error;
    }

    char* file_path;
    char* dir_name;
    int max_iter = -1;
    int freq = 1;
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

    }

    game(file_path, dir_name, max_iter, freq);

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
    
    char* buf = malloc(HEADER_SIZE * sizeof(char));
    fread(buf, sizeof(char), HEADER_SIZE, file);
    header->header = buf;
    int size = 0;
    if (buf[0] != 'B' && buf[0] != 'M') {
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
    char* pixel_bytes = malloc(num_of_bytes);
    fseek(file, header->offset, SEEK_SET);
    fread(pixel_bytes, sizeof(char), num_of_bytes, file);
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

unsigned char* set_map(struct  Header* header) {
    int w = header->width;
    int h = header->height;
    char byte;
    char* bits;
    unsigned char* map = calloc(w * h, sizeof(char));

    for (int i = 0; i < w * h / 8; i++) {

        byte = header->pixel_bytes[i];
        bits = get_bits(byte);
        for (int j = 0; j < 8; j++) {

            if (bits[j] == 1) {
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
    // printf("%s\n", filename);
    FILE* file_out = fopen(filename, "wb");
    unsigned char byte = 0;
    fwrite(header->header, sizeof(char), HEADER_SIZE, file_out);
    for (int i = 0; i < map_size / 8; i++) {
        for (int j = 0; j < 7; j++){

            byte += (map[i * 8 + j] & 0x01);
            byte <<= 1;

        }

        byte += (map[(i * 8) + 7] & 0x01);
        fputc(byte, file_out);
        byte = 0;

    } 

    fclose(file_out);
}

void game(char* file_path, char*  dir_name, int max_iter, int freq) {
    struct Header* header = NULL;
    header = parse_file(file_path);

    if (header == NULL) {
        printf("Failed to read the file setting the first generation\n");
        return;
    }
    
    int w = header->width;
    int h = header->height;
    unsigned char* map;
    unsigned char* next_map = calloc(h * w, sizeof(unsigned char));
    map = set_map(header);
    // for (int y = 0; y < h; y++) {
    //     for (int x = 0; x < w; x++) {
    //         printf("%d ", map[y * w + x]);
    //     }
    // }
    // return;
    int generation = 0; 
    int position = 0;
    int changed = 1;

    while (generation - max_iter != 0) {

        if (changed == 0) {
            printf("The life has looped - eternal meaningless of existence makes no point in keeping the civilization alive\n");
            return;
        }

        changed = 0;
        generation += 1;

        for (int y = 0; y < h; y++) {

            for (int x = 0; x < w; x++) {

                position = y * w + x;
                if (map[position] != 0) {
            
                    if (((map[position] & 1) == 0) && ((map[position] >> 1) == 0x03)) {
                        changed = 1;
                        next_map = set_cell(x, y, h, w, next_map);
                    }

                    else if ((map[position] & 1 == 1) && ((map[position] >> 1) < 4) && ((map[position] >> 1) > 1)) {

                        changed = 1;
                        next_map = set_cell(x, y, h, w, next_map);

                    }

                }

            }
        }

        memcpy(map, next_map, h * w);

        // for (int y = 0; y < h; y++) {
        //     for (int x = 0; x < w; x++) {
        //         printf("%d ", map[y * w + x]);
        //     }
        //     printf("\n");
        // }
        // printf("\n\n\n\n\n\n\n");

        next_map = calloc(h * w, sizeof(unsigned char));
        if (generation % freq == 0) {
            output(map, h * w, generation, header);
        }
    }
    // for (int i = 0; i < h; i++) {
    //     for (int j = 0; j < w; j++) {
    //         printf("%d", map[i * w + j]);
    //     }
    //     printf("\n");
    // }



}