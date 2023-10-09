TAPA_WORK_DIR = ./tapa_dir.$(TARGET).$(PLATFORM)

tapa:
	# refresh and manually place necessary common includes
	rm -rf ./device/common
	cp -r ./common ./device/

	# run tapa synthesis
	tapac -o tapa_qcf.hw.xo ./device/tapa_qcf.cpp \
		--top qcf_top \
		--platform $(PLATFORM) \
		--work-dir $(TAPA_WORK_DIR) \
		--enable-floorplan \
		--floorplan-pre-assignments tapa_floorplan.json \
		--floorplan-output constraint.tcl \
		--connectivity tapa_connectivity.ini \
		--enable-synth-util \
		--enable-hbm-binding-adjustment \
		--read-only-args "abcd_inp" \
		--read-only-args "rtwt_rys_inp" \
		--write-only-args "cart_eris_[0-7]" \
		--cflags="-DTAPA_FLOW" \
		--cflags="-DAM_ABCD=$(AM_ABCD)"
	# --run-floorplan-dse could also be enabled but will need more resources

	# run hw
	chmod +x ./tapa_qcf.hw_generate_bitstream.sh
	./tapa_qcf.hw_generate_bitstream.sh
