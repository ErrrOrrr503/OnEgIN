//! @file main.cpp
//! @mainpage Evgeniy Onegin sorter
//! \n
//! Usage: ./OnEgIN [-param] [input.file] [output.file]
//! \n
//! Params: [-l]: compare lexigraphically; [-r]: compare rythmically
//! \n
//! @author Titov.EM
//! \n
//! Files:
//! \n
//! - main.cpp
//! \n

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <assert.h>

#include <sys/stat.h>
long long get_file_size_LINUX(char* filename);

long get_lines_NUM(FILE *file, long long Fsize);
int MAS_read(FILE *file, long long Fsize, char* mas, char** pointers);
int MAS_write(char** pointers, FILE *file, long long num);
int fopen_IO(char* fileName1, char* fileName2, FILE** in, FILE** out);

int tolowerstr(char* str);
int strcheck(char* str);
int compstr(const void* a, const void* b);
int chcheck (char ch);
int strcmp_lecs (char* str1, char* str2);
int strcmp_rythms_utf8 (char* str1, char* str2);

//! Defines program behavior: 0 - lecs, 1 - rythms
int BEHAVIOR = 0;

//! 1st byte of utf-8 russion symbol
const int RUS01 = -48;
//! 1st byte of utf-8 russion symbol
const int RUS02 = -47;
//! power of russian alphabet
const int ALPH_NUM = 32;

int main(int argc, char *argv[])
{
        /**
        @return Error number
        \n
        1 - Input file unavaliable
        2 - Bad using: incorrect arguments
        3 - Bad using: unrecognised parameter
        */

    setlocale(LC_ALL, "ru_RU.UTF8");
    if (argc != 4) {
            printf("Error 2: incorrect arguments\n");
            return 2;
    }

    if (*(argv[1] + 1) == 'r')
        BEHAVIOR = 1;
    if (*(argv[1] + 1) == 'l')
        BEHAVIOR = 0;
    if (*(argv[1] + 1) != 'r' && *(argv[1] + 1) != 'l') {
        printf ("Error 3: unrecognised parameter: %s\n", argv[1]);
        return 3;
    }

    FILE* i_file = NULL;
    FILE* o_file = NULL;
    if (fopen_IO(argv[2], argv[3], &i_file, &o_file)) {
        printf("Error 1: can`t open input file!\n");
        return 1;
    }

    long NUMlines = 0;
    long long Fsize = 0;
    Fsize = get_file_size_LINUX(argv[2]);
    NUMlines = get_lines_NUM(i_file, Fsize);

    char* text = (char*) malloc((Fsize+1) * sizeof(char));
    char** pointers = (char**) malloc(NUMlines * sizeof(char*));
    printf("%Ld bytes  %ld lines\n", Fsize, NUMlines);

    MAS_read(i_file, Fsize, text, pointers);

    qsort(pointers, NUMlines, sizeof(char*), compstr);

    MAS_write(pointers, o_file, NUMlines);

    free (text);
    free (pointers);
    fclose (i_file);
    fclose (o_file);
    printf ("FINISHED IT!\n");
    return 0;
}


int fopen_IO (char* fileNameIN, char* fileNameOUT, FILE** in, FILE** out)
{
        /**
        Opens input and output files
        @param[in] fileNameIN (char*) Filename of input file
        @param[in] fileNameOUT (char*) Filename of output file
        @return Error Number
        \n
        1 - Error while file opening
        0 - all good
        */

        assert (fileNameIN != NULL);
        assert (fileNameOUT != NULL);
        assert (in != NULL);
        assert (out != NULL);

    *in = NULL;
    *out = NULL;
    *in = fopen (fileNameIN,"r");
    if (*in == NULL) {
        return 1;
    }

    *out = fopen (fileNameOUT,"wt");
    return 0;
}


int compstr(const void* a, const void* b)
{
        /**
        qsort - compatitable interface for strings comparison
        @param[in] a (const void*) 1st string
        @param[in] b (const void*) 2nd string
        @return Comparison result
        \n
        1 - a > b
        0 - a == b
        -1 - a < b
        */
        assert (a != NULL);
        assert (b != NULL);

    long len_a = strlen (*(char** ) a);
    long len_b = strlen (*(char** ) b);
    char* str_a = (char*) malloc ((len_a + 1) * sizeof(char));
    char* str_b = (char*) malloc ((len_b + 1) * sizeof(char));
    strcpy (str_a, *(char** ) a);
    strcpy (str_b, *(char** ) b);
    tolowerstr (str_a);
    tolowerstr (str_b);
    int ret = 0;
    if (BEHAVIOR == 1)
        ret = strcmp_rythms_utf8 (str_a, str_b);
    if (BEHAVIOR == 0)
        ret = strcmp_lecs (str_a, str_b);
    free (str_a);
    free (str_b);
    return ret;
}


union wch {
    char sym[2];
    unsigned int dig;
};

int tolowerstr(char* str)
{
        /**
        Converts string to lower-case
        @param[in] str (char*) string
        @return Error Number
        \n
        0 - all good
        */

        assert (str != NULL);

    wch a;
    for (long i = 0; (*(str + i) != '\0') && (*(str + i) != '\n') && (*(str + i) != EOF); i++) {
        if ((int)*(str + i) == RUS01  || (int)*(str + i) == RUS02) {
            a.sym[1] = *(str + i);
            a.sym[0] = *(str + i + 1);
            if (a.dig <= (unsigned int) 'П' && a.dig >= (unsigned int) 'А')
                a.dig += ALPH_NUM;
            if (a.dig >= (unsigned int) 'Р' && a.dig <= (unsigned int) 'Я')
                {
                    a.dig = a.dig + (unsigned int) 'р' - (unsigned int) 'Р';
                }
            if (a.dig == (unsigned int) 'Ё' || a.dig == (unsigned int) 'ё') {
                a.dig = (unsigned int) 'е';
            }
            *(str + i) = a.sym[1];
            *(str + i + 1) = a.sym[0];
            i++;
        }
        else {
            *(str + i) = tolower (*(str + i));
        }
    }
    return 0;
}

long long get_file_size_LINUX (char* filename)
{
        /**
        Gets file size in linux-based OS
        @param[in] filename (char*) no comments)
        @return File size
        */

        assert (filename != NULL);

    struct stat st;
    stat (filename, &st);
    return st.st_size;
}

long get_lines_NUM(FILE *file, long long Fsize)
{
        /**
        Gets number of lines in file
        @param[in] file (FILE*) input file
        @param[in] Fsize (long long) size of the file
        @return Number of lines
        */

        assert (file != NULL);
        assert (Fsize > 0);

    char* buf = (char*) malloc (Fsize * sizeof(char));
    fread (buf, sizeof(char), Fsize, file);
    long NUM = 0;
    for (long long i = 0; i < Fsize; i++) {
        if (*(buf + i) == '\n') {
            NUM++;
        }
    }
    free (buf);
    fseek (file, 0, 0);
    return NUM;
}


int MAS_read(FILE *file, long long Fsize, char* mas, char** pointers)
{
        /**
        Reads file to buffer
        @param[in] file (FILE*) input file
        @param[in] Fsize (long long) size of the file
        @param[in] mas (char*) buffer
        @param[in] pointers (char**) pointers to 1st elements of lines
        @return Error number
        \n
        0 - all good
        */

        assert (file != NULL);
        assert (mas != NULL);
        assert (pointers != NULL);
        assert (Fsize > 0);

    long long i = 0;
    long j = 1;
    fread(mas, sizeof(char), Fsize, file);
    *pointers = mas;
    while (i < Fsize) {
        if (*(mas + i) == '\n' || *(mas + i) == EOF) {
            *(mas + i) = '\0';
            *(pointers + j) = mas + i + 1;
            j++;
        }
        i++;
    }
    return 0;
}


int MAS_write(char** pointers, FILE *file, long long num)
{
        /**
        Reads file to buffer
        @param[in] pointers (char**) pointers to 1st elements of lines
        @param[in] file (FILE*) input file
        @param[in] num (long long) number of lines
        @return Error number
        \n
        0 - all good
        */

        assert (file != NULL);;
        assert (pointers != NULL);
        assert (num > 0);

    for(int i = 0; i < num; i++) {
        if (strcheck(*(pointers+i))) {
            fprintf(file, "%s", *(pointers + i));
            fprintf(file, "\n");
        }
    }
    return 0;
}


int strcheck(char* str)
{
        /**
        Checks if string has symbols, visible to humans
        @param[in] str (char*) string
        @return
        \n
        0 - all invisible
        1 - at least 1 visible
        */

        assert(str != NULL);

    int len = strlen(str);
    for (int i = 0; i < len - 1; i++) {
        if (*(str + i) != ' ' && *(str + i) != '\n' && *(str + i) != '\t' && *(str + i) != '.') return 1;
    }
    return 0;
}

int strcmp_lecs (char* str1, char* str2)
{
        /**
        Compares 2 strings lexigraphically. WARNING! function CHANGES strings!
        @param[in] str1 (char*) string 1
        @param[in] str2 (char*) string 2
        @return
        \n
        1 - str1 > str2
        0 - str1 == str2
        -1 - str1 < str2
        */

    #define CHECK_STR_END(x, y)\
            if ((unsigned char)*(x) == '\0') {\
                if ((unsigned char)*(y) != '\0')\
                    return -1;\
                else\
                    return 0;\
            }\
            if ((unsigned char)*(y) == '\0') {\
                if ((unsigned char)*(x) != '\0')\
                    return 1;\
                else\
                    return 0;\
            }

        assert(str1 != NULL);
        assert(str2 != NULL);

    long j = 0, i = 0;
    while (1) {
        int check1 = chcheck (*(str1 + i));
        int check2 = chcheck (*(str2 + j));
        if (!check1 || !check2) {
            if (!check1)
                i++;
            if (!check2)
                j++;
            CHECK_STR_END(str1 + i, str2 + j);
        }
        else {
            if ((unsigned char)*(str1 + i) > (unsigned char)*(str2 + j))
                return 1;
            if ((unsigned char)*(str1 + i) < (unsigned char)*(str2 + j))
                return -1;
            i++;
            j++;
            CHECK_STR_END(str1 + i, str2 + j);
        }
    }
}

int strcmp_rythms_utf8 (char* str1, char* str2)
{
        /**
        Compares 2 strings from end. WARNING! function CHANGES strings!
        @param[in] str1 (char*) string 1
        @param[in] str2 (char*) string 2
        @return
        \n
        1 - str1 > str2
        0 - str1 == str2
        -1 - str1 < str2
        */

    #define REVERSE(x, y)\
        for (long i = 0; i < y / 2; i++) {\
            char tmp = *(x + i);\
            *(x + i) = *(x + y - 1 - i);\
            *(x + y - 1 - i) = tmp;\
        }\
        *(x + y) = '\0';\
        for (long i = 1; i < y; i++) {\
            if (chcheck(*(x + i)) == 2) {\
                char tmp = *(x + i - 1);\
                *(x + i - 1) = *(x + i);\
                *(x + i) = tmp;\
            }\
        }\

        assert(str1 != NULL);
        assert(str2 != NULL);

    long len1 = strlen (str1);
    long len2 = strlen (str2);
    REVERSE(str1, len1);
    REVERSE(str2, len2);
    return strcmp_lecs(str1, str2);
}

int chcheck (char ch)
{
        /**
        tells if symbol is worth comparison or is special
        @param[in] ch (char) symbol
        @return
        \n
        1 - worth
        0 - not worth
        2 - special rus-letter symbol
        */

    if (ch == RUS01 || ch == RUS02)
        return 2;
    if ((ch >= ' ' && ch < '0') || (ch > '9' && ch < 'A') || (ch > 'Z' && ch < 'a') || (ch > 'z' && ch < 127))
        return 0;
    return 1;
}
