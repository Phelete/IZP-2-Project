/**
 * @author Behari Youssef
 * @name Figsearch
 * @date 24 November 2024
 * @version 1.0
 *
 * Description:
 * Algorithm to find some kinds of figures in bitmap image.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HORIZONTAL_LINE 0
#define VERTICAL_LINE 1
#define EMPTY_LINE (Line){{-1, -1}, 0, -1}
#define EMPTY_SQUARE (Square){{-1, -1}, {-1, -1}};

typedef struct {
    int x_coordinate;
    int y_coordinate;
} Point;

typedef struct {
    Point start;
    int length;
    int line_type;
} Line;

typedef struct {
    Point start_point;
    Point end_point;
} Square;

typedef struct {
    int width;
    int height;
    int **bitmap;
} Image;

/**
 * @brief Testing file for correct bitmap content.
 *
 * Checks if the file contains the correct bitmap definition.
 *
 * @param[in] filename Filename to test content in.
 * @return 0 if file contains correct bitmap definition(test is passed).
 * @return 1 if file contains wrong bitmap defenition(test is not passed).
 */
int test_file(const char *filename) {
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

/**
 * @brief Allocating space for store bitmap.
 *
 * @param[in] image Pointer to image to store bitmap content in.
 * @return 0 if allocation was successful(everything went well).
 * @return 1 if allocation was not successful(malloc error).
 */
int allocate_bitmap(Image *image) {
    int **allocated_array = malloc(image->height * sizeof(int *));
    if (allocated_array == NULL) {
        return 1;
    }
    image->bitmap = allocated_array;

    for (int allocated_row = 0; allocated_row < image->height; allocated_row++) {
        int *allocated = malloc(image->width * sizeof(int));
        if (allocated == NULL) {
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

/**
 * @brief Frees space where bitmap was stored.
 *
 * @param[in] image Pointer to image where bitmap stored is.
 * @return 0 if deallocation was successful(everything went well).
 */
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

/**
 * @brief Parses bitmap from to image structure.
 *
 * @param[in] filename Pointer to image where bitmap stored is.
 * @param[out] dst Pointer to image where will bitmap stored be.
 * @return 0 if parsing was successful(everything went well).
 * @return 1 if parsing occurred with an error(error while parsing).
 */
int parse_image(Image *dst, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 1;
    }

    if (fscanf(file, "%d %d", &dst->height, &dst->width) != 2 || dst->height <= 0 || dst->width <= 0) {
        fclose(file);
        return 1;
    }

    if (test_file(filename)) {
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

/**
 * @brief Search all lines in image.
 *
 * @param[in] image Pointer to image where bitmap stored is.
 * @param[out] result Pointer to line array where line will stored be.
 * @param[in] result_size Pointer to integer value, where line array size stored is.
 * @param[in] lines_type Line type to find.
 *
 * @return lines count If function executing was without error(everything went well).
 * @return -1 If error occurred while executing the function(error occurred).
 */
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
        *result_size = 1;
        *result = malloc(sizeof(Line) * *result_size);
        if (*result == NULL) {
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
                        line.start.x_coordinate = col;
                        line.start.y_coordinate = row;
                    }
                    else {
                        line.start.x_coordinate = row;
                        line.start.y_coordinate = col;
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
                    int new_size = (*result_size) + 1;
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
                int new_size = (*result_size) + 1;
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

int search_all_squares(const Image *image, Square **result, int *result_size) {
    if (image->height <= 0 || image->width <= 0 || image->bitmap == NULL) {
        fprintf(stderr, "Image is empty.\n");
        return -1;
    }

    if (*result == NULL) {
        *result_size = 1;
        *result = malloc(sizeof(Square) * *result_size);
        if (*result == NULL) {
            return -1;
        }
    }


    Line *horizontal_lines = NULL;
    int horizontal_size = 0;
    search_all_lines(image, &horizontal_lines, &horizontal_size, HORIZONTAL_LINE);

    Line *vertical_lines = NULL;
    int vertical_size = 0;
    search_all_lines(image, &vertical_lines, &vertical_size, VERTICAL_LINE);

    *result_size = 10;
    *result = malloc(sizeof(Square) * (*result_size));
    if (*result == NULL) {
        free(horizontal_lines);
        free(vertical_lines);
        return -1;
    }

    int square_count = 0;

    for (int vertical_idx = 0; vertical_idx < vertical_size; vertical_idx++) {
        for (int horizontal_idx = 0; horizontal_idx < horizontal_size; horizontal_idx++) {
            int start_coord_check =
                horizontal_lines[horizontal_idx].start.x_coordinate == vertical_lines[vertical_idx].start.x_coordinate &&
                horizontal_lines[horizontal_idx].start.y_coordinate == vertical_lines[vertical_idx].start.y_coordinate;

            int end_coord_check =
                horizontal_lines[horizontal_idx].start.x_coordinate ==
                    vertical_lines[vertical_idx].start.x_coordinate + vertical_lines[vertical_idx].length - 1 &&
                horizontal_lines[horizontal_idx].start.y_coordinate == vertical_lines[vertical_idx].start.y_coordinate;

            int len_check = horizontal_lines[horizontal_idx].length == vertical_lines[vertical_idx].length &&
                            horizontal_lines[horizontal_idx].length > 1;

            if ((start_coord_check || end_coord_check) && len_check) {
                if (square_count >= *result_size) {
                    int new_size = (*result_size) + 1;
                    Square *extended = realloc(*result, sizeof(Square) * (*result_size));
                    if (extended == NULL) {
                        free(horizontal_lines);
                        free(vertical_lines);
                        free(*result);
                        return -1;
                    }
                    *result = extended;
                    *result_size = new_size;
                }

                (*result)[square_count].start_point.x_coordinate = horizontal_lines[horizontal_idx].start.x_coordinate;
                (*result)[square_count].start_point.y_coordinate = horizontal_lines[horizontal_idx].start.y_coordinate;
                (*result)[square_count].end_point.x_coordinate = horizontal_lines[horizontal_idx].start.x_coordinate + horizontal_lines[horizontal_idx].length - 1;
                (*result)[square_count].end_point.y_coordinate = vertical_lines[vertical_idx].start.y_coordinate + vertical_lines[vertical_idx].length - 1;

                square_count++;
            }
        }
    }

    free(horizontal_lines);
    free(vertical_lines);

    return square_count;
}


/**
 * @brief Searchs the longest line in image.
 *
 * @param[in] image Pointer to image where bitmap stored is.
 * @param[out] result Pointer to line where result line will stored be.
 * @param[in] line_type Line type to finding.
 *
 * @return 0...n count of find lines(everything went well).
 * @return -1 if image definition is not correct (image error).
 */
int search_longest_line(const Image *image, Line *result, int line_type) {
    if (image->height < 0 || image->width < 0 || image->bitmap == NULL)
        return -1;

    Line *lines = NULL;
    int size = 0;
    int found_count = search_all_lines(image, &lines, &size, line_type);
    if (found_count > 0) {
        Line longest_line = EMPTY_LINE;
        for (int line = 0; line < found_count; line++) {
            if (lines[line].length > longest_line.length) {
                longest_line = lines[line];
            }
        }
        free(lines);
        lines = NULL;
        *result = longest_line;
        return longest_line.length;
    }
    return 0;
}

int search_biggest_square(const Image *image, Square *result) {
    if (result == NULL){}
    if (image->height < 0 || image->width < 0 || image->bitmap == NULL)
        return -1;
    Square *squares = NULL;
    int size = 0;
    int squares_count = search_all_squares(image, &squares, &size);
    if (squares_count == 0) {
        return 0;
    }
    Square biggest_square = EMPTY_SQUARE;
    for (int square_idx = 0; square_idx < squares_count; square_idx++) {
        int square_idx_perimeter = (squares[square_idx].end_point.x_coordinate - squares[square_idx].start_point.x_coordinate + 1) * 4;
        int biggest_perimeter = (biggest_square.end_point.x_coordinate - biggest_square.start_point.x_coordinate + 1) * 4;
        if (square_idx_perimeter > biggest_perimeter) {
            biggest_square = squares[square_idx];
        }
    }
    *result = biggest_square;
    return (biggest_square.end_point.x_coordinate - biggest_square.start_point.x_coordinate + 1) * 4;
}

/**
 * @brief Prints help message.
 */
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

int main(int argc, char *argv[]) {
    char *command = "help";
    if (argc > 2) {
        command = argv[1];
    }

    if (strcmp(command, "test") == 0) {
        if (argc >= 3) {
            if (test_file(argv[2])) {
                fprintf(stderr,"%s", "Invalid");
                return 1;
            }
            printf("%s", "Valid");
        }
    }
    else if (strstr(command, "line")) {
        if (strcmp(command, "hline") != 0 && strcmp(command, "vline") != 0) {
            return 0;
        }

        if (argc < 3) {
            printf("%s", "Invalid argument count");
            show_help();
            return 1;
        }

        int line_type = VERTICAL_LINE;
        if (strcmp(command, "hline") == 0) {
            line_type = HORIZONTAL_LINE;
        }

        Image image;

        if (parse_image(&image, argv[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Line longest_line = EMPTY_LINE;
        int hline_len = search_longest_line(&image, &longest_line, line_type);
        free_bitmap(&image);

        if (hline_len == -1) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }
        if (longest_line.length-1 == 0) {
            printf("%s", "Not found");
            return 0;
        }
        if (line_type == HORIZONTAL_LINE) {
            printf("%i %i %i %i", longest_line.start.x_coordinate, longest_line.start.y_coordinate,
                longest_line.start.x_coordinate, longest_line.start.y_coordinate+longest_line.length-1);
        }
        else {
            printf("%i %i %i %i", longest_line.start.x_coordinate, longest_line.start.y_coordinate,
                longest_line.start.x_coordinate+longest_line.length-1, longest_line.start.y_coordinate);
        }
    }
    else if (strcmp(command, "square") == 0) {
        if (argc < 3) {
            printf("%s", "Invalid argument count");
            show_help();
            return 1;
        }
        Image image;
        if (parse_image(&image, argv[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Square biggest_square = EMPTY_SQUARE;
        int biggest_perimeter = search_biggest_square(&image, &biggest_square);

        if (biggest_perimeter <= 0) {
            printf("%s", "Not found");
            return 0;
        }
        printf("%i %i %i %i\n", biggest_square.start_point.x_coordinate, biggest_square.start_point.y_coordinate,
            biggest_square.end_point.x_coordinate, biggest_square.end_point.y_coordinate);
    }
    else {
        show_help();
    }
    return 0;
}