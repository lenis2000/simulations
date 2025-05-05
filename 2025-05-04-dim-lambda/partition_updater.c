/**
 * Partition Data Updater
 *
 * This program parses partition data from heuristic_results.txt, calculates
 * c(lambda) values using high-precision arithmetic, and updates a JSON file
 * with the new data when the dimension is greater than what's currently stored.

 COMPILE COMMAND:
 clang -o partition_updater partition_updater.c -lgmp -lmpfr -lm -O2 -I/opt/homebrew/include -L/opt/homebrew/lib

 USAGE:
 ./partition_updater heuristic_results.txt output.json
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gmp.h>
#include <mpfr.h>

#define MAX_LINE_LENGTH 1048576  /* 1 MB buffer for line reading */
#define MAX_PARTITION_SIZE 1024
#define DEFAULT_GMP_PRECISION 512

typedef struct {
    char *dimension;
    int *partition;
    int partition_size;
    double c_lambda;
} PartitionEntry;

typedef struct {
    char *key;
    PartitionEntry *entry;
} JsonEntry;

typedef struct {
    JsonEntry **entries;
    int size;
    int capacity;
} JsonObject;

/**
 * Calculate c(lambda) = -log(f^lambda / sqrt(n!)) / sqrt(n)
 * Using pure MPFR for high-precision calculation throughout
 */
void compute_c_lambda(mpf_t c_lambda_result, const mpz_t f_lambda, unsigned long int n) {
    // Use MPFR for the entire calculation to maintain maximum precision
    mpz_t n_factorial;
    mpfr_t mpfr_n_factorial, mpfr_sqrt_n_factorial, mpfr_f_lambda;
    mpfr_t mpfr_ratio, mpfr_log_ratio, mpfr_sqrt_n, mpfr_result;

    // Initialize GMP variables for n!
    mpz_init(n_factorial);

    // Initialize all MPFR variables with high precision
    mpfr_init2(mpfr_n_factorial, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_sqrt_n_factorial, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_f_lambda, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_ratio, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_log_ratio, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_sqrt_n, DEFAULT_GMP_PRECISION);
    mpfr_init2(mpfr_result, DEFAULT_GMP_PRECISION);

    // Calculate n!
    mpz_fac_ui(n_factorial, n);

    // Convert n! to MPFR
    mpfr_set_z(mpfr_n_factorial, n_factorial, MPFR_RNDN);

    // Calculate sqrt(n!) using MPFR
    mpfr_sqrt(mpfr_sqrt_n_factorial, mpfr_n_factorial, MPFR_RNDN);

    // Convert f_lambda to MPFR
    mpfr_set_z(mpfr_f_lambda, f_lambda, MPFR_RNDN);

    // Calculate ratio = f^lambda / sqrt(n!) using MPFR
    mpfr_div(mpfr_ratio, mpfr_f_lambda, mpfr_sqrt_n_factorial, MPFR_RNDN);

    // Check if ratio > 0 before taking log
    if (mpfr_sgn(mpfr_ratio) <= 0) {
        fprintf(stderr, "Warning: Ratio <= 0, cannot compute logarithm. Setting c_lambda to NaN.\n");
        mpf_set_d(c_lambda_result, NAN);
    } else {
        // Calculate log(ratio) using MPFR
        mpfr_log(mpfr_log_ratio, mpfr_ratio, MPFR_RNDN);

        // Calculate sqrt(n) using MPFR
        mpfr_set_ui(mpfr_sqrt_n, n, MPFR_RNDN);
        mpfr_sqrt(mpfr_sqrt_n, mpfr_sqrt_n, MPFR_RNDN);

        // Calculate result = log(ratio) / sqrt(n) using MPFR
        mpfr_div(mpfr_result, mpfr_log_ratio, mpfr_sqrt_n, MPFR_RNDN);

        // Negate the result using MPFR
        mpfr_neg(mpfr_result, mpfr_result, MPFR_RNDN);

        // Calculate c_lambda value with high precision

        // Convert the final MPFR result to mpf_t
        mpfr_get_f(c_lambda_result, mpfr_result, MPFR_RNDN);
    }

    // Clear all variables
    mpz_clear(n_factorial);
    mpfr_clear(mpfr_n_factorial);
    mpfr_clear(mpfr_sqrt_n_factorial);
    mpfr_clear(mpfr_f_lambda);
    mpfr_clear(mpfr_ratio);
    mpfr_clear(mpfr_log_ratio);
    mpfr_clear(mpfr_sqrt_n);
    mpfr_clear(mpfr_result);
}

/**
 * Parse a partition string like "[1, 2, 3]" into an array
 */
int *parse_partition_string(const char *partition_str, int *partition_size) {
    // Find the start and end of the partition array
    const char *start = strchr(partition_str, '[');
    if (!start) {
        fprintf(stderr, "Error: Invalid partition string format (missing '[').\n");
        return NULL;
    }

    const char *end = strchr(start, ']');
    if (!end) {
        fprintf(stderr, "Error: Invalid partition string format (missing ']').\n");
        return NULL;
    }

    // Copy the partition content without brackets for parsing
    char *partition_content = (char *)malloc(end - start);
    if (!partition_content) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    strncpy(partition_content, start + 1, end - start - 1);
    partition_content[end - start - 1] = '\0';

    // Count the number of commas to determine array size
    int count = 1;
    for (char *p = partition_content; *p; p++) {
        if (*p == ',') count++;
    }

    // Allocate memory for the partition array
    int *partition = (int *)malloc(count * sizeof(int));
    if (!partition) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(partition_content);
        return NULL;
    }

    // Parse each comma-separated value
    char *token = strtok(partition_content, ",");
    int i = 0;
    while (token && i < count) {
        // Trim leading/trailing spaces
        while (*token == ' ') token++;

        partition[i++] = atoi(token);
        token = strtok(NULL, ",");
    }

    *partition_size = i;
    free(partition_content);
    return partition;
}

// Initialize a new JSON object
JsonObject *json_object_new() {
    JsonObject *obj = (JsonObject *)malloc(sizeof(JsonObject));
    if (!obj) return NULL;

    obj->capacity = 16;  // Initial capacity
    obj->size = 0;
    obj->entries = (JsonEntry **)malloc(obj->capacity * sizeof(JsonEntry *));

    if (!obj->entries) {
        free(obj);
        return NULL;
    }

    return obj;
}

// Free a partition entry
void partition_entry_free(PartitionEntry *entry) {
    if (!entry) return;

    if (entry->dimension) free(entry->dimension);
    if (entry->partition) free(entry->partition);
    free(entry);
}

// Free a JSON entry
void json_entry_free(JsonEntry *entry) {
    if (!entry) return;

    if (entry->key) free(entry->key);
    if (entry->entry) partition_entry_free(entry->entry);
    free(entry);
}

// Free a JSON object
void json_object_free(JsonObject *obj) {
    if (!obj) return;

    for (int i = 0; i < obj->size; i++) {
        json_entry_free(obj->entries[i]);
    }

    free(obj->entries);
    free(obj);
}

// Get an entry from a JSON object by key
PartitionEntry *json_object_get(JsonObject *obj, const char *key) {
    if (!obj || !key) return NULL;

    for (int i = 0; i < obj->size; i++) {
        if (strcmp(obj->entries[i]->key, key) == 0) {
            return obj->entries[i]->entry;
        }
    }

    return NULL;
}

// Set or update an entry in a JSON object
int json_object_set(JsonObject *obj, const char *key, PartitionEntry *entry) {
    if (!obj || !key || !entry) return 0;

    // Check if key already exists
    for (int i = 0; i < obj->size; i++) {
        if (strcmp(obj->entries[i]->key, key) == 0) {
            // Replace existing entry
            partition_entry_free(obj->entries[i]->entry);
            obj->entries[i]->entry = entry;
            return 1;
        }
    }

    // Check if we need to expand the array
    if (obj->size >= obj->capacity) {
        obj->capacity *= 2;
        JsonEntry **new_entries = (JsonEntry **)realloc(obj->entries,
                                                      obj->capacity * sizeof(JsonEntry *));
        if (!new_entries) return 0;
        obj->entries = new_entries;
    }

    // Add new entry
    JsonEntry *new_entry = (JsonEntry *)malloc(sizeof(JsonEntry));
    if (!new_entry) return 0;

    new_entry->key = strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        return 0;
    }

    new_entry->entry = entry;
    obj->entries[obj->size++] = new_entry;

    return 1;
}

// Load a JSON object from file
JsonObject *json_load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        // File doesn't exist, create a new object
        return json_object_new();
    }

    JsonObject *obj = json_object_new();
    if (!obj) {
        fclose(file);
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    char current_key[32] = {0};
    PartitionEntry *current_entry = NULL;
    int parsing_object = 0;

    // Simple JSON parser (assumes well-formed JSON)
    while (fgets(line, sizeof(line), file)) {
        // Trim whitespace
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;

        // Check for object start/end
        if (strstr(trimmed, "{")) {
            if (strstr(trimmed, "\"")) {
                // Extract key
                char *key_start = strchr(trimmed, '"') + 1;
                char *key_end = strchr(key_start, '"');
                if (key_start && key_end) {
                    strncpy(current_key, key_start, key_end - key_start);
                    current_key[key_end - key_start] = '\0';

                    // Create new entry
                    current_entry = (PartitionEntry *)malloc(sizeof(PartitionEntry));
                    if (current_entry) {
                        current_entry->dimension = NULL;
                        current_entry->partition = NULL;
                        current_entry->partition_size = 0;
                        current_entry->c_lambda = 0.0;
                        parsing_object = 1;
                    }
                }
            }
        } else if (strstr(trimmed, "}")) {
            if (parsing_object && current_entry) {
                // Add entry to object
                json_object_set(obj, current_key, current_entry);

                // Reset for next entry
                memset(current_key, 0, sizeof(current_key));
                current_entry = NULL;
                parsing_object = 0;
            }
        } else if (parsing_object && current_entry) {
            // Check for "dimension"
            if (strstr(trimmed, "\"dimension\"")) {
                char *value_start = strchr(trimmed, ':') + 1;
                while (*value_start == ' ' || *value_start == '\t') value_start++;

                if (*value_start == '"') {
                    value_start++;
                    char *value_end = strchr(value_start, '"');
                    if (value_end) {
                        size_t len = value_end - value_start;
                        // Make sure we allocate enough space and don't truncate
                        current_entry->dimension = (char *)malloc(len + 1);
                        if (current_entry->dimension) {
                            memcpy(current_entry->dimension, value_start, len);
                            current_entry->dimension[len] = '\0';
                            // We've successfully read the dimension
                        } else {
                            fprintf(stderr, "Error: Memory allocation failed for dimension.\n");
                        }
                    }
                }
            }
            // Check for "partition"
            else if (strstr(trimmed, "\"partition\"")) {
                char *array_start = strchr(trimmed, '[');
                char *array_end = strrchr(trimmed, ']');

                if (array_start && array_end) {
                    // Extract array as string
                    char array_str[MAX_LINE_LENGTH];
                    int len = array_end - array_start + 1;
                    if (len < MAX_LINE_LENGTH) {
                        strncpy(array_str, array_start, len);
                        array_str[len] = '\0';

                        // Parse partition
                        current_entry->partition = parse_partition_string(array_str,
                                                                     &current_entry->partition_size);
                    }
                }
            }
            // Check for "c_lambda"
            else if (strstr(trimmed, "\"c_lambda\"")) {
                char *value_start = strchr(trimmed, ':') + 1;
                while (*value_start == ' ' || *value_start == '\t') value_start++;

                current_entry->c_lambda = atof(value_start);
            }
        }
    }

    fclose(file);
    return obj;
}

// Write a JSON object to file
int json_dump_file(JsonObject *obj, const char *filename) {
    if (!obj || !filename) return 0;

    FILE *file = fopen(filename, "w");
    if (!file) return 0;

    // Write opening brace
    fprintf(file, "{\n");

    // Write entries
    for (int i = 0; i < obj->size; i++) {
        JsonEntry *entry = obj->entries[i];
        PartitionEntry *partition_entry = entry->entry;

        fprintf(file, "    \"%s\": {\n", entry->key);

        // Write dimension - ensure we're writing the full, exact dimension string
        fprintf(file, "        \"dimension\": \"%s\",\n", partition_entry->dimension);

        // Write dimension to JSON

        // Write partition array
        fprintf(file, "        \"partition\": [");
        for (int j = 0; j < partition_entry->partition_size; j++) {
            fprintf(file, "%d", partition_entry->partition[j]);
            if (j < partition_entry->partition_size - 1) {
                fprintf(file, ", ");
            }
        }
        fprintf(file, "],\n");

        // Write c_lambda with high precision
        fprintf(file, "        \"c_lambda\": %.16f\n", partition_entry->c_lambda);

        // Close object
        if (i < obj->size - 1) {
            fprintf(file, "    },\n");
        } else {
            fprintf(file, "    }\n");
        }
    }

    // Write closing brace
    fprintf(file, "}\n");

    fclose(file);
    return 1;
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <input_txt_file> <json_file>\n", program_name);
    fprintf(stderr, "  <input_txt_file>: path to text file with partition data\n");
    fprintf(stderr, "  <json_file>: path to JSON file to update\n");
}

int main(int argc, char *argv[]) {
    // Set GMP precision
    mpf_set_default_prec(DEFAULT_GMP_PRECISION);

    // Parse command line arguments
    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Define filenames from command line arguments
    const char *input_txt_file = argv[1];
    const char *input_json_file = argv[2];
    const char *output_json_file = argv[2]; // Update the same file

    // Open input text file
    FILE *txt_file = fopen(input_txt_file, "r");
    if (!txt_file) {
        fprintf(stderr, "Error: Could not open input text file '%s'.\n", input_txt_file);
        return EXIT_FAILURE;
    }

    // Load JSON file or create a new one if it doesn't exist
    JsonObject *root = json_load_file(input_json_file);
    if (!root) {
        fprintf(stderr, "Error: Failed to create or load JSON object.\n");
        fclose(txt_file);
        return EXIT_FAILURE;
    }

    // Process the text file
    char line[MAX_LINE_LENGTH];
    unsigned long int current_n = 0;
    int parsing_block = 0;
    mpz_t current_f_lambda, existing_f_lambda;
    char f_lambda_str[MAX_LINE_LENGTH] = {0};
    int *partition = NULL;
    int partition_size = 0;

    // Read the file line by line
    while (fgets(line, MAX_LINE_LENGTH, txt_file)) {
        // Check for size marker
        if (strstr(line, "--- Size ")) {
            // Extract n value
            sscanf(line, "--- Size %lu ---", &current_n);
            parsing_block = 1;

            // Report progress
            printf("Processing n=%lu...\n", current_n);

            // Clear previous block data
            memset(f_lambda_str, 0, sizeof(f_lambda_str));
            if (partition) {
                free(partition);
                partition = NULL;
            }
            partition_size = 0;
        }

        // Extract Max f^lambda
        if (parsing_block && strstr(line, "Max f^lambda: ")) {
            // Extract just the digits
            char *start = strstr(line, "Max f^lambda: ") + 14;
            char *end = strchr(start, ' ');

            if (end) {
                size_t len = end - start;
                // Ensure we don't exceed buffer size
                if (len >= MAX_LINE_LENGTH) {
                    fprintf(stderr, "Error: Dimension value too large for buffer (size=%zu).\n", len);
                    continue;
                }
                memcpy(f_lambda_str, start, len);
                f_lambda_str[len] = '\0';
            } else {
                // If there's no space, just copy the rest of the line
                // Create a temporary copy to handle the newline
                char temp[MAX_LINE_LENGTH];
                strncpy(temp, start, MAX_LINE_LENGTH - 1);
                temp[MAX_LINE_LENGTH - 1] = '\0';

                // Remove potential newline
                char *newline = strchr(temp, '\n');
                if (newline) *newline = '\0';

                // Check length before copying
                if (strlen(temp) >= MAX_LINE_LENGTH) {
                    fprintf(stderr, "Error: Dimension value too large for buffer (size=%zu).\n", strlen(temp));
                    continue;
                }

                strcpy(f_lambda_str, temp);
            }

            // Successfully extracted the dimension
        }

        // Extract partitions
        if (parsing_block && strstr(line, "Partitions achieving maximum: ")) {
            // Parse the partition string
            partition = parse_partition_string(line, &partition_size);

            // We have all the data for this block, process it
            if (current_n > 0 && f_lambda_str[0] != '\0' && partition) {
                // Initialize GMP variables
                mpz_init(current_f_lambda);
                mpz_init(existing_f_lambda);

                // Convert f_lambda string to mpz_t
                if (mpz_set_str(current_f_lambda, f_lambda_str, 10) != 0) {
                    fprintf(stderr, "Error: Invalid f^lambda value '%s'.\n", f_lambda_str);
                    continue;
                }

                // Create key for JSON object
                char n_str[32];
                snprintf(n_str, sizeof(n_str), "%lu", current_n);

                // Check if we need to update
                int needs_update = 1;
                PartitionEntry *existing_entry = json_object_get(root, n_str);

                if (existing_entry && existing_entry->dimension) {
                    // Convert existing dimension to mpz_t
                    if (mpz_set_str(existing_f_lambda, existing_entry->dimension, 10) == 0) {
                        // Compare dimensions
                        if (mpz_cmp(current_f_lambda, existing_f_lambda) <= 0) {
                            needs_update = 0;
                            printf("Skipping n=%lu: keeping existing dimension\n", current_n);
                        } else {
                            printf("Found larger dimension for n=%lu\n", current_n);
                        }
                    } else {
                        fprintf(stderr, "Error: Failed to parse existing dimension '%s', will update anyway.\n",
                                existing_entry->dimension);
                    }
                } else {
                    printf("Adding new entry for n=%lu\n", current_n);
                }

                // Update JSON if needed
                if (needs_update) {
                    // Calculate c_lambda
                    mpf_t c_lambda_val;
                    mpf_init(c_lambda_val);
                    compute_c_lambda(c_lambda_val, current_f_lambda, current_n);

                    // Create new entry
                    PartitionEntry *new_entry = (PartitionEntry *)malloc(sizeof(PartitionEntry));
                    if (!new_entry) {
                        fprintf(stderr, "Error: Memory allocation failed.\n");
                        continue;
                    }

                    // Set dimension - ensure the full value is captured without truncation
                    char *dimension_str = mpz_get_str(NULL, 10, current_f_lambda);
                    // Double-check to make sure we got the full dimension string
                    if (dimension_str) {
                        new_entry->dimension = dimension_str;
                    } else {
                        fprintf(stderr, "Error: Failed to convert dimension to string.\n");
                        free(new_entry);
                        continue;
                    }

                    // Copy partition array
                    new_entry->partition = (int *)malloc(partition_size * sizeof(int));
                    if (new_entry->partition) {
                        memcpy(new_entry->partition, partition, partition_size * sizeof(int));
                        new_entry->partition_size = partition_size;
                    } else {
                        new_entry->partition_size = 0;
                    }

                    // Set c_lambda
                    new_entry->c_lambda = mpf_get_d(c_lambda_val);

                    // Update JSON object
                    json_object_set(root, n_str, new_entry);

                    // Clean up
                    mpf_clear(c_lambda_val);

                    printf("Updated n=%lu: new dimension saved\n", current_n);
                }

                // Clean up GMP variables
                mpz_clear(current_f_lambda);
                mpz_clear(existing_f_lambda);

                // Reset parsing state
                parsing_block = 0;
            }
        }
    }

    // Clean up any remaining partition array
    if (partition) {
        free(partition);
    }

    // Write updated JSON to file
    if (!json_dump_file(root, output_json_file)) {
        fprintf(stderr, "Error: Failed to write JSON to file '%s'.\n", output_json_file);
        json_object_free(root);
        fclose(txt_file);
        return EXIT_FAILURE;
    }

    // Clean up
    json_object_free(root);
    fclose(txt_file);

    printf("Successfully completed: results written to '%s'.\n", output_json_file);
    return EXIT_SUCCESS;
}
