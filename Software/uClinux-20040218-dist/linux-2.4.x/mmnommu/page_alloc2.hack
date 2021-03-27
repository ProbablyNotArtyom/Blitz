Index: fs/block_dev.c
===================================================================
RCS file: /cvs/sw/linux-2.4.x/fs/block_dev.c,v
retrieving revision 1.1.1.4
diff -u -r1.1.1.4 block_dev.c
--- fs/block_dev.c	7 Jan 2002 23:15:54 -0000	1.1.1.4
+++ fs/block_dev.c	5 Feb 2002 01:55:11 -0000
@@ -123,7 +123,7 @@
 	return block_write_full_page(page, blkdev_get_block);
 }
 
-static int blkdev_readpage(struct file * file, struct page * page)
+int blkdev_readpage(struct file * file, struct page * page)
 {
 	return block_read_full_page(page, blkdev_get_block);
 }
Index: fs/ext2/inode.c
===================================================================
RCS file: /cvs/sw/linux-2.4.x/fs/ext2/inode.c,v
retrieving revision 1.1.1.4
diff -u -r1.1.1.4 inode.c
--- fs/ext2/inode.c	7 Jan 2002 23:15:55 -0000	1.1.1.4
+++ fs/ext2/inode.c	5 Feb 2002 01:55:12 -0000
@@ -580,7 +580,7 @@
 {
 	return block_write_full_page(page,ext2_get_block);
 }
-static int ext2_readpage(struct file *file, struct page *page)
+int ext2_readpage(struct file *file, struct page *page)
 {
 	return block_read_full_page(page,ext2_get_block);
 }
Index: fs/romfs/inode.c
===================================================================
RCS file: /cvs/sw/linux-2.4.x/fs/romfs/inode.c,v
retrieving revision 1.3
diff -u -r1.3 inode.c
--- fs/romfs/inode.c	8 Jan 2002 00:51:17 -0000	1.3
+++ fs/romfs/inode.c	5 Feb 2002 01:55:13 -0000
@@ -390,7 +390,7 @@
  * we can't use bmap, since we may have looser alignments.
  */
 
-static int
+int
 romfs_readpage(struct file *file, struct page * page)
 {
 	struct inode *inode = page->mapping->host;
Index: drivers/block/rd.c
===================================================================
RCS file: /cvs/sw/linux-2.4.x/drivers/block/rd.c,v
retrieving revision 1.1.1.3
diff -u -r1.1.1.3 rd.c
--- drivers/block/rd.c	7 Jan 2002 23:15:32 -0000	1.1.1.3
+++ drivers/block/rd.c	5 Feb 2002 01:55:14 -0000
@@ -191,7 +191,7 @@
  *               2000 Transmeta Corp.
  * aops copied from ramfs.
  */
-static int ramdisk_readpage(struct file *file, struct page * page)
+int ramdisk_readpage(struct file *file, struct page * page)
 {
 	if (!Page_Uptodate(page)) {
 		memset(kmap(page), 0, PAGE_CACHE_SIZE);
