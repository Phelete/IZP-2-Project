#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HORIZONTAL_LINE 0
#define VERTICAL_LINE 1
#define EMPTY_LINE (Line){-1, -1, 0, -1}

typedef struct {
    int x_coordinate;
    int y_coordinate;
    int length;
    int line_type;
} Line;

typedef struct {
    int width;
    int height;
    int **bitmap;
} Image;

int test_file(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    int rows;
    int cols;
    if (fscanf(file, "%d %d", &rows, &cols) != 2 || rows <= 0 || cols <= 0) {
        return 1;
    }

    int sym;
    int current_row = 0;
    int current_col = 0;
    while (fscanf(file, "%i", &sym) != EOF) {
        if (sym < 0 || sym > 1) {
            fclose(file);
            return 1;
        }

        current_col++;
        if (current_col == cols) {
            current_col = 0;
            current_row++;
        }
    }
    fclose(file);
    return !(current_row == rows && current_col == 0);
}

int allocate_bitmap(Image *image) {
    int **allocated_array = malloc(image->height * sizeof(int *));
    if(allocated_array == NULL) {
        return 1;
    }
    image->bitmap = allocated_array;

    for (int allocated_row = 0; allocated_row < image->height; allocated_row++) {
        int *allocated = malloc(image->width * sizeof(int));
        if(allocated == NULL) {
            for (int was_allocated = 0; was_allocated < allocated_row; was_allocated++) {
                free(image->bitmap[was_allocated]);
            }
            free(allocated_array);
            image->bitmap = NULL;
            return 1;
        }
        image->bitmap[allocated_row] = allocated;
    }
    return 0;
}

int free_bitmap(Image *image) {
    if (image->bitmap != NULL) {
        for(int row = 0; row < image->height; row++) {
            free(image->bitmap[row]);
            image->bitmap[row] = NULL;
        }
        free(image->bitmap);
        image->bitmap = NULL;
    }
    return 0;
}

int parse_image(Image *dst, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 1;
    }

    if (fscanf(file, "%d %d", &dst->height, &dst->width) != 2 || dst->height <= 0 || dst->width <= 0) {
        fclose(file);
        return 1;
    }

    if (allocate_bitmap(dst)) {
        fclose(file);
        return 1;
    }

    for (int row = 0; row < dst->height; row++) {
        for(int col = 0; col < dst->width; col++) {
            int value;
            if (fscanf(file, "%i", &value) != 1) {
                free_bitmap(dst);
                fclose(file);
                return 1;
            }
            dst->bitmap[row][col] = value;
        }
    }

    fclose(file);
    return 0;
}

void show_help() {
    printf("Usage: ./figsearch <operation> [...].\n");
    printf("Operations: \n");
    printf("  --help    Show help message.\n");
    printf("  test      Checking the input file for correct bitmap image content.\n");
    printf("  hline     Find the longest horizontal line in the image.\n");
    printf("  vline     Find the longest vertical line in the image.\n");
    printf("  square    Find the biggest square in the image.\n");
    printf("Example: ./figsearch --help\n");
}

int search_all_lines(const Image *image, Line **result, int *result_size, int lines_type) {
    if (image->height <= 0 || image->width <= 0 || image->bitmap == NULL) {
        fprintf(stderr, "Image is empty.\n");
        return -1;
    }
    int row_iterator = image->height;
    int col_iterator = image->width;
    if (lines_type == VERTICAL_LINE) {
        row_iterator = image->width;
        col_iterator = image->height;
    }

    if (*result == NULL) {
        *result_size = 10;
        *result = malloc(sizeof(Line) * *result_size);
        if (*result == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return -1;
        }
    }

    int lines_idx = 0;
    for (int row = 0; row < row_iterator; row++) {
        Line line = EMPTY_LINE;
        int in_line = 0;
        for (int col = 0; col < col_iterator; col++) {
            int pos_color;
            if (lines_type == VERTICAL_LINE)
                pos_color = image->bitmap[col][row];
            else
                pos_color = image->bitmap[row][col];

            if (pos_color) {
                if (!in_line) {
                    if (lines_type == VERTICAL_LINE) {
                        line.x_coordinate = col;
                        line.y_coordinate = row;
                    }
                    else {
                        line.x_coordinate = row;
                        line.y_coordinate = col;
                    }
                    line.length = 1;
                    in_line = 1;
                    continue;
                }
                line.length++;
                continue;
            }
            if (in_line) {
                if (lines_idx >= *result_size) {
                    int new_size = (*result_size) * 2;
                    Line *lines_extend = realloc(*result, sizeof(Line) * new_size);
                    if (lines_extend == NULL) {
                        return -1;
                    }
                    *result = lines_extend;
                    *result_size = new_size;
                }
                (*result)[lines_idx++] = line;
                in_line = 0;
                line = EMPTY_LINE;
            }
        }
        if (in_line) {
            if (lines_idx >= *result_size) {
                int new_size = (*result_size) * 2;
                Line *lines_extend = realloc(*result, sizeof(Line) * new_size);
                if (lines_extend == NULL) {
                    return -1;
                }
                *result = lines_extend;
                *result_size = new_size;
            }
            (*result)[lines_idx++] = line;
        }
    }
    return lines_idx;
}

int search_hline(const Image *image, Line *result) {
    if(image->height < 0 || image->width < 0 || image->bitmap == NULL)
        return -1;
    Line *lines = NULL;
    int size = 0;
    int found_count = search_all_lines(image, &lines, &size, HORIZONTAL_LINE);
    if (found_count > 0) {
        Line longest_line = EMPTY_LINE;
        for (int found_index = 0; found_index < found_count; found_index++) {
            if (lines[found_index].length > longest_line.length) {
                longest_line = lines[found_index];
            }
        }
        *result = longest_line;
        return longest_line.length;
    }
    free(lines);
    return 0;
}

int search_vline(const Image *image, Line *result) {
    if(image->height < 0 || image->width < 0 || image->bitmap == NULL)
        return -1;
    Line *lines = NULL;
    int size = 0;
    int found_count = search_all_lines(image, &lines, &size, VERTICAL_LINE);
    if (found_count > 0) {
        Line longest_line = EMPTY_LINE;
        for (int vline = 0; vline < found_count; vline++) {
            if (lines[vline].length > longest_line.length) {
                longest_line = lines[vline];
            }
        }
        *result = longest_line;
        return longest_line.length;
    }
    free(lines);
    return 0;
}

int main(int argc, char *argv[]) {
    char *command = "help";
    if(argc > 2) {
        command = argv[1];
    }

    if(strcmp(command, "test") == 0) {
        if (argc >= 3) {
            if(test_file(argv[2])) {
                fprintf(stderr,"%s", "Invalid");
                return 1;
            }
            printf("%s", "Valid");
        }
    }
    else if(strcmp(command, "hline") == 0) {
        if (argc < 3) {
            printf("%s", "Invalid argument count");
            show_help();
            return 1;
        }
        Image image;
        parse_image(&image, argv[2]);

        if(test_file(argv[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Line longest_line = EMPTY_LINE;
        int hline_len = search_hline(&image, &longest_line);
        free_bitmap(&image);

        if (hline_len == -1) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }
        if (longest_line.length-1 == 0) {
            printf("%s", "Not found");
            return 0;
        }
        printf("%i %i %i %i", longest_line.x_coordinate, longest_line.y_coordinate, longest_line.x_coordinate, longest_line.y_coordinate+longest_line.length-1);
    }
    else if(strcmp(command, "vline") == 0) {
        if (argc < 3) {
            printf("%s", "Invalid argument count");
            show_help();
            return 1;
        }
        Image image;
        parse_image(&image, argv[2]);

        if(test_file(argv[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Line longest_line = EMPTY_LINE;
        int vline_len = search_vline(&image, &longest_line);
        free_bitmap(&image);

        if (vline_len == -1) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }
        if (longest_line.length-1 == 0) {
            printf("%s", "Not found");
            return 0;
        }
        printf("%i %i %i %i", longest_line.x_coordinate, longest_line.y_coordinate, longest_line.x_coordinate+longest_line.length-1, longest_line.y_coordinate);
    }
    else if(strcmp(command, "square") == 0) {
        if (argc < 3) {
            printf("%s", "Invalid argument count");
            show_help();
            return 1;
        }
        Image image;
        parse_image(&image, argv[2]);

        if(test_file(argv[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }
    }
    else {
        show_help();
    }
    return 0;
}