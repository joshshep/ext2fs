# created by Joshua Shepherd

CC=gcc
CFLAGS=-std=c99 -Wall -O0 -m32# -DVERBOSE# -Iinclude

#CFLAGS += #-Wno-unused-variable
#VPATH = src include
#CPPFLAGS=-std=c++11 -O3 -Wall
#LDFLAGS=
LEVEL1SRC=main.c \
          cd_pwd.c \
          chmod.c \
          iget_iput_getino.c \
          link_unlink.c \
          miscl.c \
          mkdir_creat.c \
          ls.c \
          rmdir.c \
          stat.c \
          symlink_readlink.c \
          touch.c \
          util.c

LEVEL2SRC = open_close_lseek.c \
            opendir_readdir.c \
            read.c \
            write.c \
            cat.c \
            cp.c \
            mv.c

LEVEL3SRC = mount.c \
            umount.c

INCLUDE=globals.h \
        level1cmd.h \
        level2cmd.h \
        level3cmd.h \
        type.h
LEVEL1OBJ = $(LEVEL1SRC:.c=.o)
LEVEL2OBJ = $(LEVEL2SRC:.c=.o)
LEVEL3OBJ = $(LEVEL3SRC:.c=.o)
TARGET=ext2fs

level1_src_dir = src/level1
level2_src_dir = src/level2
level3_src_dir = src/level3
level1_obj_dir = bin/level1
level2_obj_dir = bin/level2
level3_obj_dir = bin/level3
bin_dir = bin
include_dir = include
disks_dir = disks

SRCS        = $(addprefix $(level1_src_dir)/, $(LEVEL1SRC))
SRCS       += $(addprefix $(level2_src_dir)/, $(LEVEL2SRC))
SRCS       += $(addprefix $(level3_src_dir)/, $(LEVEL3SRC))
INCLUDES    = $(addprefix $(include_dir)/,    $(INCLUDE))
LEVEL1OBJS  = $(addprefix $(level1_obj_dir)/, $(LEVEL1OBJ))
LEVEL2OBJS  = $(addprefix $(level2_obj_dir)/, $(LEVEL2OBJ))
LEVEL3OBJS  = $(addprefix $(level3_obj_dir)/, $(LEVEL3OBJ))


all: $(TARGET)

$(TARGET): $(LEVEL1OBJS) $(LEVEL2OBJS) $(LEVEL3OBJS)
	$(CC) -o $(TARGET) $(LEVEL1OBJS) $(LEVEL2OBJS) $(LEVEL3OBJS) $(CFLAGS)

$(LEVEL1OBJS): $(level1_obj_dir)/%.o : $(level1_src_dir)/%.c $(INCLUDES)
	$(CC) -c $(CFLAGS) $< -o $@

$(LEVEL2OBJS): $(level2_obj_dir)/%.o : $(level2_src_dir)/%.c $(INCLUDES)
	$(CC) -c $(CFLAGS) $< -o $@

$(LEVEL3OBJS): $(level3_obj_dir)/%.o : $(level3_src_dir)/%.c $(INCLUDES)
	$(CC) -c $(CFLAGS) $< -o $@

test: all
	./$(TARGET) $(disks_dir)/mydisk
test2: all
	./$(TARGET) $(disks_dir)/diskimage
testexam: all
	./$(TARGET) $(disks_dir)/diskimage

verbose: CFLAGS += -DVERBOSE
verbose: all

newdisks:
	cp $(disks_dir)/mydisk_unadult $(disks_dir)/mydisk
	cp $(disks_dir)/mydisk_unadult mydisk
	cp $(disks_dir)/diskimage_unadult $(disks_dir)/diskimage
	cp $(disks_dir)/diskimage_unadult diskimage
	cp $(disks_dir)/exam_image_unadult $(disks_dir)/exam_image
#concise:

#disk:
run: all
	./$(BINS)

clean:
	rm -rf $(LEVEL1OBJS) $(LEVEL2OBJS) $(LEVEL3OBJS) $(TARGET)
