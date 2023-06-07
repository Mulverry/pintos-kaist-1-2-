#include "devices/input.h"
#include <debug.h>
#include "devices/intq.h"
#include "devices/serial.h"

/* Stores keys from the keyboard and serial port. */
static struct intq buffer;

/* Initializes the input buffer. */
void
input_init (void) {
	intq_init (&buffer);
}

/* Adds a key to the input buffer.
   Interrupts must be off and the buffer must not be full. */
void 
input_putc (uint8_t key) {
	ASSERT (intr_get_level () == INTR_OFF);
	ASSERT (!intq_full (&buffer));

	intq_putc (&buffer, key);
	serial_notify ();
}

/* Retrieves a key from the input buffer.
   If the buffer is empty, waits for a key to be pressed.
   입력 버퍼에서 하나의 8비트 문자(uint8_t)를 읽어옴.
   버퍼가 비어있다면 기다림. */
uint8_t input_getc (void) {
	enum intr_level old_level;
	uint8_t key;

	old_level = intr_disable ();
	key = intq_getc (&buffer); //intq_getc 함수를 호출하여 입력 버퍼에서 문자 하나를 읽어옴.
	serial_notify (); // 시리얼 인터럽트를 알림. 이는 입력 버퍼에 문자가 추가되었음을 시스템에 알리는 역할
	intr_set_level (old_level);

	return key;
}

/* Returns true if the input buffer is full,
   false otherwise.
   Interrupts must be off. */
bool
input_full (void) {
	ASSERT (intr_get_level () == INTR_OFF);
	return intq_full (&buffer);
}
