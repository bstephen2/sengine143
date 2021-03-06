# sengine mingmakefile
# (c) 2017-2018, Brian Stephenson
# brian@bstephen.me.uk
#
BITS	=	32
EXE	=	sengine143
CC	=	gcc
LD	=	gcc
RELEASE	=	-O3 -DNDEBUG
DEBUG	=	-g -O0 
CFLAGS	=	${RELEASE} -DHASH_FUNCTION=HASH_OAT -c -mtune=native -m${BITS} -Wall -std=gnu11
LDFLAGS	=	-o${EXE}
IND		=	astyle
INDOPTS	=	--style=kr --align-pointer=type --indent=tab=3 --indent=spaces \
		--pad-oper --unpad-paren --break-blocks \
		--pad-header
MD5HDS	=	md5.h
MD5MODS	=	md5.c
MD5OBJS	=	md5.o
GXHDS	=	genx.h
GXMODS	=	genx.c charprops.c
GXOBJS	=	genx.o charprops.o
CHDS	=	sengine.h options.h
CMODS	=	main.c options.c init.c board.c direct.c dir_xml.c boardlist.c \
			memory.c pool.c cldir2.c dir2_class_xml.c class_util.c \
			wmate.c bmove.c wmove.c
COBJS	=	main.o options.o init.o board.o direct.o dir_xml.o boardlist.o \
			memory.o pool.o cldir2.o dir2_class_xml.o class_util.o \
			wmate.o bmove.o wmove.o
CASMS	=	main.asm options.asm init.asm board.asm direct.asm dir_xml.asm \
			boardlist.asm memory.asm pool.asm cldir2.asm dir2_class_xml.asm \
			genx.asm charprops.asm md5.asm class_util.asm wmate.asm bmove.asm wmove.asm

sengine:	${COBJS} ${MD5OBJS} ${GXOBJS}
	${LD}   ${LDFLAGS} ${COBJS} ${MD5OBJS} ${GXOBJS}
	cp ${EXE} ${HOME}/bin/${EXE}

main.o:	main.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} main.c
	objconv -fnasm main.o

class_util.o:	class_util.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} class_util.c
	objconv -fnasm class_util.o

cldir2.o:	cldir2.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} cldir2.c
	objconv -fnasm cldir2.o
	
wmate.o:	wmate.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} wmate.c
	objconv -fnasm wmate.o
	
wmove.o:	wmove.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} wmove.c
	objconv -fnasm wmove.o
	
bmove.o:	bmove.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} bmove.c
	objconv -fnasm bmove.o

dir2_class_xml.o:	dir2_class_xml.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} dir2_class_xml.c
	objconv -fnasm dir2_class_xml.o

boardlist.o:	boardlist.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} boardlist.c
	objconv -fnasm boardlist.o

direct.o:	direct.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} direct.c
	objconv -fnasm direct.o

memory.o:	memory.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} memory.c
	objconv -fnasm memory.o
	
dir_xml.o:	dir_xml.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} dir_xml.c
	objconv -fnasm dir_xml.o
	
options.o:	options.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} options.c
	objconv -fnasm options.o

init.o:	init.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} init.c
	objconv -fnasm init.o

board.o:	board.c ${CHDS} ${MD5HDS} ${GXHDS}
	${CC} ${CFLAGS} board.c
	objconv -fnasm board.o
	
md5.o:	md5.c ${MD5HDS}
	${CC} ${CFLAGS} md5.c
	objconv -fnasm md5.o
	
charprops.o:	charprops.c ${MD5HDS}
	${CC} ${CFLAGS} charprops.c
	objconv -fnasm charprops.o
	
genx.o:	genx.c ${MD5HDS}
	${CC} ${CFLAGS} genx.c
	objconv -fnasm genx.o
	
pool.o:	pool.c pool.h
	${CC} ${CFLAGS} pool.c
	objconv -fnasm pool.o
	
clean:
	rm ${COBJS} ${CASMS} ${MD5OBJS} ${GXOBJS} ${EXE}  *orig *xml sol.txt

tidy:
	${IND} ${INDOPTS} ${CMODS} ${CHDS}

touch:
	touch ${CMODS} ${CHDS} ${MD5MODS} ${GXMODS}
	
count:
	wc -l ${CMODS} ${CHDS} | sort -b -n	
