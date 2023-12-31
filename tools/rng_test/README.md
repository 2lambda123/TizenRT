# RNG(Random Number Generator) duplication testing tool

This tool can be used to find duplicate random numbers generated by auth_generate_random() function.

It repeats the following two operations.

1. Call auth_generate_random() (executed by 'security_api' TASH command)
2. Reboot



### Prerequisites

* Python 2.7 & 3.x

* Install 'pySerial'

  ```
  ~$ pip install pyserial
  ```

  

### How to use

1. Change settings if you need

   * Change the size of random value in 'security_api_auth_test.c'. (default : 100 bytes)

     ```c
     // apps/examples/security_test/security_api/security_api_auth_test.c
     void test_authenticate(void)
     {
         ...
     	printf("ok\n");
     	printf("  . SEC Get Random ...\n");
     	fflush(stdout);
     	if (0 != auth_generate_random(hnd, 100, &rand)) { // change here
     		printf("Fail\n	! auth_generate_random\n");
     		goto exit;
     	}
     	printf("ok\n");
     	PrintBuffer("Random", rand.data, rand.length);
         ...
     }
     ```

     

   * This tool is made for the purpose of testing 16 bytes random values. If you want to test a larger random value, you need to modify 'PrintBuffer()' in 'security_api_utils.c' (to output random values in a single line).

     ```c
     // apps/examples/security_test/security_api/security_api_utils.c
     void PrintBuffer(const char *header, unsigned char* buffer, uint32_t len)
     {
     	register uint32_t i = 0;
     	printf("%s : %d\n", header, len);
     	for (i = 0; i < len; i++) {
     		if (i != 0 && i % 16 == 0) {
     			printf("\n"); // comment out here
     		}
     		printf(" %02X", buffer[i]);
     	}
     	printf("\n");
     }
     ```

   

2. Enable Security API configurations in menuconfig.

   * Application Configuration - Examples - Security Test 
     * [*] Security API Test

   * Security Framework 
     * [*] Security API
       * [*] Authentication
       * [*] Cryptography
       * [*] Key manager
       * [*] Secure storage



3. Build & download the binary.

   

4. Run the script

````bash
$ python rng_test.py [port_num] [baudrate] [iteration]
````



### Sample execution & results

1. No duplicate case

```bash
~/TizenRT/tools/rng_test$ python rng_test.py /dev/ttyUSB1 115200 100

------------- Start RNG Duplication Test -------------

Test 1 :  A3 2E 0B D9 60 03 49 29 87 3A EB D7 13 2F B5 E6

Test 2 :  05 20 41 09 38 06 4E 25 A9 62 88 DB 62 1F D5 8E

Test 3 :  9F 95 A4 24 B0 3C 42 02 96 96 40 8C 10 CE 09 59

...

Test 98 :  16 12 A2 50 00 C2 86 05 71 4A 21 3B 3B 71 42 21

Test 99 :  E7 AA 2A D5 E8 E7 A5 29 9B 0B 2B 3B A4 70 DB 22

Test 100 :  F9 6B 2D 2B 98 DA 51 35 2E CC 2C A7 6E 3C 27 37

------------- End iteration -------------

Total duplicated cases : 0 / 100

No duplicated cases found


------------- End RNG Duplication Test -------------

~/TizenRT/tools/rng_test$
```



2. Duplicate case occurred

```bash
~/TizenRT/tools/rng_test$ python rng_test.py /dev/ttyUSB1 115200 400

------------- Start RNG Duplication Test -------------

Test 1 :  FD 1B 91 48 00 A6 47 21 B6 6D A9 D9 B8 F3 08 B0

Test 2 :  77 79 CA 53 28 2D 44 05 94 45 01 8F C4 53 DB 16

Test 3 :  8C A7 4F 7D C8 E8 4A 22 AE BE E9 2D 49 A8 1E F6

...

Test 399 :  9F 69 61 4B 68 17 4C 16 B2 07 05 D7 67 16 CA 1F

Test 400 :  C9 67 5D AB 10 2B 49 3C 91 DC 6E 47 08 C5 81 77

------------- End iteration -------------

Total duplicated cases : 5 / 400

----------!!! Duplication occurred !!!----------

Test 243 :  73 A1 00 85 A0 7C 16 09 34 05 02 E2 48 FC 89 15

Test 355 :  73 A1 00 85 A0 7C 16 09 34 05 02 E2 48 FC 89 15

----------------------------------------
Test 11 :  A7 83 6A D4 C0 68 B3 29 9B 0E 2B 19 EA 60 D0 36

Test 174 :  A7 83 6A D4 C0 68 B3 29 9B 0E 2B 19 EA 60 D0 36

----------------------------------------
Test 8 :  53 80 38 84 C0 01 04 09 D4 03 02 0D 11 8C 00 06

Test 324 :  53 80 38 84 C0 01 04 09 D4 03 02 0D 11 8C 00 06

----------------------------------------
Test 49 :  B7 14 B1 88 98 B2 D1 09 68 32 C2 B0 8B 53 EA C8

Test 81 :  B7 14 B1 88 98 B2 D1 09 68 32 C2 B0 8B 53 EA C8

----------------------------------------
Test 65 :  A3 80 56 B4 20 08 46 2F F9 80 0A 52 0F 77 AB 07

Test 66 :  A3 80 56 B4 20 08 46 2F F9 80 0A 52 0F 77 AB 07

----------------------------------------

------------- End RNG Duplication Test -------------

~/TizenRT/tools/rng_test$
```
