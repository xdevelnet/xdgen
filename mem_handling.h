void *get_new_memory (size_t values, size_t valsize) {
    void *new_mem = malloc(values*valsize);
    if (new_mem == NULL) {
        perror("Failed to obtain new memory");
        exit(EXIT_FAILURE);
    }
    return new_mem;
}

function_seek *doc_references;
size_t doc_space = 5;
size_t doc_current = 0;

void more_doc_if_needed () {
    if (doc_current == doc_space) {
        doc_space *= 2;
        if ((doc_references = realloc(doc_references, sizeof(doc_references)*doc_space)) == NULL) {
            perror("Failed to get more memory");
            exit(EXIT_FAILURE);
        }
    }
}

void put_to_doc (function_seek *z) {
    *(doc_references + doc_current) = *z;
    doc_current++;
    more_doc_if_needed();
}


char *storage; //multipurpose octet storage
size_t storage_space = 4096;
size_t storage_current = 0;

void more_storage_if_needed() {
    if (storage_current == storage_space) {
        storage_space *= 2;
        if ((storage = realloc(storage, storage_space)) == NULL) {
            perror("Failed to get more memory");
            exit(EXIT_FAILURE);
        }
    }
}

void put_to_mem (char value) {
    *(storage+storage_current++) = value;
    more_storage_if_needed();
}
int get_readonly_descriptor(char *file_addr) {
    int d = open(file_addr, O_RDONLY);
    if (d < 0) {
        perror(file_addr);
        exit(EXIT_FAILURE);
    }
    return d;
}
