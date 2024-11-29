/**
 * @author Behari Youssef
 * @name Figsearch
 * @date 29 November 2024
 * @file figsearch.c
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

/**
 * @brief Structure, describes Point object, contains 2 points.
 */
typedef struct {
    int x_coordinate;
    int y_coordinate;
} Point;

/**
 * @brief Structure, describes Line object, contains start point, line lenght, line type
 */
typedef struct {
    Point start;
    int length;
    int line_type;
} Line;

/**
 * @brief Structure, describes square object, contains start and end Points objects.
 */
typedef struct {
    Point start_point;
    Point end_point;
} Square;

/**
 * @brief Structure, describes image object, contains width, height and bitmap data.
 */
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

    int value;
    int current_row = 0;
    int current_col = 0;
    while (fscanf(file, "%i", &value) != EOF) { //Reading till the end
        if (value < 0 || value > 1) {
            fclose(file);
            return 1;
        }

        current_col++;
        if (current_col == cols) { //If line end, new line
            current_col = 0;
            current_row++;
        }
    }
    fclose(file);
    return !(current_row == rows && current_col == 0); // If last row and last col
}

/**
 * @brief Allocating space for store bitmap.
 *
 * @param[in] image Pointer to image to store bitmap content in.
 * @return 0 if allocation was successful(everything went well).
 * @return 1 if allocation was not successful(malloc error).
 */
int allocate_bitmap(Image *image) {
    int **allocated_array = malloc(image->height * sizeof(int *)); // 2-dimensional array(matrix) allocation
    if (allocated_array == NULL) { // If allocated
        return 1;
    }
    image->bitmap = allocated_array;

    for (int allocated_row = 0; allocated_row < image->height; allocated_row++) { // Allocating space for each row
        int *allocated = malloc(image->width * sizeof(int));
        if (allocated == NULL) {
            for (int was_allocated = 0; was_allocated < allocated_row; was_allocated++) { // Cleaning before allocated rows
                image->bitmap[was_allocated] = NULL;
                free(image->bitmap[was_allocated]);
            }
            free(allocated_array);
            image->bitmap = NULL;
            return 1;
        }
        image->bitmap[allocated_row] = allocated; // Setting bitmap value to new allocated matrix
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
    if (image->bitmap == NULL) {
        return 1;
    }
    for(int row = 0; row < image->height; row++) { // For each row in bitmap
        free(image->bitmap[row]);
        image->bitmap[row] = NULL;
    }
    free(image->bitmap);
    image->bitmap = NULL;
    return 0;
}

/**
 * @brief Parses bitmap from file to image structure.
 *
 * @param[in] filename Name of the image file.
 * @param[out] dst Pointer to image where will bitmap stored be.
 * @return 0 if parsing was successful(everything went well).
 * @return 1 if parsing occurred with an error(error while parsing).
 */
int parse_image(Image *dst, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return 1;
    }
    // Scanf must read 2 values, and they must be not 0
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

    for (int row = 0; row < dst->height; row++) { // For each row
        for(int col = 0; col < dst->width; col++) { // For each col in row
            int value;
            if (fscanf(file, "%d", &value) != 1) { // Reading data
                free_bitmap(dst);
                fclose(file);
                return 1;
            }
            dst->bitmap[row][col] = value; // Setting value
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
    if (image->height <= 0 || image->width <= 0 || image->bitmap == NULL) { // If image contains data
        fprintf(stderr, "Image is empty.\n");
        return -1;
    }
    int row_iterator = image->height;
    int col_iterator = image->width;
    if (lines_type == VERTICAL_LINE) { // Set the iteration method depending on line type
        row_iterator = image->width;
        col_iterator = image->height;
    }

    if (*result == NULL) { // Allocating space for the result array if it needed
        *result_size = 1;
        *result = malloc(sizeof(Line) * *result_size);
        if (*result == NULL) {
            return -1;
        }
    }

    int lines_idx = 0;
    for (int row = 0; row < row_iterator; row++) { // Iterating according to the setted iteration method
        Line line = EMPTY_LINE;
        int in_line = 0;
        for (int col = 0; col < col_iterator; col++) {
            int pos_color;
            if (lines_type == VERTICAL_LINE)
                pos_color = image->bitmap[col][row];
            else
                pos_color = image->bitmap[row][col];

            if (pos_color) {
                if (!in_line) { // Writing start pos to the line object
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
            if (in_line) { // Writing the next line parts(size)
                if (lines_idx >= *result_size) {
                    int new_size = (*result_size) + 1;
                    Line *lines_extend = realloc(*result, sizeof(Line) * new_size);
                    if (lines_extend == NULL) {
                        free(*result);
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
            if (lines_idx >= *result_size) { // Writing the last part of the line
                int new_size = (*result_size) + 1;
                Line *lines_extend = realloc(*result, sizeof(Line) * new_size);
                if (lines_extend == NULL) {
                    free(*result);
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

/**
 * @brief Search all squares in image.
 *
 * @param[in] image Pointer to image where bitmap stored is.
 * @param[out] result Pointer to square array where square will stored be.
 * @param[in] result_size Pointer to integer value, where square array size stored is.
 *
 * @return squares count If function executing was without error(everything went well).
 * @return -1 If error occurred while executing the function(error occurred).
 */
int search_all_squares(const Image *image, Square **result, int *result_size) {
    if (image->height <= 0 || image->width <= 0 || image->bitmap == NULL) { // If image contains data
        fprintf(stderr, "Image is empty.\n");
        return -1;
    }

    int rows = image->height;
    int cols = image->width;

    *result_size = 1;
    *result = malloc(sizeof(Square) * (*result_size));
    if (*result == NULL) { // Trying to allocate space for the result array
        return -1;
    }

    int square_count = 0;

    for (int start_row = 0; start_row < rows; start_row++) { // Iterating in the range of rows
        for (int end_row = start_row + 1; end_row < rows; end_row++) {
            int *add_row = malloc(sizeof(int) * rows); // Arrays for the row and col sum in the range
            int *add_col = malloc(sizeof(int) * cols);

            if (add_row == NULL || add_col == NULL) { // Trying to allocate space for the result array
                free(add_row);
                free(add_col);
                free(*result);
                return -1;
            }

            for (int col = 0; col < cols; col++) { // Iterating through the cols in the given row range
                if (image->bitmap[start_row][col]) { // If square found
                    if (square_count >= *result_size) { // If result array size must be resized
                        *result_size += 1;
                        Square *extended = realloc(*result, sizeof(Square) * (*result_size));
                        if (extended == NULL) {
                            free(*result);
                            return -1;
                        }
                        *result = extended;
                    }
                    (*result)[square_count].start_point.x_coordinate = start_row; // Writing square info
                    (*result)[square_count].start_point.y_coordinate = col;
                    (*result)[square_count].end_point.x_coordinate = start_row;
                    (*result)[square_count].end_point.y_coordinate = col;
                    square_count++;
                }
                if (image->bitmap[end_row][col]) {// If square found
                    if (square_count >= *result_size) { // If result array size must be resized
                        *result_size += 1;
                        Square *extended = realloc(*result, sizeof(Square) * (*result_size));
                        if (extended == NULL) {
                            free(*result);
                            return -1;
                        }
                        *result = extended;
                    }
                    (*result)[square_count].start_point.x_coordinate = end_row; // Writing square info
                    (*result)[square_count].start_point.y_coordinate = col;
                    (*result)[square_count].end_point.x_coordinate = end_row;
                    (*result)[square_count].end_point.y_coordinate = col;
                    square_count++;
                }
                add_row[col] = image->bitmap[start_row][col] + image->bitmap[end_row][col];
            }
            for (int row = 0; row < rows; row++) { // Summ of each row in the range
                add_col[row] = image->bitmap[row][start_row] + image->bitmap[row][end_row];
            }

            int row_distance = end_row - start_row; // Distance between start and end rows
            int row_sum = 0, col_sum = 0;

            for (int row_in_range = 0; row_in_range <= row_distance; row_in_range++) { // For each row between start and end rows
                row_sum += add_row[row_in_range];
                col_sum += add_col[row_in_range];
            }

            for (int start_col = 0; start_col <= cols - row_distance - 1; start_col++) { // Searching for the possible left side of the square
                int end_col = start_col + row_distance;
                if (row_sum == 2 * (row_distance + 1)) {
                    if (square_count >= *result_size) {
                        *result_size += 1;
                        Square *extended = realloc(*result, sizeof(Square) * (*result_size));
                        if (extended == NULL) { // Extend if needed
                            free(add_row);
                            free(add_col);
                            free(*result);
                            return -1;
                        }
                        *result = extended;
                    }
                    (*result)[square_count].start_point.x_coordinate = start_row; // Write the result
                    (*result)[square_count].start_point.y_coordinate = start_col;
                    (*result)[square_count].end_point.x_coordinate = end_row;
                    (*result)[square_count].end_point.y_coordinate = end_col;
                    square_count++;
                }

                row_sum = (end_col + 1 < cols) ? row_sum - add_row[start_col] + add_row[end_col + 1] : row_sum;
                col_sum = (end_col + 1 < rows) ? col_sum - add_col[start_col] + add_col[end_col + 1] : col_sum;
            }

            free(add_row);
            free(add_col);
        }
    }

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
    if (image->height < 0 || image->width < 0 || image->bitmap == NULL) // Checking the image for rhe correct bitmap
        return -1;

    Line *lines = NULL;
    int size = 0;
    int found_count = search_all_lines(image, &lines, &size, line_type); // Searching for all lines
    if (found_count <= 0) {
        return 0;
    }
    Line longest_line = EMPTY_LINE;
    for (int line = 0; line < found_count; line++) { // Comparing each line with the biggest
        if (lines[line].length > longest_line.length) {
            longest_line = lines[line];
        }
    }
    if (longest_line.length == 1 && line_type == VERTICAL_LINE) { // Searching each 1 lenght vertical line
        return search_longest_line(image, result, HORIZONTAL_LINE);
    }
    free(lines);
    lines = NULL;
    *result = longest_line;
    return longest_line.length;
}

/**
 * @brief Checks is square empty
 *
 * @param square
 * @return 1 If square is empty
 * @return 0 If square is not empty
 */
int is_empty(Square *square) {
    Square empty_square = EMPTY_SQUARE;
    return  square->start_point.x_coordinate == empty_square.start_point.x_coordinate && // Checking is square empty, by comparing square parameter
        square->end_point.x_coordinate == empty_square.end_point.x_coordinate &&
            square->start_point.y_coordinate == empty_square.start_point.y_coordinate &&
                square->end_point.y_coordinate == empty_square.end_point.y_coordinate;
}

/**
 * @brief Searchs biggest square in squares array.
 *
 * @param image
 * @param result
 * @return perimeter of the biggest square.
 * @return -1 if exectuion was not successful
 */
int search_biggest_square(const Image *image, Square *result) {
    if (image->height < 0 || image->width < 0 || image->bitmap == NULL)
        return -1;
    Square *squares = NULL;

    int size = 0;
    int squares_count = search_all_squares(image, &squares, &size);
    if (squares_count == -1) {
        return -1; // If error
    }

    Square biggest_square = EMPTY_SQUARE;
    for (int square_idx = 0; square_idx < squares_count; square_idx++) { // Searching the biggest square by comparing each perimiter
        int square_idx_perimeter = (squares[square_idx].end_point.x_coordinate - squares[square_idx].start_point.x_coordinate + 1) * 4;
        int biggest_perimeter = (biggest_square.end_point.x_coordinate - biggest_square.start_point.x_coordinate + 1) * 4;
        if (square_idx_perimeter > biggest_perimeter || is_empty(&biggest_square)) { // If current is biggest or empty
            biggest_square = squares[square_idx];
        }
    }
    free(squares);
    *result = biggest_square;
    return (biggest_square.end_point.x_coordinate - biggest_square.start_point.x_coordinate + 1) * 4; // Returning perimeter if the biggest square
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

/**
 *
 * @param command command to execute
 * @param arguments arguments to the given command
 * @return 0 if command execution was ok
 * @return 1 if command execution was with error
 */
int command_handler(char *command, char **arguments) {
    if (strcmp(command, "test") == 0) {
        if (test_file(arguments[2])) {
            fprintf(stderr,"%s", "Invalid");
            return 1;
        }
        printf("%s", "Valid");
    }
    else if (strstr(command, "line")) { // Handling all *line commands
        if (strcmp(command, "hline") != 0 && strcmp(command, "vline") != 0) { // If only hline or vline
            return 0;
        }

        int line_type = VERTICAL_LINE;
        if (strcmp(command, "hline") == 0) { // Setting line type by the picked method
            line_type = HORIZONTAL_LINE;
        }

        Image image;

        if (parse_image(&image, arguments[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Line longest_line = EMPTY_LINE;
        int hline_len = search_longest_line(&image, &longest_line, line_type);
        free_bitmap(&image);

        if (hline_len == -1) {
            fprintf(stderr,"%s", "Invalid"); // If error while searching
            return -1;
        }
        if (longest_line.length == 0) { // If no longest line(no lines)
            printf("%s", "Not found");
            return 0;
        }
        if (line_type == HORIZONTAL_LINE) { // Printing line according to the line type
            printf("%i %i %i %i", longest_line.start.x_coordinate, longest_line.start.y_coordinate,
                longest_line.start.x_coordinate, longest_line.start.y_coordinate+longest_line.length-1);
        }
        else {
            printf("%i %i %i %i", longest_line.start.x_coordinate, longest_line.start.y_coordinate,
                longest_line.start.x_coordinate+longest_line.length-1, longest_line.start.y_coordinate);
        }
    }
    else if (strcmp(command, "square") == 0) {
        Image image;
        if (parse_image(&image, arguments[2])) {
            fprintf(stderr,"%s", "Invalid");
            return -1;
        }

        Square biggest_square = EMPTY_SQUARE;
        int biggest_perimeter = search_biggest_square(&image, &biggest_square);

        free_bitmap(&image);

        if (biggest_perimeter <= 0) {
            printf("%s", "Not found"); // If no biggest square(no squares)
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

/**
 * @brief Entery point
 *
 * @param argc program argument count
 * @param argv program argument array
 * @return 0 if command execution was ok
 * @return 1 if command execution was with error
 */
int main(int argc, char *argv[]) {
    char *command = "help";
    if (argc >= 2) {
        command = argv[1];
    }

    if (strstr(command, "line") || strcmp(command, "square") == 0 || strcmp(command, "test") == 0) {
        if (argc < 3) {
            fprintf(stderr, "%s", "Invalid argument count\n");
            show_help();
            return 1;
        }
    }

    if (command_handler(command, argv)) {
        return 1;
    }
    return 0;
}