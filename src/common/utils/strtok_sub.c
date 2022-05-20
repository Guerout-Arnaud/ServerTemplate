#include <stddef.h>
#include <string.h>

char *strtok_sub(char *str_in, const char *sub)
{
    static char *save_str = NULL;
    char *str = str_in ? str_in : save_str;
    char *end = NULL;

    if (str == NULL || str[0] == '\0')
        return (NULL);

    end = strstr(str, sub);

    if (end == NULL) {
        save_str = NULL;
    } else {
        save_str = end + strlen(sub);
        *end = '\0';
    }
    return (str);
}