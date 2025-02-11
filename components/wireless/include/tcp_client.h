/**
 * @file tcp_client.h
 * @author ultralyj (yijie_luo@tongji.edu.cn)
 * @brief 
 * @version 0.1
 * @date 2025-02-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <stdint.h>

#ifdef __cplusplus

extern "C"
{
#endif
    int tcp_client(char *host_ip, uint16_t port, int *sock);

#ifdef __cplusplus
}
#endif

#endif
