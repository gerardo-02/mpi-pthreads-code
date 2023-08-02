#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4

double *vec;

/*
 * A cada hebra se le pasarán dos números enteros que conforman el rango del
 * vector en el que la hebra
 * va a buscar el mayor elemento.
 */
typedef struct t_struct{
	int primer_elemento;
	int ultimo_elemento;
} rango;

/*
 * Función ejecutada por cada hebra. Los parámetros de entrada constituyen
 * el rango del vector en el que se va a buscar el mayor elemento.
 * Simplemente se recorre el vector entero actualizando la variable max
 * si se encuentra un elemento mayor que ella, y cuando ha terminado la devuelve
 */
void* body(void* param){
	rango *MiRango = (rango*) param;
	double *max = malloc(sizeof(double));
	*max = 0;
	for (int i = MiRango->primer_elemento; i < MiRango->ultimo_elemento; i++){
		if (vec[i] > *max){
			*max = vec[i];
		}
	}
	return max;
}

int main(int argc, char* argv[]){

	// Declaración de variables y asignación de valores al vector
	srand(time(NULL));
	int NUM_ELEMENTOS = atoi(argv[1]);
	struct t_struct rangos[NUM_THREADS];
	int cociente = NUM_ELEMENTOS / NUM_THREADS;
	int resto = NUM_ELEMENTOS % NUM_THREADS;
	double max_hilos = 0;

	vec = (double*)malloc(NUM_ELEMENTOS * sizeof(double));

	if (vec == NULL){
		printf("Memoria no reservada\n");
		exit(0);
	}

	for (int i=0; i<NUM_ELEMENTOS; i++){
		vec[i] = (double)rand();
	}

	// Asignación de la parte del vector que va a examinar cada hebra
	for (int i=0; i<NUM_THREADS; i++){
		rangos[i].primer_elemento = i * cociente;
		if (i == NUM_THREADS-1)
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente + resto;
		else
			rangos[i].ultimo_elemento = rangos[i].primer_elemento + cociente;

	}

	// Búsqueda con hilos
	pthread_t threads[NUM_THREADS];

	for(long int i = 0; i<NUM_THREADS; i++){
		pthread_create(&threads[i], 0, body, &(rangos[i]));
	}

	// Se recogen
	void* buffer = 0;
	for(long int i = 0; i<NUM_THREADS; i++){
		pthread_join(threads[i], &buffer);
		if (*((double*) buffer) > max_hilos)
			max_hilos = *((double*) buffer);
		free(buffer);
	}

	// Búsqueda secuencial
	double max_secuencial = 0.0;
	for (int i=0; i<NUM_ELEMENTOS; i++){
		if (vec[i] > max_secuencial)
			max_secuencial = vec[i];
	}
	printf("El elemento mayor del vector es %f\n", max_secuencial);
	printf("El elemento mayor del vector es %f\n", max_hilos);

	pthread_exit(0);

}
