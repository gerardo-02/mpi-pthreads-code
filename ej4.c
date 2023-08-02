// Calculo aproximado de PI mediante la serie de Leibniz e integral del cuarto de circulo
// https://es.wikipedia.org/wiki/Serie_de_Leibniz
// N.C. Cruz, Universidad de Granada, 2023

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <pthread.h>

// Parámetros que se le pasarán a cada hebra
typedef struct t_struct{
	int first;
	int last;

} tasks;

// Función que ejecuta cada hebra. Devuelve la suma de una porción
// de la serie.
void* body(void* param){
	tasks *this_task = (tasks*) param;
	double* partpi = malloc(sizeof(double));
	*partpi = 0.0;
	double num;
	if (this_task->first % 2 == 0)
		num = 1.0;
	else
		num = -1.0;
	double denom = 2*this_task->first + 1;
	printf("Empezando en first %d y hasta %d\n", this_task->first, this_task->last);
	for(int i = this_task->first; i < this_task->last; i++){
		*partpi += num/denom;
		num = -1.0*num; // Alternamos el signo
		denom += 2.0;
	}
	*partpi = *partpi * 4.0;
	return partpi;
}

int main(int argc, char* argv[]){

	// Comprobación del número de parámetros y declaración de variables
	if (argc != 3){
		printf("Numero incorrecto de parámetros introducido. Se debe introducir un único parámetro\n");
		return 0;
	}

	int rank, size, num_threads = 4, steps = 100000, mpi_quotient, mpi_rest, mpi_first, mpi_last, thread_quotient, thread_rest, thread_first, thread_last;
	double pi = 0.0;
	double tIni, tEnd;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	steps = atoi(argv[1]);
	num_threads = atoi(argv[2]);

	if(num_threads <= 0){
		printf("Numero de hebras incorrecto.\n");
		return 0;
	}

	// El proceso 0 comienza a calcular el tiempo
	if (rank == 0)
		tIni = MPI_Wtime();

	// División del número de iteraciones entre procesos

	mpi_quotient = steps / size;
	mpi_rest = steps % size;

	if (mpi_rest != 0){
		if (rank < mpi_rest){
			mpi_first = mpi_quotient*rank + rank;
			mpi_last = mpi_quotient*(rank+1) + rank+1;
		}
		else{
			mpi_first = mpi_quotient*rank + mpi_rest;
			mpi_last = mpi_quotient*(rank+1) + mpi_rest;
		}
	}
	else{
		mpi_first = mpi_quotient*rank;
		mpi_last = mpi_quotient*(rank+1);
	}

	pthread_t threads[num_threads];
	struct t_struct myTasks[num_threads];

	for (int i=0; i<num_threads; i++){

		// División del número de iteraciones entre cada hebra
		thread_quotient = (mpi_last-mpi_first) / num_threads;
		thread_rest = (mpi_last-mpi_first) % num_threads;
		if (thread_rest != 0){
			if (i < thread_rest){
				myTasks[i].first = thread_quotient*i + i + mpi_first;
				myTasks[i].last = thread_quotient*(i+1) + i+1 + mpi_first;
			}
			else{
				myTasks[i].first = thread_quotient*i + thread_rest + mpi_first;
				myTasks[i].last = thread_quotient*(i+1) + thread_rest + mpi_first;
			}
		}
		else{
			myTasks[i].first = thread_quotient*i + mpi_first;
			myTasks[i].last = thread_quotient*(i+1) + mpi_first;
		}

		// Ejecución de las hebras
		pthread_create(&threads[i], 0, body, &(myTasks[i]));
	}

	// Recepción de las hebras en cada proceso
	void* buffer = 0;
	for (int i=0; i<num_threads; i++){
		pthread_join(threads[i], &buffer);
		pi += *((double*) buffer);
		free(buffer);
	}

	if(rank==0){

		// Recepción de los procesos en el proceso 0
		double pi_recv = 0.0;
		for (int i=1; i<size; i++){
			MPI_Recv(&pi_recv, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
			pi += pi_recv;
		}

		// Finalización del cronómetro y puesta en pantalla del resultado
		tEnd = MPI_Wtime();
		printf("PI por la serie de G. Leibniz [%d iteraciones] =\t%17.15lf\nEjecutado con %d procesos y %d hilos en cada proceso\nTiempo total: %.3lf s\n", steps, pi, size, num_threads, tEnd-tIni);
	}
	else{
		// Todos los procesos que no son el 0 envían su suma al 0
		MPI_Send(&pi, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	pthread_exit(0);

	return 0;
}
