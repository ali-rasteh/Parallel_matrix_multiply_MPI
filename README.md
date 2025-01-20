# Parallel Matrix Multiply in MPI

This project implements parallel matrix multiplication using MPI (Message Passing Interface) in C.

## Project Structure

- `Makefile`: File for compiling the project.
- `README.md`: Project documentation.
- `mpi_mm/mm.c`: Contains detailed implementation of matrix multiplication and timing functions.

## Requirements

- MPI Library (e.g., OpenMPI)

## Compilation

To compile the program, use the following command:

```sh
make
```

## Usage

To run the program, use the following command:

```sh
mpirun -np <number_of_processes> ./mpi_mm
```

Replace `<number_of_processes>` with the desired number of processes.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.

Feel free to modify the content as needed.

Replace <number_of_processes> with the desired number of processes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.
