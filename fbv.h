struct formathandler 
{
    struct formathandler *next;
    int (*get_size)(char *,int *,int*);
    int (*get_pic)(char *,unsigned char *,int,int);
    int (*id_pic)(char *);
};

#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1
#define FH_ERROR_FORMAT 2
