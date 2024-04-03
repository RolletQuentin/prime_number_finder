import subprocess
import time
import matplotlib.pyplot as plt


def benchmark(upper_bound, num_processes):
    """Benchmark the performance of a C program.
    """
    # Command to execute the C program
    cmd = ["./bin/main", str(upper_bound), str(num_processes)]

    # Start the timer
    start_time = time.time()

    # Execute the command
    process = subprocess.run(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # End the timer
    end_time = time.time()

    # Calculate the execution time
    execution_time = end_time - start_time

    # Print the execution time
    print(
        f"Execution time for upper bound {upper_bound} and {num_processes} processes: {execution_time} seconds")

    # Return the execution time
    return execution_time


if __name__ == "__main__":

    # List of upper bounds and number of processes to test
    upper_bounds = [100000, 200000, 300000, 400000,
                    500000, 600000, 700000, 800000, 900000, 1000000]
    num_processes = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    # List to store the results
    results_upper_bounds = []
    results_processes = []

    # Test the C program with each combination of upper bound and number of processes
    for upper_bound in upper_bounds:
        execution_time = benchmark(upper_bound, 7)
        results_upper_bounds.append((upper_bound, 7, execution_time))

    for num_process in num_processes:
        execution_time = benchmark(1000000, num_process)
        results_processes.append((1000000, num_process, execution_time))

    # Extract data for the graph
    upper_bounds_data = list(zip(*results_upper_bounds))
    processes_data = list(zip(*results_processes))

    # Create the graph
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Graph 1: Execution time in function of the upper bound
    ax1.plot(upper_bounds_data[0],
             upper_bounds_data[2], marker='o', linestyle='-')
    ax1.set_title('Exucution time in function of the upper bound')
    ax1.set_xlabel('Upper bound')
    ax1.set_ylabel('Execution time (s)')

    # Graph 2: Execution time in function of the number of processes
    ax2.plot(processes_data[1], processes_data[2], marker='o', linestyle='-')
    ax2.set_title('Execution time in function of the number of processes')
    ax2.set_xlabel('Number of processes')
    ax2.set_ylabel('Execution time (s)')

    # Print the graphs
    plt.tight_layout()
    plt.show()
