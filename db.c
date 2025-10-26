#include <string.h>
#include <assert.h>
#include "db.h"
#include "indexer.h"
#include "utils.h"
#include "log.h"
#include <pthread.h>


DB* db_open_ex(const char* basedir, uint64_t cache_size)
{
    DB* self = calloc(1, sizeof(DB));

    if (!self)
        PANIC("NULL allocation");

    strncpy(self->basedir, basedir, MAX_FILENAME);
    self->sst = sst_new(basedir, cache_size);

    Log* log = log_new(self->sst->basedir);
    self->memtable = memtable_new(log);

    return self;
}

DB* db_open(const char* basedir)
{
    return db_open_ex(basedir, LRU_CACHE_SIZE);
}

void db_close(DB *self)
{
    INFO("Closing database %d", self->memtable->add_count);

    if (self->memtable->list->count > 0)
    {
        sst_merge(self->sst, self->memtable);
        skiplist_release(self->memtable->list);
        self->memtable->list = NULL;
    }

    sst_free(self->sst);
    log_remove(self->memtable->log, self->memtable->lsn);
    log_free(self->memtable->log);
    memtable_free(self->memtable);
    free(self);
}

int addwritenum(int writer_num){
	writer_num=writer_num +1;
	return writer_num;
}
int subwritenum(int writer_num){
	writer_num=writer_num-1;
	return writer_num;
}

int db_add(DB* self, Variant* key, Variant* value) // returns 1
{
    pthread_mutex_lock(&self->writer); // enter critical section, mutual exclusion with locks

    // insert key and value into memtable or sst if compaction is needed
    int memtableadd=0;	
    if (memtable_needs_compaction(self->memtable))
    {
        INFO("Starting compaction of the memtable after %d insertions and %d deletions",
             self->memtable->add_count, self->memtable->del_count);
        sst_merge(self->sst, self->memtable);
        memtable_reset(self->memtable);
    }
    memtableadd=memtable_add(self->memtable, key, value);

    pthread_mutex_unlock(&self->writer); // exit critical section
    // store the return value in memtableadd because the thread must see the function's return
    // before unlock, so we save it in a variable for the next thread call
    return memtableadd;
}

int addreadnum(int readers_num){
	readers_num=readers_num+1;
	return readers_num;
}
int subreadnum(int readers_num){
	readers_num=readers_num-1;
	return readers_num;
}

int db_get(DB* self, Variant* key, Variant* value)
{
    pthread_mutex_lock(&self->reader); // mutual exclusion
    self->readers_num=addreadnum(self->readers_num); // increment number of readers to allow only one writer
    if (self->readers_num==1) // lock if a writer is inside
		pthread_mutex_lock(&self->writer);
    pthread_mutex_unlock(&self->reader);
	
    int sstget=0;
    // first check in memtable, if not found
    if (memtable_get(self->memtable->list, key, value) == 1)
        sstget=1;
    // search in sst and return
    sstget=sst_get(self->sst, key, value);
    
    pthread_mutex_lock(&self->reader); // enter critical section
    self->readers_num =subreadnum(self->readers_num); // decrement reader count until 0 to allow writers
    if (self->readers_num==0) // exit readers and allow writers
		pthread_mutex_unlock(&self->writer);
    pthread_mutex_unlock(&self->reader); // exit critical section

    return sstget;
}

int db_remove(DB* self, Variant* key)
{
    return memtable_remove(self->memtable, key);
}

DBIterator* db_iterator_new(DB* db)
{
    DBIterator* self = calloc(1, sizeof(DBIterator));
    self->iterators = vector_new();
    self->db = db;

    self->sl_key = buffer_new(1);
    self->sl_value = buffer_new(1);

    self->list = db->memtable->list;
    self->prev = self->node = self->list->hdr;

    skiplist_acquire(self->list);

    // Acquire the immutable list if it exists
    pthread_mutex_lock(&self->db->sst->immutable_lock);

    if (self->db->sst->immutable_list)
    {
        skiplist_acquire(self->db->sst->immutable_list);

        self->imm_list = self->db->sst->immutable_list;
        self->imm_prev = self->imm_node = self->imm_list->hdr;
        self->has_imm = 1;
    }

    pthread_mutex_unlock(&self->db->sst->immutable_lock);

    // TODO: At this point we should get the current sequence of the active
    // SkipList to avoid polluting the iteration

    self->use_memtable = 1;
    self->use_files = 1;

    self->advance = ADV_MEM | ADV_MEM;

    return self;
}

void db_iterator_free(DBIterator* self)
{
    for (int i = 0; i < vector_count(self->iterators); i++)
        chained_iterator_free((ChainedIterator *)vector_get(self->iterators, i));

    heap_free(self->minheap);
    vector_free(self->iterators);

    buffer_free(self->sl_key);
    buffer_free(self->sl_value);

    if (self->has_imm)
    {
        buffer_free(self->isl_key);
        buffer_free(self->isl_value);
    }

    skiplist_release(self->list);

    if (self->imm_list)
        skiplist_release(self->imm_list);

    free(self);
}
