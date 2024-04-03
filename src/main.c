/**
 * @file main.c
 * @author Quentin Rollet (rolletquen@gmail.com)
 * @brief A program to find all the prime numbers between 0 and a given number
 * with a pool of multiple processes
 * @version 0.1
 * @date 2024-03-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "prime.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define BATCH_SIZE 10 // the number of numbers to check in each batch

/**
 * @brief A structure to store the data sent through the pipe
 * 
 */
struct PipeData {
    int i;
    int j;
};

/**
 * @brief Check if all the processes in the pool have finished
 * 
 * @param pool the pool of processes
 * @param pool_size the size of the pool
 * @return int 1 if all the processes have finished, 0 otherwise
 */
int all_processes_finished(int *pool, int pool_size)
{
    for (int i = 0; i < pool_size; i++)
    {
        if (pool[i] == 0)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Sort an array of integers
 * 
 * @param array the array to sort
 * @param size the size of the array
 * @return int* the sorted array
 */
int* sort(int *array, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            if (array[i] > array[j]) {
                int tmp = array[i];
                array[i] = array[j];
                array[j] = tmp;
            }
        }
    }

    return array;
}

int main(int argc, char const *argv[])
{
    // verify the number of arguments
    if (argc != 3)
    {
        printf("Usage: %s <max_number> <pool_size>\n", argv[0]);
        return 1;
    }

    // get the number from the arguments
    int max_number = atoi(argv[1]);
    int pool_size = atoi(argv[2]);

    // declare some variables
    int *primes_number = malloc((max_number) * sizeof(int)); // TODO : optimize the size of the array

    // create the pool of processes
    pid_t *pool = malloc(pool_size * sizeof(pid_t));

    // create a pipe to communicate with the parent process
    int *prime_numbers_pipe = malloc(2 * sizeof(int));
    if (pipe(prime_numbers_pipe) < 0)
    {
        perror("pipe");
        return 1;
    }
    
    // create pipes to communicate with each child processes
    int **pipes = malloc(pool_size * sizeof(int *));
    for (int i = 0; i < pool_size; i++)
    {
        pipes[i] = malloc(2 * sizeof(int));
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            return 1;
        }
    }

    // create the processes
    for (int i = 0; i < pool_size; i++)
    {
        if ((pool[i] = fork()) < 0)
        {
            perror("fork");
            return 1;
        }
        else if (pool[i] == 0)
        {
            int is_finished = 0;
            while (!is_finished)
            {
                int start;
                int end;

                // read the range of numbers to check
                read(pipes[i][0], &start, sizeof(int));
                read(pipes[i][0], &end, sizeof(int));

                // printf("Process %d: %d %d\n", i, start, end);

                // we have nothing to do
                if (start == 0 && end == 0)
                {
                    is_finished = 1;
                    continue;
                }

                for (int j = start; j <= end; j++)
                {
                    if (isPrime(j))
                    {
                        struct PipeData data;
                        data.i = i;
                        data.j = j;

                        write(prime_numbers_pipe[1], &data, sizeof(struct PipeData));
                    }
                }
                struct PipeData data;
                data.i = i;
                data.j = 0;
                write(prime_numbers_pipe[1], &data, sizeof(struct PipeData));
            }

            exit(0);
        }
    }

    // the last end range sent to the child
    int last_end = 0;

    // send to each child the range of numbers to check
    for (int i = 0; i < pool_size; i++)
    {
        // we take the min of BATCH_SIZE and the number of numbers to check divided by the number of processes
        int start = i * (MIN(BATCH_SIZE, max_number / pool_size));
        int end = (i + 1) * (MIN(BATCH_SIZE, max_number / pool_size));
        
        if (end > max_number)
        {
            end = max_number;
        }

        last_end = end;

        // send the range to the child
        write(pipes[i][1], &start, sizeof(int));
        write(pipes[i][1], &end, sizeof(int));
    }

    int is_main_process_finished = 0;
    int *pool_finished = malloc(pool_size * sizeof(int));
    for (int i = 0; i < pool_size; i++)
    {
        pool_finished[i] = 0;
    }
    int i = 0;
    int zero = 0;

    // while we have not reached the end
    while (!is_main_process_finished)
    {
        /* 
        * Read the prime numbers from the named pipe "prime_numbers_pipe".
        * To identify the child process that sent the prime numbers,
        * this one write the prime numbers with the following format:
        * - the first number is the number of the child process
        * - the following number is the prime number found by the child process
        * To know when the child process has finished, it writes 0 instead of
        * the prime number.
        * When a line is read, the parent delete this one from the pipe.
        */
        int prime_number;
        int child_number;
        read(prime_numbers_pipe[0], &child_number, sizeof(int));
        read(prime_numbers_pipe[0], &prime_number, sizeof(int));

        // the child process has finished
        if (prime_number == 0) {
            // we have to send a new range to the child
            if (last_end < max_number) {
                int start = last_end + 1;
                int end = start + MIN(BATCH_SIZE, max_number - last_end+1);
                last_end = end;
                write(pipes[child_number][1], &start, sizeof(int));
                write(pipes[child_number][1], &end, sizeof(int));
            } 
            // we don't have a new range to send
            else {
                pool_finished[child_number] = 1;
                write(pipes[child_number][1], &zero, sizeof(int));
                write(pipes[child_number][1], &zero, sizeof(int));
            }
        } else {
            primes_number[i] = prime_number;
            i++;
        }

        is_main_process_finished = all_processes_finished(pool_finished, pool_size);
        
    }

    int last_index_prime_number = i;

    // sort the prime numbers
    primes_number = sort(primes_number, last_index_prime_number);

    // print the prime numbers
    for (int i = 0; i < last_index_prime_number; i++)
    {
        if (primes_number[i] != 0)
        {
            printf("%d\n", primes_number[i]);
        }
    }

    return 0;
}
