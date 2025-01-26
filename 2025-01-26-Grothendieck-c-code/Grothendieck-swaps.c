#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 20000
#define T_MAX (2 * N - 3)
#define TRIES 1
#define COARSE 2000
#define PROB 0.5
#define Q 0

void generateSwaps(int t, int *swaps) {
    for (int i = 1; i < N; i++) {
        if ((t + i >= N) && (t - i <= N - 2) && ((t - i + N) % 2 == 0)) {
            swaps[i-1] = 1;
        } else {
            swaps[i-1] = 0;
        }
    }
}

void applyRandomSwap(int *sigma, int *swaps) {
    for (int i = 0; i < N - 1; i++) {
        if (swaps[i] == 1) {
            int count_swap = 1;
            if (sigma[i] < sigma[i + 1]  && ((double) rand() / RAND_MAX) < PROB) { // Swap criteria
                int temp = sigma[i];
                sigma[i] = sigma[i + 1];
                sigma[i + 1] = temp;
                continue;
            }
           	if (sigma[i] > sigma[i + 1] && ((double) rand() / RAND_MAX) < PROB * Q ) { // Swap criteria
                int temp1 = sigma[i];
                sigma[i] = sigma[i + 1];
                sigma[i + 1] = temp1;
                continue;
            }
        }
    }
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    int *sigma = malloc(N * sizeof(int));
    int *swaps = malloc((N - 1) * sizeof(int));

    int **tbl = malloc(TRIES * sizeof(int*));
    for (int i = 0; i < TRIES; i++) {
        tbl[i] = malloc(N * sizeof(int));
    }

    for (int trry = 0; trry < TRIES; trry++) {
        if (trry % 50 == 0) {
            fprintf(stderr, "%d / %d\n", trry, TRIES);
        }

        for (int i = 0; i < N; i++) {
            sigma[i] = i + 1;
            // sigma[i] = N-i;
        }

        for (int t = 1; t <= T_MAX; t++) {
            generateSwaps(t, swaps);
            // printf("\nSwaps: ");
            // for (int i = 0; i < N - 1; i++) {
            //     printf("%d ", swaps[i]);
            // }
            applyRandomSwap(sigma, swaps);
        }

        for (int i = 0; i < N; i++) {
            tbl[trry][i] = sigma[i];
        }
    }

		printf("{{");  // Begin the outer list
		for (int trry = 0; trry < TRIES; trry++) {
				printf("{");  // Begin an inner list
				for (int i = 0; i < N; i++) {
						printf("%d", tbl[trry][i]);  // Print the element
						if (i < N - 1) {
								printf(", ");  // Separate elements with a comma
						}
				}
				printf("}");  // End of an inner list
				if (trry < TRIES - 1) {
						printf(", ");  // Separate the inner lists with a comma
				}
		}
		printf("},");  // End the outer list


		int nCoarse = N / COARSE; // Size of the coarsened matrix
		// Coarsened result
    printf("{"); // Begin the outer list for coarse output
    for (int trry = 0; trry < TRIES; trry++) {
        // Create and initialize coarsened matrix
        int coarseMatrix[nCoarse][nCoarse];
        for (int i = 0; i < nCoarse; i++) {
            for (int j = 0; j < nCoarse; j++) {
                coarseMatrix[i][j] = 0;
            }
        }

        // Count placements
        for (int i = 0; i < N; i++) {
            int src = (i / COARSE);
            int dest = ((tbl[trry][i] - 1) / COARSE);
            coarseMatrix[src][dest]++;
        }

        // Output coarsened matrix
        printf("{");
        for (int i = 0; i < nCoarse; i++) {
            printf("{");
            for (int j = 0; j < nCoarse; j++) {
                printf("%d", coarseMatrix[i][j]);
                if (j < nCoarse - 1)
                    printf(", ");
            }
            printf("}");
            if (i < nCoarse - 1)
                printf(", ");
        }
        printf("}");
        if (trry < TRIES - 1)
            printf(", ");
    }
		/* printf("}}"); // End the outer list for coarse output */

		printf("},{"); // End the outer list for coarse output

		/* Now make one list which is the sum of all the coarsened matrices */
		int sumMatrix[nCoarse][nCoarse];
		for (int i = 0; i < nCoarse; i++) {
				for (int j = 0; j < nCoarse; j++) {
						sumMatrix[i][j] = 0;
				}
		}
		for (int trry = 0; trry < TRIES; trry++) {

				int coarseMatrix[nCoarse][nCoarse];
				for (int i = 0; i < nCoarse; i++) {
						for (int j = 0; j < nCoarse; j++) {
								coarseMatrix[i][j] = 0;
						}
				}

				// Count placements
				for (int i = 0; i < N; i++) {
						int src = (i / COARSE);
						int dest = ((tbl[trry][i] - 1) / COARSE);
						coarseMatrix[src][dest]++;
				}

				for (int i = 0; i < nCoarse; i++) {
						for (int j = 0; j < nCoarse; j++) {
								sumMatrix[i][j] += coarseMatrix[i][j];
						}
				}
		}
		//print this sum matrix in Mathematica-readable format

		for (int i = 0; i < nCoarse; i++) {
				printf("{");
				for (int j = 0; j < nCoarse; j++) {
						printf("%d", sumMatrix[i][j]);
						if (j < nCoarse - 1)
								printf(", ");
				}
				printf("}");
				if (i < nCoarse - 1)
						printf(", ");
		}
		printf("}}"); // End the outer list for the sum matrix
		printf("\n");  // Start a new line after the whole list




for(int i = 0; i < TRIES; i++) {
		printf("[");
		for(int j = 0; j < N; j++) {
				printf("%d", tbl[i][j]);
				if(j < N - 1) {
						printf(", ");
				}
		}
		printf("]");
		if(i < TRIES - 1) {
				printf("\n");
		}
}
    // Free memory
    free(sigma);
    free(swaps);
    for (int i = 0; i < TRIES; i++) {
        free(tbl[i]);
    }
    free(tbl);

printf("\n");

    return 0;
}


//print tbl in python format
