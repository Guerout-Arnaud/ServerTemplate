/*!
** ServerTemplate PROJECT, 2021
** @file common/function.h
** File description:
** @brief Common functions prototypes
** @author
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#ifndef COMMON_FUNCTION_H_
    #define COMMON_FUNCTION_H_

    /*! @fn setup_signals
    * @brief Signal catcher initializer.
    *
    * @param void
    * @return int fd on which signals are catch
    */
    int setup_signals(void);

    /*! @fn strtok_sub
     * @brief Repplicate strtok but with substring instead of delim
     *
     * @param void
     * @return char* : first string ending with sub (sub is removed)
     */
    char *strtok_sub(char *str_in, const char *sub);


    /*! @fn msleep
     * @brief Sleep for a given number of milliseconds
     *
     * @param int : number of milliseconds to sleep
     * @return int : 0 on success, -1 on error
     */
    int msleep(long msec);



#endif