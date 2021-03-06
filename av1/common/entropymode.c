/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#include "aom_mem/aom_mem.h"

#include "av1/common/reconinter.h"
#include "av1/common/scan.h"
#include "av1/common/onyxc_int.h"
#include "av1/common/seg_common.h"
#include "av1/common/txb_common.h"

static const aom_cdf_prob default_newmv_cdf[NEWMV_MODE_CONTEXTS][CDF_SIZE(2)] =
    {
      { AOM_CDF2(128 * 155) }, { AOM_CDF2(128 * 116) }, { AOM_CDF2(128 * 94) },
      { AOM_CDF2(128 * 32) },  { AOM_CDF2(128 * 96) },  { AOM_CDF2(128 * 30) }
    };

static const aom_cdf_prob default_zeromv_cdf[GLOBALMV_MODE_CONTEXTS][CDF_SIZE(
    2)] = { { AOM_CDF2(128 * 45) }, { AOM_CDF2(128 * 13) } };

static const aom_cdf_prob default_refmv_cdf[REFMV_MODE_CONTEXTS][CDF_SIZE(2)] =
    {
      { AOM_CDF2(128 * 178) }, { AOM_CDF2(128 * 212) }, { AOM_CDF2(128 * 135) },
      { AOM_CDF2(128 * 244) }, { AOM_CDF2(128 * 203) }, { AOM_CDF2(128 * 122) }
    };

static const aom_cdf_prob default_drl_cdf[DRL_MODE_CONTEXTS][CDF_SIZE(2)] = {
  { AOM_CDF2(128 * 119) },
  { AOM_CDF2(128 * 189) },
  { AOM_CDF2(128 * 134) },
};

static const aom_cdf_prob
    default_inter_compound_mode_cdf[INTER_MODE_CONTEXTS][CDF_SIZE(
        INTER_COMPOUND_MODES)] = {
      { AOM_CDF8(8923, 11946, 15028, 16879, 18399, 19766, 27581) },
      { AOM_CDF8(8032, 15054, 17634, 19079, 20749, 22202, 28726) },
      { AOM_CDF8(6561, 13379, 15613, 17159, 19041, 20594, 26752) },
      { AOM_CDF8(13968, 15752, 20444, 23598, 24277, 24950, 25748) },
      { AOM_CDF8(13861, 18611, 21783, 23962, 24696, 25424, 30609) },
      { AOM_CDF8(7885, 12311, 15976, 19024, 20515, 21661, 23147) },
      { AOM_CDF8(11407, 16588, 19365, 21657, 22748, 23629, 28912) },
      { AOM_CDF8(10681, 18953, 20791, 22468, 23935, 25024, 28506) }
    };

static const aom_cdf_prob
    default_compound_type_cdf[BLOCK_SIZES_ALL][CDF_SIZE(COMPOUND_TYPES - 1)] = {
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) },  // 255, 1
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) }, { AOM_CDF2(16384) }, { AOM_CDF2(16384) },
      { AOM_CDF2(16384) },  // 208, 1
      { AOM_CDF2(16384) }
    };

#if WEDGE_IDX_ENTROPY_CODING
static const aom_cdf_prob default_wedge_idx_cdf[BLOCK_SIZES_ALL][CDF_SIZE(16)] =
    { { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(1774, 4089, 6638, 8345, 9971, 11534, 14522, 17604, 19347,
                  20943, 22583, 24124, 26402, 28419, 30844) },
      { AOM_CDF16(700, 3119, 5981, 6655, 7235, 7425, 7892, 14701, 16022, 17010,
                  18095, 19071, 22367, 25729, 29458) },
      { AOM_CDF16(2432, 3782, 5015, 7434, 8519, 8941, 9893, 14677, 17691, 20555,
                  23331, 26186, 28214, 29573, 31297) },
      { AOM_CDF16(1360, 3473, 6001, 7459, 9262, 10863, 14014, 16851, 18510,
                  20086, 21800, 23406, 25670, 27887, 30449) },
      { AOM_CDF16(1024, 3423, 6449, 7409, 8281, 8544, 9160, 14154, 15720, 16878,
                  18297, 19379, 22278, 25678, 29261) },
      { AOM_CDF16(2119, 3907, 5840, 7908, 9492, 10033, 11292, 14209, 16886,
                  19484, 22099, 24499, 26535, 28536, 31027) },
      { AOM_CDF16(1427, 3732, 6257, 7811, 9493, 11450, 15253, 18736, 20135,
                  21395, 22949, 24272, 26148, 28096, 30633) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(274, 1379, 2780, 3024, 3133, 3200, 3327, 21242, 22126, 22814,
                  23759, 24661, 26435, 28307, 30535) },
      { AOM_CDF16(1210, 1526, 1852, 3140, 3335, 3456, 3606, 21336, 23001, 24949,
                  26857, 28843, 29924, 30832, 31925) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) },
      { AOM_CDF16(2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432,
                  20480, 22528, 24576, 26624, 28672, 30720) } };
#endif

static const aom_cdf_prob default_interintra_cdf[BLOCK_SIZE_GROUPS][CDF_SIZE(
    2)] = { { AOM_CDF2(128 * 128) },
            { AOM_CDF2(226 * 128) },
            { AOM_CDF2(244 * 128) },
            { AOM_CDF2(254 * 128) } };

static const aom_cdf_prob
    default_interintra_mode_cdf[BLOCK_SIZE_GROUPS][CDF_SIZE(INTERINTRA_MODES)] =
        { { AOM_CDF4(16384, 24576, 28672) },
          { AOM_CDF4(3072, 7016, 18987) },
          { AOM_CDF4(4864, 8461, 17481) },
          { AOM_CDF4(6528, 8681, 19031) } };

static const aom_cdf_prob
    default_wedge_interintra_cdf[BLOCK_SIZES_ALL][CDF_SIZE(2)] = {
      { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) },
      { AOM_CDF2(194 * 128) }, { AOM_CDF2(213 * 128) }, { AOM_CDF2(217 * 128) },
      { AOM_CDF2(222 * 128) }, { AOM_CDF2(224 * 128) }, { AOM_CDF2(226 * 128) },
      { AOM_CDF2(220 * 128) }, { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) },
      { AOM_CDF2(128 * 128) }, { AOM_CDF2(255 * 128) }, { AOM_CDF2(255 * 128) },
      { AOM_CDF2(255 * 128) }, { AOM_CDF2(208 * 128) }, { AOM_CDF2(208 * 128) },
      { AOM_CDF2(208 * 128) }, { AOM_CDF2(208 * 128) }, { AOM_CDF2(255 * 128) },
      { AOM_CDF2(255 * 128) },
    };

static const aom_cdf_prob
    default_motion_mode_cdf[BLOCK_SIZES_ALL][CDF_SIZE(MOTION_MODES)] = {
      { AOM_CDF3(16384, 24576) }, { AOM_CDF3(16384, 24576) },
      { AOM_CDF3(16384, 24576) }, { AOM_CDF3(7936, 19091) },
      { AOM_CDF3(4991, 19205) },  { AOM_CDF3(4992, 19314) },
      { AOM_CDF3(15104, 21590) }, { AOM_CDF3(9855, 21043) },
      { AOM_CDF3(12800, 22238) }, { AOM_CDF3(24320, 26498) },
      { AOM_CDF3(26496, 28995) }, { AOM_CDF3(25216, 28166) },
      { AOM_CDF3(30592, 31238) }, { AOM_CDF3(32256, 32656) },
      { AOM_CDF3(32256, 32656) }, { AOM_CDF3(32256, 32656) },
      { AOM_CDF3(32640, 32740) }, { AOM_CDF3(32640, 32740) },
      { AOM_CDF3(32640, 32740) }, { AOM_CDF3(32640, 32740) },
      { AOM_CDF3(32640, 32740) }, { AOM_CDF3(32640, 32740) },
    };

static const aom_cdf_prob default_obmc_cdf[BLOCK_SIZES_ALL][CDF_SIZE(2)] = {
  { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) },
  { AOM_CDF2(45 * 128) },  { AOM_CDF2(79 * 128) },  { AOM_CDF2(75 * 128) },
  { AOM_CDF2(130 * 128) }, { AOM_CDF2(141 * 128) }, { AOM_CDF2(144 * 128) },
  { AOM_CDF2(208 * 128) }, { AOM_CDF2(201 * 128) }, { AOM_CDF2(186 * 128) },
  { AOM_CDF2(231 * 128) }, { AOM_CDF2(252 * 128) }, { AOM_CDF2(252 * 128) },
  { AOM_CDF2(252 * 128) }, { AOM_CDF2(208 * 128) }, { AOM_CDF2(208 * 128) },
  { AOM_CDF2(208 * 128) }, { AOM_CDF2(208 * 128) }, { AOM_CDF2(208 * 128) },
  { AOM_CDF2(208 * 128) },
};

static const aom_cdf_prob default_delta_q_cdf[CDF_SIZE(DELTA_Q_PROBS + 1)] = {
  AOM_CDF4(28160, 32120, 32677)
};
#if CONFIG_EXT_DELTA_Q
static const aom_cdf_prob default_delta_lf_multi_cdf[FRAME_LF_COUNT][CDF_SIZE(
    DELTA_LF_PROBS + 1)] = { { AOM_CDF4(28160, 32120, 32677) },
                             { AOM_CDF4(28160, 32120, 32677) },
                             { AOM_CDF4(28160, 32120, 32677) },
                             { AOM_CDF4(28160, 32120, 32677) } };
static const aom_cdf_prob default_delta_lf_cdf[CDF_SIZE(DELTA_LF_PROBS + 1)] = {
  AOM_CDF4(28160, 32120, 32677)
};
#endif

static const aom_cdf_prob default_intra_inter_cdf[INTRA_INTER_CONTEXTS]
                                                 [CDF_SIZE(2)] = {
                                                   { AOM_CDF2(768) },
                                                   { AOM_CDF2(12416) },
                                                   { AOM_CDF2(19328) },
                                                   { AOM_CDF2(26240) }
                                                 };

static const aom_cdf_prob default_comp_inter_cdf[COMP_INTER_CONTEXTS][CDF_SIZE(
    2)] = { { AOM_CDF2(24290) },
            { AOM_CDF2(19956) },
            { AOM_CDF2(11641) },
            { AOM_CDF2(9804) },
            { AOM_CDF2(2842) } };

static const aom_cdf_prob default_comp_ref_type_cdf[COMP_REF_TYPE_CONTEXTS]
                                                   [CDF_SIZE(2)] = {
                                                     { AOM_CDF2(8 * 128) },
                                                     { AOM_CDF2(20 * 128) },
                                                     { AOM_CDF2(78 * 128) },
                                                     { AOM_CDF2(91 * 128) },
                                                     { AOM_CDF2(194 * 128) }
                                                   };
static const aom_cdf_prob
    default_uni_comp_ref_cdf[UNI_COMP_REF_CONTEXTS][UNIDIR_COMP_REFS - 1]
                            [CDF_SIZE(2)] = { { { AOM_CDF2(88 * 128) },
                                                { AOM_CDF2(30 * 128) },
                                                { AOM_CDF2(28 * 128) } },
                                              { { AOM_CDF2(218 * 128) },
                                                { AOM_CDF2(97 * 128) },
                                                { AOM_CDF2(105 * 128) } },
                                              { { AOM_CDF2(254 * 128) },
                                                { AOM_CDF2(180 * 128) },
                                                { AOM_CDF2(196 * 128) } } };

static const aom_cdf_prob
    default_comp_ref_cdf[REF_CONTEXTS][FWD_REFS - 1][CDF_SIZE(2)] = {
      { { AOM_CDF2(4412) }, { AOM_CDF2(11499) }, { AOM_CDF2(478) } },
      { { AOM_CDF2(17926) }, { AOM_CDF2(26419) }, { AOM_CDF2(8615) } },
      { { AOM_CDF2(30449) }, { AOM_CDF2(31477) }, { AOM_CDF2(28035) } }
    };

static const aom_cdf_prob
    default_comp_bwdref_cdf[REF_CONTEXTS][BWD_REFS - 1][CDF_SIZE(2)] = {
      { { AOM_CDF2(2762) }, { AOM_CDF2(1614) } },
      { { AOM_CDF2(17976) }, { AOM_CDF2(15912) } },
      { { AOM_CDF2(30894) }, { AOM_CDF2(30639) } },
    };

// TODO(zoelu): To use aom_entropy_optimizer to update the following defaults.
static const aom_cdf_prob default_single_ref_cdf[REF_CONTEXTS][SINGLE_REFS - 1]
                                                [CDF_SIZE(2)] = {
                                                  { { AOM_CDF2(6500) },
                                                    { AOM_CDF2(3089) },
                                                    { AOM_CDF2(4026) },
                                                    { AOM_CDF2(8549) },
                                                    { AOM_CDF2(184) },
                                                    { AOM_CDF2(2264) } },
                                                  { { AOM_CDF2(17037) },
                                                    { AOM_CDF2(19408) },
                                                    { AOM_CDF2(15521) },
                                                    { AOM_CDF2(27640) },
                                                    { AOM_CDF2(5047) },
                                                    { AOM_CDF2(16251) } },
                                                  { { AOM_CDF2(28292) },
                                                    { AOM_CDF2(30427) },
                                                    { AOM_CDF2(29003) },
                                                    { AOM_CDF2(31436) },
                                                    { AOM_CDF2(28466) },
                                                    { AOM_CDF2(29371) } }
                                                };

// TODO(huisu): tune these cdfs
const aom_cdf_prob
    default_palette_y_size_cdf[PALATTE_BSIZE_CTXS][CDF_SIZE(PALETTE_SIZES)] = {
      { AOM_CDF7(12288, 19408, 24627, 26662, 28499, 30667) },
      { AOM_CDF7(12288, 19408, 24627, 26662, 28499, 30667) },
      { AOM_CDF7(12288, 19408, 24627, 26662, 28499, 30667) },
      { AOM_CDF7(2815, 4570, 9416, 10875, 13782, 19863) },
      { AOM_CDF7(12032, 14948, 22187, 23138, 24756, 27635) },
      { AOM_CDF7(14847, 20167, 25433, 26751, 28278, 30119) },
      { AOM_CDF7(18816, 25574, 29030, 29877, 30656, 31506) },
      { AOM_CDF7(23039, 27333, 30220, 30708, 31070, 31826) },
      { AOM_CDF7(12543, 20838, 27455, 28762, 29763, 31546) },
    };

const aom_cdf_prob
    default_palette_uv_size_cdf[PALATTE_BSIZE_CTXS][CDF_SIZE(PALETTE_SIZES)] = {
      { AOM_CDF7(20480, 29888, 32453, 32715, 32751, 32766) },
      { AOM_CDF7(20480, 29888, 32453, 32715, 32751, 32766) },
      { AOM_CDF7(20480, 29888, 32453, 32715, 32751, 32766) },
      { AOM_CDF7(11135, 23641, 31056, 31998, 32496, 32668) },
      { AOM_CDF7(9984, 21999, 29192, 30645, 31640, 32402) },
      { AOM_CDF7(7552, 16614, 24880, 27283, 29254, 31203) },
      { AOM_CDF7(11391, 18656, 23727, 26058, 27788, 30278) },
      { AOM_CDF7(8576, 13585, 17632, 20884, 23948, 27152) },
      { AOM_CDF7(9216, 14276, 19043, 22689, 25799, 28712) },
    };

// When palette mode is enabled, following probability tables indicate the
// probabilities to code the "is_palette" bit (i.e. the bit that indicates
// if this block uses palette mode or DC_PRED mode).
const aom_cdf_prob default_palette_y_mode_cdf[PALATTE_BSIZE_CTXS]
                                             [PALETTE_Y_MODE_CONTEXTS]
                                             [CDF_SIZE(2)] = {
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                               { { AOM_CDF2(128 * 240) },
                                                 { AOM_CDF2(128 * 180) },
                                                 { AOM_CDF2(128 * 100) } },
                                             };

const aom_cdf_prob
    default_palette_uv_mode_cdf[PALETTE_UV_MODE_CONTEXTS][CDF_SIZE(2)] = {
      { AOM_CDF2(128 * 253) }, { AOM_CDF2(128 * 229) }
    };

const aom_cdf_prob default_palette_y_color_index_cdf
    [PALETTE_SIZES][PALETTE_COLOR_INDEX_CONTEXTS][CDF_SIZE(PALETTE_COLORS)] = {
      {
          { AOM_CDF2(29568), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(16384), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(8832), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(28672), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(31872), 0, 0, 0, 0, 0, 0 },
      },
      {
          { AOM_CDF3(28032, 30326), 0, 0, 0, 0, 0 },
          { AOM_CDF3(11647, 27405), 0, 0, 0, 0, 0 },
          { AOM_CDF3(4352, 30659), 0, 0, 0, 0, 0 },
          { AOM_CDF3(23552, 27800), 0, 0, 0, 0, 0 },
          { AOM_CDF3(32256, 32504), 0, 0, 0, 0, 0 },
      },
      {
          { AOM_CDF4(26112, 28374, 30039), 0, 0, 0, 0 },
          { AOM_CDF4(9472, 22576, 27712), 0, 0, 0, 0 },
          { AOM_CDF4(6656, 26138, 29608), 0, 0, 0, 0 },
          { AOM_CDF4(19328, 23791, 28946), 0, 0, 0, 0 },
          { AOM_CDF4(31744, 31984, 32336), 0, 0, 0, 0 },
      },
      {
          { AOM_CDF5(27904, 29215, 30075, 31190), 0, 0, 0 },
          { AOM_CDF5(9728, 22598, 26134, 29425), 0, 0, 0 },
          { AOM_CDF5(2688, 30066, 31058, 31933), 0, 0, 0 },
          { AOM_CDF5(22015, 25039, 27726, 29932), 0, 0, 0 },
          { AOM_CDF5(32383, 32482, 32554, 32660), 0, 0, 0 },
      },
      {
          { AOM_CDF6(24319, 26299, 27486, 28600, 29804), 0, 0 },
          { AOM_CDF6(7935, 18217, 21116, 25440, 28589), 0, 0 },
          { AOM_CDF6(6656, 25016, 27105, 28698, 30399), 0, 0 },
          { AOM_CDF6(19967, 24117, 26550, 28566, 30224), 0, 0 },
          { AOM_CDF6(31359, 31607, 31775, 31977, 32258), 0, 0 },
      },
      {
          { AOM_CDF7(26368, 27768, 28588, 29274, 29997, 30917), 0 },
          { AOM_CDF7(8960, 18260, 20810, 23986, 26627, 28882), 0 },
          { AOM_CDF7(7295, 24111, 25836, 27515, 29033, 30769), 0 },
          { AOM_CDF7(22016, 25208, 27305, 28159, 29221, 30274), 0 },
          { AOM_CDF7(31744, 31932, 32050, 32199, 32335, 32521), 0 },
      },
      {
          { AOM_CDF8(26624, 27872, 28599, 29153, 29633, 30172, 30841) },
          { AOM_CDF8(6655, 17569, 19587, 23345, 25884, 28088, 29678) },
          { AOM_CDF8(3584, 27296, 28429, 29158, 30032, 30780, 31572) },
          { AOM_CDF8(23551, 25855, 27070, 27893, 28597, 29721, 30970) },
          { AOM_CDF8(32128, 32173, 32245, 32337, 32416, 32500, 32609) },
      },
    };

const aom_cdf_prob default_palette_uv_color_index_cdf
    [PALETTE_SIZES][PALETTE_COLOR_INDEX_CONTEXTS][CDF_SIZE(PALETTE_COLORS)] = {
      {
          { AOM_CDF2(29824), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(16384), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(8832), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(30720), 0, 0, 0, 0, 0, 0 },
          { AOM_CDF2(31744), 0, 0, 0, 0, 0, 0 },
      },
      {
          { AOM_CDF3(27648, 30208), 0, 0, 0, 0, 0 },
          { AOM_CDF3(14080, 26563), 0, 0, 0, 0, 0 },
          { AOM_CDF3(5120, 30932), 0, 0, 0, 0, 0 },
          { AOM_CDF3(24448, 27828), 0, 0, 0, 0, 0 },
          { AOM_CDF3(31616, 32219), 0, 0, 0, 0, 0 },
      },
      {
          { AOM_CDF4(25856, 28259, 30584), 0, 0, 0, 0 },
          { AOM_CDF4(11520, 22476, 27944), 0, 0, 0, 0 },
          { AOM_CDF4(8064, 26882, 30308), 0, 0, 0, 0 },
          { AOM_CDF4(19455, 23823, 29134), 0, 0, 0, 0 },
          { AOM_CDF4(30848, 31501, 32174), 0, 0, 0, 0 },
      },
      {
          { AOM_CDF5(26751, 28020, 29541, 31230), 0, 0, 0 },
          { AOM_CDF5(12032, 26045, 30772, 31497), 0, 0, 0 },
          { AOM_CDF5(1280, 32153, 32458, 32560), 0, 0, 0 },
          { AOM_CDF5(23424, 24154, 29201, 29856), 0, 0, 0 },
          { AOM_CDF5(32256, 32402, 32561, 32682), 0, 0, 0 },
      },
      {
          { AOM_CDF6(24576, 26720, 28114, 28950, 31694), 0, 0 },
          { AOM_CDF6(7551, 16613, 20462, 25269, 29077), 0, 0 },
          { AOM_CDF6(6272, 23039, 25623, 28163, 30861), 0, 0 },
          { AOM_CDF6(17024, 18808, 20771, 27941, 29845), 0, 0 },
          { AOM_CDF6(31616, 31936, 32079, 32321, 32546), 0, 0 },
      },
      {
          { AOM_CDF7(23296, 25590, 27833, 29337, 29954, 31229), 0 },
          { AOM_CDF7(7552, 13659, 16570, 21695, 24506, 27701), 0 },
          { AOM_CDF7(6911, 24788, 26284, 27753, 29575, 30872), 0 },
          { AOM_CDF7(17535, 22236, 24457, 26242, 27363, 30191), 0 },
          { AOM_CDF7(30592, 31289, 31745, 31921, 32149, 32321), 0 },
      },
      {
          { AOM_CDF8(22016, 24242, 25141, 27137, 27797, 29331, 30848) },
          { AOM_CDF8(8063, 13564, 16940, 21948, 24568, 25689, 26989) },
          { AOM_CDF8(6528, 27028, 27835, 28741, 30031, 31795, 32285) },
          { AOM_CDF8(18047, 23797, 25444, 26274, 27111, 27929, 30367) },
          { AOM_CDF8(30208, 30628, 31046, 31658, 31762, 32367, 32469) },
      }
    };

static const aom_cdf_prob default_intrabc_cdf[CDF_SIZE(2)] = { AOM_CDF2(192 *
                                                                        128) };

#define MAX_COLOR_CONTEXT_HASH 8
// Negative values are invalid
static const int palette_color_index_context_lookup[MAX_COLOR_CONTEXT_HASH +
                                                    1] = { -1, -1, 0, -1, -1,
                                                           4,  3,  2, 1 };

static const aom_cdf_prob default_switchable_restore_cdf[CDF_SIZE(
    RESTORE_SWITCHABLE_TYPES)] = { AOM_CDF3(32 * 128, 144 * 128) };

static const aom_cdf_prob default_wiener_restore_cdf[CDF_SIZE(2)] = { AOM_CDF2(
    64 * 128) };

static const aom_cdf_prob default_sgrproj_restore_cdf[CDF_SIZE(2)] = { AOM_CDF2(
    64 * 128) };

#define NUM_PALETTE_NEIGHBORS 3  // left, top-left and top.
int av1_get_palette_color_index_context(const uint8_t *color_map, int stride,
                                        int r, int c, int palette_size,
                                        uint8_t *color_order, int *color_idx) {
  int i;
  // The +10 below should not be needed. But we get a warning "array subscript
  // is above array bounds [-Werror=array-bounds]" without it, possibly due to
  // this (or similar) bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59124
  int scores[PALETTE_MAX_SIZE + 10];
  const int weights[NUM_PALETTE_NEIGHBORS] = { 2, 1, 2 };
  const int hash_multipliers[NUM_PALETTE_NEIGHBORS] = { 1, 2, 2 };
  int color_index_ctx_hash;
  int color_index_ctx;
  int color_neighbors[NUM_PALETTE_NEIGHBORS];
  int inverse_color_order[PALETTE_MAX_SIZE];
  assert(palette_size <= PALETTE_MAX_SIZE);
  assert(r > 0 || c > 0);

  // Get color indices of neighbors.
  color_neighbors[0] = (c - 1 >= 0) ? color_map[r * stride + c - 1] : -1;
  color_neighbors[1] =
      (c - 1 >= 0 && r - 1 >= 0) ? color_map[(r - 1) * stride + c - 1] : -1;
  color_neighbors[2] = (r - 1 >= 0) ? color_map[(r - 1) * stride + c] : -1;

  for (i = 0; i < PALETTE_MAX_SIZE; ++i) {
    color_order[i] = i;
    inverse_color_order[i] = i;
  }
  memset(scores, 0, PALETTE_MAX_SIZE * sizeof(scores[0]));
  for (i = 0; i < NUM_PALETTE_NEIGHBORS; ++i) {
    if (color_neighbors[i] >= 0) {
      scores[color_neighbors[i]] += weights[i];
    }
  }

  // Get the top NUM_PALETTE_NEIGHBORS scores (sorted from large to small).
  for (i = 0; i < NUM_PALETTE_NEIGHBORS; ++i) {
    int max = scores[i];
    int max_idx = i;
    int j;
    for (j = i + 1; j < palette_size; ++j) {
      if (scores[j] > max) {
        max = scores[j];
        max_idx = j;
      }
    }
    if (max_idx != i) {
      // Move the score at index 'max_idx' to index 'i', and shift the scores
      // from 'i' to 'max_idx - 1' by 1.
      const int max_score = scores[max_idx];
      const uint8_t max_color_order = color_order[max_idx];
      int k;
      for (k = max_idx; k > i; --k) {
        scores[k] = scores[k - 1];
        color_order[k] = color_order[k - 1];
        inverse_color_order[color_order[k]] = k;
      }
      scores[i] = max_score;
      color_order[i] = max_color_order;
      inverse_color_order[color_order[i]] = i;
    }
  }

  // Get hash value of context.
  color_index_ctx_hash = 0;
  for (i = 0; i < NUM_PALETTE_NEIGHBORS; ++i) {
    color_index_ctx_hash += scores[i] * hash_multipliers[i];
  }
  assert(color_index_ctx_hash > 0);
  assert(color_index_ctx_hash <= MAX_COLOR_CONTEXT_HASH);

  // Lookup context from hash.
  color_index_ctx = palette_color_index_context_lookup[color_index_ctx_hash];
  assert(color_index_ctx >= 0);
  assert(color_index_ctx < PALETTE_COLOR_INDEX_CONTEXTS);

  if (color_idx != NULL) {
    *color_idx = inverse_color_order[color_map[r * stride + c]];
  }
  return color_index_ctx;
}
#undef NUM_PALETTE_NEIGHBORS
#undef MAX_COLOR_CONTEXT_HASH

static const aom_cdf_prob
    default_txfm_partition_cdf[TXFM_PARTITION_CONTEXTS][CDF_SIZE(2)] = {
      { AOM_CDF2(249 * 128) }, { AOM_CDF2(240 * 128) }, { AOM_CDF2(223 * 128) },
      { AOM_CDF2(249 * 128) }, { AOM_CDF2(229 * 128) }, { AOM_CDF2(177 * 128) },
      { AOM_CDF2(250 * 128) }, { AOM_CDF2(243 * 128) }, { AOM_CDF2(208 * 128) },
      { AOM_CDF2(226 * 128) }, { AOM_CDF2(187 * 128) }, { AOM_CDF2(145 * 128) },
      { AOM_CDF2(236 * 128) }, { AOM_CDF2(204 * 128) }, { AOM_CDF2(150 * 128) },
      { AOM_CDF2(183 * 128) }, { AOM_CDF2(149 * 128) }, { AOM_CDF2(125 * 128) },
      { AOM_CDF2(181 * 128) }, { AOM_CDF2(146 * 128) }, { AOM_CDF2(113 * 128) }
    };

static const aom_cdf_prob default_skip_mode_cdfs[SKIP_MODE_CONTEXTS][CDF_SIZE(
    2)] = { { AOM_CDF2(31609) }, { AOM_CDF2(20107) }, { AOM_CDF2(10296) } };
static const aom_cdf_prob default_skip_cdfs[SKIP_CONTEXTS][CDF_SIZE(2)] = {
  { AOM_CDF2(30224) }, { AOM_CDF2(16244) }, { AOM_CDF2(4835) }
};

static const aom_cdf_prob
    default_compound_idx_cdfs[COMP_INDEX_CONTEXTS][CDF_SIZE(2)] = {
      { AOM_ICDF(24576), AOM_ICDF(32768), 0 },
      { AOM_ICDF(16384), AOM_ICDF(32768), 0 },
      { AOM_ICDF(8192), AOM_ICDF(32768), 0 },
      { AOM_ICDF(24576), AOM_ICDF(32768), 0 },
      { AOM_ICDF(16384), AOM_ICDF(32768), 0 },
      { AOM_ICDF(8192), AOM_ICDF(32768), 0 },
    };

static const aom_cdf_prob
    default_comp_group_idx_cdfs[COMP_GROUP_IDX_CONTEXTS][CDF_SIZE(2)] = {
      { AOM_ICDF(29491), AOM_ICDF(32768), 0 },
      { AOM_ICDF(24576), AOM_ICDF(32768), 0 },
      { AOM_ICDF(16384), AOM_ICDF(32768), 0 },
      { AOM_ICDF(24576), AOM_ICDF(32768), 0 },
      { AOM_ICDF(16384), AOM_ICDF(32768), 0 },
      { AOM_ICDF(13107), AOM_ICDF(32768), 0 },
      { AOM_ICDF(13107), AOM_ICDF(32768), 0 },
    };

static const aom_cdf_prob default_filter_intra_mode_cdf[CDF_SIZE(
    FILTER_INTRA_MODES)] = { AOM_CDF5(14259, 17304, 20463, 29377) };

static const aom_cdf_prob
    default_filter_intra_cdfs[BLOCK_SIZES_ALL][CDF_SIZE(2)] = {
      { AOM_CDF2(10985) }, { AOM_CDF2(10985) }, { AOM_CDF2(10985) },
      { AOM_CDF2(10985) }, { AOM_CDF2(15723) }, { AOM_CDF2(15723) },
      { AOM_CDF2(16645) }, { AOM_CDF2(16645) }, { AOM_CDF2(16645) },
      { AOM_CDF2(27378) }, { AOM_CDF2(30378) }, { AOM_CDF2(30378) },
      { AOM_CDF2(30378) }, { AOM_CDF2(30378) }, { AOM_CDF2(30378) },
      { AOM_CDF2(30378) }, { AOM_CDF2(10985) }, { AOM_CDF2(10985) },
      { AOM_CDF2(15723) }, { AOM_CDF2(15723) }, { AOM_CDF2(30378) },
      { AOM_CDF2(30378) },
    };

// FIXME(someone) need real defaults here
static const aom_cdf_prob default_seg_tree_cdf[CDF_SIZE(MAX_SEGMENTS)] = {
  AOM_CDF8(4096, 8192, 12288, 16384, 20480, 24576, 28672)
};

static const aom_cdf_prob
    default_segment_pred_cdf[SEG_TEMPORAL_PRED_CTXS][CDF_SIZE(2)] = {
      { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) }, { AOM_CDF2(128 * 128) }
    };

static const aom_cdf_prob
    default_switchable_interp_cdf[SWITCHABLE_FILTER_CONTEXTS][CDF_SIZE(
        SWITCHABLE_FILTERS)] = {
      { AOM_CDF3(32256, 32654) }, { AOM_CDF3(2816, 32651) },
      { AOM_CDF3(512, 764) },     { AOM_CDF3(30464, 31778) },
      { AOM_CDF3(32384, 32483) }, { AOM_CDF3(3072, 32652) },
      { AOM_CDF3(256, 383) },     { AOM_CDF3(25344, 26533) },
      { AOM_CDF3(32000, 32531) }, { AOM_CDF3(2048, 32648) },
      { AOM_CDF3(384, 890) },     { AOM_CDF3(28928, 31358) },
      { AOM_CDF3(31616, 31787) }, { AOM_CDF3(4224, 32433) },
      { AOM_CDF3(128, 256) },     { AOM_CDF3(17408, 18248) }
    };

#if CONFIG_SPATIAL_SEGMENTATION
static const aom_cdf_prob
    default_spatial_pred_seg_tree_cdf[SPATIAL_PREDICTION_PROBS][CDF_SIZE(
        MAX_SEGMENTS)] = {
      {
          AOM_CDF8(5622, 7893, 16093, 18233, 27809, 28373, 32533),
      },
      {
          AOM_CDF8(14274, 18230, 22557, 24935, 29980, 30851, 32344),
      },
      {
          AOM_CDF8(27527, 28487, 28723, 28890, 32397, 32647, 32679),
      },
    };
#endif

static const aom_cdf_prob default_tx_size_cdf[MAX_TX_CATS][TX_SIZE_CONTEXTS]
                                             [CDF_SIZE(MAX_TX_DEPTH + 1)] = {
#if MAX_TX_DEPTH == 2
                                               { { AOM_CDF2(19968) },
                                                 { AOM_CDF2(19968) },
                                                 { AOM_CDF2(24320) } },
                                               { { AOM_CDF3(12272, 30172) },
                                                 { AOM_CDF3(12272, 30172) },
                                                 { AOM_CDF3(18677, 30848) } },
                                               { { AOM_CDF3(12986, 15180) },
                                                 { AOM_CDF3(12986, 15180) },
                                                 { AOM_CDF3(24302, 25602) } },
                                               { { AOM_CDF3(5782, 11475) },
                                                 { AOM_CDF3(5782, 11475) },
                                                 { AOM_CDF3(16803, 22759) } },
#elif MAX_TX_DEPTH == 3
                                               { { AOM_CDF2(19968) },
                                                 { AOM_CDF2(24320) } },
                                               { { AOM_CDF3(12272, 30172) },
                                                 { AOM_CDF3(18677, 30848) } },
                                               { { AOM_CDF4(12986, 15180,
                                                            32384) },
                                                 { AOM_CDF4(24302, 25602,
                                                            32128) } },
                                               { { AOM_CDF4(5782, 11475,
                                                            24480) },
                                                 { AOM_CDF4(16803, 22759,
                                                            28560) } },
#else
                                               { { AOM_CDF2(19968) },
                                                 { AOM_CDF2(24320) } },
                                               { { AOM_CDF3(12272, 30172) },
                                                 { AOM_CDF3(18677, 30848) } },
                                               { { AOM_CDF4(12986, 15180,
                                                            32384) },
                                                 { AOM_CDF4(24302, 25602,
                                                            32128) } },
                                               { { AOM_CDF5(5782, 11475, 24480,
                                                            32640) },
                                                 { AOM_CDF5(16803, 22759, 28560,
                                                            32640) } },
#endif  // MAX_TX_DEPTH == 2
                                             };

static const aom_cdf_prob
    default_if_y_mode_cdf[BLOCK_SIZE_GROUPS][CDF_SIZE(INTRA_MODES)] = {
      { AOM_CDF13(7168, 10680, 13913, 16928, 20294, 22790, 24706, 26275, 28139,
                  29751, 30563, 31468) },
      { AOM_CDF13(11776, 13823, 15307, 15725, 16638, 17406, 17994, 18814, 19634,
                  21513, 22198, 22928) },
      { AOM_CDF13(14720, 16459, 18091, 18299, 18757, 19125, 19423, 19924, 20504,
                  22922, 24063, 25577) },
      { AOM_CDF13(18944, 19925, 20908, 20998, 21017, 21072, 21084, 21121, 21159,
                  22064, 22820, 24290) },
    };

static const aom_cdf_prob
    default_uv_mode_cdf[CFL_ALLOWED_TYPES][INTRA_MODES][CDF_SIZE(
        UV_INTRA_MODES)] = {
      {
          { AOM_CDF13(17902, 18828, 21117, 21487, 21924, 22484, 23588, 24669,
                      25177, 28731, 29903, 31509) },
          { AOM_CDF13(9654, 23559, 23873, 24050, 24203, 24929, 25057, 25286,
                      26027, 28172, 28716, 30913) },
          { AOM_CDF13(10012, 10124, 25394, 25540, 25665, 25752, 26567, 27761,
                      27876, 29497, 30581, 31179) },
          { AOM_CDF13(15143, 15859, 16581, 21567, 21968, 22430, 22867, 24953,
                      26969, 30310, 31125, 32329) },
          { AOM_CDF13(14063, 14416, 14921, 15022, 25164, 26720, 28661, 29083,
                      29277, 31337, 31882, 32565) },
          { AOM_CDF13(12942, 14713, 15178, 15325, 16964, 27421, 27834, 28306,
                      28645, 30804, 31322, 32387) },
          { AOM_CDF13(13687, 13993, 16776, 16912, 18338, 18648, 27557, 28140,
                      28359, 30820, 31669, 32443) },
          { AOM_CDF13(14180, 14439, 16582, 17373, 17675, 17931, 18453, 26308,
                      26761, 30058, 31293, 32156) },
          { AOM_CDF13(12480, 14300, 14838, 16085, 16434, 17023, 17426, 18313,
                      26041, 29653, 30347, 32067) },
          { AOM_CDF13(17202, 18093, 19414, 19910, 20311, 20837, 21554, 22830,
                      23572, 28770, 30259, 32145) },
          { AOM_CDF13(16336, 18149, 19485, 19927, 20365, 20924, 21524, 22561,
                      23421, 28141, 30701, 32020) },
          { AOM_CDF13(16485, 17366, 19874, 20364, 20713, 21057, 21773, 23100,
                      23685, 28079, 29091, 32028) },
          { AOM_CDF13(13638, 16789, 19763, 19903, 19995, 20201, 20405, 20861,
                      21174, 22802, 23566, 24754) },
      },
      {
          { AOM_CDF14(18377, 18815, 19743, 20178, 20560, 20889, 21359, 22098,
                      22481, 24563, 25781, 26662, 28396) },
          { AOM_CDF14(5350, 16837, 17066, 17360, 17692, 18778, 18969, 19206,
                      20291, 22367, 23212, 24670, 27912) },
          { AOM_CDF14(6671, 6759, 17812, 17998, 18260, 18384, 19408, 20667,
                      20806, 22760, 24142, 24875, 28072) },
          { AOM_CDF14(7461, 8082, 8515, 15013, 15583, 16098, 16522, 18519,
                      20348, 22954, 24130, 25342, 26548) },
          { AOM_CDF14(3694, 4403, 5370, 5854, 17841, 19639, 21625, 22224, 22651,
                      24613, 25399, 26143, 26599) },
          { AOM_CDF14(3700, 5651, 6112, 6541, 8929, 20623, 21213, 21640, 22214,
                      24306, 25412, 26406, 27249) },
          { AOM_CDF14(4649, 4947, 7128, 7432, 9439, 9903, 21163, 21774, 22056,
                      24426, 25403, 26324, 27128) },
          { AOM_CDF14(7208, 7375, 8779, 9683, 10072, 10284, 10796, 19786, 20152,
                      22955, 24246, 25165, 26589) },
          { AOM_CDF14(5897, 7283, 7555, 8910, 9391, 9937, 10276, 11044, 19841,
                      22620, 23784, 25060, 26418) },
          { AOM_CDF14(12171, 12718, 13885, 14348, 14925, 15394, 16108, 17075,
                      17583, 21996, 23614, 25048, 27011) },
          { AOM_CDF14(10192, 11222, 12318, 12877, 13533, 14184, 14866, 15879,
                      16650, 20419, 23265, 24295, 26596) },
          { AOM_CDF14(10776, 11387, 12899, 13471, 14088, 14575, 15366, 16456,
                      17040, 20815, 22009, 24448, 26492) },
          { AOM_CDF14(4015, 6473, 9853, 10285, 10655, 11032, 11431, 12199,
                      12738, 14760, 16121, 17263, 28612) },
      }
    };

static const aom_cdf_prob default_partition_cdf[PARTITION_CONTEXTS][CDF_SIZE(
    EXT_PARTITION_TYPES)] = {
  // 8x8 -> 4x4 only supports the four legacy partition types
  { AOM_CDF4(25472, 28949, 31052), 0, 0, 0, 0, 0, 0 },
  { AOM_CDF4(18816, 22250, 28783), 0, 0, 0, 0, 0, 0 },
  { AOM_CDF4(18944, 26126, 29188), 0, 0, 0, 0, 0, 0 },
  { AOM_CDF4(15488, 22508, 27077), 0, 0, 0, 0, 0, 0 },
  // 16x16 -> 8x8
  { AOM_CDF10(22272, 23768, 25043, 29996, 30495, 30994, 31419, 31844, 32343) },
  { AOM_CDF10(11776, 13457, 16315, 28229, 28789, 29349, 30302, 31255, 31816) },
  { AOM_CDF10(10496, 14802, 16136, 27127, 28563, 29999, 30444, 30889, 32324) },
  { AOM_CDF10(6784, 8763, 10440, 29110, 29770, 30430, 30989, 31548, 32208) },
  // 32x32 -> 16x16
  { AOM_CDF10(22656, 23801, 24702, 30721, 31103, 31485, 31785, 32085, 32467) },
  { AOM_CDF10(8704, 9926, 12586, 28885, 29292, 29699, 30586, 31473, 31881) },
  { AOM_CDF10(6656, 10685, 11566, 27857, 29200, 30543, 30837, 31131, 32474) },
  { AOM_CDF10(2176, 3012, 3690, 31253, 31532, 31811, 32037, 32263, 32542) },
  // 64x64 -> 32x32
  { AOM_CDF10(28416, 28705, 28926, 32258, 32354, 32450, 32523, 32596, 32693) },
  { AOM_CDF10(9216, 9952, 11849, 30134, 30379, 30624, 31256, 31888, 32134) },
  { AOM_CDF10(7424, 9008, 9528, 30664, 31192, 31720, 31893, 32066, 32594) },
  { AOM_CDF10(1280, 1710, 2069, 31978, 32121, 32264, 32383, 32502, 32647) },
  // 128x128 -> 64x64
  { AOM_CDF8(28416, 28705, 28926, 32258, 32402, 32547, 32548) },
  { AOM_CDF8(9216, 9952, 11849, 30134, 30502, 30870, 30871) },
  { AOM_CDF8(7424, 9008, 9528, 30664, 31456, 32248, 32249) },
  { AOM_CDF8(1280, 1710, 2069, 31978, 32193, 32409, 32410) },
};

static const aom_cdf_prob default_intra_ext_tx_cdf
    [EXT_TX_SETS_INTRA][EXT_TX_SIZES][INTRA_MODES][CDF_SIZE(TX_TYPES)] = {
      {
          // FIXME: unused zero positions, from uncoded trivial transform set
          {
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
          },
          {
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
          },
          {
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
          },
          {
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
              { 0 },
          },
      },
      {
          {
              { AOM_CDF7(1024, 28800, 29048, 29296, 30164, 31466) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1152, 25852, 26284, 26717, 28230, 30499) },
              { AOM_CDF7(1024, 2016, 3938, 5860, 29404, 31086) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
          },
          {
              { AOM_CDF7(1024, 28800, 29048, 29296, 30164, 31466) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1152, 25852, 26284, 26717, 28230, 30499) },
              { AOM_CDF7(1024, 2016, 3938, 5860, 29404, 31086) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
          },
          {
              { AOM_CDF7(1024, 28800, 29048, 29296, 30164, 31466) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1152, 25852, 26284, 26717, 28230, 30499) },
              { AOM_CDF7(1024, 2016, 3938, 5860, 29404, 31086) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
          },
          {
              { AOM_CDF7(1024, 28800, 29048, 29296, 30164, 31466) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1152, 25852, 26284, 26717, 28230, 30499) },
              { AOM_CDF7(1024, 2016, 3938, 5860, 29404, 31086) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 27118) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 4109, 5900, 7691, 15528, 27380) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
              { AOM_CDF7(1280, 5216, 6938, 8660, 10167, 15817) },
          },
      },
      {
          {
              { AOM_CDF5(1024, 28800, 29792, 31280) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1152, 25852, 27581, 30174) },
              { AOM_CDF5(1024, 2016, 28924, 30846) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
          },
          {
              { AOM_CDF5(1024, 28800, 29792, 31280) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1152, 25852, 27581, 30174) },
              { AOM_CDF5(1024, 2016, 28924, 30846) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
          },
          {
              { AOM_CDF5(1024, 28800, 29792, 31280) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1152, 25852, 27581, 30174) },
              { AOM_CDF5(1024, 2016, 28924, 30846) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
          },
          {
              { AOM_CDF5(1024, 28800, 29792, 31280) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1152, 25852, 27581, 30174) },
              { AOM_CDF5(1024, 2016, 28924, 30846) },
              { AOM_CDF5(1280, 5216, 6938, 26310) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 4109, 13065, 26611) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
              { AOM_CDF5(1280, 5216, 6938, 13396) },
          },
      },
    };
static const aom_cdf_prob
    default_inter_ext_tx_cdf[EXT_TX_SETS_INTER][EXT_TX_SIZES][CDF_SIZE(
        TX_TYPES)] = {
      { { 0 }, { 0 }, { 0 }, { 0 } },
      { { AOM_CDF16(1280, 1453, 1626, 2277, 2929, 3580, 4232, 16717, 19225,
                    21733, 24241, 26749, 28253, 29758, 31263) },
        { AOM_CDF16(1280, 1453, 1626, 2277, 2929, 3580, 4232, 16717, 19225,
                    21733, 24241, 26749, 28253, 29758, 31263) },
        { AOM_CDF16(1280, 1453, 1626, 2277, 2929, 3580, 4232, 16717, 19225,
                    21733, 24241, 26749, 28253, 29758, 31263) },
        { AOM_CDF16(1280, 1453, 1626, 2277, 2929, 3580, 4232, 16717, 19225,
                    21733, 24241, 26749, 28253, 29758, 31263) } },
      { { AOM_CDF12(1280, 3125, 4970, 17132, 19575, 22018, 24461, 26904, 28370,
                    29836, 31302) },
        { AOM_CDF12(1280, 3125, 4970, 17132, 19575, 22018, 24461, 26904, 28370,
                    29836, 31302) },
        { AOM_CDF12(1280, 3125, 4970, 17132, 19575, 22018, 24461, 26904, 28370,
                    29836, 31302) },
        { AOM_CDF12(1280, 3125, 4970, 17132, 19575, 22018, 24461, 26904, 28370,
                    29836, 31302) } },
      { { AOM_CDF2(1536) },
        { AOM_CDF2(1536) },
        { AOM_CDF2(1536) },
        { AOM_CDF2(1536) } },
    };

static const aom_cdf_prob default_cfl_sign_cdf[CDF_SIZE(CFL_JOINT_SIGNS)] = {
  AOM_CDF8(1892, 2229, 11464, 14116, 25661, 26409, 32508)
};

static const aom_cdf_prob
    default_cfl_alpha_cdf[CFL_ALPHA_CONTEXTS][CDF_SIZE(CFL_ALPHABET_SIZE)] = {
      { AOM_CDF16(16215, 27740, 31726, 32606, 32736, 32751, 32757, 32759, 32761,
                  32762, 32763, 32764, 32765, 32766, 32767) },
      { AOM_CDF16(15213, 24615, 29704, 31974, 32545, 32673, 32713, 32746, 32753,
                  32756, 32758, 32761, 32763, 32764, 32766) },
      { AOM_CDF16(13250, 24677, 29113, 31666, 32408, 32578, 32628, 32711, 32730,
                  32738, 32744, 32749, 32752, 32756, 32759) },
      { AOM_CDF16(24593, 30787, 32062, 32495, 32656, 32707, 32735, 32747, 32752,
                  32757, 32760, 32763, 32764, 32765, 32767) },
      { AOM_CDF16(19883, 27419, 30100, 31392, 31896, 32184, 32299, 32511, 32568,
                  32602, 32628, 32664, 32680, 32691, 32708) },
      { AOM_CDF16(15939, 24151, 27754, 29680, 30651, 31267, 31527, 31868, 32001,
                  32090, 32181, 32284, 32314, 32366, 32486) }
    };

// TODO(jingning): This initial models are copied directly from the entries
// from the original table. The copied indexes are (0, 0), (0, 1), .. (4, 4).
// It is possible to re-train this model and bring back the 0.14% loss in CIF
// set key frame coding. This reduction in context model does not change the
// key frame coding stats for mid and high resolution sets.
const aom_cdf_prob
    default_kf_y_mode_cdf[KF_MODE_CONTEXTS][KF_MODE_CONTEXTS][CDF_SIZE(
        INTRA_MODES)] = {
      { { AOM_CDF13(13234, 14775, 17115, 18040, 18783, 19420, 20510, 22129,
                    23183, 28738, 30120, 32138) },
        { AOM_CDF13(8983, 14623, 16290, 17124, 17864, 18817, 19593, 20876,
                    22359, 27820, 29791, 31566) },
        { AOM_CDF13(7091, 8084, 17897, 18490, 19057, 19428, 20811, 22624, 23265,
                    28288, 29341, 31870) },
        { AOM_CDF13(11191, 12808, 14120, 16182, 16785, 17440, 18159, 20280,
                    22697, 28431, 30235, 32276) },
        { AOM_CDF13(8208, 9510, 11986, 12851, 15212, 16786, 19400, 22224, 23146,
                    28889, 30200, 32375) } },
      { { AOM_CDF13(6308, 15986, 17454, 18110, 18739, 19867, 20479, 21575,
                    22972, 28087, 30042, 31489) },
        { AOM_CDF13(3549, 21993, 22593, 22968, 23262, 24052, 24280, 24856,
                    26026, 29057, 30818, 31543) },
        { AOM_CDF13(4371, 9956, 16063, 16680, 17207, 17870, 18692, 20142, 21261,
                    26613, 28301, 30433) },
        { AOM_CDF13(6445, 12764, 13699, 15338, 15922, 16891, 17304, 18868,
                    22816, 28105, 30472, 31907) },
        { AOM_CDF13(4300, 11014, 12466, 13258, 15028, 17584, 19170, 21448,
                    22945, 28207, 30041, 31659) } },
      { { AOM_CDF13(9111, 10159, 16955, 17625, 18268, 18703, 20078, 22004,
                    22761, 28166, 29334, 31990) },
        { AOM_CDF13(7107, 11104, 15591, 16340, 17066, 17802, 18721, 20303,
                    21481, 26882, 28699, 30978) },
        { AOM_CDF13(4546, 4935, 22442, 22717, 22960, 23087, 24171, 25671, 25939,
                    29333, 29866, 32023) },
        { AOM_CDF13(8332, 9555, 12646, 14689, 15340, 15873, 16872, 19939, 21942,
                    27812, 29508, 31923) },
        { AOM_CDF13(6413, 7233, 13108, 13895, 15332, 16187, 19121, 22694, 23365,
                    28639, 29686, 32187) } },
      { { AOM_CDF13(9584, 11586, 12990, 15322, 15927, 16732, 17406, 19225,
                    22484, 28555, 30321, 32279) },
        { AOM_CDF13(5907, 11662, 12625, 14955, 15491, 16403, 16865, 18074,
                    23261, 28508, 30584, 32057) },
        { AOM_CDF13(5759, 7323, 12581, 14779, 15363, 15946, 16851, 19330, 21902,
                    27860, 29214, 31747) },
        { AOM_CDF13(7166, 8714, 9430, 14479, 14672, 14953, 15184, 17239, 24798,
                    29350, 31021, 32371) },
        { AOM_CDF13(6318, 8140, 9595, 12354, 13754, 15324, 16681, 19701, 22723,
                    28616, 30226, 32279) } },
      { { AOM_CDF13(8669, 9875, 12300, 13093, 15518, 17458, 19843, 22083, 22927,
                    28780, 30271, 32364) },
        { AOM_CDF13(6600, 10422, 12153, 12937, 15218, 18211, 19914, 21744,
                    22975, 28393, 30393, 31970) },
        { AOM_CDF13(5512, 6207, 14265, 14897, 16246, 17175, 19865, 22553, 23178,
                    28445, 29511, 31980) },
        { AOM_CDF13(8195, 9407, 10830, 13261, 14443, 15761, 16922, 20311, 22151,
                    28230, 30109, 32220) },
        { AOM_CDF13(5612, 6462, 8166, 8737, 14316, 17802, 21788, 25554, 26080,
                    30083, 30983, 32457) } }
    };

static const aom_cdf_prob default_angle_delta_cdf[DIRECTIONAL_MODES][CDF_SIZE(
    2 * MAX_ANGLE_DELTA + 1)] = {
  { AOM_CDF7(2340, 5327, 7611, 23102, 27196, 30546) },
  { AOM_CDF7(3267, 8071, 11970, 21822, 25619, 30034) },
  { AOM_CDF7(3417, 9937, 12286, 16420, 19941, 30669) },
  { AOM_CDF7(5167, 11735, 15254, 16662, 20697, 28276) },
  { AOM_CDF7(1728, 10973, 14103, 18547, 22684, 27007) },
  { AOM_CDF7(2764, 10700, 12517, 16957, 20590, 30390) },
  { AOM_CDF7(2407, 12749, 16527, 20823, 22781, 29642) },
  { AOM_CDF7(3068, 10132, 12079, 16542, 19943, 30448) }
};

static void init_mode_probs(FRAME_CONTEXT *fc) {
  av1_copy(fc->palette_y_size_cdf, default_palette_y_size_cdf);
  av1_copy(fc->palette_uv_size_cdf, default_palette_uv_size_cdf);
  av1_copy(fc->palette_y_color_index_cdf, default_palette_y_color_index_cdf);
  av1_copy(fc->palette_uv_color_index_cdf, default_palette_uv_color_index_cdf);
  av1_copy(fc->kf_y_cdf, default_kf_y_mode_cdf);
  av1_copy(fc->angle_delta_cdf, default_angle_delta_cdf);
  av1_copy(fc->comp_inter_cdf, default_comp_inter_cdf);
  av1_copy(fc->comp_ref_type_cdf, default_comp_ref_type_cdf);
  av1_copy(fc->uni_comp_ref_cdf, default_uni_comp_ref_cdf);
  av1_copy(fc->palette_y_mode_cdf, default_palette_y_mode_cdf);
  av1_copy(fc->palette_uv_mode_cdf, default_palette_uv_mode_cdf);
  av1_copy(fc->comp_ref_cdf, default_comp_ref_cdf);
  av1_copy(fc->comp_bwdref_cdf, default_comp_bwdref_cdf);
  av1_copy(fc->single_ref_cdf, default_single_ref_cdf);
  av1_copy(fc->txfm_partition_cdf, default_txfm_partition_cdf);
  av1_copy(fc->compound_index_cdf, default_compound_idx_cdfs);
  av1_copy(fc->comp_group_idx_cdf, default_comp_group_idx_cdfs);
  av1_copy(fc->newmv_cdf, default_newmv_cdf);
  av1_copy(fc->zeromv_cdf, default_zeromv_cdf);
  av1_copy(fc->refmv_cdf, default_refmv_cdf);
  av1_copy(fc->drl_cdf, default_drl_cdf);
  av1_copy(fc->motion_mode_cdf, default_motion_mode_cdf);
  av1_copy(fc->obmc_cdf, default_obmc_cdf);
  av1_copy(fc->inter_compound_mode_cdf, default_inter_compound_mode_cdf);
  av1_copy(fc->compound_type_cdf, default_compound_type_cdf);
#if WEDGE_IDX_ENTROPY_CODING
  av1_copy(fc->wedge_idx_cdf, default_wedge_idx_cdf);
#endif
  av1_copy(fc->interintra_cdf, default_interintra_cdf);
  av1_copy(fc->wedge_interintra_cdf, default_wedge_interintra_cdf);
  av1_copy(fc->interintra_mode_cdf, default_interintra_mode_cdf);
  av1_copy(fc->seg.pred_cdf, default_segment_pred_cdf);
  av1_copy(fc->seg.tree_cdf, default_seg_tree_cdf);
  av1_copy(fc->filter_intra_cdfs, default_filter_intra_cdfs);
  av1_copy(fc->filter_intra_mode_cdf, default_filter_intra_mode_cdf);
  av1_copy(fc->switchable_restore_cdf, default_switchable_restore_cdf);
  av1_copy(fc->wiener_restore_cdf, default_wiener_restore_cdf);
  av1_copy(fc->sgrproj_restore_cdf, default_sgrproj_restore_cdf);
  av1_copy(fc->y_mode_cdf, default_if_y_mode_cdf);
  av1_copy(fc->uv_mode_cdf, default_uv_mode_cdf);
  av1_copy(fc->switchable_interp_cdf, default_switchable_interp_cdf);
  av1_copy(fc->partition_cdf, default_partition_cdf);
  av1_copy(fc->intra_ext_tx_cdf, default_intra_ext_tx_cdf);
  av1_copy(fc->inter_ext_tx_cdf, default_inter_ext_tx_cdf);
  av1_copy(fc->skip_mode_cdfs, default_skip_mode_cdfs);
  av1_copy(fc->skip_cdfs, default_skip_cdfs);
  av1_copy(fc->intra_inter_cdf, default_intra_inter_cdf);
#if CONFIG_SPATIAL_SEGMENTATION
  for (int i = 0; i < SPATIAL_PREDICTION_PROBS; i++)
    av1_copy(fc->seg.spatial_pred_seg_cdf[i],
             default_spatial_pred_seg_tree_cdf[i]);
#endif
  av1_copy(fc->tx_size_cdf, default_tx_size_cdf);
  av1_copy(fc->delta_q_cdf, default_delta_q_cdf);
#if CONFIG_EXT_DELTA_Q
  av1_copy(fc->delta_lf_cdf, default_delta_lf_cdf);
  av1_copy(fc->delta_lf_multi_cdf, default_delta_lf_multi_cdf);
#endif
  av1_copy(fc->cfl_sign_cdf, default_cfl_sign_cdf);
  av1_copy(fc->cfl_alpha_cdf, default_cfl_alpha_cdf);
  av1_copy(fc->intrabc_cdf, default_intrabc_cdf);
}

void av1_set_default_ref_deltas(int8_t *ref_deltas) {
  assert(ref_deltas != NULL);

  ref_deltas[INTRA_FRAME] = 1;
  ref_deltas[LAST_FRAME] = 0;
  ref_deltas[LAST2_FRAME] = ref_deltas[LAST_FRAME];
  ref_deltas[LAST3_FRAME] = ref_deltas[LAST_FRAME];
  ref_deltas[BWDREF_FRAME] = ref_deltas[LAST_FRAME];
  ref_deltas[GOLDEN_FRAME] = -1;
  ref_deltas[ALTREF2_FRAME] = -1;
  ref_deltas[ALTREF_FRAME] = -1;
}

void av1_set_default_mode_deltas(int8_t *mode_deltas) {
  assert(mode_deltas != NULL);

  mode_deltas[0] = 0;
  mode_deltas[1] = 0;
}

static void set_default_lf_deltas(struct loopfilter *lf) {
  lf->mode_ref_delta_enabled = 1;
  lf->mode_ref_delta_update = 1;

  av1_set_default_ref_deltas(lf->ref_deltas);
  av1_set_default_mode_deltas(lf->mode_deltas);
}

void av1_setup_frame_contexts(AV1_COMMON *cm) {
  // Store the frame context into a special slot (not associated with any
  // reference buffer), so that we can set up cm->pre_fc correctly later
  // This function must ONLY be called when cm->fc has been initialized with
  // default probs, either by av1_setup_past_independence or after manually
  // initializing them
  cm->frame_contexts[FRAME_CONTEXT_DEFAULTS] = *cm->fc;
  if (cm->large_scale_tile) {
    for (int i = 0; i < FRAME_CONTEXTS; ++i) cm->frame_contexts[i] = *cm->fc;
  }
}

void av1_setup_past_independence(AV1_COMMON *cm) {
  // Reset the segment feature data to the default stats:
  // Features disabled, 0, with delta coding (Default state).
  av1_clearall_segfeatures(&cm->seg);

  cm->current_frame_seg_map = cm->cur_frame->seg_map;

  if (cm->current_frame_seg_map)
    memset(cm->current_frame_seg_map, 0, (cm->mi_rows * cm->mi_cols));

  // reset mode ref deltas
  av1_set_default_ref_deltas(cm->cur_frame->ref_deltas);
  av1_set_default_mode_deltas(cm->cur_frame->mode_deltas);
  set_default_lf_deltas(&cm->lf);

  av1_default_coef_probs(cm);
  init_mode_probs(cm->fc);
  av1_init_mv_probs(cm);
  av1_init_lv_map(cm);
  cm->fc->initialized = 1;
  av1_setup_frame_contexts(cm);

  // prev_mip will only be allocated in encoder.
  if (frame_is_intra_only(cm) && cm->prev_mip)
    memset(cm->prev_mip, 0,
           cm->mi_stride * cm->mi_rows * sizeof(*cm->prev_mip));
}
