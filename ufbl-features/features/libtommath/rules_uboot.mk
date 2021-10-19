#
# Copyright 2019 Amazon.com, Inc. or its Affiliates. All rights reserved.
#
subdir-ccflags-$(CONFIG_LIBTOMMATH) += -Os -funroll-loops \
		-fomit-frame-pointer \
		-DLTM_NO_FILE

subdir-ccflags-$(CONFIG_LIBTOMMATH) += -ffunction-sections -fdata-sections

subdir-ccflags-y += \
	-I$(UFBL_PATH)/features/libtomcrypt/uboot \
	-I$(UFBL_PATH)/features/libtommath/src \

subdir-ccflags-$(CONFIG_LIBTOMMATH) += -Os -funroll-loops \
		-fomit-frame-pointer \
		-DLTM_NO_FILE

subdir-ccflags-$(CONFIG_LIBTOMMATH) += -ffunction-sections -fdata-sections

subdir-ccflags-$(CONFIG_LIBTOMMATH) += \
	-I$(UFBL_PATH)/features/libtomcrypt/uboot \
	-I$(UFBL_PATH)/features/libtommath/src \

# Leave line above blank
obj-$(CONFIG_LIBTOMMATH) += \
	../../../../ufbl-features/features/libtommath/src/bncore.o \
	../../../../ufbl-features/features/libtommath/src/bn_error.o \
	../../../../ufbl-features/features/libtommath/src/bn_fast_mp_invmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_fast_mp_montgomery_reduce.o \
	../../../../ufbl-features/features/libtommath/src/bn_fast_s_mp_mul_digs.o \
	../../../../ufbl-features/features/libtommath/src/bn_fast_s_mp_mul_high_digs.o \
	../../../../ufbl-features/features/libtommath/src/bn_fast_s_mp_sqr.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_2expt.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_abs.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_add.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_add_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_addmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_and.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_clamp.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_clear.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_clear_multi.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_cmp.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_cmp_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_cmp_mag.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_cnt_lsb.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_copy.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_count_bits.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_div_2.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_div_2d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_div_3.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_div.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_div_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_dr_is_modulus.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_dr_reduce.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_dr_setup.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_exch.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_export.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_expt_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_expt_d_ex.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_exptmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_exptmod_fast.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_exteuclid.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_gcd.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_get_int.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_get_long.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_get_long_long.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_grow.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_import.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init_copy.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init_multi.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init_set.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init_set_int.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_init_size.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_invmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_invmod_slow.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_is_square.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_jacobi.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_karatsuba_mul.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_karatsuba_sqr.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_lcm.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_lshd.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mod_2d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mod_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_montgomery_calc_normalization.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_montgomery_reduce.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_montgomery_setup.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mul_2.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mul_2d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mul.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mul_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_mulmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_neg.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_n_root.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_n_root_ex.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_or.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_fermat.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_is_divisible.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_is_prime.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_miller_rabin.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_next_prime.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_rabin_miller_trials.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_prime_random_ex.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_radix_size.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_radix_smap.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_rand.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_read_radix.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_read_signed_bin.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_read_unsigned_bin.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_2k.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_2k_l.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_2k_setup.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_2k_setup_l.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_is_2k.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_is_2k_l.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_reduce_setup.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_rshd.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_set.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_set_int.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_set_long.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_set_long_long.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_shrink.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_signed_bin_size.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_sqr.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_sqrmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_sqrt.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_sub.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_sub_d.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_submod.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_toom_mul.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_toom_sqr.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_toradix.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_toradix_n.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_to_signed_bin.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_to_signed_bin_n.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_to_unsigned_bin.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_to_unsigned_bin_n.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_unsigned_bin_size.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_xor.o \
	../../../../ufbl-features/features/libtommath/src/bn_mp_zero.o \
	../../../../ufbl-features/features/libtommath/src/bn_prime_tab.o \
	../../../../ufbl-features/features/libtommath/src/bn_reverse.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_add.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_exptmod.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_mul_digs.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_mul_high_digs.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_sqr.o \
	../../../../ufbl-features/features/libtommath/src/bn_s_mp_sub.o\

# Leave line above blank
