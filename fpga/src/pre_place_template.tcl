create_pblock pblock_BL
resize_pblock pblock_BL -add CLOCKREGION_X0Y0:CLOCKREGION_X3Y3 -locs keep_all
resize_pblock pblock_BL -add {BUFG_GT_SYNC_X0Y0:BUFG_GT_SYNC_X0Y59 BUFG_GT_X0Y0:BUFG_GT_X0Y95 BUFGCTRL_X0Y0:BUFGCTRL_X0Y23 BUFGCE_DIV_X0Y0:BUFGCE_DIV_X0Y11 BUFGCE_X0Y0:BUFGCE_X0Y71}
# global clock buffer resources added (not included by default in left-side slots) copied from SLR derived ranges

# larger region includes CR column from BR slot to have BUFGs
#create_pblock pblock_BL_alt
#resize_pblock pblock_BL_alt -add CLOCKREGION_X0Y0:CLOCKREGION_X5Y3 -locs keep_all

create_pblock pblock_BR
resize_pblock pblock_BR -add CLOCKREGION_X4Y0:CLOCKREGION_X4Y2 -locs keep_all
resize_pblock pblock_BR -add CLOCKREGION_X5Y0:CLOCKREGION_X5Y3 -locs keep_all
resize_pblock pblock_BR -add CLOCKREGION_X6Y0 -locs keep_all
resize_pblock pblock_BR -add {URAM288_X4Y16:URAM288_X4Y63 RAMB36_X12Y0:RAMB36_X13Y11 RAMB36_X11Y12:RAMB36_X11Y47 RAMB18_X12Y0:RAMB18_X13Y23 RAMB18_X11Y24:RAMB18_X11Y95 HARD_SYNC_X24Y0:HARD_SYNC_X27Y1 HARD_SYNC_X22Y2:HARD_SYNC_X23Y7 DSP48E2_X30Y0:DSP48E2_X31Y17 DSP48E2_X25Y18:DSP48E2_X28Y89 SLICE_X230Y0:SLICE_X231Y59 SLICE_X229Y15:SLICE_X229Y59 SLICE_X222Y0:SLICE_X228Y59 SLICE_X217Y0:SLICE_X219Y59 SLICE_X216Y15:SLICE_X216Y59 SLICE_X206Y0:SLICE_X215Y59 SLICE_X196Y60:SLICE_X196Y239 SLICE_X194Y60:SLICE_X195Y179 SLICE_X184Y60:SLICE_X193Y239 SLICE_X182Y60:SLICE_X183Y179 SLICE_X176Y60:SLICE_X181Y239}
resize_pblock pblock_BR -add {URAM288_X2Y48:URAM288_X2Y63 RAMB36_X8Y36:RAMB36_X9Y47 RAMB18_X8Y72:RAMB18_X9Y95 HARD_SYNC_X16Y6:HARD_SYNC_X19Y7 DSP48E2_X16Y66:DSP48E2_X19Y89 SLICE_X141Y180:SLICE_X145Y239 SLICE_X126Y180:SLICE_X138Y239 SLICE_X118Y180:SLICE_X123Y239}
# fourth and fifth -add is for smaller than clock region rectangles copied from SLR0 definition

create_pblock pblock_ML
resize_pblock pblock_ML -add CLOCKREGION_X0Y4:CLOCKREGION_X3Y7 -locs keep_all
resize_pblock pblock_ML -add {BUFG_GT_SYNC_X0Y60:BUFG_GT_SYNC_X0Y119 BUFG_GT_X0Y96:BUFG_GT_X0Y191 BUFGCTRL_X0Y32:BUFGCTRL_X0Y63 BUFGCE_DIV_X0Y16:BUFGCE_DIV_X0Y31 BUFGCE_X0Y96:BUFGCE_X0Y191}
# global clock buffer resources added (not included by default in left-side slots) copied from SLR derived ranges

create_pblock pblock_MR
resize_pblock pblock_MR -add CLOCKREGION_X4Y4:CLOCKREGION_X5Y7 -locs keep_all
resize_pblock pblock_MR -add {URAM288_X4Y64:URAM288_X4Y127 RAMB36_X11Y48:RAMB36_X11Y95 RAMB18_X11Y96:RAMB18_X11Y191 LAGUNA_X24Y120:LAGUNA_X27Y359 DSP48E2_X25Y90:DSP48E2_X28Y185 SLICE_X195Y420:SLICE_X196Y479 SLICE_X183Y420:SLICE_X193Y479 SLICE_X176Y420:SLICE_X181Y479 SLICE_X176Y300:SLICE_X196Y419 SLICE_X195Y240:SLICE_X196Y299 SLICE_X183Y240:SLICE_X193Y299 SLICE_X176Y240:SLICE_X181Y299}
# second -add is for smaller than clock region rectangle copied from SLR1 definition


#add_cells_to_pblock pblock_BL [get_cells -hierarchical -filter "NAME=~*k_preparation*"]
#add_cells_to_pblock pblock_BL [get_cells -hierarchical -filter "NAME=~*k_recurrence_relations*"]
#add_cells_to_pblock pblock_ML [get_cells -hierarchical -filter "NAME=~*k_buffer_permutation*"]
#add_cells_to_pblock pblock_ML [get_cells -hierarchical -filter "NAME=~*k_gaussian_quadrature_a*"]
#add_cells_to_pblock pblock_MR [get_cells -hierarchical -filter "NAME=~*k_find_bmax*"]
#add_cells_to_pblock pblock_BR [get_cells -hierarchical -filter "NAME=~*k_compress_store*"]

# add_cells_to_pblock commands will be generated below
