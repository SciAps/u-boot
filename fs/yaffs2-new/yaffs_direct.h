
struct yaffs_direct_context {
	struct yaffs_dev *dev;
	u8 *spare_buffer;
};

#define yaffs_dev_to_lc(dev) ((struct yaffs_direct_context *)((dev)->os_context))
#define yaffs_dev_to_mtd(dev) ((struct mtd_info *)((dev)->driver_context))
