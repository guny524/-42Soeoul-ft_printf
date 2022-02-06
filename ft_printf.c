/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: min-jo <min-jo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/11 16:18:07 by min-jo            #+#    #+#             */
/*   Updated: 2022/02/06 17:46:07 by min-jo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

/*
* int	general_printf(size_t size, char *str, char **ret, int fd,
* 			const char *format, ...)
* fd = 1
* size = -1 -> (size_t)INT_MAX + 1로 세팅 됨
* str = NULL
* ret = NULL
* 가변길이 매개변수 리스트를 사용 것이 아니라 현재 함수에서 va_start를 사용하므로 va_list ap를
* 넘겨준다. format = NULL
* 가변길이 매개변수 리스트 대신 format과 va_list를 넘겨 줌
*/
int	ft_printf(const char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	return (general_printf(-1, NULL, NULL, 1, NULL, format, ap));
}

/*
* 다음 함수들의 모든 기능을 담기 위해 general_printf를 아래 처럼 정의함
* norm 때문에 매개변수 갯수가 넘어가는 건 가변길이 매개변수로 대체함
* int	dprintf(int fd, const char *format);
* int	snprintf(char *str, size_t size, const char *format);
* int	asprintf(char **ret, const char *format);
* int	general_printf(size_t size, char *str, char **ret, int fd,
* 			const char *format, ...)
*
* size == -1; // (size_t)INT_MAX + 1로 세팅, INT_MAX 넘어서 출력 못 함
* str == NULL && ret == NULL // fd로 바로 출력
* str == NULL && ret != NULL // 동적할당
* str != NULL // str에 출력
*/
int	general_printf(size_t size, char *str, char **ret, ...)
{
	va_list	ap;
	t_print	print;

	va_start(ap, ret);
	print = init_print(size, str, ret, ap);
	print.cnt = 0;
	print.check = 1;
	print.ret = 0;
	if (-1 == print.fd && NULL == print.buf)
		return (STATE_ERROR);
	return (run_print(print));
}

/*
* print 구조체의 대한 정보 세팅
*/
t_print	init_print(size_t size, char *str, char **ret, va_list ap)
{
	t_print	print;

	print.size = size;
	if (size > (size_t)INT_MAX)
		print.size = (size_t)INT_MAX + 1;
	print.fd = -1;
	if (str == NULL && ret == NULL)
		print.fd = va_arg(ap, int);
	print.buf = str;
	if (str == NULL && ret != NULL)
	{
		print.buf = malloc(print.size);
		*ret = print.buf;
	}
	if (print.buf)
		print.buf[print.size] = '\0';
	print.format = va_arg(ap, char *);
	print.ap = ap;
	if (print.format == NULL)
	{
		print.format = va_arg(ap, char *);
		print.ap = va_arg(ap, va_list);
	}
	return (print);
}

/*
* state machine 기반
* normal 일반 출력 상태에서 %를 만나면 conversion으로 넘어감
*/
int	run_print(t_print print)
{
	t_state	state;

	state = STATE_NORMAL;
	while (*(print.format))
	{
		if (state == STATE_ERROR)
			break ;
		else if (state == STATE_NORMAL)
			state = state_normal(&print);
		else if (state == STATE_CONVERSION)
			state = state_conversion(&print);
		print.format++;
	}
	va_end(print.ap);
	return (print.cnt);
}

/*
* write를 대체하는 함수, print->check로 실제 출력하기 전에 print->cnt만 먼저 셈
* (모든 출력 시뮬레이션 가능)
* check flag는 state에서 제어 됨
* 오류가 생기면 print->ret을 -1로 세팅, print->cnt는 size_t이기 때문에 업데이트할 필요 없음
* 이 때문에 general_write()가 끝난 모든 부분에서 print->ret 체크 필요 (근데 STATE_ERROR를
* 리턴가능한 부분에서 검사해야 함)
* 실제 출력이나 check된 만큼 nbyte로 print->ret 세팅
*/
void	general_write(t_print *print, const char *buf, size_t nbyte)
{
	size_t	i;

	i = 0;
	if (print->check)
	{
		if (print->cnt + nbyte >= print->size)
		{
			print->ret = -1;
			return ;
		}
	}
	else if (print->buf)
		while (i <= nbyte)
			print->buf[print->cnt++] = buf[i++];
	else
	{
		print->ret = write(print->fd, buf, nbyte);
		if (-1 == print->ret)
			return ;
	}
	if (!print->buf)
		print->cnt += nbyte;
	print->ret = nbyte;
}
