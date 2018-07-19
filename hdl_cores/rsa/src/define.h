#define PATH                  "./gen/"
#define DISPLAY               0
#define DEBUG                 0

#define ADDR_BASE             0x96000000

#define OPERAND_WIDTH         32
#define ADDRESS_WIDTH         8
#define ADDR_NAME1            ADDR_BASE|(0x01<<2)
#define ADDR_VERSION          ADDR_BASE|(0x02<<2)
#define ADDR_CTRL             ADDR_BASE|(0x08<<2)
#define CTRL_INIT_BIT         0
#define CTRL_NEXT_BIT         1
#define ADDR_STATUS           ADDR_BASE|(0x09<<2)
#define STATUS_READY_BIT      0

#define ADDR_CYCLES_HIGH      ADDR_BASE|(0x10<<2)
#define ADDR_CYCLES_LOW       ADDR_BASE|(0x11<<2)
#define ADDR_MODULUS_LENGTH   ADDR_BASE|(0x20<<2)
#define ADDR_EXPONENT_LENGTH  ADDR_BASE|(0x21<<2)
#define ADDR_MODULUS_PTR_RST  ADDR_BASE|(0x30<<2)
#define ADDR_MODULUS_DATA     ADDR_BASE|(0x31<<2)
#define ADDR_EXPONENT_PTR_RST ADDR_BASE|(0x40<<2)
#define ADDR_EXPONENT_DATA    ADDR_BASE|(0x41<<2)
#define ADDR_MESSAGE_PTR_RST  ADDR_BASE|(0x50<<2)
#define ADDR_MESSAGE_DATA     ADDR_BASE|(0x51<<2)
#define ADDR_RESULT_PTR_RST   ADDR_BASE|(0x60<<2)
#define ADDR_RESULT_DATA      ADDR_BASE|(0x61<<2)
#define DEFAULT_MODLENGTH     ADDR_BASE|(0x80<<2)
#define DEFAULT_EXPLENGTH     ADDR_BASE|(0x80<<2)

#define PUB_MODULUS           1
#define PUB_EXPONENT          1
#define PRI_ALGORITHM_VERSION 0
#define PRI_MODULUS           1
#define PRI_PUBLIC_EXPONENT   0
#define PRI_PRIVATE_EXPONENT  1
#define PRI_PRIME_1           0
#define PRI_PRIME_2           0
#define PRI_EXPONENT_1        0
#define PRI_EXPONENT_2        0
#define PRI_COEFFICIENT       0

