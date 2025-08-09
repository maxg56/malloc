/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgendrot <mgendrot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/14 11:33:25 by mgendrot          #+#    #+#             */
/*   Updated: 2025/01/30 18:53:24 by mgendrot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H

# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <unistd.h>
# include <stdarg.h>
# include <stdint.h>
# include <stddef.h>

//# define ft_arnalloc malloc

# define RED "\033[0;91m"
# define GREEN "\033[0;92m"
# define DEF_COLOR "\033[0;39m"

# define TRUE 1
# define FALSE 0

# define ARNA_TAB_MAX 10
# define ARNA_SIZE 64000
# define MEMORY_POOL_MAX 10
# define DEFAULT_BLOCK_SIZE 64000

typedef enum e_bool
{
	E_FALSE,
	E_TRUE
}	t_bool;

# define BUFFER_SIZE 10
# define FD 1
# define NULL_STR "(null)"

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}		t_list;

// get_next_line 
char	*get_next_line(int fd);
char	*arn_get_next_line(int fd);

// ft_is
int		ft_isalpha(int car);
int		ft_isdigit(int car );
int		ft_isprint(int car );
int		ft_isascii(int car );
int		ft_isalnum(int car);
// ft_lst
t_list	*ft_arn_lstnew(void *content);
t_list	*ft_lstnew(void *content);
int		ft_lstsize(t_list *lst);
void	ft_lstadd_front(t_list **lst, t_list *new);
void	ft_lstdelone(t_list *lst, void (*del)(void*));
void	ft_lstclear(t_list **lst, void (*del)(void*));
void	ft_lstiter(t_list *lst, void (*f)(void *));
void	ft_lstadd_back(t_list **lst, t_list *new);
t_list	*ft_lstlast(t_list *lst);
t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));
// ft_mem
int		ft_memcmp(const void *s1, const void *s2, size_t n);
void	ft_bzero(void *b, size_t n);
void	*ft_calloc(size_t count, size_t size );
void	*ft_arn_calloc(size_t count, size_t size);
void	*ft_arn_tab_calloc(size_t count, size_t size );
void	*ft_memchr(const void *s, int c, size_t n);
void	*ft_memmove(void *dst, const void *src, size_t len);
void	*ft_memset(void *pointer, int value, size_t count );
void	*ft_memcpy(void *dst, const void *src, size_t n);
// ft_arna
void	*ft_arnalloc(size_t size);
void	ft_arna_free(void);

// ft_free

void	ft_free_tab(char **tab);

// ft_put
void	ft_putchar_fd(char c, int fd);
void	ft_putstr_fd(char *s, int fd);
void	ft_putendl_fd(char *s, int fd);
void	ft_putnbr_fd(int n, int fd);

// ft_str

char	*ft_strncpy(char *dest, const char *src, size_t n);
int		ft_strcmp(const char *s1, const char *s2);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strchr(const char *s, int c);
char	*ft_strnstr(const char *haystack, const char *needle, size_t len);
char	*ft_strdup(char *src);
char	*ft_strrchr(const char *s, int c);
size_t	ft_strlen(const char *str);
int		ft_strlen_c(char *str, char c);
size_t	ft_strlcat(char *dst, const char *src, size_t dstsize);
size_t	ft_strlcpy(char *dst, const char *src, size_t size);
char	*ft_substr(char const *s, unsigned int start, size_t len);
char	*ft_strjoin(char const *s1, char const *s2);
char	*ft_strtrim(char const *s1, char const *set);
char	**ft_split(char const *s, char c);
char	*ft_strmapi(char const *s, char (*f)(unsigned int, char));
void	ft_striteri(char *s, void (*f)(unsigned int, char*));
char	*ft_strjoin3(char *s1, char *s2, char *s3);
char	*ft_arn_strmapi(char const *s, char (*f)(unsigned int, char));
char	*ft_arn_strjoin(char const *s1, char const *s2);
char	*ft_arn_strjoin3(char *s1, char *s2, char *s3);
char	*ft_arn_substr(char const *s, unsigned int start, size_t len);
char	**ft_arn_split(char const *s, char c);
char	*ft_arn_strtrim(char const *s1, char const *set);
char	*ft_arn_strdup(char *src);

//ft_to
int		ft_atoi(const char *str);
long	ft_atol(const char *str);
char	*ft_itoa(int n);
char	*ft_arn_itoa(int n);
int		ft_toupper(int car);
int		ft_tolower(int car);

// ft_printf 
int		ft_print_c_fd(char c, int fd);
int		ft_print_d_fd(int n, int fd);
int		ft_print_p_fd(uintptr_t ptr, int fd);
int		ft_print_pct_fd(int fd);
int		ft_print_s_fd(char *s, int fd);
int		ft_print_u_fd(unsigned int n, int fd);
int		ft_print_x_fd(unsigned int n, int format, int fd);

int		ft_printf(const char *str, ...);
int		ft_dprintf(int fd, const char *str, ...);

#endif 