#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


void command_handler(int argc, char* argv[]);
// void show_list();
// void extract();
void create(char* arc_name, int num_of_files, char** files_list);

int main(int argc, char* argv[]) {
    command_handler(argc, argv);

    return 0;

}

void command_handler(int argc, char* argv[]) {
    
    int i = 0;
    char** files_list = malloc(100 * sizeof(char*)); 
    char* arc_name = NULL;
    
    int num_of_files = 0;

    if(argc < 4) {
        printf("Invalid input please correct the input parameters\n");
        return;
    }

    while (i < argc) {

        if (strcmp(argv[i], "--file") == 0) {
            arc_name = argv[i + 1];
        }

        if (strcmp(argv[i], "--create") == 0) {

            i++;
            int j = i;
            while (i != argc) {
                files_list[i - j] = argv[i];
                num_of_files++;
                i++;
            }

            create(arc_name, num_of_files, files_list);

        }

        if (strcmp(argv[i], "--list") == 0) {
            // show_list();
            ;
        }

        if (strcmp(argv[i], "--extract") == 0) {
            // extract();
            ;
        }

        i++; 

    }
    

}

void create(char* arc_name, int num_of_files, char** files_list) {
    
    if (arc_name == NULL) {
        printf("Invalid input, please input the name of the archive to be created using --file comandlet");
    }
    FILE* arcfile = NULL;
    FILE* archive = fopen(arc_name, "wb");
    FILE* archeader = fopen("archeader.ah", "wb");

    for(int i = 0; i < num_of_files; i++) {

        arcfile = fopen(files_list[i], "rb");
        if (arcfile == NULL) {
            printf("Didn't  open\n");
        }
        fprintf(archeader, "%s", files_list[i]);
        fputc('\0', archeader);
        fseek(arcfile, 0, SEEK_END);
        int file_size = ftell(arcfile);
        int tmp = file_size;

        for (int i = 0; i < 5; i++) {
            fputc(tmp%256, archeader);
            tmp /= 256;
        }

        fputc('\0', archeader);
        fclose(arcfile);

    }

    freopen("archeader.ah", "r", archeader);
    
    fseek(archeader, 0, SEEK_END);
    long header_size = ftell(archeader);
    fseek(archeader, 0, SEEK_SET);
    long tmp = header_size;

    for (int i = 0; i < 5; i++) {
        fputc((char)(tmp%256), archive);
        tmp /= 256;
    }

    char* buf = malloc(sizeof(char) * header_size);
    fread(buf, sizeof(char), header_size, archeader);
    fwrite(buf, sizeof(char), header_size, archive);

    for (int i = 0; i < num_of_files; i++) {

        arcfile = fopen(files_list[i], "rb");
        fseek(arcfile, 0, SEEK_END);
        long file_size = ftell(arcfile);
        fseek(arcfile, 0, SEEK_SET);
        buf = realloc(buf, file_size);
        fread(buf, sizeof(char), file_size, arcfile);
        fwrite(buf, sizeof(char), file_size, archive);

    }

    fclose(archive);
  
}