// GTA V Script Information v1.0 by Mockba the Borg
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Script file header definition
typedef struct {
	uint32_t magic;				// 0x00
	uint32_t someNumber1;		// 0x04
	uint64_t pagesOffset;		// 0x08
	uint64_t codePagesOffset;	// 0x10
	uint32_t globalsVersion;	// 0x18
	uint32_t codeSize;			// 0x1C
	uint32_t paramCount;		// 0x20
	uint32_t staticsCount;		// 0x24
	uint32_t globalsCount;		// 0x28
	uint32_t nativesCount;		// 0x2C
	uint64_t staticsOffset;		// 0x30
	uint64_t globalsOffset;		// 0x38
	uint64_t nativesOffset;		// 0x40
	uint64_t zeroes1;			// 0x48
	uint64_t zeroes2;			// 0x50
	uint32_t nameHash;			// 0x58
	uint32_t someNumber2;		// 0x5C
	uint64_t scriptNameOffset;	// 0x60
	uint64_t stringPagesOffset;	// 0x68
	uint32_t stringsSize;		// 0x70
	uint32_t zeroes3;			// 0x74
	uint64_t zeroes4;			// 0x78
} header_t;

// Pointers
header_t *header;
uint8_t  *buffer;
uint64_t *offsetPtr;
uint64_t *nativePtr;

// Offsets
uint64_t pagesOffset;
uint64_t codePagesOffset;
uint64_t staticsOffset;
uint64_t globalsOffset;
uint64_t nativesOffset;
uint64_t scriptNameOffset;
uint64_t stringPagesOffset;

// Global variables
uint32_t stringsSize;
uint32_t codeSize;

uint32_t functionHash = 0;

#define offsetMask 0x3FFFFFF

// Program functions
uint32_t pageSize(uint32_t pageIndex, uint32_t totalSize)
{
	if (pageIndex > (totalSize >> 14)){
		return 0;
	}else if (pageIndex == (totalSize >> 14)){
		return totalSize & 0x3FFF;
	}
	return 0x4000;
}

uint64_t getNativeHash(uint32_t index, uint64_t *nativeTable)
{
	uint8_t rotate = (index + codeSize) & 0x3F;
	return nativeTable[index] << rotate | nativeTable[index] >> (64 - rotate);
}

uint8_t getCode(uint32_t index)
{
	if(index > codeSize)
		return 0xFF;
	uint32_t page = index >> 14;
	uint32_t offset = index & 0x3FFF;

	uint64_t *pagePtr = (uint64_t *)(buffer + codePagesOffset) + page;
	uint32_t pageOffset = *pagePtr & offsetMask;
	functionHash += buffer[pageOffset + offset];
	return buffer[pageOffset + offset];
}

void dumpCode(uint32_t pos, uint32_t amount)
{
	uint32_t i;
	uint8_t c;

	for(i=0; i<8; i++) {
		if(i<amount) {
			c = getCode(pos+i);
			printf("%02x ", c);
		} else {
			printf("   ");
		}
	}
}

void printChar(uint32_t pos)
{
	printf("%u", getCode(pos));
}

void printFloat(uint32_t pos)
{
	uint32_t i;
	float f = 0;
	uint8_t *p;

	p = (uint8_t *)&f;
	for(i=0; i<4; i++) {
		*(p+i) = getCode(pos+i);
	}

	printf("%f", f);
}

void printInt(uint32_t pos, uint32_t count)
{
	uint32_t i;
	uint32_t n = 0;
	uint8_t *p;

	p = (uint8_t *)&n;
	for(i=0; i<count; i++) {
		*(p+i) = getCode(pos+i);
	}

	printf("%u", n);
}

void printIntHex(uint32_t pos, uint32_t count)
{
	uint32_t i;
	uint32_t h = 0;
	uint8_t *p;

	p = (uint8_t *)&h;
	for(i=0; i<count; i++) {
		*(p+i) = getCode(pos+i);
	}

	printf("%08x", h);
}

void printjmp(uint32_t pos)
{
	uint32_t i;
	short s = 0;
	uint8_t *p;

	p = (uint8_t *)&s;
	for(i=0; i<2; i++) {
		*(p+i) = getCode(pos+i);
	}

	printf("%08x", pos + 2 + s);
}

void printNative(uint32_t pos)
{
	uint32_t i;
	uint32_t n = 0;
	uint8_t *p;

	p = (uint8_t *)&n;
	*p++ = getCode(pos+1);
	*p = getCode(pos);

	printf("0x%016llx", getNativeHash(n, nativePtr));
}

void printNativeP(uint32_t pos)
{
	uint8_t c = getCode(pos);

	printf("%u", c >> 2);
}

void printNativeR(uint32_t pos)
{
	uint8_t c = getCode(pos);

	printf("%u", c & 3);
}

const char *operands[127] = {
	"nop",				"iadd",				"isub",				"imul",
	"idiv",				"imod",				"iszero",			"ineg",
	"icmpeq",			"icmpne",			"icmpgt",			"icmpge",
	"icmplt",			"icmple",			"fadd",				"fsub",
	"fmul",				"fdiv",				"fmod",				"fneg",
	"fcmpeq",			"fcmpne",			"fcmpgt",			"fcmpge",
	"fcmplt",			"fcmple",			"vadd",				"vsub",
	"vmul",				"vdiv",				"vneg",				"iand",
	"ior",				"ixor",				"itof",				"ftoi",
	"ftov",				"push",				"push",				"push",
	"push",				"push",				"dup",				"pop",
	"native",			"enter",			"ret",				"pget",
	"pset",				"peekset",			"tostack",			"fromstack",
	"getarrayp",		"getarray",			"setarray",			"getframep",
	"getframe",			"setframe",			"getstaticp",		"getstatic",
	"setstatic",		"addi",				"multi",			"stackgetimmp",
	"getimmp",			"getimm",			"setimm",			"pushshort",
	"addi",				"multi",			"getimmp",			"getimm",
	"setimm",			"getarrayp",		"getarray",			"setarray",
	"getframep",		"getframe",			"setframe",			"getstaticp",
	"getstatic",		"setstatic",		"getglobalp",		"getglobal",
	"setglobal",		"jmp",				"jmp z",			"jmp ne",
	"jmp eq",			"jmp le",			"jmp lt",			"jmp ge",
	"jmp gt",			"call",				"getglobalp",		"getglobal",
	"setglobal",		"push",				"switch",			"pushstring",
	"gethash",			"strncpy",			"itos",				"strncat",
	"strncatint",		"memcpy",			"catch",			"throw",
	"pCall",			"push -1",			"push 0",			"push 1",
	"push 2",			"push 3",			"push 4",			"push 5",
	"push 6",			"push 7",			"push -1.000000",	"pushf 0.000000",
	"pushf 1.000000",	"pushf 2.000000",	"pushf 3.000000",	"pushf 4.000000",
	"pushf 5.000000",	"pushf 6.000000",	"pushf 7.000000"
};

const uint8_t size[127] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 2, 3, 4, 5, 5, 1, 1, 4, 5, 3, 1,
	1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4,
	4, 4, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

uint32_t disassemble(uint32_t index)
{
	uint32_t i;
	uint32_t next;

	uint8_t opcode = getCode(index);
	printf("%08x: ", index);

	if(opcode > 126) {
		dumpCode(index, 6);
		printf("--Unknown--");
		exit(1);
	}

	next = size[opcode];
	dumpCode(index, next);
	printf("%s ", operands[opcode]);
	switch(opcode) {
		case 37:
			printChar(index+1);
			break;
		case 38:
			printChar(index+1);
			printf(", ");
			printChar(index+2);
			break;
		case 39:
			printChar(index+1);
			printf(", ");
			printChar(index+2);
			printf(", ");
			printChar(index+3);
			break;
		case 40:
			printInt(index+1, 4);
			break;
		case 41:
			printFloat(index+1);
			break;
		case 44:
			printNative(index+2);
			printf(", ");
			printNativeP(index+1);
			printf(", ");
			printNativeR(index+1);
			break;
		case 45:
			printChar(index+1);
			printf(", ");
			printChar(index+2);
			functionHash = 0;
			break;
		case 46:
			printChar(index+1);
			printf(", ");
			printChar(index+2);
			printf("\n------ 0x%x", functionHash);
			break;
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
		case 64:
		case 65:
		case 66:
			printChar(index+1);
			break;
		case 67:
		case 68:
		case 69:
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
		case 75:
		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
		case 82:
		case 83:
			printInt(index+1, 2);
			break;
		case 85:
		case 86:
		case 87:
		case 88:
		case 89:
		case 90:
		case 91:
		case 92:
			printjmp(index+1);
			break;
		case 93:
			printIntHex(index+1, 3);
			break;
		case 94:
		case 95:
		case 96:
		case 97:
			printInt(index+1, 3);
			break;
		case 98:
			next=2+(getCode(index+1)*6);
			printChar(index+1);
			for(i=0; i<getCode(index+1); i++) {
				printf("\n          ");
				dumpCode(index+2+(i*6), 6);
				printf("  case ");
				printInt(index+2+(i*6), 4);
				printf(": jmp ");
				printjmp(index+2+(i*6)+4);				
			}
			break;
		case 101:
		case 102:
		case 103:
		case 104:
			printChar(index+1);
			break;
	}
	printf("\n");
	return index + next;
}

// Main app
int main( int argc, char *argv[] )
{
	FILE *f1;
	uint64_t fileLen;

	uint32_t i, j, c;
	int32_t  sizeCount;

	uint32_t stringPageCount = 0;

	uint32_t pagePos;
	uint32_t pageEnd;

	printf("GTA V Script Information/Disassembler\n");
	printf("       v1.1 by Mockba the Borg\n");
	printf("-------------------------------------\n");
	if(argc != 2) {
		printf("Usage: scriptinfo <GTA Script File>\n");
		exit(1);
	}

    f1 = fopen(argv[1], "rb");
	if(!f1) {
		fprintf(stderr, "Unable to open script file.\n");
		exit(1);
	}

	fseek(f1, 0, SEEK_END);
	fileLen=ftell(f1);
	rewind(f1);

	buffer = (char *)malloc(fileLen+1);
	if(!buffer) {
		fprintf(stderr, "Error allocating memory!");
		fclose(f1);
		exit(1);
	}

	printf("Reading %lu bytes ... ", fileLen, argv[1]);
	fread(buffer, fileLen, 1, f1);
	printf("ok!\n");

	header = (header_t*)buffer;

	if(header->magic != 0x405A9Ed0) {
		printf("File is not a GTAV script (0x%08x).\n", header->magic);
		exit(1);
	}

// Cleaning pointers to make life easier
	pagesOffset = header->pagesOffset & offsetMask;
	codePagesOffset = header->codePagesOffset & offsetMask;
	staticsOffset = header->staticsOffset & offsetMask;
	globalsOffset = header->globalsOffset & offsetMask;
	nativesOffset = header->nativesOffset & offsetMask;
	scriptNameOffset = header->scriptNameOffset & offsetMask;
	stringPagesOffset = header->stringPagesOffset & offsetMask;

	stringsSize = header->stringsSize;
	codeSize = header->codeSize;

	printf("\tInternal Name      : %s\n", buffer + scriptNameOffset);
	printf("\tPages Offset       : 0x%08x\n", pagesOffset);
	printf("\tCode Blocks Offset : 0x%08x\n", codePagesOffset);
	printf("\tGlobals Version    : %u (0x%08x)\n", header->globalsVersion, header->globalsVersion);
	printf("\tCode Size          : %u (0x%08x)\n", codeSize, codeSize);
	printf("\tParameters Count   : %u\n", header->paramCount);
	printf("\tStatics Count      : %u\n", header->staticsCount);
	printf("\tStatics Offset     : 0x%08x\n", staticsOffset);
	printf("\tGlobals Count      : %u\n", header->globalsCount);
	printf("\tGlobals Offset     : 0x%08x\n", globalsOffset);
	printf("\tNatives Count      : %u\n", header->nativesCount);
	printf("\tNatives Offset     : 0x%08x\n", nativesOffset);
	printf("\tStrings Offset     : 0x%08x\n", stringPagesOffset);
	printf("\tStrings Size       : %u (0x%08x)\n", stringsSize, stringsSize);

// Gets a list of code pages
	printf("\nCode pages:\n");

	offsetPtr = (uint64_t *)(buffer + codePagesOffset);
	sizeCount = codeSize;
	i = 0;
	while(sizeCount > 0) {
		printf("0x%08x ", *offsetPtr & offsetMask);
		offsetPtr++;
		if(3 == i++) {
			i = 0;
			printf("\n");
		}
		sizeCount -= 0x4000;
	}
	printf("\n");

// Gets a list of string pages
	if(stringsSize > 0) {
		printf("\nString pages:\n");

		offsetPtr = (uint64_t *)(buffer + stringPagesOffset);
		sizeCount = stringsSize;
		i = 0;
		while(sizeCount > 0) {
			printf("0x%08x ", *offsetPtr & offsetMask);
			offsetPtr++; stringPageCount++;
			if(3 == i++) {
				i = 0;
				printf("\n");
			}
			sizeCount -= 0x4000;
		}
		printf("\n");
	}

// Gets a list of all the strings
	if(stringsSize > 0) {
		printf("\nStrings used on this script:\n");

		offsetPtr = (uint64_t *)(buffer + stringPagesOffset);
		c = 0;
		for(i = 0; i < stringPageCount; i++) {
			pagePos = *(offsetPtr+i) & offsetMask;
			printf("Page 0x%08x:\n", pagePos);
			pageEnd = pagePos + pageSize(i, stringsSize);
			// Prints all the strings on this page
			j = 0;
			while(pagePos < pageEnd) {
				if(!j) {
					printf("0x%04x: \"", c);
					j = 1;
				}
				if(buffer[pagePos]) {
					putchar(buffer[pagePos]);
				} else {
					printf("\"\n");
					j = 0;
				}
				pagePos++; c++;
			}
		}
	}

// Gets a list of all the natives
	printf("\nNatives used on this script:\n");

	nativePtr = (uint64_t *)(buffer + nativesOffset);
	sizeCount = header->nativesCount;
	i = 0;
	j = 0;
	while(sizeCount) {
		printf("0x%04x: 0x%016llx ", j++, getNativeHash(i++, nativePtr));
		if((i+1)%2) {
			printf("\n");
		}
		sizeCount--;
	}
	printf("\n");

// Decodes the script's code
	printf("\nCode disassembly:\n");

	i = 0;
	while(i < codeSize)
		i = disassemble(i);

    fclose(f1);
    return 0;
}