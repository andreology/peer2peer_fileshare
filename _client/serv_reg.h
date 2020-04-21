#define NUM_CLIENTS 2
#define MSG_LEN 100
#define BUF_SIZE 1024

struct ServantData{
    uint32_t GUID;

    char my_file[50];

    char time_string[9];

    uint32_t alive;
};

struct Registry{
    struct ServantData servants[NUM_CLIENTS];
    int size;
};

struct Registry reg;
