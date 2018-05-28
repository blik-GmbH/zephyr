#include <zephyr.h>
#include <stdio.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "blik.pb.h"

/* This is the buffer where we will store our message. */
static u8_t buffer[128];

void main(void)
{
	size_t message_length;
	bool status;

	/* Encode our message */
	{
		/* Allocate space on the stack to store the message data.
		 *
		 * Nanopb generates simple struct definitions for all messages.
		 * - check out the contents of simple.pb.h!
		 * It is a good idea to always initialize your structures
		 * so that you do not have garbage data from RAM in there.
		 */
		SimpleMessage message = SimpleMessage_init_zero;

		/* Create a stream that will write to our buffer. */
		pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

		/* Fill in the lucky number */
		message.lucky_number = 13;
		message.unlucky.number = 42;

		/* Now we are ready to encode the message! */
		status = pb_encode(&stream, SimpleMessage_fields, &message);
		message_length = stream.bytes_written;

		/* Then just check for any errors.. */
		if (!status) {
			printk("Encoding failed: %s\n", PB_GET_ERROR(&stream));
			return;
		}
	}

	/* Now we could transmit the message over network, store it in a file or
	 * wrap it to a pigeon's leg.
	 */

	/* But because we are lazy, we will just decode it immediately. */

	{
		/* Allocate space for the decoded message. */
		SimpleMessage message = SimpleMessage_init_zero;

		/* Create a stream that reads from the buffer. */
		pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

		/* Now we are ready to decode the message. */
		status = pb_decode(&stream, SimpleMessage_fields, &message);

		/* Check for errors... */
		if (!status) {
			printk("Decoding failed: %s\n", PB_GET_ERROR(&stream));
			return;
		}

		/* Print the data contained in the message. */
		printk("Your lucky number was %d!\n", message.lucky_number);
		printk("Your unlucky number was %u!\n", message.unlucky.number);
	}

}

