#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t lock4;
pthread_mutex_t retrieveLock;
pthread_mutex_t insertLock;
int keys[NUM_KEYS];

typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;
    if (i==0){
    pthread_mutex_lock(&lock);
    }
    if(i==1){
        pthread_mutex_lock(&lock1);
    }
    if(i==2){
        pthread_mutex_lock(&lock2);
    }
    if(i==3){
        pthread_mutex_lock(&lock3);
    }
    if(i==4){
        pthread_mutex_lock(&lock4);
    }
    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;
    if (i==0){
        pthread_mutex_unlock(&lock);
    }
    if(i==1){
        pthread_mutex_unlock(&lock1);
    }
    if(i==2){
        pthread_mutex_unlock(&lock2);
    }
    if(i==3){
        pthread_mutex_unlock(&lock3);
    }
    if(i==4){
        pthread_mutex_unlock(&lock4);
    }
}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {
    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key){
            return b;
        }
    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
            insert(keys[key], tid);
    }
    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        if (retrieve(keys[key]) == NULL) lost++;
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);
    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_mutex_init(&lock3, NULL);
    pthread_mutex_init(&lock4, NULL);
    pthread_mutex_init(&retrieveLock, NULL);
    pthread_mutex_init(&insertLock, NULL);
    
    
    srandom(time(NULL));
    // Initialize keys
    for (i = 0; i < NUM_KEYS; i++) {
        keys[i] = random();
    }
    
    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }

    // Insert keys in parallel
    pthread_mutex_lock(&insertLock);
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }
    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    pthread_mutex_unlock(&insertLock);
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    pthread_mutex_lock(&retrieveLock);
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }
    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();
    pthread_mutex_unlock(&retrieveLock);
    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}
