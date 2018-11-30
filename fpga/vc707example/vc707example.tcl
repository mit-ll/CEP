# Generate the MCS file for programming the VC707 flash
write_cfgmem -force -format mcs -interface bpix16 -size 128 -loadbit "up 0x0 ./vc707example.runs/impl_1/orpsoc_top.bit" -file vc707example.mcs

