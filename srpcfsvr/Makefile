#
# SRPCF - Simple Remote Procedire Command Framework
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
CROSS_COMPILE       =
AS                  =   $(CROSS_COMPILE)as
AR                  =   $(CROSS_COMPILE)ar
CC                  =   $(CROSS_COMPILE)gcc
CPP                 =   $(CC) -E
LD                  =   $(CROSS_COMPILE)ld
NM                  =   $(CROSS_COMPILE)nm
OBJCOPY             =   $(CROSS_COMPILE)objcopy
OBJDUMP             =   $(CROSS_COMPILE)objdump
RANLIB              =   $(CROSS_COMPILE)ranlib
READELF             =   $(CROSS_COMPILE)readelf
SIZE                =   $(CROSS_COMPILE)size
STRINGS             =   $(CROSS_COMPILE)strings
STRIP               =   $(CROSS_COMPILE)strip

CFLAGS				=	-I../include -Wall -DSRPCFSVR_DEBUG -g3
LDFLAGS				=	-ldl -rdynamic -L../libsrpcf -lsrpcf
OBJS				=   srpcfsvr
LIBS				=	srpcfsvr.o

all: $(OBJS)

srpcfsvr: $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(OBJS)


