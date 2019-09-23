#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include <sys/stat.h>
long long get_file_size_LINUX(char* filename);

long getlinesNUM(FILE *file);
int tolowerstr(char* str);
char tolow(char in);
int compstr(const void* a, const void* b);
int MASread(FILE *file, char* mas, char** pointers);
int MASwrite(char** pointers, FILE *file, long long num);
long long get_file_size(FILE *file);
int fopenIO(char* fileName1, char* fileName2, FILE** in, FILE** out);
int strcheck(char* str);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "ru_RU.UTF8");
    if (argc != 3) {
            printf("Errorr 2: incorrect arguments\n");
            return 2;
    }

    FILE* i_file = NULL;
    FILE* o_file = NULL;
    if (fopenIO(argv[1], argv[2], &i_file, &o_file)) {
        printf("Error 1: can`t open input file!\n");
        return 1;
    }

    long NUMlines = 0;
    long long Fsize = 0;
     // Fsize = get_file_size(i_file);
    Fsize = get_file_size_LINUX(argv[1]);
    NUMlines = getlinesNUM(i_file);

    char* text = (char*) malloc((Fsize+1) * sizeof(char));
    char** pointers = (char**) malloc(NUMlines * sizeof(char*));
    printf("%Ld bytes  %ld lines\n", Fsize, NUMlines);

    MASread(i_file, text, pointers);

    qsort(pointers, NUMlines, sizeof(char*), compstr);

    MASwrite(pointers, o_file, NUMlines);

    free (text);
    free (pointers);
    fclose (i_file);
    fclose (o_file);
    printf ("FINISHED IT!\n");
    return 0;
}


int fopenIO(char* fileNameIN, char* fileNameOUT, FILE** in, FILE** out)
{
    *in = NULL;
    *out = NULL;
    *in = fopen(fileNameIN,"r");
    if (*in == NULL) {
        return 1;
    }

    *out = fopen(fileNameOUT,"wt");
    return 0;
}


int compstr(const void* a, const void* b)
{
    long len_a = strlen(*(char** ) a);
    long len_b = strlen(*(char** ) b);
    char* str_a = (char*) malloc((len_a + 1) * sizeof(char));
    char* str_b = (char*) malloc((len_b + 1) * sizeof(char));
    strcpy(str_a, *(char** ) a);
    strcpy(str_b, *(char** ) b);
    tolowerstr(str_a);
    tolowerstr(str_b);
    return strcmp(str_a, str_b);
}


union wch {
    char sym[2];
    unsigned int dig;
};

int tolowerstr(char* str)
{
    wch a;
    for (long i = 0; (*(str + i) != '\0') && (*(str + i) != '\n') && (*(str + i) != EOF); i++) {
        *(str + i) = tolower(*(str + i));
       if ((int)*(str + i) == -48  || (int)*(str + i) == -47) {
            a.sym[1] = *(str + i);
            a.sym[0] = *(str + i + 1);
            if (a.dig <= (unsigned int) 'П')
                a.dig += 32;
            if (a.dig >= (unsigned int) 'Р' && a.dig <= (unsigned int) 'Я')
                {
                    a.dig = a.dig + (unsigned int) 'р' - (unsigned int) 'Р';
                }
            if (a.dig == (unsigned int) 'Ё')
                a.dig = (unsigned int) 'ё';
            *(str + i) = a.sym[1];
            *(str + i + 1) = a.sym[0];
            i++;
        }
        else {
            *(str + i) = tolower(*(str + i));
        }
    }
    return 0;
}


long long get_file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);
    long long FileSize = 0;
    FileSize = ftell(file);
    fseek(file, 0, 0);
    return FileSize;
}

long long get_file_size_LINUX(char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

long getlinesNUM(FILE *file)
{
    fseek(file, 0, 0);
    long NUM = 0;
    bool isEmpty = 1;
    char tmp = getc(file);
    while (!feof(file)) {
        isEmpty = 0;
        if (tmp == '\n') {
            NUM++;
            isEmpty = 1;
        }
        tmp = getc(file);
    }
    if(!isEmpty) NUM++;
    fseek(file,0,0);
    return NUM;
}


int MASread(FILE *file, char* mas, char** pointers)
{
    long long i = 0;
    long j = 1;
    char tmp = getc(file);
    *pointers = mas;
    while (tmp != EOF) {
        if (tmp == '\n' || tmp == EOF) {
            *(mas + i) = '\0';
            *(pointers + j) = mas + i + 1;
            j++;
        }
        else {
            *(mas + i) = tmp;
        }
        i++;
        tmp = getc(file);
    }
    return 0;
}


int MASwrite(char** pointers, FILE *file, long long num)
{
    for(int i = 0; i < num; i++) {
        if (strcheck(*(pointers+i))) {
            fprintf(file, *(pointers + i));
            fprintf(file, "\n");
        }
    }
    return 0;
}


int strcheck(char* str)
{
    int len = strlen(str);
    for (int i = 0; i < len - 1; i++) {
        if (*(str + i) != ' ' && *(str + i) != '\n' && *(str + i) != '\t') return 1;
    }
    return 0;
}
