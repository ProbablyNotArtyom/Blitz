
struct fileblock_t {
    unsigned char * data;
    unsigned long pos;
    unsigned long length;
    unsigned long maxlength;
    struct fileblock_t * next;
};

extern struct fileblock_t *fileblocks;

void remove_data(int length);
