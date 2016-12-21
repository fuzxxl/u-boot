/*
 * Copyright (C) 2016 Rockchip Electronic Co.,Ltd
 * Written by Kever Yang <kever.yang@rock-chips.com>
 *
 * origin from arm-trust-firmware
 * plat/arm/common/arm_bl2_setup.c
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <atf_common.h>
//#include <mmc.h>

#if 0
int spl_load_atf_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong sector, void *fit)
{

	return 0;
}
#endif
/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31, e.g. while passing control to it from BL2, bl31_params
 * and other platform specific params
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl31_params_t bl31_params;
	atf_image_info_t bl31_image_info;
	atf_image_info_t bl32_image_info;
	atf_image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl31_ep_info;
} bl2_to_bl31_params_mem_t;


static bl2_to_bl31_params_mem_t bl31_params_mem;

/*******************************************************************************
 * This function assigns a pointer to the memory that the platform has kept
 * aside to pass platform specific and trusted firmware related information
 * to BL31. This memory is allocated by allocating memory to
 * bl2_to_bl31_params_mem_t structure which is a superset of all the
 * structure whose information is passed to BL31
 * NOTE: This function should be called only once and should be done
 * before generating params to BL31
 ******************************************************************************/
static bl31_params_t *bl2_to_bl31_params;
bl31_params_t *bl2_plat_get_bl31_params(void)
{
	entry_point_info_t *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, sizeof(bl2_to_bl31_params_mem_t));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, PARAM_BL31, VERSION_1, 0);

	debug("bl31 type %x\n", __func__, bl2_to_bl31_params->h.type);
	/* Fill BL31 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	/* Fill BL32 related information if it exists */
#ifdef BL32_BASE
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info, PARAM_EP,
		VERSION_1, 0);
	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);
#endif /* BL32_BASE */

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl33_ep_info, PARAM_EP, VERSION_1, EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = CONFIG_SYS_TEXT_BASE;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXECPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);


	return bl2_to_bl31_params;
}

void raw_write_daif(uint32_t daif)
{
	__asm__ __volatile__("msr DAIF, %0\n\t" : : "r" (daif) : "memory");
}

void bl31_entry(void)
{
	bl31_params_t *bl31_params;
	void (*entry)(bl31_params_t *params, void *plat_params) = NULL;

	bl31_params = bl2_plat_get_bl31_params();
	entry = CONFIG_SPL_ATF_TEXT_BASE;
	debug("%s %p %x\n", __func__, entry, *(int *)CONFIG_SPL_ATF_TEXT_BASE);
	raw_write_daif(SPSR_EXCEPTION_MASK);
	dcache_disable();
	entry(bl31_params, NULL);
}

#if 0
extern int spl_mmc_find_device(struct mmc **mmcp, u32 boot_device);
void load_bl31(void)
{
	u32 image_size_sectors;
	unsigned long count;
	int ret, sector;
	struct mmc *mmc = NULL;

	debug("%s\n", __func__);
	ret = spl_mmc_find_device(&mmc, spl_boot_device());
	image_size_sectors = 320;
	sector = CONFIG_SYS_MMCSD_RAW_MODE_ATF_SECTOR;
	count = blk_dread(mmc_get_blk_desc(mmc), sector, image_size_sectors,
			CONFIG_SPL_ATF_TEXT_BASE);
	debug("hdr read sector %lx, count=%lu\n", sector, count);

	return;
}
#endif
