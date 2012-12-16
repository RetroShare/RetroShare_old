/*
 * libretroshare/src/serialiser: tlvfileitem_test.cc
 *
 * RetroShare Serialiser.
 *
 * Copyright 2007-2008 by Robert Fernie, Chris Evi-Parker
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


#include <iostream>
#include <sstream>
#include "serialiser/rstlvtypes.h"
#include "serialiser/rstlvutil.h"
#include "serialiser/rstlvbase.h"
#include "util/utest.h"

INITTEST();

#define RAND_SEED 1352534

static int test_RsTlvBinData();
static int test_RsTlvFileItem();
static int test_RsTlvFileSet();
static int test_RsTlvFileData();
static int test_RsTlvStringSet();
static int test_RsTlvPeerIdSet();
static int test_RsTlvServiceIdSet();
static int test_RsTlvKeyValue();
static int test_RsTlvKeyValueSet();
static int test_RsTlvHashSet();
static int test_RsTlvImage();



int main()
{
	std::cerr << "RsTlvTypes[Item/Data/...] Tests" << std::endl;

	srand(RAND_SEED);

	test_RsTlvFileItem();
	test_RsTlvFileData();
	test_RsTlvFileSet();
	test_RsTlvPeerIdSet();
	test_RsTlvServiceIdSet();
	test_RsTlvKeyValue();
	test_RsTlvKeyValueSet();
	test_RsTlvBinData();
	test_RsTlvImage();
	test_RsTlvHashSet();

	FINALREPORT("RsTlvTypes[Item/Data/...] Tests");

	return TESTRESULT();
}

int test_RsTlvFileItem()
{
	RsTlvFileItem i1;
	RsTlvFileItem i2;

	/* initialise */
	i1.filesize = 101010;
	i1.hash = "ABCDEFEGHE";
	i1.name = "TestFile.txt";
	i1.pop  = 12;
	i1.age  = 456;

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/* check the data is the same */
	CHECK(i1.filesize == i2.filesize);
	CHECK(i1.hash == i2.hash);
	CHECK(i1.name == i2.name);
	CHECK(i1.path == i2.path);
	CHECK(i1.pop  == i2.pop);
	CHECK(i1.age  == i2.age);

	/* do it again without optional data */
	i1.filesize = 123;
	i1.name = "";
	i1.pop  = 0;
	i1.age  = 0;

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/* check the data is the same */
	CHECK(i1.filesize == i2.filesize);
	CHECK(i1.hash == i2.hash);
	CHECK(i1.name == i2.name);
	CHECK(i1.path == i2.path);
	CHECK(i1.pop  == i2.pop);
	CHECK(i1.age  == i2.age);

	/* one more time - long file name, some optional data */
	i1.filesize = 123;
	i1.name = "A Very Long File name that should fit in easily ??? with som $&%&^%* strange char (**$^%#&^$#*^%(&^ in there too!!!! ~~~!!$#(^$)$)(&%^)&\"  oiyu thend";
	i1.pop  = 666;
	i1.age  = 0;

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/* check the data is the same */
	CHECK(i1.filesize == i2.filesize);
	CHECK(i1.hash == i2.hash);
	CHECK(i1.name == i2.name);
	CHECK(i1.path == i2.path);
	CHECK(i1.pop  == i2.pop);
	CHECK(i1.age  == i2.age);

	REPORT("Serialise/Deserialise RsTlvFileItem");

	return 1;
}

int test_RsTlvFileSet()
{
	RsTlvFileSet  s1;
	RsTlvFileSet  s2;

	int i = 0;
	for(i = 0; i < 15; i++)
	{
		RsTlvFileItem fi;
		fi.filesize = 16 + i * i;
		fi.hash = "ABCDEF";
		std::ostringstream out;
		out << "File" << i << "_inSet.txt";
		fi.name = out.str();
		if (i % 2 == 0)
		{
			fi.age = 10 * i;
		}
		else
		{
			fi.age = 0;
		}
		fi.pop = 0;

		s1.items.push_back(fi);
	}

	CHECK(test_SerialiseTlvItem(std::cerr, &s1, &s2));

	/* check the data is the same - TODO */

	REPORT("Serialise/Deserialise RsTlvFileSet");

	return 1;
}


int test_RsTlvFileData()
{
	RsTlvFileData d1;
	RsTlvFileData d2;

	/* initialise */
	d1.file.filesize = 101010;
	d1.file.hash = "ABCDEFEGHE";
	d1.file.name = "";
	d1.file.age = 0;
	d1.file.pop = 0;

	char data[15];
	d1.binData.setBinData(data, 15);

	d1.file_offset = 222;

	CHECK(test_SerialiseTlvItem(std::cerr, &d1, &d2));

	/* check the data is the same */
	CHECK(d1.file.filesize == d2.file.filesize);
	CHECK(d1.file.hash == d2.file.hash);
	CHECK(d1.file.name == d2.file.name);
	CHECK(d1.file.path == d2.file.path);
	CHECK(d1.file.pop  == d2.file.pop);
	CHECK(d1.file.age  == d2.file.age);

	CHECK(d1.file_offset  == d2.file_offset);
	CHECK(d1.binData.bin_len == d2.binData.bin_len);

	REPORT("Serialise/Deserialise RsTlvFileData");

	return 1;
}


int test_RsTlvPeerIdSet()
{

	RsTlvPeerIdSet i1, i2; // one to set and other to get

	std::string testString;

	std::string randString[5];
	randString[0] = "e$424!�!�";
	randString[1] = "e~:@L{L{KHKG";
	randString[2] = "e{@O**/*/*";
	randString[3] = "e?<<BNMB>HG�!�%$";
	randString[4] = "e><?<NVBCEE�$$%*^";

	/* store a number of random ids */

	for(int i = 0; i < 15 ; i++)
	{
		testString = randString[(rand() % 4)] + randString[(rand() % 4)];

		i1.ids.push_back(testString);
	}

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/*check that the data is the same*/

	REPORT("Serialize/Deserialize RsTlvPeerIdSet");

	return 1;
}

int test_RsTlvServiceIdSet()
{
	RsTlvServiceIdSet i1, i2; // one to set and other to get


	/* store random numbers */
	for(int i = 0; i < 15 ; i++)
	{
		i1.ids.push_back(1 + rand() % 12564);
	}
	std::cout << "error here!!!?";
	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/*check that the data is the same*/
	REPORT("Serialize/Deserialize RsTlvServiceIdSet");
	return 1;
}


int test_RsTlvKeyValue()
{
	RsTlvKeyValue i1, i2; // one to set and other to get

	i1.key = "whatever";
	i1.value = "better work";

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/*check that the data is the same*/
	REPORT("Serialize/Deserialize RsTlvKeyValue");

	return 1;
}
int test_RsTlvKeyValueSet()
{
	RsTlvKeyValueSet i1, i2; // one to set and other to get

	/* instantiate the objects values */

	std::string randString[5];
	randString[0] = "e$424!�!�";
	randString[1] = "e~:@L{L{KHKG";
	randString[2] = "e{@O**/*/*";
	randString[3] = "e?<<BNMB>HG�!�%$";
	randString[4] = "e><?<NVBCEE�$$%*^";

	for(int i = 0; i < 15; i++)
	{
		RsTlvKeyValue kv;

		kv.key = randString[(rand() % 4)] + randString[(rand() % 4)];
		kv.value = randString[(rand() % 4)] + randString[(rand() % 4)];

		i1.pairs.push_back(kv);

	}

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/*check that the data is the same*/
	REPORT("Serialize/Deserialize RsTlvKeyValueSet");

	return 1;
}

int test_RsTlvBinData()
{


	RsTlvBinaryData b1(TLV_TYPE_BIN_IMAGE), b2(TLV_TYPE_BIN_IMAGE);
	unsigned char* data = NULL;
	const uint32_t bin_size = 16000;
	char alpha  = 'a';

	data = new unsigned char[bin_size];


	// initialise binary data with random values
	for(int i=0; i != bin_size; i++)
		data[i] = alpha + (rand() % 26);


	b1.setBinData(data, bin_size);
	delete data;

	//do check
	CHECK(test_SerialiseTlvItem(std::cerr, &b1, &b2));

	REPORT("Serialize/Deserialize RsTlvBinData ");

	return 1;
}

int test_RsTlvImage()
{

	RsTlvImage image1, image2;
	unsigned char* image_data = NULL;
	const uint32_t bin_size = 16000;
	char alpha  = 'a';

	image_data = new unsigned char[bin_size];


	// initialise binary data with random values
	for(int i=0; i != bin_size; i++)
		image_data[i] = alpha + (rand() % 26);

	image1.image_type = RSTLV_IMAGE_TYPE_PNG;
	image1.binData.setBinData(image_data, bin_size);

	delete image_data;

	CHECK(test_SerialiseTlvItem(std::cerr, &image1, &image2));

	REPORT("Serialize/Deserialize RsTlvBinImage ");

	return 1;
}


int test_RsTlvHashSet()
{
	RsTlvPeerIdSet i1, i2; // one to set and other to get

	int numRandStrings = rand()%30;

	std::string* randString = NULL;
	std::list<std::string> randStrings;

	char alpha = 'a';
	char* stringData = NULL;

	for(int i=0; i < numRandStrings; i++){

		int stringLength = rand()%200;
		stringData = new char[stringLength];

			for(int i=0; i != stringLength; i++)
				stringData[i] = alpha + (rand() % 26);


		randString = new std::string(stringData, stringLength);
		randStrings.push_back(*randString);

		// release memory resources
		delete randString;
		delete stringData;
		stringData = NULL;
		randString = NULL;
	}


	/* store a number of random ids */

	for(int i = 0; i < numRandStrings ; i++)
	{
		i1.ids = randStrings;
	}

	CHECK(test_SerialiseTlvItem(std::cerr, &i1, &i2));

	/*check that the data is the same*/

	REPORT("Serialize/Deserialize RsTlvHashSet");

	return 1;

}
