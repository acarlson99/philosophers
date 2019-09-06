#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock;

void *myThreadFun(void *arg) {

	(void)arg;
	printf("%d: LOCK %d\n", (int)arg, pthread_mutex_lock(&lock));
	printf("%d: THREAD SLEEPING\n", (int)arg);
	sleep(1);
	printf("%d: THREAD SLEPT\n", (int)arg);

	pthread_mutex_unlock(&lock);

	return NULL;
}

int main() {
	// pthread_t thread_id;
	pthread_t ids[5];

	pthread_mutex_init(&lock, NULL);

	printf("Begin\n");
	for (int ii = 0; ii < 5; ++ii)
		pthread_create(&ids[ii], NULL, myThreadFun, (void*)(size_t)ii);
	for (int ii = 0; ii < 5; ++ii) {
		pthread_join(ids[ii], NULL);
		printf("JOINED %d\n", ii);
	}
	printf("All threads joined\n");

	pthread_mutex_destroy(&lock);
}
