#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <bcl.h>

void bc_change_qr_value(uint64_t *id, const char *topic, void *value, void *param);
void print_qr(const uint8_t qrcode[]);
void qrcode_project(char *text);


#endif // _APPLICATION_H
