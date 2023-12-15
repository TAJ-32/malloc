#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

//is there gonna be a problem with all of these variables resetting everytime malloc is called? Or no because it's all in the heap?

#define MIN_SBRK 5000
#define ALIGN_SIZE 16

static char* break_loc = 0;
static char* init_prgrm_brk = 0;

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nelem, size_t elsize);

void *realloc(void *ptr, size_t size);

size_t malloc_usable_size(void *ptr);

struct allocation { 
	int user_size;
	int act_size; //actual size of whole chunk, sbrk_size + meta_size;
	int meta_size; //size of the struct in the chunk of memory
	struct allocation *next_alloc; //pointer to next allocation in the linked list
	struct allocation *prev_alloc;
};

static struct allocation *head = NULL;

//do we need to be making the metadata for the next allocation before it is even called
void *malloc(size_t size) {

	size = ((size / 16) + 1) * 16;

	if (head == NULL) { //if malloc hasn't been called yet basically so nothing is in linked_list so the head is NULL
		init_prgrm_brk = sbrk(0);

		if (init_prgrm_brk == (void *) -1) {
			perror("sbrk() error");
			return NULL;
		}

		if ((head = sbrk(MIN_SBRK + size)) == (void *) -1) { //arbitrary size I chose. the head will start as the heap itself. this is how we creat the linked list. actually shouldo do sbrk(5000 + size);
			perror("sbrk() error");
			return NULL;
		}
		break_loc = (char *) head + (MIN_SBRK + size);

		head->user_size = size;
		head->next_alloc = NULL;
		head->prev_alloc = NULL;

		return (char *) head + sizeof(struct allocation);
	}
 	else { //linked list has stuff in it
		int amt_allocated = 0;
		struct allocation *temp = head;
		amt_allocated += temp->user_size;

		while (temp->next_alloc != NULL) {
			temp = temp->next_alloc;
			amt_allocated += temp->user_size;
		}

		void *brk_check;
		if (init_prgrm_brk + amt_allocated + size > break_loc) {
			brk_check = sbrk(MIN_SBRK + size);
		}
		if (brk_check == (void *) -1) {
			perror("sbrk() error");
			return NULL;
		}

		struct allocation *chunk;
		struct allocation *curr = head;
		if (((char *) head != init_prgrm_brk) && (size + sizeof(struct allocation) <= (char *) head-init_prgrm_brk)) { //if the head was freed and now the space at the beginning of the heap is open. We will put it right at the start of the heap
			
			//how to deal with this ptr type mismatch
			chunk = (struct allocation *) init_prgrm_brk;

			head->prev_alloc = chunk;
			chunk->next_alloc = head;
			chunk->prev_alloc = NULL;
			head = chunk;
			
			return chunk;
		}
		else {
			while (curr->next_alloc != NULL) { //traverse through the linked list until you get to last non-NULL node
				if ((char *) (curr->next_alloc)-((char *) curr+sizeof(struct allocation)+curr->user_size) >= size + sizeof(struct allocation)) { //if space between current and next alloc
					chunk = (struct allocation *) ((char *) curr + sizeof(struct allocation) + curr->user_size);
					chunk->user_size = size;	
					(curr->next_alloc)->prev_alloc = chunk;
					chunk->next_alloc = curr->next_alloc;
					curr->next_alloc = chunk;
					chunk->prev_alloc = curr;

					return (char *) chunk + sizeof(struct allocation); //return it before it can change it to being added to end of linked list
				}
				curr = curr->next_alloc;
			}
		}		
		chunk = (struct allocation *) ((char *) curr+sizeof(struct allocation)+curr->user_size);
		chunk->user_size = size;
		curr->next_alloc = chunk;
		chunk->prev_alloc = curr;
		chunk->next_alloc = NULL; 
		return (char *) chunk + sizeof(struct allocation);
	}
	return 0;
}

void free(void *ptr) {
	struct allocation *chunk_to_free = head;

	if (ptr == NULL) {
		return;
	}

	while ((char *) chunk_to_free != ((char *) ptr - sizeof(struct allocation))) {
		chunk_to_free = chunk_to_free->next_alloc;
	}
	
	//freeing the head case
	if (chunk_to_free == head) {

		if (head->next_alloc != NULL) {
			head = head->next_alloc;
			head->prev_alloc = NULL;
		}
		else {
			head->prev_alloc = NULL;
			head = NULL;
			return;
		}
	}
	else {
		(chunk_to_free->prev_alloc)->next_alloc = chunk_to_free->next_alloc;
		if (chunk_to_free->next_alloc) {
			(chunk_to_free->next_alloc)->prev_alloc  = chunk_to_free->prev_alloc;
		}

		//are these next two lines even necessary?
		chunk_to_free->next_alloc = NULL; 
		chunk_to_free->prev_alloc = NULL;
	}
	return;
}

void *calloc(size_t nelem, size_t elsize) {
	int product = nelem * elsize;

	if (product / elsize != nelem) {
		errno = ENOMEM;
		return NULL;
	}

	void *ptr = malloc(product); //protect against integer overflow

	if (ptr != NULL) {
		memset(ptr, '\0', product);
	}

	return ptr;
}

void *realloc(void *ptr, size_t size) {

	if (ptr == NULL) {
		return malloc(size);
	}
	if (size == 0) {
		free(ptr);
		return NULL; //is this what should be returned?
	}
	struct allocation *chunk = head;

	while ((char *) chunk + sizeof(struct allocation) != ptr) {
		chunk = chunk->next_alloc;
	}

	if (size < chunk->user_size) {
		return ptr; //just give them back the allocated memory they already have
	}

	void *old = ptr;
	ptr = malloc(size);
	memcpy(ptr, old, chunk->user_size);
	free(old);
	return ptr;
}

size_t malloc_usable_size(void *ptr) {
	if (ptr == NULL) {
		return 0;
	}
	struct allocation *chunk = head;

	while ((char *) chunk + sizeof(struct allocation) != ptr) {
		chunk = chunk->next_alloc;
	}
	size_t size = chunk->user_size;
	return size;
}
