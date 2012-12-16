
/*
 * libretroshare/src/serialiser: rstlvutil.cc
 *
 * RetroShare Serialiser.
 *
 * Copyright 2007-2008 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */


/* some utility functions mainly for debugging 
 *
 *
 *
 */

/* print out a packet */
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>

#include "serialiser/rstlvbase.h"
#include "serialiser/rstlvtypes.h"
#include "util/utest.h"
#include "util/rsstring.h"

void displayRawPacket(std::ostream &out, void *data, uint32_t size)
{
	uint32_t i;
	std::string sout;
	rs_sprintf(sout, "DisplayRawPacket: Size: %ld", size);

	for(i = 0; i < size; i++)
	{
		if (i % 16 == 0)
		{
			sout += "\n";
		}
		rs_sprintf_append(sout, "%02x:", (int) (((unsigned char *) data)[i]));
	}

	out << sout << std::endl;
}


#define WHOLE_64K_SIZE 65536

int test_SerialiseTlvItem(std::ostream &str, RsTlvItem *in, RsTlvItem *out)
{
	uint16_t initsize = in->TlvSize();
	uint32_t serialOffset = 0;
	uint32_t deserialOffset  = 0;

	str << "test_SerialiseTlvItem() Testing ... Print/Serialise/Deserialise";
	str << std::endl;


	/* some space to serialise into */
	unsigned char serbuffer[WHOLE_64K_SIZE];

	CHECK(in->SetTlv(serbuffer, WHOLE_64K_SIZE, &serialOffset));

	CHECK(serialOffset == initsize); 		/* check that the offset matches the size */
	CHECK(in->TlvSize() == initsize);	/* check size hasn't changed */

	REPORT("Serialise RsTlvItem");

	/* now we try to read it back in! */
	CHECK(out->GetTlv(serbuffer, serialOffset, &deserialOffset));

	/* again check sizes */
	CHECK(serialOffset == deserialOffset); 
	CHECK(deserialOffset == initsize); 
	CHECK(out->TlvSize() == initsize);

	str << "Class In/Serialised/Out!" << std::endl;
	in->print(str, 0);
	displayRawPacket(str, serbuffer, serialOffset);
	out->print(str, 0);

	/* Can't check the actual data -> should add function */
	REPORT("DeSerialise RsTlvFileItem");

	/* print it out */


	return 1;
}

/* This function checks the TLV header, and steps on to the next one
 */

bool test_StepThroughTlvStack(std::ostream &str, void *data, int size)
{
	uint32_t offset = 0;
	uint32_t index = 0;
	while (offset + 4 <= size)
	{
        	uint16_t tlvtype = GetTlvType( &(((uint8_t *) data)[offset])  );
        	uint16_t tlvsize = GetTlvSize( &(((uint8_t *) data)[offset])  );
		str << "Tlv Entry[" << index << "] => Offset: " << offset;
		str << " Type: " << tlvtype;
		str << " Size: " << tlvsize;
		str << std::endl;

        	offset += tlvsize;
	}
	CHECK(offset == size); /* we match up exactly */

	REPORT("Step Through RsTlvStack");
	return 1;
}


int test_CreateTlvStack(std::ostream &str, 
		std::vector<RsTlvItem *> items, void *data, uint32_t *totalsize)
{
	/* (1) select a random item 
	 * (2) check size -> if okay serialise onto the end
	 * (3) loop!.
	 */
	uint32_t offset = 0;
	uint32_t count = 0;

	while(1)
	{
		int idx = (int) (items.size() * (rand() / (RAND_MAX + 1.0)));
		uint32_t tlvsize = items[idx] -> TlvSize();

		if (offset + tlvsize > *totalsize)
		{
			*totalsize = offset;
			return count;
		}

		str << "Stack[" << count << "]";
		str << " Offset: " << offset;
		str << " TlvSize: " << tlvsize;
		str << std::endl;
			
		/* serialise it */
		items[idx] -> SetTlv(data, *totalsize, &offset);
		items[idx] -> print(str, 10);
		count++;
	}
	*totalsize = offset;
	return 0;
}

int test_TlvSet(std::vector<RsTlvItem *> items, int maxsize)
{
	int totalsize = maxsize;
	void *data = malloc(totalsize);
	uint32_t size = totalsize;

	test_CreateTlvStack(std::cerr, items, data, &size);
	test_StepThroughTlvStack(std::cerr, data, size);

	return 1;
}


